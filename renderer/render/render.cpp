// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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
		\brief Implements the structures and functions for system specific renderer initialisation.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	"aqsis.h"

#ifdef AQSIS_SYSTEM_WIN32
#include	<windows.h>
#include	<io.h>
#include	<fcntl.h>
#endif // AQSIS_SYSTEM_WIN32

#include	"sstring.h"
#include	"render.h"

using namespace Aqsis;

#ifdef AQSIS_SYSTEM_WIN32

BOOL APIENTRY DllMain( HINSTANCE hModule,
                       DWORD ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
	switch ( ul_reason_for_call )
	{
			case DLL_PROCESS_ATTACH:
			break;
			case DLL_THREAD_ATTACH:
			break;
			case DLL_THREAD_DETACH:
			break;
			case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}

#endif // AQSIS_SYSTEM_WIN32


START_NAMESPACE( Aqsis )

//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )
