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
		\brief Implements CqPluginBase plugin loader/deloader for textures/riprocedural/shared objects for shaders
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
extern "C"
{
	// For Mac OS X, define MACOSX_NO_LIBDL if libdl not installed
#ifndef MACOSX_NO_LIBDL
#include <dlfcn.h>                /* dlopen() */
#endif
}
#endif /* AQSIS_SYSTEM_MACOSX */
#ifdef AQSIS_SYSTEM_POSIX
extern "C"
{
	// For Mac OS X, define MACOSX_NO_LIBDL if libdl not installed
#ifndef MACOSX_NO_LIBDL
#include <dlfcn.h>                /* dlopen() */
#endif
}
#endif /* AQSIS_SYSTEM_POSIX */


#include "plugins.h"


START_NAMESPACE( Aqsis )


//---------------------------------------------------------------------
/** dlfunc wrappers
 * These function abstract dlopen,dlsym and dlclose for win32, OS-X, BeOS 
 * and POSIX etc.
 */
void *
CqPluginBase::DLOpen(CqString *library)
{
  	void *handle = NULL;

#ifdef	PLUGINS
#ifdef AQSIS_SYSTEM_WIN32 
	handle = ( void* ) LoadLibrary( library->c_str() );
#elif defined (AQSIS_SYSTEM_MACOSX) 
#  ifndef MACOSX_NO_LIBDL
	handle = ( void * ) dlopen( library->c_str(), RTLD_NOW | RTLD_GLOBAL );
#  endif
#elif defined (AQSIS_SYSTEM_BEOS)
		// We probably need an interface for CFPlugins here
		// But for now, we will not implement plugins
#else  
	handle = ( void * ) dlopen( library->c_str(), RTLD_LAZY );
#endif
#endif
	if (handle) m_activeHandles.push_back(handle);
	return handle;
}

void *
CqPluginBase::DLSym(void *handle, CqString *symbol )
{
	void *location = NULL;

#ifdef	PLUGINS
	if ( handle )
	{

#if   defined (AQSIS_SYSTEM_WIN32) //Win32 LoadProc support
		location = ( void * ) GetProcAddress( ( HINSTANCE ) handle, symbol->c_str() );
#elif defined (AQSIS_SYSTEM_MACOSX) 
#  ifndef MACOSX_NO_LIBDL
		location = ( void * ) dlsym( handle, symbol->c_str() );
#  endif
#elif defined (AQSIS_SYSTEM_BEOS)
		// We probably need an interface for CFPlugins here
		// But for now, we will not implement plugins
#else
		//this is the same as MacOS-X but we might aswell keep the same seperation for the moment
		location = ( void * ) dlsym( handle, symbol->c_str() );
#endif
	};

#endif
	return location;
}

void
CqPluginBase::DLClose(void *handle)
{
#ifdef	PLUGINS
	if ( handle )
	{

#if   defined (AQSIS_SYSTEM_WIN32) //Win32 LoadProc support
		FreeLibrary( ( HINSTANCE ) handle );
#elif defined (AQSIS_SYSTEM_MACOSX) 
#  ifndef MACOSX_NO_LIBDL
		dlclose( handle );
#  endif
#elif defined (AQSIS_SYSTEM_BEOS)
		// We probably need an interface for CFPlugins here
		// But for now, we will not implement plugins
#else
		//this is the same as MacOS-X but we might aswell keep the same seperation for the moment
		dlclose( handle );
#endif
	};
	m_activeHandles.remove(handle);
#endif
}

CqPluginBase::~CqPluginBase()
{
	while ( !m_activeHandles.empty() )
	{
		if( m_activeHandles.front() != NULL )
			DLClose( m_activeHandles.front() );
	};
};


const CqString
CqPluginBase::DLError(void)
{
	CqString errorlog;
#ifdef	PLUGINS
#ifdef AQSIS_SYSTEM_WIN32
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),  // Default language
		( LPTSTR ) & lpMsgBuf,
		0,
		NULL
	);
	// Process any inserts in lpMsgBuf.
	// ...
	// Display the string.
	errorlog = (CqString) ( LPCTSTR ) lpMsgBuf ;

	// Free the buffer.
 	LocalFree( lpMsgBuf );
#else
        errorlog = dlerror() ;
#endif

#else
	errorlog = "Aqsis was built without plugin support\n";
#endif
	return errorlog;
}

END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------

