#include <iostream>
#include <list>
#include <algorithm>
#include <queue>
#include <limits>
#include <cassert>

//------------------------------------------------------------------------------
/** A region-based memory manager with page recycling and O(1) page-location
 * cost when deallocating a pointer.
 *
 * The idea of this allocator is that we keep a list of allocated pages which
 * are much larger than the size of the objects to be allocated.
 *
 * Allocation:
 *   Objects are allocated starting at the beginning of a page, and
 *   incrementing toward the end.  When a page fills up, a new page is created
 *   and pushed onto the front of the list.  Whenever an object is allocated, a
 *   pointer to the associated page is kept in the sizeof(MemPage*) bytes
 *   before the returned pointer address.
 *
 *   No attempt is made to reuse the memory inside a page while live objects
 *   still exist within the page.
 *
 * Deallocation:
 *   Memory is deallocated by finding the page in which the pointer lives, and
 *   decrementing the reference count for that page.  If the reference count
 *   for a page falls to 0, the page is removed from the active list.  The
 *   largest recently deallocated page is kept in a cache and reused the next
 *   time a page needs to be allocated.
 *
 *   The page associated with a given pointer is found by reading the memory
 *   just preceding the given pointer.  This is an O(1) operation, but
 *   surprisingly doesn't perform as well as initially expected.  I suspect
 *   this is because reading the memory causes a cache miss.  This probably
 *   shows up the artificiality of the test - in the real case, we're probably
 *   going to be stomping over that memory anyway with our destructor :-/
 */
class RegionManager
{
	private:
		/** Hold information about a memory page
		 */
		class MemPage
		{
			private:
				unsigned char* const m_basePtr;
				unsigned char* m_currPtr;
				const size_t m_pageSize;
				int m_refCount;

				// Embedded linked list
				MemPage* m_nextPage;
				MemPage* m_prevPage;
			public:
				MemPage(size_t pageSize = defaultPageSize)
					: m_basePtr(new unsigned char[pageSize+sizeof(MemPage*)]),
					m_currPtr(m_basePtr),
					m_pageSize(pageSize),
					m_refCount(0),
					m_nextPage(0),
					m_prevPage(0)
				{ }
				~MemPage()
				{
					delete[] m_basePtr;
				}
				/// Reset the page for memory recycling.
				void reset()
				{
					m_currPtr = m_basePtr;
					m_refCount = 0;
				}

				/** Allocate a block of memory from the page
				 *
				 * Return 0 if the page has not enough memory left.
				 */
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
				/** Return the page which allocated the given pointer.
				 */
				static MemPage* pageForPointer(void* ptr)
				{
					return *(reinterpret_cast<MemPage**>(ptr)-1);
				}
				/** Release one reference to the page.
				 *
				 * Return true if the number of references has fallen to zero so
				 * that the page should be freed.
				 */
				bool releaseRef()
				{
					m_refCount--;
					return m_refCount <= 0;
				}

				/** Link in the given page after this one.
				 */
				void linkAfter(MemPage* newNext)
				{
					MemPage* oldNext = m_nextPage;
					newNext->m_prevPage = this;
					newNext->m_nextPage = oldNext;
					if(oldNext)
						oldNext->m_prevPage = newNext;
					m_nextPage = newNext;
				}
				/** Unlink this page from the list.
				 */
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
	private:
		// Default memory page size.  In a real allocator this should proabably
		// be replaced with a dynamically-adjusted page size...
		static const size_t defaultPageSize;

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
		/// delete all the pages held by this object
		void deleteAllPages()
		{
			delete m_cachePage;
			MemPage* page = m_firstPage;
			while(page)
			{
				MemPage* tmp = page;
				page = page->next();
				delete tmp;
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
				delete m_cachePage;
				m_cachePage = page;
			}
			else
			{
				delete page;
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
				newPage = new MemPage(std::max(minSize, defaultPageSize));
			}
			m_currPage->linkAfter(newPage);
			m_currPage = newPage;
		}

	public:
		RegionManager()
			: m_firstPage(new MemPage),
			m_currPage(m_firstPage),
			m_cachePage(0),
			m_maxPages(1),
			m_allocCount(0),
			m_freeCount(0)
		{ }
		~RegionManager()
		{
			m_maxPages = std::max(m_maxPages, countPages());
			deleteAllPages();
			std::cout << "Maximum number of memory pages = " << m_maxPages << "\n";
			std::cout << "alloc() was called " << m_allocCount << " times\n";
			std::cout << "free() was called " << m_freeCount << " times\n";
		}

		/** Allocate numBytes of memory, and return a pointer to it.
		 */
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
		/** Mark the memory associated with a pointer as no longer used.
		 */
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

// Use 1M as the default page size.
const size_t RegionManager::defaultPageSize = 1 << 20;

// Static instance of RegionManager where we will do our allocation for the test.
static RegionManager regions;

//------------------------------------------------------------------------------
/** Wrap RegionManager with the standard STL allocator interface.
 *
 * Allocates memory in the static RegionManager named "regions".
 */
template<class T>
class RegionAllocator
{
	public:
		typedef T value_type;
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;

		typedef T* pointer;
		typedef const T* const_pointer;

		typedef T& reference;
		typedef const T& const_reference;

		pointer address(reference r) const { return &r; }
		const_pointer address(const_reference r) const { return &r; }

		RegionAllocator() throw() {}
		template<class U> RegionAllocator(const RegionAllocator<U>&) throw() {}
		~RegionAllocator() throw() {}

		pointer allocate(size_type n, const void* hint = 0)
		{
			return reinterpret_cast<pointer>(regions.alloc(n*sizeof(T)));
		}
		void deallocate(pointer p, size_type n)
		{
			regions.free(reinterpret_cast<void*>(p));
		}

		void construct(pointer p, const T& val)
		{
			new(p) T(val);
		}
		void destroy(pointer p)
		{
			p->~T();
		}

		size_type max_size() const throw()
		{
			return std::numeric_limits<size_t>::max();
		}

		template<class U>
		struct rebind { typedef RegionAllocator<U> other; };
};

template<class T>
bool operator==(const RegionAllocator<T>& alloc1, const RegionAllocator<T>& alloc2)
{ return true; }

template<class T>
bool operator!=(const RegionAllocator<T>& alloc1, const RegionAllocator<T>& alloc2)
{ return false; }

//------------------------------------------------------------------------------
/** A class which simply overrides the new and delete operators to use the
 * custom memory allocator.
 *
 * Contains sizeOfA bytes of storage.
 */
class A
{
	public:
		//*********************************************************
		// Overridden operators new and delete.  To see how fast the program
		// runs with the default system allocator, just comment these two
		// functions out.
		//*********************************************************
		/*
		*/
		void* operator new(size_t size)
		{
			return regions.alloc(size);
		}
		void operator delete(void* ptr)
		{
			regions.free(ptr);
		}
	private:
		static const int m_sizeOfA = 1000;
		char m_a[m_sizeOfA];
};


//------------------------------------------------------------------------------
int main()
{
	// Number of instances of A held at any one time
	const int numObjects = 200000;
	// Total number of allocations/deallocations
	const int numAllocs = 10000000;

	// Simulate an ideal queue-like allocation pattern.  This is the best
	// possible pattern we can hope for in terms of the memory efficiency of
	// the allocator.  Random-access allocation patterns are likely to cause
	// the memory usage to grow very nastily.

	// Put some objects onto a queue.
	
	// Note: Here we're using the same allocator for memQueue as for our "A"
	// objects - with memory recycling it seems not to affect the performance.
	std::queue<A*, std::deque<A*,RegionAllocator<A*> > > memQueue;
	//std::queue<A*> memQueue;
	for(int i = 0; i < numObjects; ++i)
	{
		memQueue.push(new A);
	}

	// Allocates stuff onto the front of the queue and deallocates a
	// corresponding object off the back so that the total number of objects
	// remains constant
	for(int i = 0; i < numAllocs; ++i)
	{
		memQueue.push(new A);
		delete(memQueue.front());
		memQueue.pop();
	}

	// Remove the remaining objects from the queue.
	while(!memQueue.empty())
	{
		delete(memQueue.front());
		memQueue.pop();
	}

	return 0;
}
