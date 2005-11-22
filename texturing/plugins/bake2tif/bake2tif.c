// Aqsis
// Copyright ) 1997 - 2001, Paul C. Gregory
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
		\brief Implements a BAKE importer; 5 floats or 3 floats BAKE FILE
		\author Michel Joron (joron@sympatico.ca)
*/


// The conversion process assumes a minimal conversion
// the function to do convertion must be name
// as the name of the dll or .so on Unix
// The conversion must create a valid .tif file and return
// the filename of the new file created.
// Eg. this function bake2tif() created a .tif file at the same place
// as the .bake filename provided.
// IN ORDER TO CONVERSION OCCURRED the dll/dso must be present in
// procedures directory under <AQSIS_BASE_PATH>

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>

#include "../common/pixelsave.h"

#ifdef	WIN32
#define	__export	__declspec(dllexport)
#else	// WIN32
#define	__export
#endif	// WIN32

static char *bake_open( FILE *bakefile, char *tiffname );

static char tiffname[ 1024 ];

/* Main function to convert any bake to tif format
 * It used the standard standard definition of bake file defined in bake() from
 * Siggraph 2001.
 *
 * It created texture file of only 64x64 pixels not a lot 
 * but which some changes in teqser we should be able to 
 * defined as much as the user needs via environment variable BAKE 
 * Surprising 64x64 is good enough for small tests; I won't be surprised we
 * need more and agressive filtering (between subsamples s/t)
 */
#define SIZE 64
#define REMOVEDARK 1

static int size = SIZE;

__export char *bake2tif( char *in )
{
	FILE * bakefile;
	char *result = NULL;


	result = ( char * ) getenv( "BAKE" );

	if ( result && ( result[ 0 ] >= '0' ) && ( result[ 0 ] <= '9' ) )
		size = atoi( result );

	strcpy( tiffname, in );
	if ( ( result = strstr( tiffname, ".bake" ) ) != 0 ) strcpy( result, ".tif" );
	if ( !result )
	{
		if ( ( result = strstr( tiffname, ".bake" ) ) != 0 ) strcpy( result, ".tif" );
	}
	if ( !result ) return result;

	bakefile = fopen( in, "rb" );
	result = bake_open( bakefile, tiffname );
	fclose( bakefile );


	return result;
}

/* PRIVATE FUNCTIONS */
/*
 * linear interpretation between an sample at the left side and ride side of black pixel.
 */

static int lerp(int maxy, int miny, int maxx, int minx, int x)
{
int result = minx;
float ratio = 1.0;

    if (x == maxx) return maxy;
    if (x == minx) return miny;
	if (maxx == minx) return miny;
	else 
		ratio = 1.0f - (float) (maxx - x) / (float) (maxx - minx);
	result = miny + (int) (ratio * (maxy - miny));

	
    return result;
}

/*
 * bake_open(): extract the colourmap of BAKE; compute the 
 *   pixels in RGB format and finally save to a tiff file
 */
static char *bake_open( FILE *bakefile, char *tiffname )
{
	unsigned short h, w;
	float s, t, r1, g1, b1;
	unsigned char *pixels;
	unsigned char *xpixels;
	char buffer[ 200 ];
	int i, j, n, o;
	int x, y;
	int bytesize = size * size * 3;

	h = w = size;
	pixels = ( unsigned char * ) calloc( 3, size * size );

	while ( fgets( buffer, 200, bakefile ) != NULL )
	{

		if ( 5 == sscanf( buffer, "%f %f %f %f %f", &s, &t, &r1, &g1, &b1 ) );
		else
		{
			sscanf( buffer, "%f %f %f", &s, &t, &r1 );
			g1 = b1 = r1;
		}

		// When we have some collision ? What should be nice
		// to accumulate the RGB values instead ?
		x = (int)( s * ( size - 1 ) );
		y = (int)( t * ( size - 1 ) );
		n = ( y * size ) + x;
		n *= 3;

		pixels [ n ] = (int)(r1 * 255);
		pixels [ n + 1 ] = (int)(g1 * 255);
		pixels [ n + 2 ] = (int)(b1 * 255);

	}

	xpixels = ( unsigned char * ) calloc( 3, size * size );

	memcpy(xpixels, pixels, bytesize);


#ifdef REMOVEDARK 
   
	/* in X */
	
	for (i=0; i < size; i++)
	{
		for(j =0; j < size* 3; j+=3)
		{
			n = (i * size * 3) + j;
			if ( ( pixels[ n ] == pixels[ n + 1 ] ) &&
			( pixels[ n + 1 ] == pixels[ n + 2 ] ) &&
			( pixels[ n + 2 ] == 0 ) )
			{
				int m = n;
				while (m > 0) {
					if (!( ( pixels[ m ] == pixels[ m + 1 ] ) &&
					( pixels[ m + 1 ] == pixels[ m + 2 ] ) &&
					( pixels[ m + 2 ] == 0 ) ))
					break;
					m-=3;
				}
				o = n;
				while (o < bytesize-2) {
					if (!( ( pixels[ o ] == pixels[ o + 1 ] ) &&
					( pixels[ o + 1 ] == pixels[ o + 2 ] ) &&
					( pixels[ o + 2 ] == 0 ) ))
					break;
					o+=3;
				}
				if ((o < bytesize-2) && (m < bytesize-2) && (n < bytesize-2)) {
					xpixels[ n ] = lerp(pixels[o], pixels[m], o, m, n);
					xpixels[ n +1] = lerp(pixels[o+1], pixels[m+1], o, m, n);
					xpixels[ n +2] = lerp(pixels[o+2], pixels[m+2], o, m, n);
				}
			}
		}
	}

	memcpy(pixels, xpixels, 3 * size * size);

#endif
	// Should we do some filterings prior to save to tif file ?
	// I will used the general scheme of mipmap from texturemap.cpp for now

	/* convert each scan line */
	save_tiff( tiffname, pixels, w, h, 3, "bake2tif" );

	free( pixels );
	free( xpixels );

	return tiffname;
}

#ifdef MAIN
int main( int argc, char *argv[] )
{
	char * result;

	if ( argc != 2 )
	{
		fprintf( stderr, "Usage %s: %s some.bake", argv[ 0 ], argv[ 1 ] );
		exit( 2 );
	}
	result = bake2tif( argv[ 1 ] );
	if ( result )
	{
		puts( result );
	}
	return 1;
}
#endif
