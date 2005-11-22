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
		\brief Implements a tga importer; use cjpeg to convert the tga to jpg than jpg2tif is called with the new created file
		\author Michel Joron (joron@sympatico.ca) 
*/


// The conversion process assumes a minimal conversion
// the function to do convertion must be name
// as the name of the dll or .so on Unix
// The conversion must create a valid .tif file and return
// the filename of the new file created.
// Eg. this function tga2tif() created a .tif file at the same place
// as the .tga filename provided.
// IN ORDER TO CONVERSION OCCURRED the dll/dso must be present in
// procedures directory under <AQSIS_BASE_PATH>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/pixelsave.h"

#ifdef	WIN32
#define	__export	__declspec(dllexport)
#else	// WIN32
#define	__export
#endif	// WIN32

static char tiffname[ 1024 ];

/* Main function to convert any tga to tif format
 * It used the standard standard definition of tga structure and
 * output RGB tif file for PC and Unix/
 */
__export char *tga2tif( char *in )
{
int errcode;
char cmd[1024];
char *result = NULL;


	strcpy( tiffname, in );
	if ( ( result = strstr( tiffname, ".TGA" ) ) != 0 ) strcpy( result, ".jpg" );
	if ( !result )
	{
		if ( ( result = strstr( tiffname, ".tga" ) ) != 0 ) strcpy( result, ".jpg" );
	}
	if ( !result ) return result;

#ifdef AQSIS_SYSTEM_WIN32
	sprintf(cmd, "cjpeg.exe -targa %s > %s", in, tiffname);
#else
	sprintf(cmd, "cjpeg -targa %s > %s", in, tiffname);
#endif

	errcode = system(cmd);

	if (errcode == 0) 
	{
		/* SUCCESS */
		extern char *jpg2tif(char *name);
		
        char *jpgresult = jpg2tif(tiffname);

		/* delete the temporary .ppm file */
		unlink(tiffname);
		if (jpgresult) 
		{
			strcpy(tiffname, jpgresult);
			result = tiffname;
		} else result = NULL;
	} else {
		perror(cmd);
		result = NULL; 
	}


	return result;
}


#ifdef MAIN
int main( int argc, char *argv[] )
{
	char * result;

	if ( argc != 2 )
	{
		fprintf( stderr, "Usage %s: %s some.tga", argv[ 0 ], argv[ 1 ] );
		exit( 2 );
	}
	result = tga2tif( argv[ 1 ] );
	if ( result )
	{
		puts( result );
	}
	return 1;
}
#endif
