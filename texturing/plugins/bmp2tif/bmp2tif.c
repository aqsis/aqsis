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
		\brief Implements a bmp importer
		\author Michel Joron (joron@sympatico.ca)
*/


// The conversion process assumes a minimal conversion
// the function to do convertion must be name
// as the name of the dll or .so on Unix
// The conversion must create a valid .tif file and return
// the filename of the new file created.
// Eg. this function bmp2tif() created a .tif file at the same place
// as the .bmp filename provided.
// Minimal implemantation is just doing a call or two to libjpeg provided with the
// standard libjpeg source codes.
// IN ORDER TO CONVERSION OCCURRED the dll/dso must be present in
// procedures directory under <AQSIS_BASE_PATH>

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

#include "../common/pixelsave.h"

#ifdef	WIN32
#define	__export	__declspec(dllexport)
#else	// WIN32
#define	__export
#include <unistd.h>
#endif	// WIN32

static char tiffname[ 1024 ];



/* Main function to convert any bmp to tif format
 * It used the standard tiff tool name bmp2tiff.exe on PC
 * or bmp2tiff on unix
 */
extern char *jpg2tif( char *in);
__export char *bmp2tif( char *in )
{
	char *result = NULL;
	char jpgname[1024];
	char call[1024];


	strcpy( tiffname, in );
	strcpy( jpgname, tiffname);
	if ( ( result = strstr( jpgname, ".bmp" ) ) != 0 )
		strcpy( result, ".jpg" );
	if ( !result )
	{
		if ( ( result = strstr( jpgname, ".BMP" ) ) != 0 )
			strcpy( result, ".jpg" );
	}
	sprintf( call, "cjpeg %s > %s", tiffname, jpgname );
	if(system(call))
		return("");
	result = jpg2tif(jpgname);
	unlink(jpgname);
	return result;
}

