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
		\brief Implements the aqsltell utility equivalent to slctell in BMRT.
		\author Michel Joron (joron@sympatico.ca)
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void print( FILE *fp, char *tile, char *realname );

/*
 *
 */
main( int argc, char *argv[] )
{
	char name[ 1024 ];
	char envshader[ 1024 ];
	FILE *cshder;

	if ( argc != 2 )
	{
		fprintf( stderr, "Usage %s: %s plastic.slx\n", argv[ 0 ], argv[ 0 ] );
		fprintf( stderr, "Usage %s: %s paintedplastic\n", argv[ 0 ], argv[ 0 ] );
		exit( 2 );
	}
	if ( getenv( "AQSIS_BASE_PATH" ) != NULL )
	{
		strcpy( envshader, getenv( "AQSIS_BASE_PATH" ) );
#ifdef AQSIS_SYSTEM_WIN32
		strcat( envshader, "\\shaders" );
#else
		strcat( envshader, "/shaders" );
#endif

	}
	else
	{
		strcpy( envshader, "." );
	}

	if ( !strstr( argv[ 1 ], ".slx" ) )
	{
		/* I did not find the extension; I will load the compiled shader code from
		 * env. variable
		*/
#ifdef AQSIS_SYSTEM_WIN32
		sprintf( name, "%s\\%s.slx", envshader, argv[ 1 ] );
#else
		sprintf( name, "%s/%s.slx", envshader, argv[ 1 ] );
#endif

	}
	else
	{
		/* load from the current directory than */
		strcpy( name, argv[ 1 ] );
	}

	cshder = fopen( name, "rt" );

	if ( !cshder )
	{
		/* Could not found the compiled code than exit ungracefully */
		fprintf( stderr, "Cannot open %s\n", name );
		exit( 3 );
	}

	/* Parse and print the compiled code */
	print( cshder, argv[ 1 ], name );

	/* goodbye */
	fclose( cshder );
}

/*
 * Parse & Print info. about the compiled shader code
 */
static void print( FILE *fp, char *shader, char *realname )
{
	char kind[ 80 ];
	char buffer[ 80 ];
	int state = 0;
	int major, minor, patch;

	while ( fgets( buffer, 80, fp ) != NULL )
	{
		if ( state == 3 ) break;
		if ( state == 2 )
		{
			if ( strstr( buffer, "param uniform" ) )
			{
				char variable[ 80 ];
				char type[ 80 ];
				/* List the parameters for the shader */
				sscanf( buffer, "param uniform %s %s", type, variable );
				printf( "\t\"uniform %s %s\"\n", type, variable );
			}
			else if ( strstr( buffer, "varying" ) )
				state = 3;
			/* don't need to parse further down                     */
			/* We have already listed the important information now */
		}
		if ( state == 1 )
		{
			/* wait the parameters to come inline */
			state = 2;
		}
		if ( state == 0 )
		{
			if ( strstr( buffer, "AQSIS_V" ) )
			{
				char * pt;
				state = 1;
				/* parse the type/version */
				if ( strstr( kind, "surface" ) ) strcpy( kind, "Surface" );
				if ( strstr( kind, "lightsource" ) ) strcpy( kind, "LightSource" );
				if ( strstr( kind, "volume" ) ) strcpy( kind, "Volume" );
				if ( strstr( kind, "displacement" ) ) strcpy( kind, "Displacement" );
				if ( strstr( kind, "imager" ) ) strcpy( kind, "Imager" );
				sscanf( buffer, "AQSIS_V %d.%d.%d", &major, &minor, &patch );
				pt = strstr( shader, ".slx" );
				if ( pt ) * pt = '\0';
				printf( "%s \"%s\" #%s compiled with version %d.%d.%d\n", kind, shader, realname, major, minor, patch );
			}
		}
		if ( state == 0 )
			strcpy( kind, buffer );
	}
}
