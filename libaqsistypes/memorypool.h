// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


/** \file
		\brief Declares the CqMemoryPool class for providing managed memory allocation.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef MEMORYPOOL_H_INCLUDED 
//{
#define MEMORYPOOL_H_INCLUDED 1

#include	<vector>

#include	"aqsis.h"

#include	"ri.h"

START_NAMESPACE( Aqsis )


#define	MEMORYPOOL_DEFAULTBLOCKSIZE 512

//----------------------------------------------------------------------
/** \class CqMemoryPool
 * Template class for memory pool allocation
 */

template <class T, long S = MEMORYPOOL_DEFAULTBLOCKSIZE>
class CqMemoryPool
{
public:
    CqMemoryPool() : m_pHead( 0 ), m_pFirstBlock( 0 )
    {}
    virtual	~CqMemoryPool()
    {
        // Delete any remaining objects.
        // Note if this happens and anyone is pointing to the
        // allocated objects, you're in trouble.
		Flush();
    }

    /** Allocate a block from the pool.
     * \param size The requested size of the block, should be sizeof T, if not something is wrong.
     * \return void* pointer to memory block.
     */
    void*	Alloc( size_t size )
    {

        // send requests of the "wrong" size to ::operator new();
        if ( size != sizeof( T ) )
            return ::operator new( size );

        T* p = m_pHead;    // Point to the first free block.

        // if p is valid, just move the list head to the
        // next element in the free list
        if ( p ) m_pHead = p->m_pNext;
        else
        {
            // The free list is empty. Allocate a block of memory
            // big enough hold S objects
            char* p_newBlock = static_cast<char*>( ::operator new( S*sizeof( T ) + sizeof(char*) ) );
            T* newBlock = reinterpret_cast<T*>( p_newBlock + sizeof(char*) );

			// Either initialise, or continue the linked list of blocks.
			if( m_pFirstBlock != NULL )
			{
				void* pLastBlock = m_pFirstBlock;
				while( static_cast<void**>( pLastBlock )[0] != 0 )
					pLastBlock = static_cast<void**>( pLastBlock )[0];
				static_cast<void**>( pLastBlock )[0] = p_newBlock;
				reinterpret_cast<void**>( p_newBlock )[0] = 0;
			}
			else
			{
				reinterpret_cast<void**>( p_newBlock )[0] = 0;
				m_pFirstBlock = p_newBlock;
			}

            // form a new free list by linking the memory chunks
            // together; skip the zeroth element, because you'll
            // return that to the caller of operator new
            for ( int i = 1; i < S - 1; ++i )
                newBlock[ i ].m_pNext = &newBlock[ i + 1 ];

            // terminate the linked list with a null pointer
            newBlock[ S - 1 ].m_pNext = 0;

            // set p to front of list, headOfFreeList to
            // chunk immediately following
            p = newBlock;
            m_pHead = &newBlock[ 1 ];
        }
        return ( p );
    }

    /** Deallocate a block from the pool.
     * \param p Pointer to the block, should be in a valid pool, if not something is wrong.
     * \param size The requested size of the block, should be sizeof T, if not something is wrong.
     */
    void	DeAlloc( void* p, size_t size )
    {
        if ( p == 0 ) return ;

        if ( size != sizeof( T ) )
        {
            ::operator delete( p );
            return;
        }

        T *carcass = static_cast<T*>( p );

        carcass->m_pNext = m_pHead;
        m_pHead = carcass;
    }

	/** Flush the pool block allocations.
	 */
	void	Flush()
	{
		void* p_Block = m_pFirstBlock;
		while( p_Block )
		{
			void* p_nextBlock = static_cast<void**>( p_Block )[0];
			delete( p_Block );
			p_Block = p_nextBlock;
		}
		m_pFirstBlock = 0;
		m_pHead = 0;
	}

private:
    T*	m_pHead;		///< Pointer to the first free block in the pool.
	void*	m_pFirstBlock;
}
;


//----------------------------------------------------------------------
/** \class CqPoolable
 * Tamplate class to add the ability for a derived class to use a memory pool.
 */

template <class T, long S = MEMORYPOOL_DEFAULTBLOCKSIZE>
class CqPoolable
{
public:
    CqPoolable() : m_pNext( 0 )
    {}
    ~CqPoolable()
    {}

    /** Overridden operator new to allocate micropolys from a pool.
     */
    void* operator new( size_t size )
    {
        return( m_thePool.Alloc( size ) );
    }

    /** Overridden operator delete to allocate micropolys from a pool.
     */
    void operator delete( void* p )
    {
        m_thePool.DeAlloc( p, sizeof( T ) );
    }

    T*	m_pNext;	///< Pointer to the next object.

    static	CqMemoryPool<T, S>	m_thePool;	///< Static pool to allocated micropolys from.
    static void Flush()	{	m_thePool.Flush();	}
}
;


//-----------------------------------------------------------------------
/** Define the static memory pool for a poolable class.
 */

// the first line is the _explicit instantiation_ of m_thePool
// the second line is the _definition_ of m_thePool
#define	DEFINE_STATIC_MEMORYPOOL(A, S) \
	template CqMemoryPool<A, S> CqPoolable<A, S>::m_thePool; \
	template<> CqMemoryPool<A, S> CqPoolable<A, S>::m_thePool

//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

//}  // End of #ifdef MEMORYPOOL_H_INCLUDED
#endif
