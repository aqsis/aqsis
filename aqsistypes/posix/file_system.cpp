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
		\brief Implements the system specific parts of the CqFile class for handling files.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	<fstream>

#include	<string.h>
#include	<ctype.h>
#include	<glob.h>

#include	"aqsis.h"

#include	"file.h"
#ifndef	SCONS_BUILD
#ifndef	AQSIS_SYSTEM_MACOSX
#include	"config.h"
#endif	// AQSIS_SYSTEM_MACOSX
#endif	// SCONS_BUILD

START_NAMESPACE( Aqsis )

//---------------------------------------------------------------------
/** Given a string representing a filename with wildcards, return a list
 * of filenames that match that string.
*/
std::list<CqString*> CqFile::Glob ( const CqString& strFileGlob )
{
	glob_t globbuf;
	const char *pt = strFileGlob.c_str();

	globbuf.gl_offs = 0;
	glob( pt, GLOB_DOOFFS, NULL, &globbuf );

	std::list<CqString*> result;
	TqUint i;
	for ( i = 0;i < globbuf.gl_pathc;i++ )
	{
		result.push_front( new CqString( globbuf.gl_pathv[ i ] ) );
	}

	globfree( &globbuf );
	return result;
}


CqString CqFile::FixupPath(CqString& strPath)
{
	return( strPath );
}


END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------
