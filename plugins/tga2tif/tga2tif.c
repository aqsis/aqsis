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

//Copyright (c) 1996, 1997, 1998 Thomas E. Burge.  All rights reserved.
//
//Affine (R) is a registered trademark of Thomas E. Burge
//
//THIS SOFTWARE IS DISTRIBUTED "AS-IS" WITHOUT WARRANTY OF ANY KIND
//AND WITHOUT ANY GUARANTEE OF MERCHANTABILITY OR FITNESS FOR A
//PARTICULAR PURPOSE.
//
//In no event shall Thomas E. Burge be liable for any indirect or
//consequential damages or loss of data resulting from use or performance
//of this software.
//
//Permission is granted to include compiled versions of this code in
//noncommercially sold software provided the following copyrights and
//notices appear in all software and any related documentation:
//
//                The Affine (R) Libraries and Tools are
//         Copyright (c) 1995, 1996, 1997, 1998 Thomas E. Burge.
//                         All rights reserved.
//        Affine (R) is a registered trademark of Thomas E. Burge.


/** \file
		\brief Implements a tga importer; ONLY SUPPORT RLE PALETTE tga FILE
		\author Michel Joron (joron@sympatico.ca) I just ported the code from affine toolkit 
		to AQSIS... and I made sure the orientation of tga is used to justify the tiff result.
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
#include <string.h>
#include <malloc.h>

#include "../common/pixelsave.h"

#define	_qShareName	tga2tif
#include	"share.h"


/* structure for Targa file */
#define NO_ENCODING   0
#define ENCODING      1

typedef struct _TGAHEADER
{
	unsigned char idlength;
	unsigned char clrmaptype;
	unsigned char imagetype;
	unsigned char clrmapoffset1;
	unsigned char clrmapoffset2;
	unsigned char clrmaplen1;
	unsigned char clrmaplen2;
	unsigned char clrmapdepth;
	unsigned char xorigin1;
	unsigned char xorigin2;
	unsigned char yorigin1;
	unsigned char yorigin2;
	unsigned char width1;
	unsigned char width2;
	unsigned char height1;
	unsigned char height2;
	unsigned char pixeldepth;
	unsigned char origin_alphabits;
}
TGAHEADER;
typedef TGAHEADER *PTGAHEADER;

/* Masks to AND with origin_alphabits. */
#define kTGA_RIGHT_TO_LEFT  0x10
#define kTGA_TOP_TO_BOTTOM  0x20



static char *tga_open( FILE *tgafile, char *tiffname ) ;

static char tiffname[ 1024 ];

/* Main function to convert any tga to tif format
 * It used the standard standard definition of tga structure and
 * output RGB tif file for PC and Unix/
 */
_qShareM char *tga2tif( char *in )
{
	FILE * tgafile;
	char *result = NULL;


	strcpy( tiffname, in );
	if ( ( result = strstr( tiffname, ".tga" ) ) != 0 ) strcpy( result, ".tif" );
	if ( !result )
	{
		if ( ( result = strstr( tiffname, ".tga" ) ) != 0 ) strcpy( result, ".tif" );
	}
	if ( !result ) return result;

	tgafile = fopen( in, "rb" );
	result = tga_open( tgafile, tiffname );
	fclose( tgafile );


	return result;
}

/* PRIVATE FUNCTIONS */
/*
 * tga_open(): extract the colourmap of tga; compute the 
 *   pixels in RGB format and finally save to a tiff file
 */
