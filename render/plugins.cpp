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
		\brief Implements CqPlugins plugin loader/deloader for textures/riprocedural/shared objects for shaders
		\author Michel Joron (joron@sympatico.ca)
*/


#include <stdio.h>
#include <string.h>



#include "aqsis.h"

#ifdef AQSIS_SYSTEM_WIN32
#include <Windows.h>                /* LoadLibrary() */
#endif /* AQSIS_SYSTEM_WIN32 */

#ifdef AQSIS_SYSTEM_BEOS
#endif /* AQSIS_SYSTEM_BEOS */

#ifdef AQSIS_SYSTEM_MACOSX
extern "C" {
#include <dlfcn.h>                /* dlopen() */
}
#endif /* AQSIS_SYSTEM_MACOSX */

#ifdef AQSIS_SYSTEM_POSIX
extern "C" {
#include <dlfcn.h>                /* dlopen() */
}
#endif /* AQSIS_SYSTEM_POSIX */

#include "plugins.h"


START_NAMESPACE( Aqsis )

//---------------------------------------------------------------------
/** Constructor.
 * Set up the search path, library (.dll, .dso, .so) name, and function name
 */
CqPlugins::CqPlugins(char *searchpath, char *library, char *function)
{
       strcpy(dynamicsearch, searchpath);
       strcpy(dynamiclibrary, library);
#ifdef AQSIS_SYSTEM_WIN32
       strcpy(dynamicfunction, function);
#elif defined(AQSIS_SYSTEM_MACOSX)
       sprintf(dynamicfunction,"_%s", function);
#else
       strcpy(dynamicfunction, function);
#endif
	   handle = NULL;
}
//---------------------------------------------------------------------
/** Main function to call!
 * return the function pointer based on the construction information 
 * if possible otherwise it returns NULL;
 */
void *CqPlugins::Function() 
{
void *function_pt = NULL;

#ifdef AQSIS_SYSTEM_WIN32
	/* dll was never loaded before */
	if ( !handle )
		handle = (void*) LoadLibrary( dynamiclibrary );
	if ( handle )
	{
		function_pt = ( void * ) GetProcAddress( ( HINSTANCE ) handle, dynamicfunction );
		if (function_pt == NULL) 
		{
			LPVOID lpMsgBuf;
			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
			);
			// Process any inserts in lpMsgBuf.
			// ...
			// Display the string.
			printf("%s: %s", dynamicfunction, (LPCTSTR)lpMsgBuf);
			// Free the buffer.
			LocalFree( lpMsgBuf );

		}
	} else 
	{
			LPVOID lpMsgBuf;
			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
			);
			// Process any inserts in lpMsgBuf.
			// ...
			// Display the string.
			printf("%s: %s", dynamiclibrary, (LPCTSTR)lpMsgBuf);
			// Free the buffer.
			LocalFree( lpMsgBuf );

	}
#elif defined(AQSIS_SYSTEM_MACOSX)
	// We probably need an interface for CFPlugins here
	// But for now, we will not implement plugins
	handle = (void *) dlopen( dynamiclibrary, RTLD_NOW | RTLD_GLOBAL);
    if (handle) 
    {
        function_pt = ( void * ) dlsym( handle, dynamicfunction );
    }
#elif defined(AQSIS_SYSTEM_BEOS)
	// We probably need an interface for CFPlugins here
	// But for now, we will not implement plugins
	function_pt = NULL;
#else

	/* so was never loaded before */
	if ( !handle )
		handle = dlopen( dynamiclibrary, RTLD_LAZY );
	if ( handle )
	{
		function_pt = ( void  * ) dlsym( handle, dynamicfunction );
	}

#endif

    return function_pt;
}

//---------------------------------------------------------------------
/** Close and unload the .dll, .dso, .so
 */
void CqPlugins::Close()
{

#ifdef AQSIS_SYSTEM_WIN32

        if (handle)
	   FreeLibrary( ( HINSTANCE ) handle );
#elif defined(AQSIS_SYSTEM_MACOSX)
	if (handle)
	   dlclose( handle );
#elif defined(AQSIS_SYSTEM_BEOS)
	// Do nothing for now
#else
        if (handle)
			dlclose( handle );
#endif
        handle = NULL;

}

END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------

