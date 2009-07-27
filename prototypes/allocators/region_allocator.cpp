#include <iostream>
#include <list>
#include <algorithm>
#include <queue>
#include <limits>
#include <cassert>

//------------------------------------------------------------------------------
/** A simple (and quite possible braindead) region-based memory manager.
 *
 * The idea of this allocator is that we keep a list of allocated pages which
 * are much larger than the size of the objects to be allocated.
 *
 * Allocation:
 * Objects are allocated starting at the beginning of a page, and incrementing
 * toward the end.  When a page fills up, a new page is created and pushed onto
 * the front of the list.
 *
 * No attempt is made to reuse the memory inside a page.
 *
 * Deallocation:
 * Memory is deallocated by finding the page in which the pointer lives, and
 * decrementing the reference count for that page.  If the reference count for
 * a page falls to 0, the page is removed.
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

			public:
				MemPage(size_t pageSize = defaultPageSize)
					: m_basePtr(new unsigned char[pageSize]),
					m_currPtr(m_basePtr),
					m_pageSize(pageSize),
					m_refCount(0)
				{ }
				~MemPage()
				{
					delete[] m_basePtr;
				}

				/** Allocate a block of memory from the page
				 *
				 * Return 0 if the page has not enough memory left.
				 */
				void* alloc(size_t numBytes)
				{
					// Align size with 8 byte boundaries.
					numBytes = (numBytes + 7) & ~7;
					if(static_cast<ptrdiff_t>(numBytes) + (m_currPtr - m_basePtr)
							> static_cast<ptrdiff_t>(m_pageSize))
					{
						return 0;
					}
					void* memPtr = reinterpret_cast<void*>(m_currPtr);
					m_currPtr += numBytes;
					m_refCount++;
					return memPtr;
				}
				/** Return true if the given pointer is contained within this page.
				 */
				bool inPage(void* ptr) const
				{
					ptrdiff_t offset = (reinterpret_cast<unsigned char*>(ptr) - m_basePtr);
					return offset >= 0 && offset < static_cast<ptrdiff_t>(m_pageSize);
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
		};
	private:
		// Types
		typedef std::list<MemPage*> MemStackType;

		// Default memory page size.  In a real allocator this should proabably
		// be replaced with a dynamically-adjusted page size...
		static const size_t defaultPageSize;

		/// Instance variables
		// List of in-use pages
		MemStackType m_pageList;

		/// Statistics
		// Records how many pages we've used at any one time.
		size_t m_maxPages;
		int m_allocCount;
		int m_freeCount;

	public:
		RegionManager()
			: m_pageList(),
			m_maxPages(1),
			m_allocCount(0),
			m_freeCount(0)
		{
			m_pageList.push_back(new MemPage());
		}
		~RegionManager()
		{
			m_maxPages = std::max(m_maxPages, m_pageList.size());
			for(MemStackType::iterator i = m_pageList.begin(); i != m_pageList.end(); ++i)
			{
				delete *i;
				*i = 0;
			}
			std::cout << "Maximum number of memory pages = " << m_maxPages << "\n";
			std::cout << "alloc() was called " << m_allocCount << " times\n";
			std::cout << "free() was called " << m_freeCount << " times\n";
		}

		/** Allocate numBytes of memory, and return a pointer to it.
		 */
		void* alloc(size_t numBytes)
		{
			m_allocCount++;
			MemPage* currPage = m_pageList.back();
			// Get some memory from the current page if there's enough.
			void* memPtr = currPage->alloc(numBytes);
			if(memPtr != 0)
				return memPtr;
			// else allocate a new page
			currPage = new MemPage(std::max(numBytes, defaultPageSize));
			//std::cout << "allocate new page at " << currPage << "\n";
			m_pageList.push_back(currPage);
			return currPage->alloc(numBytes);
		}
		/** Mark the memory associated with a pointer as no longer used.
		 */
		void free(void* ptr)
		{
			// Make sure it's safe to free 0.
			if(!ptr)
				return;
			m_freeCount++;
			// Braindead linear search.  We should probably use a std::map
			// instead of a list here, and then the correct page can be found
			// very quickly.
			for(MemStackType::iterator i = m_pageList.begin();
					i != m_pageList.end(); ++i)
			{
				if((*i)->inPage(ptr))
				{
					if((*i)->releaseRef())
					{
						size_t numPages = m_pageList.size();
						// Always keep one page allocated
						if(numPages > 1)
						{
							delete *i;
							m_maxPages = std::max(m_maxPages, numPages);
//							std::cout << "deleting page " << pageCount << " of "
//								<< numPages << "\n";
							m_pageList.erase(i);
						}
					}
					return;
				}
			}
			// If we get here then it's an invalid free.
			assert(0);
		}
};

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
//	std::queue<A*, std::deque<A*,RegionAllocator<A*> > > memQueue;
	std::queue<A*> memQueue;
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
