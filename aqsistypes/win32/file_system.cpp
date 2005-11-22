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
		\brief Implements the system specific parts of the CqFile class for handling files.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<fstream>
#include	<list>

#include	<string.h>
#include	<ctype.h>
#include	<io.h>
#include	<direct.h>

#include	"aqsis.h"

#include	"file.h"

START_NAMESPACE( Aqsis )

//---------------------------------------------------------------------
/** Given a string representing a filename with wildcards, return a list
 * of filenames that match that string.
*/
std::list<CqString*> CqFile::Glob ( const CqString& strFileGlob )
{
    _finddata_t c_file;
    long hFile;
    const char *pt = strFileGlob.c_str();

    char drive[_MAX_PATH];
    char dir[_MAX_PATH];
    char fname[_MAX_PATH];
    char ext[_MAX_PATH];

    _splitpath( pt, drive, dir, fname, ext);

    CqString strPath(drive);
    strPath += CqString(dir);

    std::list<CqString*> result;
    if ( ( hFile = _findfirst( pt, &c_file ) ) != -1L )
    {
        /* we found something here; then we list
        * all of them with the directory first
        */
	CqString* strFound = new CqString(strPath + CqString(c_file.name));
        result.push_front( strFound );
        while ( _findnext( hFile, &c_file ) == 0 )
        {
	    CqString* strFound = new CqString(strPath + CqString(c_file.name));
	    result.push_front( strFound );
            result.push_front( strFound );
        }
        _findclose( hFile );
    }

    return result ;
}


CqString CqFile::FixupPath(CqString& strPath)
{
    if( strPath.find("//",0,2) == 0 )
    {
        if(strPath[3] == '/')
        {
            CqString strNewPath(strPath.substr(2,1));
            strNewPath.append(":/");
            strNewPath.append(strPath.substr(4));
            //std::cout << strNewPath.c_str() << std::endl;
            return(strNewPath);
        }
    }

    return( strPath );
}


END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------
