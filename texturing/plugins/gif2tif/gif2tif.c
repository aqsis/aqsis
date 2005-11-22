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
		\brief Implements a gif importer
		\author Michel Joron (joron@sympatico.ca)
*/


// The conversion process assumes a minimal conversion
// the function to do convertion must be name
// as the name of the dll or .so on Unix
// The conversion must create a valid .tif file and return
// the filename of the new file created.
// Eg. this function gif2tif() created a .tif file at the same place
// as the .gif filename provided.
// Minimal implemantation is just doing a call to gif2tif.exe provided with the
// standard libtiff source codes.
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

#define MAXCOLORMAPSIZE  256

#define CM_RED     0
#define CM_GREEN   1
#define CM_BLUE    2

#define MAX_LWZ_BITS  12

#define INTERLACE      0x40
#define LOCALCOLORMAP  0x80
#define BitSet(byte, bit) (((byte) & (bit)) == (bit))

#define ReadOK(file,buffer,len) (fread(buffer, len, 1, file) != 0)

#define LM_to_uint(a,b)  (((b)<<8)|(a))

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
static struct
{
	unsigned int Width;
	unsigned int Height;
	unsigned char ColorMap[ 3 ][ MAXCOLORMAPSIZE ];
	unsigned int BitPixel;
	unsigned int ColorResolution;
	unsigned int Background;
	unsigned int AspectRatio;
	/*
	**
	*/
	int GrayScale;
}
GifScreen;

static struct
{
	int transparent;
	int delayTime;
	int inputFlag;
	int disposal;
}
Gif89 = { -1, -1, -1, 0 };

static char tiffname[ 1024 ];
static char *gif_open( FILE * infile, char *filename );
static char *ReadGIF( FILE* fd, char* );
static int ReadColorMap( FILE* fd, int number,
                         unsigned char buffer[ 3 ][ MAXCOLORMAPSIZE ], int *flag );
static int DoExtension( FILE* fd, int label );
static int GetDataBlock( FILE* fd, unsigned char *buf );
static int GetCode( FILE* fd, int code_size, int flag );
static int LWZReadByte( FILE *fd, int flag, int input_code_size );
static char *ReadImage( FILE *fd, int len, int height,
                        unsigned char cmap[ 3 ][ MAXCOLORMAPSIZE ],
                        int gray, int interlace, int ignore, char* );
static void throw_exception( char *error );
static unsigned short swap2( unsigned short s );

/* Main function to convert any gif to tif format
 * It used the standard tiff tool name gif2tiff.exe on PC
 * or gif2tiff on unix
 */
__export char *gif2tif( char *in )
{
	FILE * giffile;
	char *result = NULL;


	strcpy( tiffname, in );
	if ( ( result = strstr( tiffname, ".gif" ) ) != 0 ) strcpy( result, ".tif" );
	if ( !result )
	{
		if ( ( result = strstr( tiffname, ".gif" ) ) != 0 ) strcpy( result, ".tif" );
	}
	if ( !result ) return result;

	giffile = fopen( in, "rb" );
	result = gif_open( giffile, tiffname );
	fclose( giffile );


	return result;

}



/*
 * read the gif, convert to RGB pixels and save to tiff file
 */
