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
		\brief Declares the CqCriticalSection class which wraps the system specific critical section.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#ifndef CRITICALSECTION_H_INCLUDED
#define CRITICALSECTION_H_INCLUDED 

#include "ri.h"

//--------------------------------------------------------------------------
/** Provide an encapsulation around the system critical section implementation. 
 *
 */
class CqCriticalSection
{
	friend class CqEnterCriticalSection;

	public:
		CqCriticalSection();
		~CqCriticalSection();

	private:
		void* m_hMutex;		///< A pointer to the mutex. Use a void* so that clients don't have to include system specific headers.h
};


//--------------------------------------------------------------------------
/** Used to enter/leave a critical section. The constructor enters the critical section, the destructor leaves it. 
 *
 * Leaving a critical section from a thread other than the one which entered it is an error.
 */

class CqEnterCriticalSection
{
	public:
		CqEnterCriticalSection(CqCriticalSection* pObject);
		~CqEnterCriticalSection();

	private:
		CqCriticalSection* m_pCriticalSection;	///< a pointer to the criticial section class.
};


#endif // CRITICALSECTION_H_INCLUDED
