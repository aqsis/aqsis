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
		\brief Declares an intrusive (as opposed to STL) list class.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef LIST_H_INCLUDED
//{
#define LIST_H_INCLUDED 1

#include	<aqsis/aqsis.h>

namespace Aqsis {

//----------------------------------------------------------------------
/** \class CqListEntry
 * List entry base class, all classes to be used in a list should be derived from this.
 */

template <class T>
class /*AQSIS_UTIL_SHARE*/ CqListEntry
{
	public:
		CqListEntry() : m_pPrevious( 0 ), m_pNext( 0 ), m_Invalid( false )
		{}
		virtual	~CqListEntry()
		{
			UnLink();
		}

		/** Get a pointer to the next element in the list.
		 * \return a pointer to the templatised list member class.
		 */
		T*	pNext() const
		{
			if ( m_pNext == 0 )
				return ( 0 );
			return ( ( m_pNext->m_Invalid ) ? m_pNext->pNext() : m_pNext );
		}
		/** Get a pointer to the previous element in the list.
		 * \return a pointer to the templatised list member class.
		 */
		T*	pPrevious() const
		{
			if ( m_pPrevious == 0 )
				return ( 0 );
			return ( ( m_pPrevious->m_Invalid ) ? m_pPrevious->pPrevious() : m_pPrevious );
		}
		/** Mark this class as an invalid place holder.
		 */
		void	Invalid()
		{
			m_Invalid = true;
		}
		/** Mark this class as not an invalid place holder.
		 */
		void	Valid()
		{
			m_Invalid = false;
		}

		/** Link this class into the list after the specified entry.
		 * \param pPrevious The list member to link this node in after.
		 */
		virtual	void	LinkAfter( CqListEntry*	pPrevious )
		{
			// Unlink from the current previous.
			if ( m_pPrevious != 0 )
				m_pPrevious->m_pNext = 0;
			m_pPrevious = 0;

			if ( pPrevious == 0 )
				return ;

			T*	pTemp = pPrevious->m_pNext;

			// Link the entire chain after this node
			// under the specified entry.
			T* pTail = static_cast<T*>( this );
			while ( pTail->m_pNext != 0 )
				pTail = pTail->m_pNext;
			pTail->m_pNext = pTemp;
			if ( pTemp != 0 )
				pTemp->m_pPrevious = static_cast<T*>( this );
			pPrevious->m_pNext = static_cast<T*>( this );
			m_pPrevious = static_cast<T*>( pPrevious );

			assert( !( m_pPrevious == m_pNext && m_pNext != 0 ) );
		}

		/** Link this class into the list before the specified entry.
		 * \param pNext The list member to link this node in before.
		 */
		virtual	void	LinkBefore( CqListEntry*	pNext )
		{
			// Unlink from the current next.
			if ( m_pNext != 0 )
				m_pNext->m_pPrevious = 0;
			m_pNext = 0;

			if ( pNext == 0 )
				return ;

			T*	pTemp = pNext->m_pPrevious;

			// Link the entire chain before the specified entry.
			T* pHead = static_cast<T*>( this );
			while ( pHead->m_pPrevious != 0 )
				pHead = pHead->m_pPrevious;
			pHead->m_pPrevious = pTemp;
			if ( pTemp != 0 )
				pTemp->m_pNext = static_cast<T*>( this );
			pNext->m_pPrevious = static_cast<T*>( this );
			m_pNext = static_cast<T*>( pNext );

			assert( !( m_pPrevious == m_pNext && m_pNext != 0 ) );
		}
		/** Unlink this node from the listm repairing the gap afterwards.
		 */
		virtual	void	UnLink()
		{
			if ( m_pNext != 0 )
				m_pNext->m_pPrevious = m_pPrevious;

			if ( m_pPrevious != 0 )
				m_pPrevious->m_pNext = m_pNext;

			m_pPrevious = m_pNext = 0;
		}
	protected:
		T*	m_pPrevious;		///< Pointer to the previous in the list.
		T*	m_pNext;			///< Pointer to the next in the list.
		bool	m_Invalid;		///< Used by list to denote a dummy place holder.
}
;


//----------------------------------------------------------------------
/** \class CqList
 * List class, templatized by the derived list class, T MUST be derived from CqListEntry
 */

template <class T>
class AQSIS_UTIL_SHARE CqList
{
	public:
		CqList()
		{
			m_Tail.LinkAfter( &m_Head );
			m_Tail.Invalid();
			m_Head.Invalid();
		}
		virtual	~CqList()
		{}

		/** Get a pointer to the head of the list.
		 * \return Pointer to the first entry in the list, after the head place holder.
		 */
		virtual T*	pFirst()
		{
			if ( m_Head.pNext() == &m_Tail )
				return ( 0 );
			return ( static_cast<T*>( m_Head.pNext() ) );
		}

		/** Link the spcified node at the start of the list, after the head place holder.
		 * \param pEntry Pointer to the node to link into the list.
		 */
		virtual void	LinkFirst( T* pEntry )
		{
			assert( pEntry != 0 );

			pEntry->LinkAfter( &m_Head );
		}
		/** Link the spcified node at the end of the list, before the tail place holder.
		 * \param pEntry Pointer to the node to link into the list.
		 */
		virtual void	LinkLast( T* pEntry )
		{
			assert( pEntry != 0 );

			pEntry->LinkBefore( &m_Tail );
		}
	private:
		CqListEntry<T>	m_Head;		///< Default head place holder.
		CqListEntry<T>	m_Tail;		///< Default tail place holder.
}
;


//-----------------------------------------------------------------------

} // namespace Aqsis

//}  // End of #ifdef LIST_H_INCLUDED
#endif
