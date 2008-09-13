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
		\brief Implements a BAKE importer; the format is quite simple; 
                   it use a tile like "Aqsis bake file"
                   followed by    "3" or "5" to indicate how many channels
                   followed by     5 floats or 3 floats 
		   all the information is written in ASCII
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
#include <math.h>

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
#define FILTER_TBL_SIZE 16
#define FILTER_WIDTH 8.0f
static float filter_width = FILTER_WIDTH;
static int filter_size = FILTER_TBL_SIZE;
static int size = SIZE;

#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

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

/*
	result = ( char * ) getenv( "BAKE_WIDTH" );

	if ( result && ( result[ 0 ] >= '0' ) && ( result[ 0 ] <= '9' ) )
		filter_width = atoi( result );

	result = ( char * ) getenv( "BAKE_SIZE" );

	if ( result && ( result[ 0 ] >= '0' ) && ( result[ 0 ] <= '9' ) )
		filter_size = atoi( result );
*/

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

static float disk( float x, float y, float xwidth, float ywidth )
{
	double d, xx, yy;

	xx = x * x;
	yy = y * y;
	xwidth *= 0.5;
	ywidth *= 0.5;

	d = ( xx ) / ( xwidth * xwidth ) + ( yy ) / ( ywidth * ywidth );
	if ( d < 1.0 )
	{
		return 1.0;
	}
	else
	{
		return 0.0;
	}
}

/*
 * bake_open(): extract the colourmap of BAKE; compute the
 *   pixels in RGB format and finally save to a tiff file
 */
