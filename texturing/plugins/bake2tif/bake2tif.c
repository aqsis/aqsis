// Aqsis
// Copyright ) 1997 - 2001, Paul C. Gregory
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

/*
 * reversed strstr()
 */
static char *strrstr(char *s1, char *sub)
{
	int s, t;
	char *pt = NULL;
	
	s = t = 0;
	if (s1)
		s = strlen(s1);
	if (sub)
		t = strlen(sub);
	
  	if ((pt = strstr(&s1[s - t], sub)) != NULL)
		return pt;

  	return pt;
}

__export char *bake2tif( char *in )
{
	FILE * bakefile;
	char *result = NULL;


	result = ( char * ) getenv( "BAKE" );

	if ( result && ( result[ 0 ] >= '0' ) && ( result[ 0 ] <= '9' ) )
		size = atoi( result );

	strcpy( tiffname, in );
	if ( ( result = strrstr( tiffname, ".bake" ) ) != 0 )
		strcpy( result, ".tif" );
	if ( !result )
	{
		if ( ( result = strrstr( tiffname, ".bake" ) ) != 0 )
			strcpy( result, ".tif" );
	}
	if ( !result )
		return result;

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

	if (x == maxx)
		return maxy;
	if (x == minx)
		return miny;
	if (maxx == minx)
		return miny;
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
	int i, j, k, n, o;
	int x, y;
	float mins, mint, maxs, maxt;
	int normalized = 1;
	int bytesize = size * size * 3;
	int count, number;
	float *temporary;

	count = 1024 * 1024;
	number = 0;

	h = w = size;
	pixels = ( unsigned char * ) calloc( 3, size * size );

        temporary = (float *) malloc(count *  5 * sizeof(float));

	/* printf( "Parse the bake file \n"); */
	while ( fgets( buffer, 200, bakefile ) != NULL )
	{
	 	k = number * 5;

		if ( 5 == sscanf( buffer, "%f %f %f %f %f", &s, &t, &r1, &g1, &b1 ) )
			;
		else
		{
			sscanf( buffer, "%f %f %f", &s, &t, &r1 );
			g1 = b1 = r1;
		}
		temporary[k] = s;
		temporary[k+1] = t;
		temporary[k+2] = r1;
		temporary[k+3] = g1;
		temporary[k+4] = b1;

		number++;
		if (number >= (count - 1))
		{
			count += 1024;
			temporary = (float *)  realloc((void *) temporary, count * 5 * sizeof(float));
		}
		/* printf("%d\n", number); */
	}

	/* printf("done\nFind the max, min of s,t.\n"); */
	mins = maxs = temporary[0]; 
	mint = maxt = temporary[1];

        /* Find the min,max of s and t */
	for (i=0; i < number; i++)
	{
		k = i * 5;

		if (mins > temporary[k])
			mins = temporary[k];
		if (mint > temporary[k+1])
			mint = temporary[k+1];
		if (maxs < temporary[k])
			maxs = temporary[k];
		if (maxt < temporary[k+1])
			maxt = temporary[k+1];
	}


	if ((mins >= 0.0 && maxs <= 1.0) &&
	    (maxt >= 0.0 && maxt <= 1.0) )
		normalized = 0;

	if (normalized == 1)
	{
		printf("bake2tif normalizes the keys (normally s,t)\n");
		printf("\t(min_s, max_s): (%f, %f)\n\t(min_t, max_t): (%f, %f)\n", mins, maxs, mint, maxt );
	}

	/* Now save the s,t */
	for (i=0; i < number; i++)
	{
		k = i * 5;

		s = temporary[k];
		t = temporary[k+1];
		r1 = temporary[k+2];
		g1 = temporary[k+3];
		b1 = temporary[k+4];

		/* Normalize the s,t between 0..1.0  only if required
                 */
		if (normalized)
		{
			if ( (maxs - mins) != 0.0)
			{
				s = (s - mins) / (maxs - mins);
			}
			else {
				if (s < 0.0) s *= -1.0;
				if (s > 1.0) s = 1.0;
			}
			if ((maxt - mint) != 0.0)
			{
				t = (t - mint) / (maxt - mint);
			}
			else {
				if (t < 0.0) t *= -1.0;
				if (t > 1.0) t = 1.0;
			}
		}
		/* When we have some collision ? What should be nice
		 * to accumulate the RGB values instead ?
		 */
		x = (int)( s * ( size - 1 ) );
		y = (int)( t * ( size - 1 ) );
		n = ( y * size ) + x;
		n *= 3;

		pixels [ n ] = (int)(r1 * 255);
		pixels [ n + 1 ] = (int)(g1 * 255);
		pixels [ n + 2 ] = (int)(b1 * 255);

	}
	/* printf("convert to rgb\n"); */

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
				while (m > 0)
				{
					if (!( ( pixels[ m ] == pixels[ m + 1 ] ) &&
					        ( pixels[ m + 1 ] == pixels[ m + 2 ] ) &&
					        ( pixels[ m + 2 ] == 0 ) ))
						break;
					m-=3;
				}
				o = n;
				while (o < bytesize-2)
				{
					if (!( ( pixels[ o ] == pixels[ o + 1 ] ) &&
					        ( pixels[ o + 1 ] == pixels[ o + 2 ] ) &&
					        ( pixels[ o + 2 ] == 0 ) ))
						break;
					o+=3;
				}
				if ((o < bytesize-2) && (m < bytesize-2) && (n < bytesize-2))
				{
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
	free(temporary);

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
