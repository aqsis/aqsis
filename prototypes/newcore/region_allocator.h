// Aqsis
// Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the software's owners nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// (This is the New BSD license)

/// \file Region-based memory allocator for data pipelines.
/// \author Chris Foster chris42f (at) gmail _dot_ com

#ifndef AQSIS_REGION_ALLOCATOR_H_INCLUDED
#define AQSIS_REGION_ALLOCATOR_H_INCLUDED

#include <iostream>
#include <list>
#include <algorithm>
#include <queue>
#include <limits>
#include <cassert>

#include <sys/mman.h>

namespace Aqsis {

/// Allocate a block of memory using mmap()
inline void* mmap_alloc(size_t size)
{
    return mmap(NULL, size, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

/// Deallocate a block of memory using munmap()
inline void mmap_free(void* p, size_t size)
{
    munmap(p, size);
}

//------------------------------------------------------------------------------
/// A region-based memory manager with page recycling and O(1) page-location
/// cost when deallocating a pointer.
///
/// The idea of this allocator is that we keep a list of allocated pages which
/// are much larger than the size of the objects to be allocated.
///
/// Allocation:
///   Objects are allocated starting at the beginning of a page, and
///   incrementing toward the end.  When a page fills up, a new page is created
///   and pushed onto the front of the list.  Whenever an object is allocated,
///   a pointer to the associated page is kept in the sizeof(MemPage*) bytes
///   before the returned pointer address.
///
///   No attempt is made to reuse the memory inside a page while live objects
///   still exist within the page.
///
///   RegionAllocator gets pages of memory directly from the OS using mmap();
///   we can then release this memory back to the system directly with munmap()
///   when we're finished with each page.  This prevents the pages from
///   contributing to memory fragmentation on the system heap.
///
/// Deallocation:
///   Memory is deallocated by finding the page in which the pointer lives by
///   reading the memory just preceding the given point, and decrementing the
///   reference count for that page.  If the reference count for a page falls
///   to 0, the page is removed from the active list.  The largest recently
///   deallocated page is kept in a cache and reused the next time a page needs
///   to be allocated.
///
class RegionAllocator
{
    private:
        /// Hold information about a memory page
        class MemPage
        {
            private:
                char* const m_basePtr;
                char* m_currPtr;
                const size_t m_pageSize;
                int m_refCount;

                // Embedded linked list
                MemPage* m_nextPage;
                MemPage* m_prevPage;
            public:
                MemPage(size_t pageSize)
                    : m_basePtr(static_cast<char*>(mmap_alloc(pageSize+sizeof(MemPage*)))),
                    m_currPtr(m_basePtr),
                    m_pageSize(pageSize),
                    m_refCount(0),
                    m_nextPage(0),
                    m_prevPage(0)
                { }
                ~MemPage()
                {
                    mmap_free(m_basePtr, m_pageSize+sizeof(MemPage*));
                }
                /// Reset the page for memory recycling.
                void reset()
                {
                    m_currPtr = m_basePtr;
                    m_refCount = 0;
                }

                /// Allocate a block of memory from the page
                ///
                /// Return 0 if the page has not enough memory left.
                ///
                void* alloc(size_t numBytes)
                {
                    numBytes += sizeof(MemPage*);
                    if(static_cast<ptrdiff_t>(numBytes) + (m_currPtr - m_basePtr)
                            > static_cast<ptrdiff_t>(m_pageSize))
                    {
                        return 0;
                    }
                    *reinterpret_cast<MemPage**>(m_currPtr) = this;
                    void* memPtr = reinterpret_cast<void*>(m_currPtr+sizeof(MemPage*));
                    m_currPtr += numBytes;
                    m_refCount++;
                    return memPtr;
                }
                /// Return the page which allocated the given pointer.
                static MemPage* pageForPointer(void* ptr)
                {
                    return *(reinterpret_cast<MemPage**>(ptr)-1);
                }
                /// Release one reference to the page.
                ///
                /// Return true if the number of references has fallen to zero so
                /// that the page should be freed.
                ///
                bool releaseRef()
                {
                    m_refCount--;
                    return m_refCount <= 0;
                }

                /// Link in the given page after this one.
                void linkAfter(MemPage* newNext)
                {
                    MemPage* oldNext = m_nextPage;
                    newNext->m_prevPage = this;
                    newNext->m_nextPage = oldNext;
                    if(oldNext)
                        oldNext->m_prevPage = newNext;
                    m_nextPage = newNext;
                }
                /// Unlink this page from the list.
                void unlink()
                {
                    if(m_prevPage)
                        m_prevPage->m_nextPage = m_nextPage;
                    if(m_nextPage)
                        m_nextPage->m_prevPage = m_prevPage;
                    m_prevPage = 0;
                    m_nextPage = 0;
                }
                /// Get the next page in the linked list
                MemPage* next()
                {
                    return m_nextPage;
                }
                const MemPage* next() const
                {
                    return m_nextPage;
                }
                /// Return the size of the current page
                size_t pageSize() const
                {
                    return m_pageSize - sizeof(MemPage*);
                }
        };

        // Memory page size
        size_t m_currentPageSize;

        /// Instance variables
        // List of in-use pages
        MemPage* m_firstPage;
        MemPage* m_currPage;
        MemPage* m_cachePage;

        /// Statistics
        // Records how many pages we've used at any one time.
        size_t m_maxPages;
        int m_allocCount;
        int m_freeCount;
        const char* m_name;

        /// Count the number of pages in the list.
        size_t countPages() const
        {
            size_t pageCount = 0;
            const MemPage* page = m_firstPage;
            while(page)
            {
                page = page->next();
                ++pageCount;
            }
            return pageCount;
        }

        /// Delete a single page
        void deletePage(MemPage* page) const
        {
            //std::cout << "- page " << m_name << std::endl;
            delete page;
        }

        /// delete all the pages held by this object
        void deleteAllPages()
        {
            deletePage(m_cachePage);
            MemPage* page = m_firstPage;
            while(page)
            {
                MemPage* tmp = page;
                page = page->next();
                deletePage(tmp);
            }
        }

        /// Remove a page from the list, and cache for recycling if necessary.
        void removeFromList(MemPage* page)
        {
            page->unlink();
            page->reset();
            if(m_cachePage == 0)
            {
                m_cachePage = page;
            }
            else if(m_cachePage->pageSize() < page->pageSize())
            {
                deletePage(m_cachePage);
                m_cachePage = page;
            }
            else
            {
                deletePage(page);
            }
        }

        /// Allocate a new page and push it onto the list.
        void pushNewPage(size_t minSize)
        {
            MemPage* newPage = 0;
            if(m_cachePage != 0 && m_cachePage->pageSize() >= minSize)
            {
                newPage = m_cachePage;
                m_cachePage = 0;
            }
            else
            {
                //std::cout << "+ page " << m_name << std::endl;
                newPage = new MemPage(std::max(minSize, m_currentPageSize));
            }
            m_currPage->linkAfter(newPage);
            m_currPage = newPage;
        }

    public:
        RegionAllocator(const char* name = "unnamed", size_t pageSize = 1<<20)
            : m_currentPageSize(pageSize),
            m_firstPage(new MemPage(pageSize)),
            m_currPage(m_firstPage),
            m_cachePage(0),
            m_maxPages(1),
            m_allocCount(0),
            m_freeCount(0),
            m_name(name)
        { }

        ~RegionAllocator()
        {
            m_maxPages = std::max(m_maxPages, countPages());
            deleteAllPages();
            std::cout << "Region allocator \"" << m_name << "\":\n";
            std::cout << "  Memory per page = " << m_currentPageSize/(1024.0*1024) << " MB" << "\n";
            std::cout << "  Maximum number of memory pages = " << m_maxPages << "\n";
            std::cout << "  alloc() was called " << m_allocCount << " times\n";
            std::cout << "  free() was called " << m_freeCount << " times\n";
        }

        /// Allocate numBytes of memory, and return a pointer to it.
        void* alloc(size_t numBytes)
        {
            m_allocCount++;
            // Get some memory from the current page if there's enough.
            void* memPtr = m_currPage->alloc(numBytes);
            if(memPtr == 0)
            {
                pushNewPage(numBytes);
                memPtr = m_currPage->alloc(numBytes);
            }
            assert(MemPage::pageForPointer(memPtr) == m_currPage);
            return memPtr;
        }

        /// Mark the memory associated with a pointer as no longer used.
        void free(void* ptr)
        {
            // As usual, it's safe to free 0.
            if(!ptr)
                return;
            m_freeCount++;
            MemPage* page = MemPage::pageForPointer(ptr);
            if(page->releaseRef())
            {
                // We need to free the page.
                m_maxPages = std::max(m_maxPages, countPages());
                if(page == m_currPage)
                    m_currPage->reset();
                else
                {
                    if(page == m_firstPage)
                        m_firstPage = m_firstPage->next();
                    removeFromList(page);
                }
            }
        }
};

} // namespace Aqsis

#endif // AQSIS_REGION_ALLOCATOR_H_INCLUDED