static char *bake_open( FILE *bakefile, char *tiffname )
{
	unsigned short h, w;
	float s, t, r1, g1, b1;
	float *pixels;
	float *xpixels;
	float *filter, *pf;
	char buffer[ 200 ];
	int i, k, n;
	int x, y;
	float mins, mint, maxs, maxt;
	int normalized = 1;
	int count, number;
	float *temporary;
	int elmsize = 3;
	float invWidth, invSize;

	count = 1024 * 1024;
	number = 0;


	h = w = size;
	pixels = ( float * ) calloc( 4, size * size * sizeof(float));
	temporary = (float *) malloc(count *  5 * sizeof(float));

	/* printf( "Parse the bake file \n"); */
	/* Ignore the first line of text eg. Aqsis bake file */
	fgets( buffer, 200, bakefile ) ;
	/* Read the second line of text to configure how many floats 
         * we expect to read
         */
	fgets( buffer, 200, bakefile ) ;
	sscanf(buffer, "%d", &elmsize);

	while ( fgets( buffer, 200, bakefile ) != NULL )
	{
		k = number * 5;

		switch (elmsize)
		{
			case 3:
				{
					sscanf( buffer, "%f %f %f %f %f", &s, &t, &r1, &g1, &b1 );
				}
				break;
			case 2:
				{
					sscanf( buffer, "%f %f %f %f", &s, &t, &r1, &g1);
					b1 = (r1 + g1) / 2.0;
				}
				break;
			default:
			case 1:
				{
					sscanf( buffer, "%f %f %f", &s, &t, &r1);
					g1 = b1 = r1;
				}
				break;
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
	{
		normalized = 0;
	}

	if (normalized == 1)
	{
		printf("bake2tif normalizes the keys (normally s,t)\n");
		printf("\t(min_s, max_s): (%f, %f)\n\t(min_t, max_t): (%f, %f)\n", mins, maxs, mint, maxt );
	}

	/* Try to adjust with the final resolution of mipmap */
	/* filter_size = MAX((int) log((double)size)/log(2.0) + 2, (int) FILTER_TBL_SIZE); 
	 * filter_size = (int) ceil(log((double)MIN((int) ( (maxs - mins) * size), (int) ((maxt -mint ) * size)))/log(2.0));
         */
	invWidth = 1.0f/filter_width;
	invSize = 1.0f/filter_size;

	/* init the filter' table */
	filter = (float *) calloc(filter_size*filter_size,sizeof(float));
	pf = filter;
	for (y=0; y < filter_size; ++y)
	{
		float fy = (float)( y+ 0.5f) * filter_width * invSize;
		for (x=0; x < filter_size; ++x)
		{
			float fx = ((float) x+ 0.5f) * filter_width * invSize;
			/* we will use a disk filter the point will dispose in
                         * a circle around each point; more pleasant visually
                         */
			*pf++ = disk(fx,fy,filter_width, filter_width);
		}
	}


	/* Now it is time to save s,t, r1, g1, b1 into pixels array (along with the sum/area filtering accumalor  */
	for (i=0; i < number; i++)
	{
		int x0, x1, y0, y1;
		int *ifx, *ify;
		float dImageX;
		float dImageY;

		k = i * 5;


		s = temporary[k];
		t = temporary[k+1];
		r1 = temporary[k+2];
		g1 = temporary[k+3];
		b1 = temporary[k+4];

		/* printf("%d\n", number);  */

		/* Normalize the s,t between 0..1.0  only if required
		               */
		if (normalized)
		{
			if ( (maxs - mins) != 0.0)
			{
				s = (s - mins) / (maxs - mins);
			}
			else
			{
				if (s < 0.0) s *= -1.0;
				if (s > 1.0) s = 1.0;
			}
			if ((maxt - mint) != 0.0)
			{
				t = (t - mint) / (maxt - mint);
			}
			else
			{
				if (t < 0.0) t *= -1.0;
				if (t > 1.0) t = 1.0;
			}
		}
		/* When we have some collision ? What should be nice
		 * to accumulate the RGB values instead ?
		 */
		x = (int)( s * ( size - 1 ) );
		y = (int)( t * ( size - 1 ) );

		/* printf("x %d y %d rgb %f %f %f\n", x, y, r1, g1, b1); */

		/* each each pixels accumulated it in xpixels but
		* make sure we use
		  	 * a filtering of 16x16 to garantee spreading of the 
		* values across x,y pixels.
		 */

		dImageX = x - 0.5f;
		dImageY = y - 0.5f;

		x0 = (int) ceil(dImageX - filter_width);
		x1 = (int) floor(dImageX + filter_width);
		y0 = (int) ceil(dImageY - filter_width);
		y1 = (int) floor(dImageY + filter_width);

		x0 = MAX(x0, 0);
		x1 = MIN(x1, size -1);
		y0 = MAX(y0, 0);
		y1 = MIN(y1, size -1);

		if ( ( (x1-x0)<0) || ((y1-y0)<0 )) continue;

		/* filter delta indexes*/
		ifx = (int*)calloc(x1-x0+1, sizeof(int));
		for (x = x0; x <= x1; ++x)
		{
			float fx = fabsf(x -dImageX) * invWidth * filter_size;
			ifx[x-x0] = MIN((int) floor(fx), filter_size - 1);
		}
		ify = (int*)calloc(y1-y0+1, sizeof(int));
		for (y = y0; y <= y1; ++y)
		{
			float fy = fabsf(y -dImageY) * invWidth * filter_size;
			ify[y-y0] = MIN((int) floor(fy), filter_size - 1);
		}

		/* Fill all the right pixels now */
		for (y = y0; y < y1; ++y)
		{
			for (x = x0; x <= x1; ++x)
			{
				int offset;
				float filterWt;
				offset = ify[y-y0]*filter_size + ifx[x-x0];
				/* printf("offset %d ", offset); */
				filterWt = filter[offset];

				/* Remove the negative lob maybe ? 
				 * if (filterWt < 0.0) continue;
				 */

				/* printf("wt %f\n", filterWt); */
				n = (y * size + x);
				n *= 4;
				pixels[n] += (filterWt * r1);
				pixels[n+1] += (filterWt * g1);
				pixels[n+2] += (filterWt * b1);
				pixels[n+3] += filterWt;
				/* printf("x %d y %d rgb wt %f %f %f %f\n", x, y, pixels[n], pixels[n+1], pixels[n+2], pixels[n+3]); */
			}
		}


		free(ifx);
		free(ify);
	}


	/* Now it is time to unroll the filterWt and save into xpixels */

	xpixels = ( float * ) calloc( 3, size * size * sizeof(float));

	for (y=0; y < size -1 ; ++y)
	{
		for (x=0; x < size -1 ; ++x)
		{
			int m;
			n = (y * size + x);
			m = n;
			m *= 3;
			n *= 4;
			if (pixels[n+3] > 0.0)
			{
				/* printf("unroll weightSum factor \n"); */
				float area = 1.0/pixels[n+3];
				xpixels[m] =  pixels[n] * area;
				xpixels[m+1] =  pixels[n+1] * area;
				xpixels[m+2] =  pixels[n+2] * area;
			}
		}
	}


	/* Should we do some filterings prior to save to tif file ? */
	/* Used the general scheme of mipmap from texturemap.cpp for now */
	/* but for tex but when "bake2tif" is used floating value will be */
	/* saved instead of RGB/bytes encoding */

	/* convert each scan line */
	save_tiff( tiffname, (char *) xpixels, w, h, 3, "bake2tif" );

	free( pixels );
	free( xpixels );
	free( filter );
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
