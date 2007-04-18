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
		\brief Implements a pcx importer; ONLY SUPPORT RLE PALETTE PCX FILE
		\author Michel Joron (joron@sympatico.ca)
*/


// The conversion process assumes a minimal conversion
// the function to do convertion must be name
// as the name of the dll or .so on Unix
// The conversion must create a valid .tif file and return
// the filename of the new file created.
// Eg. this function pcx2tif() created a .tif file at the same place
// as the .jpg filename provided.
// Minimal implemantation is just doing a call or two to libjpeg provided with the
// standard libjpeg source codes.
// IN ORDER TO CONVERSION OCCURRED the dll/dso must be present in
// procedures directory under <AQSIS_BASE_PATH>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../common/pixelsave.h"

#ifdef	WIN32
#define	__export	__declspec(dllexport)
#else	// WIN32
#define	__export
#endif	// WIN32

/* structure for PaintBrush file */

/* this structure needs to be byte aligned! */
/* Program need a directive pack(1) on UNIX!! */
typedef struct
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
}
CMAP;


/* this structure needs to be byte aligned! */
typedef struct
{
	unsigned char manuf;                 /* don't ask */
	unsigned char ver;                   /* PCX format version */
	unsigned char rle_flag;              /* 1 = run-length encoded */
	unsigned char bits_per_pixel;        /* number of bits per pixel */
	unsigned short X1;                     /* upper LH pixel co-ord */
	unsigned short Y1;                     /* upper LH pixel co-ord */
	unsigned short X2;                     /* lower RH pixel co-ord */
	unsigned short Y2;                     /* lower RH pixel co-ord */
	unsigned short Hres;                   /* horizontal resolution of device */
	unsigned short Vres;                   /* vertical resolution of device */
	CMAP cmap_16[ 16 ];            /* first 16 entries of colour map - MUST be
		                                 * 48 bytes */
	unsigned char Vmode;                 /* ??? */
	unsigned char nplanes;               /* number of image planes */
	unsigned short bytes_per_line;         /* number of bytes per scan line */
	unsigned char filler[ 60 ];            /* pad out to 128 bytes */
}
PCX_HDR;


static void extract_pcx_colour_map( FILE * f, int size, unsigned char *r, unsigned char *g, unsigned char *b );
static void read_pcx_rle_line( FILE * f, unsigned char *s, int n );
static char *pcx_open( FILE *pcxfile, char *tiffname ) ;
static unsigned short swap2( unsigned short s );

static char tiffname[ 1024 ];

/* Main function to convert any pcx to tif format
 * It used the standard standard definition of pcx/paintbrush structure and
 * output RGB tif file for PC and Unix/
 */
__export char *pcx2tif( char *in )
{
	FILE * pcxfile;
	char *result = NULL;


	strcpy( tiffname, in );
	if ( ( result = strstr( tiffname, ".pcx" ) ) != 0 )
		strcpy( result, ".tif" );
	if ( !result )
	{
		if ( ( result = strstr( tiffname, ".pcx" ) ) != 0 )
			strcpy( result, ".tif" );
	}
	if ( !result )
		return result;

	pcxfile = fopen( in, "rb" );
	result = pcx_open( pcxfile, tiffname );
	fclose( pcxfile );


	return result;
}

/* PRIVATE FUNCTIONS */
/*
 * pcx_open(): extract the colourmap of PCX; compute the 
 *   pixels in RGB format and finally save to a tiff file
 */
