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

	std::list<CqString*> result;
	if ( ( hFile = _findfirst( pt, &c_file ) ) != -1L )
	{
		/* we found something here; then we list
		* all of them with the directory first
		*/
		CqString strFile( c_file.name );
		result.push_front( &strFile );
		while ( _findnext( hFile, &c_file ) == 0 )
		{
			result.push_front( new CqString( c_file.name ) );
		}
		_findclose( hFile );
	}

	return result ;
}



//---------------------------------------------------------------------
/** Get the searchpath for a the specified asset type. The way in which the
 *  searchpath is specified is OS specific. Under Windows, environment variables
 *  are used.
 *
 *  \param strAsset The name of the asset whose searchpath is to be returned, i.e. "shaders".
 *  \return A string containing the ":" separated list of searchpaths.
 */

CqString CqFile::GetSystemSetting( const CqString& strAsset )
{
	char * env;
	char* base_path = ".";
	CqString result( "" );

	if ( ( env = getenv( "AQSIS_BASE_PATH" ) ) != NULL )
		base_path = env;


	if ( strAsset.compare( "base" ) == 0 )
	{
		result = base_path;
	}
	else if ( strAsset.compare( "config" ) == 0 )
	{
		if ( ( env = getenv( "AQSIS_CONFIG" ) ) != NULL )
			result = env;
		else
		{
			result = base_path;
			result.append( "/.aqsisrc" );
		}

		std::ifstream cfgfile( result.c_str() );
		if ( !cfgfile.is_open() )
			if ( ( env = getenv( "HOME" ) ) != NULL )
			{
				result = env;
				result.append( "/.aqsisrc" );
				std::ifstream cfgfile( result.c_str() );
				if ( !cfgfile.is_open() )
				{
					result = ".aqsisrc";
				}
			}
	}
	else if ( strAsset.compare( "shaders" ) == 0 )
	{
		if ( ( env = getenv( "AQSIS_SHADERS_PATH" ) ) != 0 )
			result = env;
		else
		{
			result = base_path;
			result.append( "/shaders" );
		}
	}
	else if ( strAsset.compare( "archives" ) == 0 )
	{
		if ( ( env = getenv( "AQSIS_ARCHIVES_PATH" ) ) != 0 )
			result = env;
		else
		{
			result = base_path;
			result.append( "/archives" );
		}
	}
	else if ( strAsset.compare( "textures" ) == 0 )
	{
		if ( ( env = getenv( "AQSIS_TEXTURES_PATH" ) ) != 0 )
			result = env;
		else
		{
			result = base_path;
			result.append( "/textures" );
		}
	}
	else if ( strAsset.compare( "displays" ) == 0 )
	{
		if ( ( env = getenv( "AQSIS_DISPLAYS_PATH" ) ) != 0 )
			result = env;
		else
		{
			result = base_path;
			result.append( "/displays" );
		}
	}
	else if ( strAsset.compare( "dsolibs" ) == 0 )
	{
		if ( ( env = getenv( "AQSIS_DSO_LIBS" ) ) != 0 )
			result = env;
		else
		{
			result = base_path;
			result.append( "/dso" );
		}
	}

	return ( result );
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