static char *
gif_open( FILE* fd, char* filename )
{
	unsigned char buf[ 16 ];
	unsigned char c;
	unsigned char localColorMap[ 3 ][ MAXCOLORMAPSIZE ];
	int grayScale;
	int useGlobalColormap;
	int bitPixel;
	int imageCount = 0;
	char version[ 4 ];
	char buff[ 80 ];
	char *r = NULL;

	if ( ! ReadOK( fd, buf, 6 ) )
	{
		throw_exception( "error reading magic number" );
		return NULL;
	}

	if ( strncmp( ( char * ) buf, "GIF", 3 ) != 0 )
	{
		throw_exception( "not a GIF file" );
		return NULL;
	}

	strncpy( version, ( char * ) buf + 3, 3 );
	version[ 3 ] = '\0';

	if ( ( strcmp( version, "87a" ) != 0 ) && ( strcmp( version, "89a" ) != 0 ) )
	{
		throw_exception( "bad version number, not '87a' or '89a'" );
		return NULL;
	}

	if ( ! ReadOK( fd, buf, 7 ) )
	{
		throw_exception( "failed to read screen descriptor" );
		return NULL;
	}

	GifScreen.Width = LM_to_uint( buf[ 0 ], buf[ 1 ] );
	GifScreen.Height = LM_to_uint( buf[ 2 ], buf[ 3 ] );
	GifScreen.BitPixel = 2 << ( buf[ 4 ] & 0x07 );
	GifScreen.ColorResolution = ( ( ( buf[ 4 ] & 0x70 ) >> 3 ) + 1 );
	GifScreen.Background = buf[ 5 ];
	GifScreen.AspectRatio = buf[ 6 ];

	if ( BitSet( buf[ 4 ], LOCALCOLORMAP ) )
	{    /* Global Colormap */
		if ( ReadColorMap( fd, GifScreen.BitPixel, GifScreen.ColorMap,
		                   &GifScreen.GrayScale ) )
			throw_exception( "error reading global colormap" );
	}

	for ( ;; )
	{
		if ( ! ReadOK( fd, &c, 1 ) )
		{
			throw_exception( "EOF / read error on image data" );
			return NULL;
		}

		if ( c == ';' )
		{         /* GIF terminator */
			if ( imageCount != 1 )
			{
				sprintf( buff, "%d image%s found in file, this importer supports only one image per GIF",
				         imageCount, imageCount > 1 ? "s" : "" );
				throw_exception( buff );
				return NULL;
			}
		}

		if ( c == '!' )
		{         /* Extension */
			if ( ! ReadOK( fd, &c, 1 ) )
				throw_exception( "OF / read error on extention function code" );
			DoExtension( fd, c );
			continue;
		}

		if ( c != ',' )
		{         /* Not a valid start character */
			sprintf( buff, "bogus character 0x%02x, ignoring", ( int ) c );
			throw_exception( buff );
			continue;
		}

		++imageCount;

		if ( ! ReadOK( fd, buf, 9 ) )
		{
			throw_exception( "couldn't read left/top/width/height" );
			return NULL;
		}

		useGlobalColormap = ! BitSet( buf[ 8 ], LOCALCOLORMAP );

		bitPixel = 1 << ( ( buf[ 8 ] & 0x07 ) + 1 );

		if ( ! useGlobalColormap )
		{
			if ( ReadColorMap( fd, bitPixel, localColorMap, &grayScale ) )
			{
				throw_exception( "error reading local colormap" );
				return NULL;
			}
			r = ReadImage( fd, LM_to_uint( buf[ 4 ], buf[ 5 ] ),
			               LM_to_uint( buf[ 6 ], buf[ 7 ] ),
			               localColorMap, grayScale,
			               BitSet( buf[ 8 ], INTERLACE ), imageCount != 1,
			               filename );
			return r;
		}
		else
		{
			r = ReadImage( fd, LM_to_uint( buf[ 4 ], buf[ 5 ] ),
			               LM_to_uint( buf[ 6 ], buf[ 7 ] ),
			               GifScreen.ColorMap, GifScreen.GrayScale,
			               BitSet( buf[ 8 ], INTERLACE ), imageCount != 1,
			               filename );
			return r;
		}

	}
	return r;
}

/*
 * ReadColorMap(): Read the colormapped with a GIF file  
 */

static int
ReadColorMap(
    FILE* fd,
    int number,
    unsigned char buffer[ 3 ][ MAXCOLORMAPSIZE ],
    int* pbm_format )
{
	int i;
	unsigned char rgb[ 3 ];
	int flag;

	flag = TRUE;

	for ( i = 0; i < number; ++i )
	{
		if ( ! ReadOK( fd, rgb, sizeof( rgb ) ) )
		{
			throw_exception( "bad colormap" );
		}

		buffer[ CM_RED ][ i ] = rgb[ 0 ] ;
		buffer[ CM_GREEN ][ i ] = rgb[ 1 ] ;
		buffer[ CM_BLUE ][ i ] = rgb[ 2 ] ;

		flag &= ( rgb[ 0 ] == rgb[ 1 ] && rgb[ 1 ] == rgb[ 2 ] );
	}

	return FALSE;
}

/*
 * DoExtension(): pass over the extension within a GIF    
 */
static int
DoExtension( FILE* fd, int label )
{
	/* It would be nice to preserve comments and things... if there was      */
	/* an associated naming context to dump this into in it could tag along. */

	static char buf[ 256 ];
	char *str;

	switch ( label )
	{
			case 0x01:               /* Plain Text Extension */
			str = "Plain Text Extension";
#ifdef DEAL_WITH_THE_COMMENTS
			if ( GetDataBlock( fd, ( unsigned char* ) buf ) == 0 )
				;

			lpos = LM_to_uint( buf[ 0 ], buf[ 1 ] );
			tpos = LM_to_uint( buf[ 2 ], buf[ 3 ] );
			width = LM_to_uint( buf[ 4 ], buf[ 5 ] );
			height = LM_to_uint( buf[ 6 ], buf[ 7 ] );
			cellw = buf[ 8 ];
			cellh = buf[ 9 ];
			foreground = buf[ 10 ];
			background = buf[ 11 ];

			while ( GetDataBlock( fd, ( unsigned char* ) buf ) != 0 )
			{
				PPM_ASSIGN( image[ ypos ][ xpos ],
				            cmap[ CM_RED ][ v ],
				            cmap[ CM_GREEN ][ v ],
				            cmap[ CM_BLUE ][ v ] );
				++index;
			}

			return FALSE;
#else
			break;
#endif
			case 0xff:               /* Application Extension */
			str = "Application Extension";
			break;
			case 0xfe:               /* Comment Extension */
			str = "Comment Extension";
			while ( GetDataBlock( fd, ( unsigned char* ) buf ) != 0 )
			{
				/* if (showComment)
				 *    pm_message("gif comment: %s", buf );
				*/
			}
			return FALSE;
			case 0xf9:               /* Graphic Control Extension */
			str = "Graphic Control Extension";
			( void ) GetDataBlock( fd, ( unsigned char* ) buf );
			Gif89.disposal = ( buf[ 0 ] >> 2 ) & 0x7;
			Gif89.inputFlag = ( buf[ 0 ] >> 1 ) & 0x1;
			Gif89.delayTime = LM_to_uint( buf[ 1 ], buf[ 2 ] );
			if ( ( buf[ 0 ] & 0x1 ) != 0 )
			{
				Gif89.transparent = buf[ 3 ];
			}
			else
			{
				Gif89.transparent = -1;
			}

			while ( GetDataBlock( fd, ( unsigned char* ) buf ) != 0 )
				;
			return FALSE;
			default:
			str = buf;
			sprintf( buf, "UNKNOWN (0x%02x)", label );
			break;
	}

	while ( GetDataBlock( fd, ( unsigned char* ) buf ) != 0 )
		;

	return FALSE;
}

int ZeroDataBlock = FALSE;

/*
 * GetDataBlock(): find the datablock with a GIF file 
 */
static int
GetDataBlock( FILE* fd, unsigned char* buf )
{
	unsigned char count;

	if ( ! ReadOK( fd, &count, 1 ) )
	{
		throw_exception( "error in getting DataBlock size" );
		return -1;
	}

	ZeroDataBlock = count == 0;

	if ( ( count != 0 ) && ( ! ReadOK( fd, buf, count ) ) )
	{
		throw_exception( "error in reading DataBlock" );
		return -1;
	}

	return count;
}

/*
 * GetCode():   find the code/bits of GIF.
 */
static int
GetCode( FILE* fd, int code_size, int flag )
{
	static unsigned char buf[ 280 ];
	static int curbit, lastbit, done, last_byte;
	int i, j, ret;
	unsigned char count;

	if ( flag )
	{
		curbit = 0;
		lastbit = 0;
		done = FALSE;
		return 0;
	}

	if ( ( curbit + code_size ) >= lastbit )
	{
		if ( done )
		{
			if ( curbit >= lastbit )
				throw_exception( "ran off the end of my bits" );
			return -1;
		}
		buf[ 0 ] = buf[ last_byte - 2 ];
		buf[ 1 ] = buf[ last_byte - 1 ];

		if ( ( count = GetDataBlock( fd, &buf[ 2 ] ) ) == 0 )
			done = TRUE;

		last_byte = 2 + count;
		curbit = ( curbit - lastbit ) + 16;
		lastbit = ( 2 + count ) * 8 ;
	}

	ret = 0;
	for ( i = curbit, j = 0; j < code_size; ++i, ++j )
		ret |= ( ( buf[ i / 8 ] & ( 1 << ( i % 8 ) ) ) != 0 ) << j;

	curbit += code_size;

	return ret;
}

/*
 * LWZReadByte(): read compress bytes 
 */
static int
LWZReadByte( FILE* fd, int flag, int input_code_size )
{
	static int fresh = FALSE;
	int code, incode;
	static int code_size, set_code_size;
	static int max_code, max_code_size;
	static int firstcode, oldcode;
	static int clear_code, end_code;
	static int table[ 2 ][ ( 1 << MAX_LWZ_BITS ) ];
	static int stack[ ( 1 << ( MAX_LWZ_BITS ) ) * 2 ], *sp;
	register int i;

	if ( flag )
	{
		set_code_size = input_code_size;
		code_size = set_code_size + 1;
		clear_code = 1 << set_code_size ;
		end_code = clear_code + 1;
		max_code_size = 2 * clear_code;
		max_code = clear_code + 2;

		GetCode( fd, 0, TRUE );

		fresh = TRUE;

		for ( i = 0; i < clear_code; ++i )
		{
			table[ 0 ][ i ] = 0;
			table[ 1 ][ i ] = i;
		}
		for ( ; i < ( 1 << MAX_LWZ_BITS ); ++i )
			table[ 0 ][ i ] = table[ 1 ][ 0 ] = 0;

		sp = stack;

		return 0;
	}
	else if ( fresh )
	{
		fresh = FALSE;
		do
		{
			firstcode = oldcode =
			                GetCode( fd, code_size, FALSE );
		}
		while ( firstcode == clear_code );
		return firstcode;
	}

	if ( sp > stack )
		return * --sp;

	while ( ( code = GetCode( fd, code_size, FALSE ) ) >= 0 )
	{
		if ( code == clear_code )
		{
			for ( i = 0; i < clear_code; ++i )
			{
				table[ 0 ][ i ] = 0;
				table[ 1 ][ i ] = i;
			}
			for ( ; i < ( 1 << MAX_LWZ_BITS ); ++i )
				table[ 0 ][ i ] = table[ 1 ][ i ] = 0;
			code_size = set_code_size + 1;
			max_code_size = 2 * clear_code;
			max_code = clear_code + 2;
			sp = stack;
			firstcode = oldcode =
			                GetCode( fd, code_size, FALSE );
			return firstcode;
		}
		else if ( code == end_code )
		{
			int count;
			unsigned char buf[ 260 ];

			if ( ZeroDataBlock )
				return -2;

			while ( ( count = GetDataBlock( fd, buf ) ) > 0 )
				;

			/* if (count != 0)
			 *    pm_message("missing EOD in data stream (common occurence)");
			*/ 
			return -2;
		}

		incode = code;

		if ( code >= max_code )
		{
			*sp++ = firstcode;
			code = oldcode;
		}

		while ( code >= clear_code )
		{
			*sp++ = table[ 1 ][ code ];
			if ( code == table[ 0 ][ code ] )
				throw_exception( "circular table entry BIG ERROR" );
			code = table[ 0 ][ code ];
		}

		*sp++ = firstcode = table[ 1 ][ code ];

		if ( ( code = max_code ) < ( 1 << MAX_LWZ_BITS ) )
		{
			table[ 0 ][ code ] = oldcode;
			table[ 1 ][ code ] = firstcode;
			++max_code;
			if ( ( max_code >= max_code_size ) &&
			        ( max_code_size < ( 1 << MAX_LWZ_BITS ) ) )
			{
				max_code_size *= 2;
				++code_size;
			}
		}

		oldcode = incode;

		if ( sp > stack )
			return * --sp;
	}
	return code;
}

