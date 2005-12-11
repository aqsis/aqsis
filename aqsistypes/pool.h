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
		\brief A simple, efficient object pool based on code from Stroustrups
		"The C++ Programming Language - Third Edition"
*/

//? Is .h included already?
#ifndef POOL_H_INCLUDED
#define POOL_H_INCLUDED 1

#include	"aqsis.h"

START_NAMESPACE( Aqsis )

template <class T, TqInt CS=8>
class CqObjectPool
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

END_NAMESPACE( Aqsis )

#endif	// !MICROPOLYGON_H_INCLUDED
