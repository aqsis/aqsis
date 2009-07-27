region_allocator.cpp
====================
This prototype tries to analyse how much better we might do with a custom
region-based memory allocator than with the default system allocator.

When testing under ideal conditions (FIFO-like memory allocation) it's
possible to get 10x speedup, which suggests that it might be helpful in some
specific non-ideal situations.  The performance degrades rapidly as the
alloc/free pattern becomes more random however.  For instance, using the same
allocator for the std::queue which is holding the objects for the test
increases the time to unusable levels.  This might be alleviated by recycling
the most recently unlinked page (see comments below).

Tests show that the search time for the correct memory page on free is
critically important to performance.  In particular, linear search for more
than a few pages won't suffice - this can be seen by reversing the search
order.  Perhaps some most-recently-used-page hint pointer combined with a
std::map and careful implementation of the operator< for the map key might be
better...


region_allocator2.cpp
=====================
This is an alternative implementation which tries out one possible method for
fixing the search time.  The idea is to do the standard trick of putting in a
chunk header just before the memory location which is returned by alloc().
The header contains a pointer to the associated MemPage and makes getting the
relevant page an O(1) operation in the number of allocated pages.

Unfortunately (and surprisingly) the end result is much slower - it can be
made to beat the standard allocator, but apparently by less than a factor of
two.  I attribute this to the fact that we're actually touching the memory
which was allocated, which probably means a cache miss.  I guess that speaks
for the artificiality of the test conditions :-/

It also seems to require memory recycling, otherwise the system-time goes up
dramatically.  strace() indicates that the system time is spent in many calls
to mmap and munmap which are used by the standard allocator for getting large
blocks of memory...

More positively, it's apparently possible to get good performance while using
the allocator for STL objects.  I'm putting this down to the recycling
of the most recently discarded page, compared to region_allocator.cpp, but
further investigation is required.

Since I was going low-level with storing pointers etc, I also embedded the
linked list inside the MemPage objects rather than using std::list.  This
means that everything is quite explicit in this example, at the cost of more
code, and possibly bugs...

Notes:
======
Make sure it's compiled with optimizations, or it'll be much slower than
the system allocator!  See the Makefile.

