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
		\brief Implements CqPluginBase plugin loader/deloader for textures/riprocedural/shared objects for shaders
		\author Michel Joron (joron@sympatico.ca)
*/


#include <stdio.h>
#include <string.h>



#include <aqsis/aqsis.h>

#ifdef AQSIS_SYSTEM_WIN32
#include <Windows.h>                /* LoadLibrary() */
#endif /* AQSIS_SYSTEM_WIN32 */

#ifdef AQSIS_SYSTEM_POSIX
extern "C"
{
	// For Mac OS X, define MACOSX_NO_LIBDL if libdl not installed
#ifndef MACOSX_NO_LIBDL
#include <dlfcn.h>                /* dlopen() */
#endif
}
#endif /* AQSIS_SYSTEM_POSIX */


#include <aqsis/util/plugins.h>
#include <aqsis/util/logging.h>


namespace Aqsis {


//---------------------------------------------------------------------
/** dlfunc wrappers
 * These function abstract dlopen,dlsym and dlclose for win32, OS-X, BeOS 
 * and POSIX etc.
 */
void *
CqPluginBase::DLOpen( CqString *library )
{
	void * handle = NULL;
	Aqsis::log() << info << "Loading plugin \"" << library->c_str() << "\"" << std::endl;

#ifndef AQSIS_NO_PLUGINS
#ifdef AQSIS_SYSTEM_WIN32

	handle = ( void* ) LoadLibrary( library->c_str() );
#else

	CqString tstring = *library;
	CqString::size_type pos = tstring.find ("/");
	if (pos == CqString::npos)
		tstring = CqString("./") + *library;
	handle = ( void * ) dlopen( tstring.c_str(), RTLD_NOW);
#endif
#endif // AQSIS_NO_PLUGINS

	if ( handle )
		m_activeHandles.push_back( handle );
	else
		/** \todo <b>Code Review</b>: Re-evaluate how the error handling works
		 * here.  For now we report the error, but perhaps it would be better
		 * to throw an exception and let the calling code handle the problem.
		 */
		//Aqsis::log() << error << "Error loading plugin: \"" << *library << "\" \"" << DLError() << "\"\n";
		AQSIS_THROW_XQERROR(XqPluginError, EqE_NoFile, "Error loading plugin: \"" << *library << "\" \"" << DLError() << "\"\n");
	return handle;
}

void *
CqPluginBase::DLSym( void *handle, CqString *symbol )
{
	void * location = NULL;

#ifndef AQSIS_NO_PLUGINS

	if ( handle )
	{

#if   defined (AQSIS_SYSTEM_WIN32) //Win32 LoadProc support
		location = ( void * ) GetProcAddress( ( HINSTANCE ) handle, symbol->c_str() );
#else

		location = ( void * ) dlsym( handle, symbol->c_str() );
#endif

	};

#endif // AQSIS_NO_PLUGINS

	return location;
}

void
CqPluginBase::DLClose( void *handle )
{
#ifndef AQSIS_NO_PLUGINS
	if ( handle )
	{

#if   defined (AQSIS_SYSTEM_WIN32) //Win32 LoadProc support
		FreeLibrary( ( HINSTANCE ) handle );
#else

		dlclose( handle );
#endif

	};
	m_activeHandles.remove( handle );
#endif // AQSIS_NO_PLUGINS
}

CqPluginBase::~CqPluginBase()
{
	while ( !m_activeHandles.empty() )
	{
		if ( m_activeHandles.front() != NULL )
			DLClose( m_activeHandles.front() );
	};
};


const CqString
CqPluginBase::DLError( void )
{
	CqString errorlog;
#ifndef AQSIS_NO_PLUGINS
#ifdef AQSIS_SYSTEM_WIN32

	LPVOID lpMsgBuf;
	FormatMessage(
	    FORMAT_MESSAGE_ALLOCATE_BUFFER |
	    FORMAT_MESSAGE_FROM_SYSTEM |
	    FORMAT_MESSAGE_IGNORE_INSERTS,
	    NULL,
	    GetLastError(),
	    MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),   // Default language
	    ( LPTSTR ) & lpMsgBuf,
	    0,
	    NULL
	);
	// Process any inserts in lpMsgBuf.
	// ...
	// Display the string.
	errorlog = ( CqString ) ( LPCTSTR ) lpMsgBuf ;

	// Free the buffer.
	LocalFree( lpMsgBuf );
#else //not defined AQSIS_SYSTEM_MACOSX

	char* error = dlerror();
	if( error )
		errorlog = error;
#endif

#else

	errorlog = "Aqsis was built without plugin support\n";
#endif // AQSIS_NO_PLUGINS

	return errorlog;
}

} // namespace Aqsis
//---------------------------------------------------------------------

