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
		\brief Implements a jpg importer
		\author Michel Joron (joron@sympatico.ca)
*/


// The conversion process assumes a minimal conversion
// the function to do convertion must be name
// as the name of the dll or .so on Unix
// The conversion must create a valid .tif file and return
// the filename of the new file created.
// Eg. this function jpg2tif() created a .tif file at the same place
// as the .jpg filename provided.
// Minimal implemantation is just doing a call or two to libjpeg provided with the
// standard libjpeg source codes.
// IN ORDER TO CONVERSION OCCURRED the dll/dso must be present in
// procedures directory under <AQSIS_BASE_PATH>

#include <stdio.h>
#include <setjmp.h>
#include <string.h>
#include <stdlib.h>

#include "jpeglib.h"

#include "../common/pixelsave.h"

#ifdef	WIN32
#define	__export	__declspec(dllexport)
#else	// WIN32
#define	__export
#endif	// WIN32

typedef struct tagMyErrorMgr
{
	struct jpeg_error_mgr pub;        /* "public" fields */

	jmp_buf setjmp_buffer;        /* for return to caller */
}
MyErrorMgr;



typedef struct tagJpegReader
{
	FILE *infile;
	struct jpeg_decompress_struct *cinfo;
	MyErrorMgr *jerr_;
	unsigned char *raster;
}
JpegReader;


static char tiffname[ 1024 ];


static char *jpeg_open( FILE * infile, char *filename );
static void my_error_exit( j_common_ptr cinfo );
static void run( JpegReader * jerr );

/* Main function to convert any jpg to tif format
 * It used the standard tiff tool name jpg2tiff.exe on PC
 * or jpg2tiff on unix
 */
__export char *jpg2tif( char *in )
{
	FILE * jpgfile;
	char *result = NULL;


	strcpy( tiffname, in );
	if ( ( result = strstr( tiffname, ".jpg" ) ) != 0 )
		strcpy( result, ".tif" );
	if ( !result )
	{
		if ( ( result = strstr( tiffname, ".jpg" ) ) != 0 )
			strcpy( result, ".tif" );
	}
	if ( !result )
		return result;

	jpgfile = fopen( in, "rb" );
	result = jpeg_open( jpgfile, tiffname );

	return result;
}

/* PRIVATE FUNCTIONS */
/*
 * jpeg_open(): Open the jpg, validate if good convert to  
 *    rgbtriple than save to tiff 24 bitplanes file format.
 */
static
char *
jpeg_open( FILE * infile, char *filename )
{

	unsigned char * buffer;
	int i;
	long size;

	struct jpeg_decompress_struct *cinfo;
	MyErrorMgr *jerr;
	JpegReader *jp;

	jerr = ( MyErrorMgr * ) calloc( 1, sizeof( MyErrorMgr ) );
	cinfo = ( struct jpeg_decompress_struct * ) calloc( 1, sizeof( struct jpeg_decompress_struct ) );
	cinfo->err = jpeg_std_error( &jerr->pub );
	jerr->pub.error_exit = my_error_exit;
	/* Establish the setjmp return context for my_error_exit to use. */
	if ( setjmp( jerr->setjmp_buffer ) )
	{
		/*
		 * If we get here, the JPEG code has signaled an error. We need to
		 * clean up the JPEG object, close the input file, and return.
		 */
		jpeg_destroy_decompress( cinfo );
		free( cinfo );
		free( jerr );
		fclose( infile );
		return 0;
	}
	jpeg_create_decompress( cinfo );
	jpeg_stdio_src( cinfo, infile );
	jpeg_read_header( cinfo, TRUE );
	if ( jpeg_has_multiple_scans( cinfo ) )
	{
		cinfo->buffered_image = TRUE;
	}
	jpeg_start_decompress( cinfo );

	/* Allocate raster pixel map. in to buffer */

	size = cinfo->output_width * cinfo->output_height;
	buffer = ( unsigned char * ) malloc( size * cinfo->num_components );

	jp = ( JpegReader * ) calloc( 1, sizeof( JpegReader ) );
	jp->infile = infile;
	jp->cinfo = cinfo;
	jp->raster = buffer;
	jp->jerr_ = jerr;

	run( jp );
	/* done */
	fclose( infile );


	if ( cinfo->num_components == 1 )
	{
		/* the jpeg was a grayscale */
		unsigned char * grayscale = malloc( size * 3 );
		for ( i = 0; i < size; i ++ )
		{
			grayscale[ i * 3 ] = grayscale[ i * 3 + 1 ] = grayscale[ i * 3 + 2 ] = buffer[ i ];
		}
		save_tiff( filename, ( unsigned char * ) grayscale, cinfo->output_width , cinfo->output_height, 3, "jpg2tif" );
		free( grayscale );
	}
	else
	{
		save_tiff( filename, ( unsigned char * ) buffer, cinfo->output_width , cinfo->output_height, cinfo->num_components, "jpg2tif" );
	}
	free( jerr );
	free( cinfo );
	free( buffer );
	free( jp );

	return filename;
}

