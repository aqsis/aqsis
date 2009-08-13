// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
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
		\brief Implements the default display devices for Aqsis.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include <aqsis/aqsis.h>

#include <iostream>
#include <iomanip>
#include <ios>
#include <memory>
#include <sstream>
#include <string>
#include <fstream>
#include <algorithm>
#include <float.h>
#include <time.h>
#include <cstring>

#include <tiffio.h>

#ifndef	AQSIS_NO_FLTK
#	include <FL/Fl.H>
#	include <FL/Fl_Window.H>
#	include <FL/Fl_Box.H>
#	include <FL/Fl_Image.H>
#	include <FL/fl_draw.H>
#endif // AQSIS_NO_LTK

#include <aqsis/math/math.h>
#include "dspyhlpr.h"
#include <aqsis/util/logging.h>
#include <aqsis/util/logging_streambufs.h>
#include <aqsis/ri/ndspy.h>

using namespace Aqsis;

#define	ZFILE_HEADER		"Aqsis ZFile" AQSIS_VERSION_STR
#define	SHADOWMAP_HEADER	"Shadow"

#include	<aqsis/version.h>

namespace {

//------------------------------------------------------------------------------
/** FLTK Widget used to show a constantly updating image.
 *
 */
#ifndef	AQSIS_NO_FLTK
class Fl_FrameBuffer_Widget : public Fl_Widget
{
	public:
		Fl_FrameBuffer_Widget(int x, int y, int imageW, int imageH, int depth, unsigned char* imageD) : Fl_Widget(x,y,imageW,imageH)
		{
			w = imageW;
			h = imageH;
			d = depth;
			image = imageD;
		}

		void draw(void)
		{
			fl_draw_image(image,x(),y(),w,h,d,w*d); // draw image
		}

	private:
		int w,h,d;
		unsigned char* image;
};
#endif // AQSIS_NO_FLTK


//------------------------------------------------------------------------------
enum EqDisplayTypes
{
    Type_File = 0,
    Type_Framebuffer,
    Type_ZFile,
    Type_ZFramebuffer,
    Type_Shadowmap,
};

struct SqDisplayInstance
{
	SqDisplayInstance() :
			m_filename(),
			m_width(0),
			m_height(0),
			m_iFormatCount(0),
			m_format(PkDspyUnsigned8),
			m_entrySize(0),
			m_lineLength(0),
			m_compression(COMPRESSION_LZW), m_quality(90),
			m_hostname(),
			m_RenderWholeFrame(false),
			m_imageType(Type_File),
			m_append(0),
			m_pixelsReceived(0),
			m_data(0),
			m_zfbdata(0)
#ifndef	AQSIS_NO_FLTK
			,
			m_theWindow(0),
			m_uiImageWidget(0),
			m_uiImage(0)
#endif // AQSIS_NO_FLTK
	{}
	std::string	m_filename;
	TqInt		m_width;
	TqInt		m_height;
	TqInt		m_OriginalSize[2];
	TqInt		m_origin[2];
	TqInt		m_iFormatCount;
	TqInt		m_format;
	TqInt		m_entrySize;
	TqInt		m_lineLength;
	uint16		m_compression, m_quality;
	std::string	m_hostname;
	bool		m_RenderWholeFrame;
	TqInt		m_imageType;
	TqInt		m_append;
	TqFloat		m_matWorldToCamera[ 4 ][ 4 ];
	TqFloat		m_matWorldToScreen[ 4 ][ 4 ];
	// The number of pixels that have already been rendered (used for progress reporting)
	TqInt		m_pixelsReceived;

	void*		m_data;
	unsigned char*	m_zfbdata;

#ifndef	AQSIS_NO_FLTK

