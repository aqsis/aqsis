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
		\brief Implements the CqFile class for handling files with RenderMan searchpath option support.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<fstream>

#include	<string.h>
#include	<ctype.h>

#include	"aqsis.h"

#include	"file.h"

START_NAMESPACE( Aqsis )

//---------------------------------------------------------------------
/** Constructor
 */

CqFile::CqFile( const char* strFilename, const char* strSearchPathOption ) : m_pStream( 0 )
{
	Open( strFilename, strSearchPathOption );
}


//---------------------------------------------------------------------
/** Attach this CqFile object to a new file if we can find it.
 * \param strFilename Character pointer to the filename.
 * \param strSearchPathOption Character pointer to name of RI "searchpath" option to use as the searchpath.
 * \param mode iostream mode used to open the file.
 */

void CqFile::Open( const char* strFilename, const char* strSearchPathOption, std::ios::openmode mode )
{
	// Search in the current directory first.
	m_strRealName = strFilename;
	m_bInternal = TqTrue;
	std::ifstream* pFStream = new std::ifstream( strFilename, mode );
	if ( !pFStream->is_open() )
	{
		// If a searchpath option name was specified, use it.
		if ( strcmp( strSearchPathOption, "" ) != 0 )
		{
			// if not found there, search in the specified option searchpath.
			CqString SearchPath( strSearchPathOption );
			// Search each specified path in the search path (separated by ':' or ';')
			unsigned int start = 0;
			while ( 1 )
			{
				// Find the next search path in the spec.
				unsigned int len = SearchPath.find_first_of( ";:", start ) - start;
				// Check if it is realy meant as a drive spec.
				if ( len == 1 && isalpha( SearchPath[ start ] ) )
					len += strcspn( &SearchPath[ start + 2 ], ";:" ) + 1;
				CqString strPath = SearchPath.substr( start, len );
				if ( strPath == "" )
					break;

				// Apply any system specific string modification.
				strPath = FixupPath( strPath );

				// See if the shader can be found in this directory
				CqString strAlternativeFilename = strPath;
				// Check the path is correctly terminated
				if ( strAlternativeFilename[ strAlternativeFilename.size() - 1 ] != '/' &&
				        strAlternativeFilename[ strAlternativeFilename.size() - 1 ] != '\\' )
#ifdef AQSIS_SYSTEM_WIN32

					strAlternativeFilename += "\\";
#else // AQSIS_SYSTEM_WIN32

					strAlternativeFilename += "/";
#endif // !AQSIS_SYSTEM_WIN32

				strAlternativeFilename += strFilename;

				// Clear the previous error first.
				pFStream->clear();
				pFStream->open( strAlternativeFilename.c_str(), std::ios::in );
				if ( pFStream->is_open() )
				{
					m_pStream = pFStream;
					m_strRealName = strAlternativeFilename;
					break;
				}
				if ( len < strlen( &SearchPath[ start ] ) )
					start += len + 1;
				else
					break;
			}
		}
	}
	else
		m_pStream = pFStream;
}


END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------