/*
 * my_error_exit() handle for JPEG library... isolate ourself 
 *   of potential bug with the JPEG library itself.    
 */
static void my_error_exit( j_common_ptr cinfo )
{
	/* cinfo->err really points to a MyErrorMgr struct, so coerce pointer */
	MyErrorMgr * myerr = ( MyErrorMgr * ) cinfo->err;

	/* Always display the message. */
	/* We could postpone this until after returning, if we chose. */
	( *cinfo->err->output_message ) ( cinfo );

	/* Return control to the setjmp point */
	longjmp( myerr->setjmp_buffer, 1 );
}

/*
 *  read_image(): Read the image as rgbtriple
 */
static void
read_image(
    struct jpeg_decompress_struct * cinfo, unsigned char * raster )
{

	/* JSAMPLEs per row in output buffer */
	JSAMPROW row;
	unsigned int i, j;
	int row_stride;
	JSAMPARRAY buffer;
	unsigned int samples = cinfo->output_components;

	row_stride = cinfo->output_width * cinfo->output_components;
	/* Make a one-row-high sample array that will go away when done with image */
	buffer = ( *cinfo->mem->alloc_sarray ) ( ( j_common_ptr ) cinfo, JPOOL_IMAGE, row_stride, 1 );

	while ( cinfo->output_scanline < cinfo->output_height )
	{
		/*
		 * jpeg_read_scanlines expects an array of pointers to scanlines. Here
		 * the array is only one element long, but you could ask for more than
		 * one scanline at a time if that's more convenient.
		 */
		( void ) jpeg_read_scanlines( cinfo, buffer, 1 );

		row = buffer[ 0 ];
		for ( i = 0; i < cinfo->output_width; i ++ )
		{
			int index = ( ( ( cinfo->output_scanline - 1 ) * cinfo->output_width ) + i ) * samples;
			for ( j = 0; j < samples; j++ )
				raster[ index + j ] = *row++;        /* r, g, b */
		}
	}

}


/*
 * run(): this is the core convertion occurring!
 */
static void
run( JpegReader * jerr )
{
	if ( setjmp( jerr->jerr_->setjmp_buffer ) == 0 )
	{
		if ( jpeg_has_multiple_scans( jerr->cinfo ) )
		{
			while ( !jpeg_input_complete( jerr->cinfo ) )
			{
				jpeg_start_output( jerr->cinfo, jerr->cinfo->input_scan_number );
				read_image( jerr->cinfo, jerr->raster );
				jpeg_finish_output( jerr->cinfo );
			}
		}
		else
		{
			read_image( jerr->cinfo, jerr->raster );
		}
		jpeg_finish_decompress( jerr->cinfo );
		/*
		 * At this point you may want to check to see whether any corrupt-data
		 * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
		 */
	}
	jpeg_destroy_decompress( jerr->cinfo );

}

#ifdef MAIN
int main( int argc, char *argv[] )
{
	char * result;

	if ( argc != 2 )
	{
		fprintf( stderr, "Usage %s: %s some.jpg", argv[ 0 ], argv[ 1 ] );
		exit( 2 );
	}
	result = jpg2tif( argv[ 1 ] );
	if ( result )
	{
		puts( result );
	}
	return 1;
}
#endif