static char *tga_open( FILE *tgafile, char *tiffname )
{
	char * result = NULL;
	unsigned char b[ 5 ];
	unsigned char *pBmp = NULL;
	TGAHEADER tgaheader;
	unsigned int xres, yres, nsamples;
	int bitsperpel;
	int rle;
	register char *p, *pp;
	register unsigned int i, j, n, nbytes;
	unsigned char *lines;
	int orientation = 0;




	/* Read TGA header. */
	if ( fread( ( char* ) & tgaheader, 1, sizeof( TGAHEADER ), tgafile )
	        < sizeof( TGAHEADER ) )
		goto Error;

	/* TGA uses little-endian. */
#ifdef WIN32
	xres = tgaheader.width2 << 8 | tgaheader.width1;
	yres = tgaheader.height2 << 8 | tgaheader.height1;
#else
	xres = tgaheader.width1 << 8 | tgaheader.width2;
	yres = tgaheader.height1 << 8 | tgaheader.height2;
#endif
	bitsperpel = tgaheader.pixeldepth;

	/* Allow only 16, 24 and 32 bits per pixel. */
	switch ( bitsperpel )
	{
			case 16:
			case 24:
			nsamples = 3;
			break;
			case 32:
			nsamples = 4;
			break;
			default:
			{
				fprintf( stderr, "tga2tif doesn't support %d bitperpel\n", bitsperpel );
				goto Error;
			}
	}

	/* Get the image type. */
	switch ( tgaheader.imagetype )
	{
			case 10:  /* RLE Truecolor */
			rle = ENCODING;
			break;
			case 2:  /* Truecolor */
			rle = NO_ENCODING;
			break;
			/* case 0: */  /* No image in file */
			/* case 1: */  /* Colormapped */
			/* case 3: */  /* Monochrome */
			/* case 9: */  /* RLE Colormapped */
			/* case 11: */ /* RLE Monchrome */
			default:
			goto Error;
	}

	orientation = 0; /*  bottom-left by default */

	if ( !( tgaheader.origin_alphabits & kTGA_TOP_TO_BOTTOM ) )
	{
		orientation = 1; /* top-left */
	}


	if ( tgaheader.origin_alphabits & kTGA_RIGHT_TO_LEFT )
	{
		/* If BITMAP_TOPLEFT, then BITMAP_TOPRIGHT.
		* If BITMAP_BOTLEFT, then BITMAP_BOTRIGHT.
		*/
		if ( orientation == 1 )  /* top-left */
			orientation = 3; /* top-right */
		else
			orientation = 2; /* bottom-right */
	}
	pBmp = malloc( xres * yres * nsamples );
	if ( !pBmp )
		goto Error;

	/* Get image ID string length. */
	/* This is a string located after the header. */
	nbytes = tgaheader.idlength;

	/* Color maps can be present even when the image doesn't need it. */
	if ( tgaheader.clrmaptype == 1 )
	{
		/* Get color map length. */
		nbytes += tgaheader.clrmaplen2 << 8 | tgaheader.clrmaplen1;
	}
	else if ( tgaheader.clrmaptype != 0 )
	{
		goto Error;
	}

	/* Skip over image ID string and possible color map. */
	if ( nbytes && fseek( tgafile, ( long ) nbytes, SEEK_CUR ) )
		goto Error;

	/* Read in image data. */
	p = ( char* ) ( pBmp );
	pp = p + xres * nsamples * yres;
	if ( rle )
	{
		if ( bitsperpel == 32 )
		{
			while ( p < pp )
			{
				if ( fread( b, 1, 5, tgafile ) < 5 )
					goto Error;
				i = b[ 0 ];
				n = i & 0x007f; /* n is now from 0 to 127. */
				if ( i & 0x80 )
				{
					n++;
					for ( i = 0;i < n; i++ )
					{
						*p++ = b[ 3 ]; /* R */
						*p++ = b[ 2 ]; /* G */
						*p++ = b[ 1 ]; /* B */
						*p++ = b[ 4 ]; /* A */
					}
				}
				else
				{
					*p++ = b[ 3 ]; /* R */
					*p++ = b[ 2 ]; /* G */
					*p++ = b[ 1 ]; /* B */
					*p++ = b[ 4 ]; /* A */
					for ( i = 0;i < n; i++ )
					{
						if ( fread( b, 1, 4, tgafile ) < 4 )
							goto Error;
						*p++ = b[ 2 ]; /* R */
						*p++ = b[ 1 ]; /* G */
						*p++ = b[ 0 ]; /* B */
						*p++ = b[ 3 ]; /* A */
					}
				}
			}
		}
		else if ( bitsperpel == 24 )
		{
			while ( p < pp )
			{
				if ( fread( b, 1, 4, tgafile ) < 4 )
					goto Error;
				i = b[ 0 ];
				n = i & 0x007f;
				if ( i & 0x0080 )
				{
					n++;
					for ( i = 0; i < n; i++ )
					{
						*p++ = b[ 3 ]; /* R */
						*p++ = b[ 2 ]; /* G */
						*p++ = b[ 1 ]; /* B */
					}
				}
				else
				{
					*p++ = b[ 3 ]; /* R */
					*p++ = b[ 2 ]; /* G */
					*p++ = b[ 1 ]; /* B */
					for ( i = 0; i < n; i++ )
					{
						if ( fread( b, 1, 3, tgafile ) < 3 )
							goto Error;
						*p++ = b[ 2 ]; /* R */
						*p++ = b[ 1 ]; /* G */
						*p++ = b[ 0 ]; /* B */
					}
				}
			}
		}
		else if ( bitsperpel == 16 )
		{
			while ( p < pp )
			{
				if ( fread( b, 1, 3, tgafile ) < 3 )
					goto Error;
				i = b[ 0 ];
				n = i & 0x7f;
				if ( i & 0x80 )
				{
					n++;
					b[ 0 ] = ( b[ 2 ] & 0x7c ) << 1;                     /* R */
					b[ 2 ] = ( ( b[ 2 ] & 0x03 ) << 6 ) | ( ( b[ 1 ] & 0xe0 ) >> 2 );  /* G */
					b[ 1 ] = ( b[ 1 ] & 0x1f ) << 3;                     /* B */
					for ( i = 0;i < n; i++ )
					{
						*p++ = b[ 0 ];  /* R */
						*p++ = b[ 2 ];  /* G */
						*p++ = b[ 1 ];  /* B */
					}
				}
				else
				{
					*p++ = ( b[ 2 ] & 0x7c ) << 1;                     /* R */
					*p++ = ( ( b[ 2 ] & 0x03 ) << 6 ) | ( ( b[ 1 ] & 0xe0 ) >> 2 );  /* G */
					*p++ = ( b[ 1 ] & 0x1f ) << 3;                     /* B */
					for ( i = 0;i < n; i++ )
					{
						if ( fread( b, 1, 2, tgafile ) < 2 )
							goto Error;
						*p++ = ( b[ 1 ] & 0x7c ) << 1;                     /* R */
						*p++ = ( ( b[ 1 ] & 0x03 ) << 6 ) | ( ( b[ 0 ] & 0xe0 ) >> 2 );  /* G */
						*p++ = ( b[ 0 ] & 0x1f ) << 3;                     /* B */
					}
				}
			}
		}
		else
		{
			goto Error;
		}
	}
	else /* not rle */
	{
		if ( bitsperpel == 32 )
		{
			while ( p < pp )
			{
				if ( fread( b, 1, 4, tgafile ) < 4 )
					goto Error;
				*p++ = b[ 2 ]; /* R */
				*p++ = b[ 1 ]; /* G */
				*p++ = b[ 0 ]; /* B */
				*p++ = b[ 3 ]; /* A */
			}
		}
		else if ( bitsperpel == 24 )
		{
			while ( p < pp )
			{
				if ( fread( b, 1, 3, tgafile ) < 3 )
					goto Error;
				*p++ = b[ 2 ]; /* R */
				*p++ = b[ 1 ]; /* G */
				*p++ = b[ 0 ]; /* B */
			}
		}
		else if ( bitsperpel == 16 )
		{
			/* For the 16 bit format each channel gets 5 bits.
			 *    A 5 bit value is turned into an 8 bit value
			 *    by shifting everything up by 3 bits.
			 *    The format for 16 bit is 
			 * (junk or alpha bit)(5 Red bits)(5 green bits)(5 blue bits).
			 */
			while ( p < pp )
			{
				if ( fread( b, 1, 2, tgafile ) < 2 )
					goto Error;
				*p++ = ( b[ 1 ] & 0x7c ) << 1;                     /* R */
				*p++ = ( ( b[ 1 ] & 0x03 ) << 6 ) | ( ( b[ 0 ] & 0xe0 ) >> 2 );  /* G */
				*p++ = ( b[ 0 ] & 0x1f ) << 3;                     /* B */
			}
		}
		else
		{
			goto Error;
		}
	}

	result = tiffname;


	/* reverse the direction of on Y axis */
	if ( orientation & 0x1 )
	{
		lines = malloc( xres * nsamples );
		if ( lines == 0 ) goto Error;

		for ( i = 0; i < yres / 2; i++ )
		{
			memcpy( lines, &pBmp[ i * xres * nsamples ], xres * nsamples );
			memcpy( &pBmp[ i * xres * nsamples ], &pBmp[ ( yres - i - 1 ) * xres * nsamples ], xres * nsamples );
			memcpy( &pBmp[ ( yres - i - 1 ) * xres * nsamples ], lines, xres * nsamples );
		}
		free( lines );
	}
	/* reverse the direction of on X axis */
	if ( orientation & 0x2 )
	{

		lines = malloc( nsamples );
		if ( lines == 0 ) goto Error;

		for ( j = 0; j < yres; j++ )
		{
			for ( i = 0; i < xres / 2; i++ )
			{
				memcpy( lines, &pBmp[ j * xres * nsamples + i * nsamples ], nsamples );
				memcpy( &pBmp[ j * xres * nsamples + i * nsamples ], &pBmp[ ( yres - j - 1 ) * xres * nsamples + i * nsamples ], nsamples );
				memcpy( &pBmp[ ( yres - j - 1 ) * xres * nsamples + i * nsamples ], lines, nsamples );
			}
		}
		free( lines );
	}

	save_tiff( tiffname, pBmp, xres, yres, nsamples, "tga2tif" );



Error:

	if ( pBmp )
		free( pBmp );





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