	Fl_Window*	m_theWindow;
	Fl_FrameBuffer_Widget* m_uiImageWidget;
	Fl_RGB_Image*	m_uiImage;
#endif // AQSIS_NO_FLTK
};
//------------------------------------------------------------------------------


static char datetime[21];
static time_t start;
static std::string description;

//----------------------------------------------------------------------
/** SaveAsShadowMap() Save as a tiff an shadowmap
*
*/

void SaveAsShadowMap(const std::string& filename, SqDisplayInstance* image, char *mydescription)
{
	TqChar version[ 80 ];
	TqUint twidth = 32;
	TqUint tlength = 32;

	struct tm *ct;
	int year;

	time_t long_time;

	time( &long_time );           /* Get time as long integer. */
	ct = localtime( &long_time ); /* Convert to local time. */


	year=1900 + ct->tm_year;
	sprintf(datetime, "%04d:%02d:%02d %02d:%02d:%02d", year, ct->tm_mon + 1,
	        ct->tm_mday, ct->tm_hour, ct->tm_min, ct->tm_sec);


	const char* mode = (image->m_append)? "a" : "w";

	// Save the shadowmap to a binary file.
	if ( filename.compare( "" ) != 0 )
	{
		TIFF * pshadow = TIFFOpen( filename.c_str(), mode );
		if( pshadow != NULL )
		{
			// Set common tags
			TIFFCreateDirectory( pshadow );

			sprintf( version, "Aqsis %s (%s %s)", AQSIS_VERSION_STR, __DATE__, __TIME__);

			TIFFSetField( pshadow, TIFFTAG_SOFTWARE, ( char* ) version );
			TIFFSetField( pshadow, TIFFTAG_PIXAR_MATRIX_WORLDTOCAMERA, image->m_matWorldToCamera );
			TIFFSetField( pshadow, TIFFTAG_PIXAR_MATRIX_WORLDTOSCREEN, image->m_matWorldToScreen );
			TIFFSetField( pshadow, TIFFTAG_PIXAR_TEXTUREFORMAT, SHADOWMAP_HEADER );
			TIFFSetField( pshadow, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK );

			if (!image->m_hostname.empty())
				TIFFSetField( pshadow, TIFFTAG_HOSTCOMPUTER, image->m_hostname.c_str() );
			TIFFSetField( pshadow, TIFFTAG_IMAGEDESCRIPTION, mydescription);
			// Write the floating point image to the directory.
			TIFFSetField( pshadow, TIFFTAG_IMAGEWIDTH, image->m_width );
			TIFFSetField( pshadow, TIFFTAG_IMAGELENGTH, image->m_height );
			TIFFSetField( pshadow, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG );
			TIFFSetField( pshadow, TIFFTAG_BITSPERSAMPLE, 32 );
			TIFFSetField( pshadow, TIFFTAG_SAMPLESPERPIXEL, image->m_iFormatCount );
			TIFFSetField( pshadow, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
			TIFFSetField( pshadow, TIFFTAG_TILEWIDTH, twidth );
			TIFFSetField( pshadow, TIFFTAG_TILELENGTH, tlength );
			TIFFSetField( pshadow, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP );
			TIFFSetField( pshadow, TIFFTAG_COMPRESSION, image->m_compression );
			TIFFSetField( pshadow, TIFFTAG_DATETIME, datetime);

			TqDouble minz = FLT_MAX;
			TqFloat* ptdata = reinterpret_cast<TqFloat*>(image->m_data) ;
			for (TqInt y = 0; y < image->m_height; y++)
				for (TqInt x = 0; x < image->m_width; x++)
				{
					TqDouble value = (TqDouble) ptdata[y*image->m_width + x];
					if (value < minz) minz = value;	
				}
			TIFFSetField( pshadow, TIFFTAG_SMINSAMPLEVALUE, minz );

			TqUint tsize = twidth * tlength;
			TqUint tperrow = ( image->m_width + twidth - 1 ) / twidth;
			TqFloat* ptile = static_cast<TqFloat*>( _TIFFmalloc( tsize * sizeof( TqFloat ) ) );


			if ( ptile != NULL )
			{
				TqUint ctiles = tperrow * ( ( image->m_width + tlength - 1 ) / tlength );
				TqUint itile;
				for ( itile = 0; itile < ctiles; itile++ )
				{
					TqUint x = ( itile % tperrow ) * twidth;
					TqUint y = ( itile / tperrow ) * tlength;
					ptdata = reinterpret_cast<TqFloat*>(image->m_data) + ( ( y * image->m_width ) + x ) * image->m_iFormatCount;
					// Clear the tile to black.
					memset( ptile, 0, tsize * sizeof( TqFloat ) );
					for ( TqUlong i = 0; i < tlength; i++ )
					{
						for ( TqUlong j = 0; j < twidth; j++ )
						{
							if ( ( x + j ) < (TqUlong) image->m_width && ( y + i ) < (TqUlong) image->m_height )
							{
								TqInt ii;
								for ( ii = 0; ii < image->m_iFormatCount; ii++ ) 
								{
									TqFloat value = ptdata[ ( ( j * image->m_iFormatCount ) + ii ) ];
									ptile[ ( i * twidth * image->m_iFormatCount ) + ( ( ( j * image->m_iFormatCount ) + ii ) ) ] = value;
								}
							}
						}
						ptdata += ( image->m_width * image->m_iFormatCount );
					}
					TIFFWriteTile( pshadow, ptile, x, y, 0, 0 );
				}
				TIFFWriteDirectory( pshadow );

			}


			TIFFClose( pshadow );
		}
	}
}

//----------------------------------------------------------------------
/** WriteTIFF() Save as a tiff the output of the renderer
*
*/

void WriteTIFF(const std::string& filename, SqDisplayInstance* image)
{
	uint16 photometric = PHOTOMETRIC_RGB;
	uint16 config = PLANARCONFIG_CONTIG;
	struct tm *ct;
	char mydescription[80];
	int year;

	time_t long_time;

	time( &long_time );           /* Get time as long integer. */
	ct = localtime( &long_time ); /* Convert to local time. */


	year=1900 + ct->tm_year;
	sprintf(datetime, "%04d:%02d:%02d %02d:%02d:%02d", year, ct->tm_mon + 1,
	        ct->tm_mday, ct->tm_hour, ct->tm_min, ct->tm_sec);

	if (description.empty())
	{
		double nSecs = difftime(long_time, start);
		sprintf(mydescription,"Aqsis Renderer, %d secs rendertime", static_cast<TqInt>(nSecs));
		start = long_time;
	}
	else
	{
		strcpy(mydescription, description.c_str());
	}


	// Set common tags
	// If in "shadowmap" mode, write as a shadowmap.
	if( image->m_imageType == Type_Shadowmap )
	{
		SaveAsShadowMap(filename, image, mydescription);
		return;
	}
	else if( image->m_imageType == Type_ZFile )
	{
		std::ofstream ofile( filename.c_str(), std::ios::out | std::ios::binary );
		if ( ofile.is_open() )
		{
			// Save a file type and version marker
			ofile << ZFILE_HEADER;

			// Save the xres and yres.
			ofile.write( reinterpret_cast<char* >( &image->m_width ), sizeof( image->m_width ) );
			ofile.write( reinterpret_cast<char* >( &image->m_height ), sizeof( image->m_height ) );

			// Save the transformation matrices.
			ofile.write( reinterpret_cast<char*>( image->m_matWorldToCamera[ 0 ] ), sizeof( image->m_matWorldToCamera[ 0 ][ 0 ] ) * 4 );
			ofile.write( reinterpret_cast<char*>( image->m_matWorldToCamera[ 1 ] ), sizeof( image->m_matWorldToCamera[ 0 ][ 0 ] ) * 4 );
			ofile.write( reinterpret_cast<char*>( image->m_matWorldToCamera[ 2 ] ), sizeof( image->m_matWorldToCamera[ 0 ][ 0 ] ) * 4 );
			ofile.write( reinterpret_cast<char*>( image->m_matWorldToCamera[ 3 ] ), sizeof( image->m_matWorldToCamera[ 0 ][ 0 ] ) * 4 );

			ofile.write( reinterpret_cast<char*>( image->m_matWorldToScreen[ 0 ] ), sizeof( image->m_matWorldToScreen[ 0 ][ 0 ] ) * 4 );
			ofile.write( reinterpret_cast<char*>( image->m_matWorldToScreen[ 1 ] ), sizeof( image->m_matWorldToScreen[ 0 ][ 0 ] ) * 4 );
			ofile.write( reinterpret_cast<char*>( image->m_matWorldToScreen[ 2 ] ), sizeof( image->m_matWorldToScreen[ 0 ][ 0 ] ) * 4 );
			ofile.write( reinterpret_cast<char*>( image->m_matWorldToScreen[ 3 ] ), sizeof( image->m_matWorldToScreen[ 0 ][ 0 ] ) * 4 );

			// Now output the depth values
			ofile.write( reinterpret_cast<char*>( image->m_data ), sizeof( TqFloat ) * ( image->m_width * image->m_height ) );
			ofile.close();
		}
		return;
	}

	TIFF* pOut = TIFFOpen( filename.c_str(), "w" );

	if ( pOut )
	{
		// Write the image to a tiff file.
		char version[ 80 ];

		short ExtraSamplesTypes[ 1 ] = {EXTRASAMPLE_ASSOCALPHA};

		sprintf( version, "Aqsis %s (%s %s)", AQSIS_VERSION_STR, __DATE__, __TIME__);
		bool use_logluv = false;

		TIFFSetField( pOut, TIFFTAG_SOFTWARE, ( char* ) version );
		TIFFSetField( pOut, TIFFTAG_IMAGEWIDTH, ( uint32 ) image->m_width );
		TIFFSetField( pOut, TIFFTAG_IMAGELENGTH, ( uint32 ) image->m_height );
		TIFFSetField( pOut, TIFFTAG_XRESOLUTION, (float) 1.0 );
		TIFFSetField( pOut, TIFFTAG_YRESOLUTION, (float) 1.0 );
		TIFFSetField( pOut, TIFFTAG_BITSPERSAMPLE, (short) 8 );
		TIFFSetField( pOut, TIFFTAG_PIXAR_MATRIX_WORLDTOCAMERA, image->m_matWorldToCamera );
		TIFFSetField( pOut, TIFFTAG_PIXAR_MATRIX_WORLDTOSCREEN, image->m_matWorldToScreen );
		TIFFSetField( pOut, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
		TIFFSetField( pOut, TIFFTAG_SAMPLESPERPIXEL, image->m_iFormatCount );
		TIFFSetField( pOut, TIFFTAG_DATETIME, datetime);
		if (!image->m_hostname.empty())
			TIFFSetField( pOut, TIFFTAG_HOSTCOMPUTER, image->m_hostname.c_str() );
		TIFFSetField( pOut, TIFFTAG_IMAGEDESCRIPTION, mydescription);


		// Write out an 8 bits per pixel integer image.
		if ( image->m_format == PkDspyUnsigned8 )
		{
			TIFFSetField( pOut, TIFFTAG_BITSPERSAMPLE, 8 );
			TIFFSetField( pOut, TIFFTAG_PLANARCONFIG, config );
			TIFFSetField( pOut, TIFFTAG_COMPRESSION, image->m_compression );
			if ( image->m_compression == COMPRESSION_JPEG )
				TIFFSetField( pOut, TIFFTAG_JPEGQUALITY, image->m_quality );
			TIFFSetField( pOut, TIFFTAG_PHOTOMETRIC, photometric );
			TIFFSetField( pOut, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize( pOut, 0 ) );

			if ( image->m_iFormatCount == 4 )
				TIFFSetField( pOut, TIFFTAG_EXTRASAMPLES, 1, ExtraSamplesTypes );

			// Set the position tages in case we aer dealing with a cropped image.
			TIFFSetField( pOut, TIFFTAG_XPOSITION, ( float ) image->m_origin[0] );
			TIFFSetField( pOut, TIFFTAG_YPOSITION, ( float ) image->m_origin[1] );

			TqInt row;
			for ( row = 0; row < image->m_height; row++ )
			{
				if ( TIFFWriteScanline( pOut, reinterpret_cast<void*>(reinterpret_cast<char*>(image->m_data) + ( row * image->m_lineLength ))
				                        , row, 0 ) < 0 )
					break;
			}
			TIFFClose( pOut );
		}
		else
		{
			// Write out a floating point image.
			TIFFSetField( pOut, TIFFTAG_STONITS, ( double ) 1.0 );

			//			if(/* user wants logluv compression*/)
			//			{
			//				if(/* user wants to save the alpha channel */)
			//				{
			//					warn("SGI LogLuv encoding does not allow an alpha channel"
			//							" - using uncompressed IEEEFP instead");
			//				}
			//				else
			//				{
			//					use_logluv = true;
			//				}
			//
			//				if(/* user wants LZW compression*/)
			//				{
			//					warn("LZW compression is not available with SGI LogLuv encoding\n");
			//				}
			//			}

			if ( use_logluv )
			{
				/* use SGI LogLuv compression */
				TIFFSetField( pOut, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT );
				TIFFSetField( pOut, TIFFTAG_BITSPERSAMPLE, 16 );
				TIFFSetField( pOut, TIFFTAG_COMPRESSION, COMPRESSION_SGILOG );
				TIFFSetField( pOut, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_LOGLUV );
				TIFFSetField( pOut, TIFFTAG_SGILOGDATAFMT, SGILOGDATAFMT_FLOAT );
			}
			else
			{
				/* use uncompressed IEEEFP pixels */
				TIFFSetField( pOut, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP );
				TIFFSetField( pOut, TIFFTAG_BITSPERSAMPLE, 32 );
				TIFFSetField( pOut, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB );
				TIFFSetField( pOut, TIFFTAG_COMPRESSION, image->m_compression );
			}
			if (image->m_format == PkDspyUnsigned16)
			{
				TIFFSetField( pOut, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT );
				TIFFSetField( pOut, TIFFTAG_BITSPERSAMPLE, 16 );
			}

			TIFFSetField( pOut, TIFFTAG_SAMPLESPERPIXEL, image->m_iFormatCount );

			if ( image->m_iFormatCount == 4 )
				TIFFSetField( pOut, TIFFTAG_EXTRASAMPLES, 1, ExtraSamplesTypes );
			// Set the position tages in case we aer dealing with a cropped image.
			TIFFSetField( pOut, TIFFTAG_XPOSITION, ( float ) image->m_origin[0] );
			TIFFSetField( pOut, TIFFTAG_YPOSITION, ( float ) image->m_origin[1] );
			TIFFSetField( pOut, TIFFTAG_PLANARCONFIG, config );

			TqInt row = 0;
			for ( row = 0; row < image->m_height; row++ )
			{
				if ( TIFFWriteScanline( pOut, reinterpret_cast<void*>(reinterpret_cast<TqUchar*>(image->m_data) + ( row * image->m_lineLength )), row, 0 )
				        < 0 )
					break;
			}
			TIFFClose( pOut );
		}
	}
}

//----------------------------------------------------------------------
/** CompositeAlpha() Composite with the alpha the end result RGB
*
*/

void CompositeAlpha(TqInt r, TqInt g, TqInt b, TqUchar &R, TqUchar &G, TqUchar &B, 
		    TqUchar alpha )
{ 
#	define INT_MULT(a,b,t) ( (t) = (a) * (b) + 0x80, ( ( ( (t)>>8 ) + (t) )>>8 ) )
#	define INT_PRELERP(p, q, a, t) ( (p) + (q) - INT_MULT( a, p, t) )
	TqInt t;
	// C’ = INT_PRELERP( A’, B’, b, t )
	TqInt R1 = static_cast<TqInt>(INT_PRELERP( R, r, alpha, t ));
	TqInt G1 = static_cast<TqInt>(INT_PRELERP( G, g, alpha, t ));
	TqInt B1 = static_cast<TqInt>(INT_PRELERP( B, b, alpha, t ));
	R = clamp<TqUchar>(R1, 0, 255);
	G = clamp<TqUchar>(G1, 0, 255);
	B = clamp<TqUchar>(B1, 0, 255);
#	undef INT_MULT
#	undef INT_PRELERP
}


} // unnamed namespace


extern "C" PtDspyError DspyImageOpen(PtDspyImageHandle * image,
                                     const char *drivername,
                                     const char *filename,
                                     int width,
                                     int height,
                                     int paramCount,
                                     const UserParameter *parameters,
                                     int iFormatCount,
                                     PtDspyDevFormat *format,
                                     PtFlagStuff *flagstuff)
{
	SqDisplayInstance* pImage;

	pImage = new SqDisplayInstance;
	flagstuff->flags = 0;

	time(&start);
	if(pImage)
	{
		// Store the instance information so that on re-entry we know which display is being referenced.
		pImage->m_height = height;
		pImage->m_width = width;
		// Determine the display type from the list that we support.
		if(strcmp(drivername, "file")==0 || strcmp(drivername, "tiff")==0)
			pImage->m_imageType = Type_File;
#ifndef	AQSIS_NO_FLTK

		else if((strcmp(drivername, "framebuffer")==0) || (strcmp(drivername, "legacyframebuffer")==0))
			pImage->m_imageType = Type_Framebuffer;
#endif // AQSIS_NO_FLTK

		else if(strcmp(drivername, "zfile")==0)
			pImage->m_imageType = Type_ZFile;
#ifndef AQSIS_NO_FLTK

		else if(strcmp(drivername, "zframebuffer")==0)
			pImage->m_imageType = Type_ZFramebuffer;
#endif // AQSIS_NO_FLTK

		else if(strcmp(drivername, "shadow")==0)
			pImage->m_imageType = Type_Shadowmap;
		else
			pImage->m_imageType = Type_File;
		pImage->m_iFormatCount = iFormatCount;

		*image = pImage;

		pImage->m_filename = filename;

		// Scan the formats table to see what the widest channel format specified is.
		TqUint widestFormat = PkDspySigned8;
		TqInt i;
		for(i=0; i<iFormatCount; i++)
			if(format[i].type < widestFormat)
				widestFormat = format[i].type;

		if(widestFormat == PkDspySigned8)
			widestFormat = PkDspyUnsigned8;
		else if(widestFormat == PkDspySigned16)
			widestFormat = PkDspyUnsigned16;
		else if(widestFormat == PkDspySigned32)
			widestFormat = PkDspyUnsigned32;

		int scanorder;
		if( DspyFindIntInParamList("scanlineorder", &scanorder, paramCount, parameters ) == PkDspyErrorNone )
		{
			flagstuff->flags = PkDspyFlagsWantsScanLineOrder;
		}

		// If we are recieving "rgba" data, ensure that it is in the correct order.
		if(pImage->m_imageType == Type_File || pImage->m_imageType == Type_Framebuffer )
		{
			PtDspyDevFormat outFormat[] =
				{
					{tokenCast("r"), widestFormat},
					{tokenCast("g"), widestFormat},
					{tokenCast("b"), widestFormat},
					{tokenCast("a"), widestFormat},
				};
			PtDspyError err = DspyReorderFormatting(iFormatCount, format, min(iFormatCount,4), outFormat);
			if( err != PkDspyErrorNone )
			{
				return(err);
			}
		}
		else if(pImage->m_imageType == Type_ZFramebuffer)
		{
			// For the zframebuffer, ensure that we're actually receiving a
			// single channel of z data in floating point format.
			if(iFormatCount != 1 || strcmp(format->name, "z") != 0 || format->type != PkDspyFloat32)
				return PkDspyErrorBadParams;
		}

		// Create and initialise a byte array if rendering 8bit image, or we are in framebuffer mode
		if(pImage->m_imageType == Type_Framebuffer)
		{
#ifndef	AQSIS_NO_FLTK
			// Allocate the buffer, even if the formatcount <3, always allocated 3, as that is what's needed for the
			// display.
			pImage->m_data = malloc( pImage->m_width * pImage->m_height * pImage->m_iFormatCount * sizeof(TqUchar));
			pImage->m_entrySize = pImage->m_iFormatCount * sizeof(TqChar);

			// Initialise the display to a checkerboard to show alpha
			for (TqInt i = 0; i < pImage->m_height; i ++)
			{
				for (TqInt j = 0; j < pImage->m_width; j++)
				{
					int     t       = 0;
					TqUchar d = 255;

					if ( ( (pImage->m_height - 1 - i) & 31 ) < 16 )
						t ^= 1;
					if ( ( j & 31 ) < 16 )
						t ^= 1;

					if ( t )
					{
						d      = 128;
					}
					reinterpret_cast<TqUchar*>(pImage->m_data)[pImage->m_iFormatCount * (i*pImage->m_width + j) ] = d;
					reinterpret_cast<TqUchar*>(pImage->m_data)[pImage->m_iFormatCount * (i*pImage->m_width + j) + 1] = d;
					reinterpret_cast<TqUchar*>(pImage->m_data)[pImage->m_iFormatCount * (i*pImage->m_width + j) + 2] = d;
				}
			}
			widestFormat = PkDspyUnsigned8;


			pImage->m_theWindow = new Fl_Window(pImage->m_width, pImage->m_height);
			pImage->m_uiImageWidget = new Fl_FrameBuffer_Widget(0,0, pImage->m_width, pImage->m_height, pImage->m_iFormatCount, reinterpret_cast<TqUchar*>(pImage->m_data));
			pImage->m_theWindow->resizable(pImage->m_uiImageWidget);
			pImage->m_theWindow->label(pImage->m_filename.c_str());
			pImage->m_theWindow->end();
			Fl::visual(FL_RGB);
			pImage->m_theWindow->show();
#endif // AQSIS_NO_FLTK

		}
		else
		{
			// Determine the appropriate format to save into.
			if(widestFormat == PkDspyUnsigned8)
			{
				pImage->m_data = malloc( pImage->m_width * pImage->m_height * pImage->m_iFormatCount * sizeof(PtDspyUnsigned8));
				pImage->m_entrySize = pImage->m_iFormatCount * sizeof(PtDspyUnsigned8);
			}
			else if(widestFormat == PkDspyUnsigned16)
			{
				pImage->m_data = malloc( pImage->m_width * pImage->m_height * pImage->m_iFormatCount * sizeof(PtDspyUnsigned16));
				pImage->m_entrySize = pImage->m_iFormatCount * sizeof(PtDspyUnsigned16);
			}
			else if(widestFormat == PkDspyUnsigned32)
			{
				pImage->m_data = malloc( pImage->m_width * pImage->m_height * pImage->m_iFormatCount * sizeof(PtDspyUnsigned32));
				pImage->m_entrySize = pImage->m_iFormatCount * sizeof(PtDspyUnsigned32);
			}
			else if(widestFormat == PkDspyFloat32)
			{
				pImage->m_data = malloc( pImage->m_width * pImage->m_height * pImage->m_iFormatCount * sizeof(PtDspyFloat32));
				pImage->m_entrySize = pImage->m_iFormatCount * sizeof(PtDspyFloat32);
			}
		}
		pImage->m_lineLength = pImage->m_entrySize * pImage->m_width;
		pImage->m_format = widestFormat;

		// If in "zframebuffer" mode, we need another buffer for the displayed depth data.
		if(pImage->m_imageType == Type_ZFramebuffer)
		{
#ifndef	AQSIS_NO_FLTK
			pImage->m_zfbdata = reinterpret_cast<TqUchar*>(malloc( pImage->m_width * pImage->m_height * 3 * sizeof(TqUchar)));

			pImage->m_theWindow = new Fl_Window(pImage->m_width, pImage->m_height);
			pImage->m_uiImageWidget = new Fl_FrameBuffer_Widget(0,0, pImage->m_width, pImage->m_height, 3, reinterpret_cast<TqUchar*>(pImage->m_zfbdata));
			pImage->m_theWindow->resizable(pImage->m_uiImageWidget);
			pImage->m_theWindow->label(pImage->m_filename.c_str());
			pImage->m_theWindow->end();
			Fl::visual(FL_RGB);
			pImage->m_theWindow->show();
#endif // AQSIS_NO_FLTK

		}


		// Extract any important data from the user parameters.
		char* compression;
		char* hostname;
		if( DspyFindStringInParamList("HostComputer", &hostname, paramCount, parameters ) == PkDspyErrorNone )
		{
			pImage->m_hostname = hostname;
		}
		if( DspyFindStringInParamList("compression", &compression, paramCount, parameters ) == PkDspyErrorNone )
		{
			if ( strstr( compression, "none" ) != 0 )
				pImage->m_compression = COMPRESSION_NONE;
			else if ( strstr( compression, "lzw" ) != 0 )
				pImage->m_compression = COMPRESSION_LZW;
			else if ( strstr( compression, "deflate" ) != 0 )
				pImage->m_compression = COMPRESSION_DEFLATE;
			else if ( strstr( compression, "jpeg" ) != 0 )
				pImage->m_compression = COMPRESSION_JPEG;
			else if ( strstr( compression, "packbits" ) != 0 )
				pImage->m_compression = COMPRESSION_PACKBITS;
		}
		
		// Check if the requested compression format is available in libtiff, if not resort to "none"
		if(!TIFFIsCODECConfigured(pImage->m_compression))
		{
			/* Aqsis::log() << "Compression type " << compression << " not supported by the libtiff implementation" << std::endl; */
			pImage->m_compression = COMPRESSION_NONE;
		}

		int quality;
		if( DspyFindIntInParamList("quality", &quality, paramCount, parameters ) == PkDspyErrorNone )
			pImage->m_quality = quality;
		// Extract the transformation matrices if they are there.
		DspyFindMatrixInParamList( "NP", reinterpret_cast<float*>(pImage->m_matWorldToScreen), paramCount, parameters );
		DspyFindMatrixInParamList( "Nl", reinterpret_cast<float*>(pImage->m_matWorldToCamera), paramCount, parameters );

		// Extract any clipping information.
		pImage->m_origin[0] = pImage->m_origin[1] = 0;
		pImage->m_OriginalSize[0] = pImage->m_width;
		pImage->m_OriginalSize[1] = pImage->m_height;
		TqInt count = 2;
		DspyFindIntsInParamList("origin", &count, pImage->m_origin, paramCount, parameters);
		DspyFindIntsInParamList("OriginalSize", &count, pImage->m_OriginalSize, paramCount, parameters);

		// Determine if we are creating a combined shadow map for ambient occlusion.
		float append;
		if( DspyFindFloatInParamList("append", &append, paramCount, parameters )
		        == PkDspyErrorNone )
			pImage->m_append = (append != 0.0f)? 1:0;

		char *ydesc = NULL;
		if (DspyFindStringInParamList("description", &ydesc, paramCount, parameters )
		        == PkDspyErrorNone )
		{
			// Do something about it; the user will want to add its copyright notice.
			if (ydesc && *ydesc)
				description = ydesc;
		}
	}
	else
		return(PkDspyErrorNoMemory);

	return(PkDspyErrorNone);
}


extern "C" PtDspyError DspyImageData(PtDspyImageHandle image,
                                     int xmin,
                                     int xmaxplus1,
                                     int ymin,
                                     int ymaxplus1,
                                     int entrysize,
                                     const TqUchar *data)
{
	SqDisplayInstance* pImage;
	pImage = reinterpret_cast<SqDisplayInstance*>(image);

	// If the image is not cropped, then the origin shouldn't be used.
	if(pImage->m_OriginalSize[0] == pImage->m_width && pImage->m_OriginalSize[1] == pImage->m_height)
	{
		pImage->m_origin[0] = 0;
		pImage->m_origin[1] = 0;
	}
		

	TqInt xmin__ = max((xmin-pImage->m_origin[0]), 0);
	TqInt ymin__ = max((ymin-pImage->m_origin[1]), 0);
	TqInt xmaxplus1__ = min((xmaxplus1-pImage->m_origin[0]), pImage->m_width);
	TqInt ymaxplus1__ = min((ymaxplus1-pImage->m_origin[1]), pImage->m_height);
	TqInt bucketlinelen = entrysize * (xmaxplus1 - xmin);
	TqInt copylinelen = entrysize * (xmaxplus1__ - xmin__);

	pImage->m_pixelsReceived += (xmaxplus1__-xmin__)*(ymaxplus1__-ymin__);

	// Calculate where in the bucket we are starting from if the window is cropped.
	TqInt row = max(pImage->m_origin[1] - ymin, 0);
	TqInt col = max(pImage->m_origin[0] - xmin, 0);
	const TqUchar* pdatarow = data;
	pdatarow += (row * bucketlinelen) + (col * entrysize);

	if( pImage && data && xmin__ >= 0 && ymin__ >= 0 && xmaxplus1__ <= pImage->m_width && ymaxplus1__ <= pImage->m_height )
	{
		// If rendering to a file, or an "rgb" framebuffer, we can just copy the data.
		if (pImage->m_imageType == Type_Framebuffer)
		{
			TqUint comp = entrysize/pImage->m_iFormatCount;
			TqInt y;
			TqUchar *unrolled = static_cast< TqUchar *>(pImage->m_data);

			for ( y = ymin__; y < ymaxplus1__; y++ )
			{
				TqInt x;
				TqUchar* _pdatarow = (TqUchar* )(pdatarow);
				for ( x = xmin__; x < xmaxplus1__; x++ )
				{
					TqInt so = pImage->m_iFormatCount * (( y * pImage->m_width ) +  x );

					switch (comp)
					{

							case 2 :
							{
								TqUshort *svalue = reinterpret_cast<TqUshort *>(_pdatarow);
								TqUchar alpha = 255;
								if (pImage->m_iFormatCount == 4)
								{
									alpha = (svalue[3]/256);
								}
								CompositeAlpha((TqInt) svalue[0]/256, (TqInt) svalue[1]/256, (TqInt) svalue[2]/256, 
                    unrolled[so + 0], unrolled[so + 1], unrolled[so + 2], 
		    alpha);
								if (pImage->m_iFormatCount == 4)
									unrolled[ so + 3 ] = alpha;
							}
							break;
							case 4:
							{

								TqUlong *lvalue = reinterpret_cast<TqUlong *>(_pdatarow);
								TqUchar alpha = 255;
								if (pImage->m_iFormatCount == 4)
								{
									alpha = (TqUchar) (lvalue[3]/256);
								}
								CompositeAlpha((TqInt) lvalue[0]/256, (TqInt) lvalue[1]/256, (TqInt) lvalue[2]/256, 
                    unrolled[so + 0], unrolled[so + 1], unrolled[so + 2], 
		    alpha);
								if (pImage->m_iFormatCount == 4)
									unrolled[ so + 3 ] = alpha;
							}
							break;

							case 1:
							default:
							{
								TqUchar *cvalue = reinterpret_cast<TqUchar *>(_pdatarow);
								TqUchar alpha = 255;
								if (pImage->m_iFormatCount == 4)
								{
									alpha = (TqUchar) (cvalue[3]);
								}
								CompositeAlpha((TqInt) cvalue[0], (TqInt) cvalue[1], (TqInt) cvalue[2], 
                    unrolled[so + 0], unrolled[so + 1], unrolled[so + 2], 
		    alpha);
								if (pImage->m_iFormatCount == 4)
									unrolled[ so + 3 ] = alpha;
							}
							break;
					}
					_pdatarow += entrysize;

				}
				pdatarow += bucketlinelen;
			}
		}
		else if( pImage->m_imageType != Type_Framebuffer || pImage->m_iFormatCount <= 3 )
		{
			TqInt y;
			for ( y = ymin__; y < ymaxplus1__; y++ )
			{
				// Copy a whole row at a time, as we know it is being sent in the proper format and order.
				TqInt so = ( y * pImage->m_lineLength ) + ( xmin__ * pImage->m_entrySize );
				memcpy(reinterpret_cast<char*>(pImage->m_data)+so, reinterpret_cast<const void*>(pdatarow), copylinelen);
				pdatarow += bucketlinelen;
			}
		}
		// otherwise we need to do alpha blending for the alpha data to show in the framebuffer
		else
		{
			TqInt y;
			for ( y = ymin__; y < ymaxplus1__; y++ )
			{
				TqInt x;
				const TqUchar* _pdatarow = pdatarow;
				for ( x = xmin__; x < xmaxplus1__; x++ )
				{
					TqUchar alpha = _pdatarow[3];
					if( alpha > 0 )
					{
						TqInt so = ( y * pImage->m_lineLength ) + ( x * pImage->m_entrySize );
						TqInt r = _pdatarow[0];
						TqInt g = _pdatarow[1];
						TqInt b = _pdatarow[2];
						
						TqUchar R = reinterpret_cast<TqUchar*>(pImage->m_data)[ so + 0 ];
						TqUchar G = reinterpret_cast<TqUchar*>(pImage->m_data)[ so + 1 ];
						TqUchar B = reinterpret_cast<TqUchar*>(pImage->m_data)[ so + 2 ];
						CompositeAlpha(r, g, b, R, G, B, alpha );

					}
					_pdatarow += entrysize;
				}
				pdatarow += bucketlinelen;
			}
		}

		// If rendering into a zframebuffer, we need to setup a separate image store for the displayed data.
		if(pImage->m_imageType == Type_ZFramebuffer)
		{
#ifndef AQSIS_NO_FLTK
			const TqUchar * pdatarow = data;
			pdatarow += (row * bucketlinelen) + (col * entrysize);
			TqInt y;
			for ( y = ymin__; y < ymaxplus1__; y++ )
			{
				TqInt x;
				const TqUchar* _pdatarow = pdatarow;
				for ( x = xmin; x < xmaxplus1; x++ )
				{
					TqFloat value = reinterpret_cast<const TqFloat*>(_pdatarow)[0];
					TqInt so = ( y * pImage->m_width * 3 * sizeof(TqUchar) ) + ( x * 3 * sizeof(TqUchar) );
					pImage->m_zfbdata[ so + 0 ] =
					    pImage->m_zfbdata[ so + 1 ] =
					        pImage->m_zfbdata[ so + 2 ] = value < FLT_MAX ? 255 : 0;
					_pdatarow += entrysize;
				}
				pdatarow += bucketlinelen;
			}
#endif // AQSIS_NO_FLTK

		}
	}

	if(pImage->m_imageType == Type_Framebuffer || pImage->m_imageType == Type_ZFramebuffer)
	{
#ifndef AQSIS_NO_FLTK
		pImage->m_uiImageWidget->damage(1, xmin__, ymin__, xmaxplus1__-xmin__, ymaxplus1__-ymin__);
		Fl::check();
		TqFloat percent = pImage->m_pixelsReceived / (TqFloat) (pImage->m_width * pImage->m_height);
		percent *= 100.0f;
		percent = clamp(percent, 0.0f, 100.0f);
		std::stringstream strTitle;
		if (percent < 99.9f)
			strTitle << pImage->m_filename << ": " << std::fixed << std::setprecision(1) << std::setw(5) << percent << "% complete" << std::ends;
		else
			strTitle << pImage->m_filename << std::ends;
		pImage->m_theWindow->label(strTitle.str().c_str());
#endif // AQSIS_NO_FLTK

	}
	return(PkDspyErrorNone);
}


extern "C" PtDspyError DspyImageClose(PtDspyImageHandle image)
{
	SqDisplayInstance* pImage;
	pImage = reinterpret_cast<SqDisplayInstance*>(image);

	// Write the image to disk
	if( pImage->m_imageType == Type_File ||
	        pImage->m_imageType == Type_ZFile ||
	        pImage->m_imageType == Type_Shadowmap )
		WriteTIFF( pImage->m_filename, pImage);

	// Delete the image structure.
	if (pImage->m_data)
		free(pImage->m_data);
	if(pImage->m_imageType == Type_ZFramebuffer)
		free(pImage->m_zfbdata);
	description = "";
	delete(pImage);


	return(PkDspyErrorNone);
}


extern "C" PtDspyError DspyImageDelayClose(PtDspyImageHandle image)
{
	SqDisplayInstance* pImage;
	pImage = reinterpret_cast<SqDisplayInstance*>(image);

	if(pImage && pImage->m_data)
	{
		if(pImage->m_imageType == Type_Framebuffer || pImage->m_imageType == Type_ZFramebuffer)
		{
#ifndef	AQSIS_NO_FLTK
			if( pImage->m_imageType == Type_ZFramebuffer )
			{
				// Now that we have all of our data, calculate some handy statistics ...
				TqFloat mindepth = FLT_MAX;
				TqFloat maxdepth = -FLT_MAX;
				TqUint totalsamples = 0;
				TqUint samples = 0;
				TqFloat totaldepth = 0;
				for ( TqInt i = 0; i < pImage->m_width * pImage->m_height; i++ )
				{
					totalsamples++;

					// Skip background pixels ...
					if( reinterpret_cast<const TqFloat*>(pImage->m_data)
					        [i] >= FLT_MAX )
						continue;

					mindepth = min( mindepth, reinterpret_cast<const TqFloat*>(pImage->m_data)[ i ] );
					maxdepth = max( maxdepth, reinterpret_cast<const TqFloat*>(pImage->m_data)[ i ] );

					totaldepth += reinterpret_cast<const TqFloat*>(pImage->m_data)[ i ];
					samples++;
				}

				const TqFloat dynamicrange = maxdepth - mindepth;

				/*		Aqsis::log() << info << g_Filename << " total samples: " << totalsamples << std::endl;
						Aqsis::log() << info << g_Filename << " depth samples: " << samples << std::endl;
						Aqsis::log() << info << g_Filename << " coverage: " << static_cast<TqFloat>( samples ) / static_cast<TqFloat>( totalsamples ) << std::endl;
						Aqsis::log() << info << g_Filename << " minimum depth: " << mindepth << std::endl;
						Aqsis::log() << info << g_Filename << " maximum depth: " << maxdepth << std::endl;
						Aqsis::log() << info << g_Filename << " dynamic range: " << dynamicrange << std::endl;
						Aqsis::log() << info << g_Filename << " average depth: " << totaldepth / static_cast<TqFloat>( samples ) << std::endl;
				*/

				const TqInt linelength = pImage->m_width * 3;
				for ( TqInt y = 0;
				        y < pImage->m_height;
				        y++ )
				{
					for ( TqInt x = 0; x < pImage->m_height; x++ )
					{
						const TqInt imageindex = ( y * linelength ) + ( x * 3 );
						const TqInt dataindex = ( y * pImage->m_width ) + x;

						if( reinterpret_cast<const TqFloat*>(pImage->m_data)
						        [dataindex] == FLT_MAX)
						{
							pImage->m_zfbdata[imageindex + 0] =
							    pImage->m_zfbdata[imageindex + 1] =
							        pImage->m_zfbdata[imageindex + 2] = 0;
						}
						else
						{
							const TqFloat normalized = ( reinterpret_cast<const TqFloat*>(pImage->m_data)[ dataindex ] - mindepth ) / dynamicrange;
							pImage->m_zfbdata[imageindex + 0] = static_cast<TqUchar>( 255 * ( 1.0 - normalized ) );
							pImage->m_zfbdata[imageindex + 1] = static_cast<TqUchar>( 255 * ( 1.0 - normalized ) );
							pImage->m_zfbdata[imageindex + 2] = 255;
						}
					}
				}
				pImage->m_uiImageWidget->damage(1);
				Fl::check();
			}
			Fl::run();
#endif // AQSIS_NO_FLTK

		}
		return(DspyImageClose(image));
	}
	return(PkDspyErrorNone);
}


extern "C" PtDspyError DspyImageQuery(PtDspyImageHandle image,
                                      PtDspyQueryType type,
                                      size_t size,
                                      void *data)
{
	SqDisplayInstance* pImage;
	pImage = reinterpret_cast<SqDisplayInstance*>(image);

	PtDspyOverwriteInfo overwriteInfo;
	PtDspySizeInfo sizeInfo;

	if(size <= 0 || !data)
		return PkDspyErrorBadParams;

	switch (type)
	{
			case PkOverwriteQuery:
			if ((TqUint) size > sizeof(overwriteInfo))
				size = sizeof(overwriteInfo);
			overwriteInfo.overwrite = 1;
			overwriteInfo.interactive = 0;
			memcpy(data, &overwriteInfo, size);
			break;
			case PkSizeQuery:
			if ((TqUint) size > sizeof(sizeInfo))
				size = sizeof(sizeInfo);
			if(pImage)
			{
				if(!pImage->m_width || !pImage->m_height)
				{
					pImage->m_width = 640;
					pImage->m_height = 480;
				}
				sizeInfo.width = pImage->m_width;
				sizeInfo.height = pImage->m_height;
				sizeInfo.aspectRatio = 1.0f;
			}
			else
			{
				sizeInfo.width = 640;
				sizeInfo.height = 480;
				sizeInfo.aspectRatio = 1.0f;
			}
			memcpy(data, &sizeInfo, size);
			break;
			default:
			return PkDspyErrorUnsupported;
	}

	return(PkDspyErrorNone);
}
