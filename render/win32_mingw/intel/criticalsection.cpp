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
		\brief Implements the CqCriticalSection class which wraps the system specific critical section.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#ifdef	WIN32
	#include <windows.h>
#endif

#include	"aqsis.h"
#include	"specific.h"

#include "criticalsection.h"

///--------------------------------------------------------------------------
///
/// CqCriticalSection::CqCriticalSection

CqCriticalSection::CqCriticalSection() :
	m_hMutex(CreateMutex(NULL, FALSE, NULL))
{
	assert(m_hMutex != NULL);
}


///--------------------------------------------------------------------------
///
/// CqCriticalSection::~CqCriticalSection

CqCriticalSection::~CqCriticalSection()
{
	CloseHandle(m_hMutex);
}


///--------------------------------------------------------------------------
///
/// CqEnterCriticalSection::CqEnterCriticalSection

CqEnterCriticalSection::CqEnterCriticalSection(CqCriticalSection* pCriticalSection) :
	m_pCriticalSection(pCriticalSection)
{
	while(true)
	{
		DWORD dwReason = MsgWaitForMultipleObjects(1, &(m_pCriticalSection->m_hMutex),
			FALSE, INFINITE, QS_SENDMESSAGE);
		if(dwReason == WAIT_OBJECT_0)
		{
			return;
		}
		else
		{
			assert(dwReason == (WAIT_OBJECT_0 + 1));

			// There are sent messages waiting call PeekMessage so they can be delivered
			// PeekMessage causes a yield to occur according to the MSDN docs.
			MSG msg;
			PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
		}
	}
}


///--------------------------------------------------------------------------
///
/// CqEnterCriticalSection::~CqEnterCriticalSection

CqEnterCriticalSection::~CqEnterCriticalSection()
{
	ReleaseMutex(m_pCriticalSection->m_hMutex);
}