static char *pcx_open( FILE *pcxfile, char *tiffname )
{
	PCX_HDR pcx_hdr;
	register int i, j;
	unsigned short h, w;
	unsigned char *s;
	unsigned char *r, *g, *b;
	int palette_size = 256;
	unsigned char * pixels;


	/*
	 * read the pbrush header
	*/
	if ( fread( &pcx_hdr, sizeof( pcx_hdr ), 1, pcxfile ) != 1 )
	{
		fprintf( stderr, "pcx2bmp: can't read PCX header\n" );
		return NULL;
	}
	/*
	 * should really validate header here to make sure it is 256 RLE colour
	 * mapped !
	 */
#ifndef WIN32
	/* swap the short on Unix */
	pcx_hdr.X1 = swap2( pcx_hdr.X1 );
	pcx_hdr.X2 = swap2( pcx_hdr.X2 );
	pcx_hdr.Y1 = swap2( pcx_hdr.Y1 );
	pcx_hdr.Y2 = swap2( pcx_hdr.Y2 );
#endif

	w = pcx_hdr.X2 - pcx_hdr.X1 + 1;
	h = pcx_hdr.Y2 - pcx_hdr.Y1 + 1;


#ifdef DEBUG

	printf( "%dx%d\n", w, h );
#endif

	if ( !( s = ( unsigned char * ) malloc( w ) ) )
	{
#ifdef DEBUG
		fprintf( stderr, "pcx2bmp: not enough memory for scanline!\n" );
#endif

		return NULL;
	}

	/* read the palette R,G, B values */
	r = ( unsigned char * ) calloc( palette_size, sizeof( unsigned char ) );
	b = ( unsigned char * ) calloc( palette_size, sizeof( unsigned char ) );
	g = ( unsigned char * ) calloc( palette_size, sizeof( unsigned char ) );

	extract_pcx_colour_map( pcxfile, palette_size, r, g, b );

	/*
	 * read image complete than now create the 256 colours/NOCOMPRESSION to
	* tiff file
	 */
	pixels = malloc( w * h * 3 );

	/* convert each scan line */
	for ( j = 0; j < h; j++ )
	{
		read_pcx_rle_line( pcxfile, s, w );

		for ( i = 0; i < w; i++ )
		{
			pixels[ ( j * w * 3 ) + i * 3 + 0 ] = r[ s[ i ] ]; /* red */
			pixels[ ( j * w * 3 ) + i * 3 + 1 ] = g[ s[ i ] ]; /* green */
			pixels[ ( j * w * 3 ) + i * 3 + 2 ] = b[ s[ i ] ]; /* blue */
		}
	}
	save_tiff( tiffname, pixels, w, h, 3, "pcx2tif" );

	free( pixels );
	free( r );
	free( g );
	free( b );
	return tiffname;
}

/*
 * extract_pc_colour_map(): extract the colourmap of PCX 
 */
static void
extract_pcx_colour_map(
    FILE * f,
    int size,
    unsigned char *r, unsigned char *g, unsigned char *b )
{
	long posn;
	int i;
	CMAP rgb;


	/* save current position in file */
	posn = ftell( f );

	/* position to presumed start of PCX colour map */
	fseek( f, -3L * size, 2 );


	/* read colour map */
	for ( i = 0; i < size; i++ )
	{
		size_t len_read = fread( &rgb, sizeof( rgb ), 1, f );
		if(len_read != 1)
			return;
		r[ i ] = rgb.r;
		g[ i ] = rgb.g;
		b[ i ] = rgb.b;
	}

	/* reposition to where we started */
	fseek( f, posn, 0 );

}


/*
 * read_pcx_rle_line() : extract the bits of PCX.
 */
static
void
read_pcx_rle_line( FILE * f, unsigned char *s, int n )
{
	int i, j, cnt;
	int c;


	for ( i = 0; i < n; )
	{

		if ( ( c = getc( f ) ) == EOF )
		{
			if ( feof( f ) )
				return ;
		}
		if ( ( c & 0xc0 ) == 0xc0 )
		{

			cnt = c & 0x3f;
			if ( ( c = getc( f ) ) == EOF )
			{
				if ( feof( f ) )
					return ;
			}
			if ( i + cnt > n )
				cnt = n - i;
			for ( j = 0; j < cnt; j++ )
				s[ i + j ] = c;
			i += cnt;
		}
		else
		{
			s[ i++ ] = c;
		}
	}
}

static unsigned short swap2( unsigned short s )
{
	unsigned short n;
	unsigned char *in, *out;

	out = ( unsigned char* ) & n;
	in = ( unsigned char* ) & s;

	out[ 0 ] = in[ 1 ];
	out[ 1 ] = in[ 0 ];
	return n;

}

#ifdef MAIN
int main( int argc, char *argv[] )
{
	char * result;

	if ( argc != 2 )
	{
		fprintf( stderr, "Usage %s: %s some.pcx", argv[ 0 ], argv[ 1 ] );
		exit( 2 );
	}
	result = pcx2tif( argv[ 1 ] );
	if ( result )
	{
		puts( result );
	}
	return 1;
}
#endif