/*
 * ReadImage(): read the image from GIF and save as TIFF File
 */
static char *ReadImage( FILE* fd, int len, int height,
                        unsigned char cmap[ 3 ][ MAXCOLORMAPSIZE ],
                        int pbm_format,
                        int interlace,
                        int ignore,
                        char* tiffname )
{
	unsigned char c;
	int v, i, j;
	unsigned char* image;
	int xpos = 0, ypos = 0, pass = 0;
	unsigned char *pixels;


	/*
	**  Initialize the Compression routines
	*/
	if ( ! ReadOK( fd, &c, 1 ) )
		throw_exception( "EOF / read error on image data" );

	if ( LWZReadByte( fd, TRUE, c ) < 0 )
		throw_exception( "error reading image" );

	/*
	**  If this is an "uninteresting picture" ignore it.
	*/
	if ( ignore )
	{
		throw_exception( "skipping image..." );
		while ( LWZReadByte( fd, FALSE, c ) >= 0 );
		return NULL;
	}


	image = ( unsigned char * ) calloc( len, height );
	if ( image == NULL )
	{
		throw_exception( "couldn't alloc space for image" );
		return NULL;
	}

	/* read the image */
	while ( ( v = LWZReadByte( fd, FALSE, c ) ) >= 0 )
	{
		image[ ( ypos * len ) + xpos ] = v;

		++xpos;
		if ( xpos == len )
		{
			xpos = 0;
			if ( interlace )
			{
				switch ( pass )
				{
						case 0:
						case 1:
						ypos += 8; break;
						case 2:
						ypos += 4; break;
						case 3:
						ypos += 2; break;
				}

				if ( ypos >= height )
				{
					++pass;
					switch ( pass )
					{
							case 1:
							ypos = 4; break;
							case 2:
							ypos = 2; break;
							case 3:
							ypos = 1; break;
							default:
							goto end;
					}
				}
			}
			else
			{
				++ypos;
			}
		}
		if ( ypos >= height )
			break;
	}

end:

	if ( LWZReadByte( fd, FALSE, c ) >= 0 )
		throw_exception( "too much input data, ignoring extra..." );



	/* read image complete than now; save to a pixels RGB */
	pixels = malloc( height * len * 3 );

	for ( j = 0; j < height; j++ )
	{
		for ( i = 0; i < len; i++ )
		{
			pixels[ ( j * len * 3 ) + i * 3 + 0 ] = cmap[ CM_RED ][ image[ ( j * len ) + i ] ]; /* red */
			pixels[ ( j * len * 3 ) + i * 3 + 1 ] = cmap[ CM_GREEN ][ image[ ( j * len ) + i ] ]; /* green */
			pixels[ ( j * len * 3 ) + i * 3 + 2 ] = cmap[ CM_BLUE ][ image[ ( j * len ) + i ] ]; /* blue */
		}
	}
	save_tiff( tiffname, pixels, len, height, 3, "gif2tif" );

	free( pixels );

	free( image );

	return ( tiffname );
}


/*
 * swap2() swap an short to NT compliant 
 */
static unsigned short swap2( unsigned short s )
{
	short n;
	unsigned char *c, *d;

	c = ( unsigned char* ) & n;
	d = ( unsigned char* ) & s;

	c[ 0 ] = d[ 1 ];
	c[ 1 ] = d[ 0 ];
	return n;
}


/*
 * throw_exception: printf an error string to stdout 
 */
static void throw_exception( char *s )
{
#ifdef DEBUG
	puts( s );
#endif
}

#ifdef MAIN
int main( int argc, char *argv[] )
{
	char * result;

	if ( argc != 2 )
	{
		fprintf( stderr, "Usage %s: %s some.gif", argv[ 0 ], argv[ 1 ] );
		exit( 2 );
	}
	result = gif2tif( argv[ 1 ] );
	if ( result )
	{
		puts( result );
	}
	return 1;
}
#endif
