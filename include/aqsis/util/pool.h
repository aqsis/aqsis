// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
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
		\brief A simple, efficient object pool based on code from Stroustrups
		"The C++ Programming Language - Third Edition"
*/

//? Is .h included already?
#ifndef POOL_H_INCLUDED
#define POOL_H_INCLUDED 1

#include	<aqsis/aqsis.h>

namespace Aqsis {

template <class T, TqInt CS=8>
class /*AQSIS_UTIL_SHARE*/ CqObjectPool
{
		struct SqLink
		{
			SqLink* m_next;
		};
		struct SqChunk
		{
			enum { size = CS*1024-16, };
			SqChunk* m_next;
			char m_mem[size];
		};
		SqChunk* m_chunks;

		const unsigned int m_esize;
		SqLink* m_head;

		void grow()	// Allocate new 'chunk', organize it as a linked list of elements of size 'm_esize'
		{
			SqChunk* n = new SqChunk;
			n->m_next = m_chunks;
			m_chunks = n;

			const int nelem = SqChunk::size/m_esize;
			char* start = n->m_mem;
			char* last = &start[(nelem-1)*m_esize];
			for (char* p = start; p<last; p+=m_esize)	// assume sizeof(SqLink)<=m_esize
				reinterpret_cast<SqLink*>(p)->m_next = reinterpret_cast<SqLink*>(p+m_esize);
			reinterpret_cast<SqLink*>(last)->m_next = 0;
			m_head = reinterpret_cast<SqLink*>(start);
		}

	public:
		CqObjectPool()
				: m_esize(sizeof(T)<sizeof(SqLink*)?sizeof(SqLink*):sizeof(T))
		{
			m_head = 0;
			m_chunks = 0;
		}

		~CqObjectPool() // free all chunks
		{
			SqChunk* n = m_chunks;
			while(n)
			{
				SqChunk* p = n;
				n = n->m_next;
				delete(p);
			}
		}

		// The following is a workaround for a bug which arises when using
		// g++ (observed with versions 3.4 and 4.2).  The bug occasionaly allows
		// code-reordering such that m_head is set to p->m_next *after* the
		// memory which m_next resides in has been trashed by the constructer
		// for the type T.  The results of a trashed m_head are immediate
		// allocation of another chunk, which quickly uses up the entire
		// available memory.  The problem can be observed in SDS scenes
		// (related to CqLath allocation); see for example
		// subdivision_facevertex_bug1892037.rib in the regression test suite.
		//
		// It's not entirely clear whether this bug is our fault - caused by a
		// combination of breaking the strict aliasing rules (ie, accessing the
		// memory through incompatible pointers: SqLink* and T*), or whether
		// it's the compiler's fault for doing code reordering around an
		// inlined call to operator new() and the T constructor.
		//
		// In any case, I don't know a compiler-independent fix for this, short
		// of placing alloc() in the cpp file, which would prevent inlining on
		// all compilers.
#		if AQSIS_COMPILER_GCC
		__attribute__((noinline))
#		endif
		void* alloc()
		{
			if (m_head==0)
				grow();
			SqLink* p = m_head;
			m_head = p->m_next;
			return(p);
		}

		void free(void* b)
		{
			SqLink* p = static_cast<SqLink*>(b);
			p->m_next = m_head;
			m_head = p;
		}

};


//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	// !MICROPOLYGON_H_INCLUDED
