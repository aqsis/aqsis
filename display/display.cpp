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
		\brief Implements the default display devices for Aqsis.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include <aqsis.h>

#ifdef AQSIS_SYSTEM_WIN32
	#include <winsock2.h>
    	#define SHUT_RDWR SD_BOTH
	typedef linger LINGER;
#endif

#include <logging.h>
#include <logging_streambufs.h>
#include "sstring.h"

#include <tiffio.h>

using namespace Aqsis;

#include <iomanip>
#include <ios>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <fstream>
#include <float.h>

#ifdef	AQSIS_SYSTEM_WIN32
#pragma warning(disable : 4275 4251)
#endif

#include <boost/thread.hpp>

#ifdef AQSIS_SYSTEM_WIN32

	#include <locale>
	#include <direct.h>
	#include <algorithm>

	#define getcwd _getcwd
	#define PATH_SEPARATOR "\\"
#else // !AQSIS_SYSTEM_WIN32

	#include <stdio.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <netdb.h>
	#include <netinet/in.h>
	#include <sys/types.h>
	#include <sys/socket.h>

	typedef int SOCKET;
	typedef sockaddr_in SOCKADDR_IN;
	typedef sockaddr* PSOCKADDR;

	static const int INVALID_SOCKET = -1;
	static const int SOCKET_ERROR = -1;

	#define PATH_SEPARATOR "/"

#endif // !AQSIS_SYSTEM_WIN32

#include "tinyxml.h"
#include "base64.h"
#include "xmlmessages.h"
#include "argparse.h"

#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
#define	ZFILE_HEADER		"Aqsis ZFile" VERSION_STR
#else // AQSIS_SYSTEM_WIN32
#define ZFILE_HEADER "Aqsis ZFile" VERSION
#endif // !AQSIS_SYSTEM_WIN32
#define	SHADOWMAP_HEADER	"Shadow"

#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
#include	<version.h>
#endif

#include "display.h"

#define INT_MULT(a,b,t) ( (t) = (a) * (b) + 0x80, ( ( ( (t)>>8 ) + (t) )>>8 ) )
#define INT_PRELERP(p, q, a, t) ( (p) + (q) - INT_MULT( a, p, t) )

START_NAMESPACE( Aqsis )

enum EqDisplayTypes 
{
	Type_File = 0,
	Type_Framebuffer,
	Type_ZFile,
	Type_ZFramebuffer,
	Type_Shadowmap,
};


TqInt g_ImageWidth = 0;
TqInt g_ImageHeight = 0;
TqInt g_PixelsProcessed = 0;
TqInt g_Channels = 0;
TqInt g_offset = 0;
TqInt g_ElementSize = 0;
TqInt g_CWXmin, g_CWYmin;
TqInt g_CWXmax, g_CWYmax;
TqFloat g_QuantizeZeroVal = 0.0f;
TqFloat g_QuantizeOneVal = 0.0f;
TqFloat g_QuantizeMinVal = 0.0f;
TqFloat g_QuantizeMaxVal = 0.0f;
TqFloat g_QuantizeDitherVal = 0.0f;
TqFloat g_appliedQuantizeOneVal = 0.0f;
TqFloat g_appliedQuantizeMinVal = 0.0f;
TqFloat g_appliedQuantizeMaxVal = 0.0f;
TqFloat g_appliedQuantizeDitherVal = 0.0f;
uint16	g_Compression = COMPRESSION_NONE, g_Quality = 0;
TqInt	g_BucketsPerCol, g_BucketsPerRow;
TqInt	g_BucketWidthMax, g_BucketHeightMax;
TqBool	g_RenderWholeFrame = TqFalse;
TqInt	g_ImageType;
TqInt	g_append = 0;
TqFloat	g_matWorldToCamera[ 4 ][ 4 ];
TqFloat	g_matWorldToScreen[ 4 ][ 4 ];

unsigned char* g_byteData;
float*	g_floatData;

Fl_Window *g_theWindow;
Fl_FrameBuffer_Widget *g_uiImageWidget;
Fl_RGB_Image* g_uiImage;

// Command line arguments 
ArgParse::apstring g_type("file");
ArgParse::apstring g_mode("rgba");
ArgParse::apstring g_filename("output.tif");
ArgParse::apstring g_hostname;
ArgParse::apstring g_port;
ArgParse::apstringvec g_paramNames;
ArgParse::apintvec g_paramCounts;
ArgParse::apintvec g_paramInts;
ArgParse::apfloatvec g_paramFloats;
ArgParse::apstringvec g_paramStrings;
bool g_help = 0;

/// Hides incompatibilities between sockets and WinSock
void InitializeSockets()
{
#ifdef AQSIS_SYSTEM_WIN32
	WSADATA wsaData;
	WSAStartup( MAKEWORD( 2, 0 ), &wsaData );
#endif // AQSIS_SYSTEM_WIN32
}

/// Hides incompatibilities amongst platforms
hostent* GetHostByName(const std::string& HostName)
{
#ifdef AQSIS_SYSTEM_MACOSX
	// Remove this conditional section and use gethostbyname() if Apple ever
	// fixes the problem of resolving localhost without a connection to a DNS server
	return gethostent();	// assumes localhost defined first in /etc/hosts
#else // AQSIS_SYSTEM_MACOSX
	return gethostbyname( HostName.c_str() );
#endif // !AQSIS_SYSTEM_MACOSX
}

static void CloseSocket( SOCKET& Socket )
{
    int x = 1;
    linger ling;
    ling.l_onoff = 1;
    ling.l_linger = 10;
    setsockopt( Socket, SOL_SOCKET, SO_LINGER, reinterpret_cast<const char*>( &ling ), sizeof( ling ) );
    shutdown( Socket, SHUT_RDWR );
#ifndef	AQSIS_SYSTEM_WIN32
	close( Socket );
#else	// AQSIS_SYSTEM_WIN32
	closesocket( Socket );
#endif	// AQSIS_SYSTEM_WIN32

    Socket = INVALID_SOCKET;
}

const std::string GetCurrentWorkingDirectory()
{
    std::string result(1024, '\0');
    getcwd(const_cast<char*>(result.c_str()), result.size());
    result.resize(strlen(result.c_str()));

    return result;
}

const std::string AppendPath(const std::string& LHS, const std::string& RHS)
{
    return LHS + PATH_SEPARATOR + RHS;
}


void SaveAsShadowMap(const std::string& filename)
{
    TqChar version[ 80 ];
    TqInt twidth = 32;
    TqInt tlength = 32;

    const char* mode = (g_append)? "a" : "w";

    // Save the shadowmap to a binary file.
    if ( filename.compare( "" ) != 0 )
    {
        TIFF * pshadow = TIFFOpen( filename.c_str(), mode );
		if( pshadow != NULL )
		{
			// Set common tags
			TqInt XRes = ( g_CWXmax - g_CWXmin );
			TqInt YRes = ( g_CWYmax - g_CWYmin );

			TIFFCreateDirectory( pshadow );

	#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
			sprintf( version, "%s %s", STRNAME, VERSION_STR );
	#else
			sprintf( version, "%s %s", STRNAME, VERSION );
	#endif
			TIFFSetField( pshadow, TIFFTAG_SOFTWARE, ( uint32 ) version );
			TIFFSetField( pshadow, TIFFTAG_PIXAR_MATRIX_WORLDTOCAMERA, g_matWorldToCamera );
			TIFFSetField( pshadow, TIFFTAG_PIXAR_MATRIX_WORLDTOSCREEN, g_matWorldToScreen );
			TIFFSetField( pshadow, TIFFTAG_PIXAR_TEXTUREFORMAT, SHADOWMAP_HEADER );
			TIFFSetField( pshadow, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK );

			// Write the floating point image to the directory.
		#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
			sprintf( version, "%s %s", STRNAME, VERSION_STR );
		#else
			sprintf( version, "%s %s", STRNAME, VERSION );
		#endif
			TIFFSetField( pshadow, TIFFTAG_SOFTWARE, ( uint32 ) version );
			TIFFSetField( pshadow, TIFFTAG_IMAGEWIDTH, XRes );
			TIFFSetField( pshadow, TIFFTAG_IMAGELENGTH, YRes );
			TIFFSetField( pshadow, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG );
			TIFFSetField( pshadow, TIFFTAG_BITSPERSAMPLE, 32 );
			TIFFSetField( pshadow, TIFFTAG_SAMPLESPERPIXEL, g_Channels );
			TIFFSetField( pshadow, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
			TIFFSetField( pshadow, TIFFTAG_TILEWIDTH, twidth );
			TIFFSetField( pshadow, TIFFTAG_TILELENGTH, tlength );
			TIFFSetField( pshadow, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP );
			TIFFSetField( pshadow, TIFFTAG_COMPRESSION, g_Compression );


			TqInt tsize = twidth * tlength;
			TqInt tperrow = ( XRes + twidth - 1 ) / twidth;
			TqFloat* ptile = static_cast<TqFloat*>( _TIFFmalloc( tsize * g_Channels * sizeof( TqFloat ) ) );

			if ( ptile != NULL )
			{
				TqInt ctiles = tperrow * ( ( YRes + tlength - 1 ) / tlength );
				TqInt itile;
				for ( itile = 0; itile < ctiles; itile++ )
				{
					TqInt x = ( itile % tperrow ) * twidth;
					TqInt y = ( itile / tperrow ) * tlength;
					TqFloat* ptdata = g_floatData + ( ( y * XRes ) + x ) * g_Channels;
					// Clear the tile to black.
					memset( ptile, 0, tsize * g_Channels * sizeof( TqFloat ) );
					for ( TqUlong i = 0; i < tlength; i++ )
					{
						for ( TqUlong j = 0; j < twidth; j++ )
						{
							if ( ( x + j ) < XRes && ( y + i ) < YRes )
							{
								TqInt ii;
								for ( ii = 0; ii < g_Channels; ii++ )
									ptile[ ( i * twidth * g_Channels ) + ( ( ( j * g_Channels ) + ii ) ) ] = ptdata[ ( ( j * g_Channels ) + ii ) ];
							}
						}
						ptdata += ( XRes * g_Channels );
					}
					TIFFWriteTile( pshadow, ptile, x, y, 0, 0 );
				}
				TIFFWriteDirectory( pshadow );

			}

			TIFFClose( pshadow );
		}
    }
}


void WriteTIFF(const std::string& filename)
{
    uint16 photometric = PHOTOMETRIC_RGB;
    uint16 config = PLANARCONFIG_CONTIG;

    // Set common tags
	TqInt XRes = ( g_CWXmax - g_CWXmin );
	TqInt YRes = ( g_CWYmax - g_CWYmin );

    // If in "shadowmap" mode, write as a shadowmap.
	if( g_ImageType == Type_Shadowmap )
	{
		SaveAsShadowMap(filename);
		return;
	}
    else if( g_ImageType == Type_ZFile )
    {
        std::ofstream ofile( filename.c_str(), std::ios::out | std::ios::binary );
        if ( ofile.is_open() )
        {
            // Save a file type and version marker
            ofile << ZFILE_HEADER;

            // Save the xres and yres.
            ofile.write( reinterpret_cast<char* >( &XRes ), sizeof( XRes ) );
            ofile.write( reinterpret_cast<char* >( &YRes ), sizeof( XRes ) );

            // Save the transformation matrices.
            ofile.write( reinterpret_cast<char*>( g_matWorldToCamera[ 0 ] ), sizeof( g_matWorldToCamera[ 0 ][ 0 ] ) * 4 );
            ofile.write( reinterpret_cast<char*>( g_matWorldToCamera[ 1 ] ), sizeof( g_matWorldToCamera[ 0 ][ 0 ] ) * 4 );
            ofile.write( reinterpret_cast<char*>( g_matWorldToCamera[ 2 ] ), sizeof( g_matWorldToCamera[ 0 ][ 0 ] ) * 4 );
            ofile.write( reinterpret_cast<char*>( g_matWorldToCamera[ 3 ] ), sizeof( g_matWorldToCamera[ 0 ][ 0 ] ) * 4 );

            ofile.write( reinterpret_cast<char*>( g_matWorldToScreen[ 0 ] ), sizeof( g_matWorldToScreen[ 0 ][ 0 ] ) * 4 );
            ofile.write( reinterpret_cast<char*>( g_matWorldToScreen[ 1 ] ), sizeof( g_matWorldToScreen[ 0 ][ 0 ] ) * 4 );
            ofile.write( reinterpret_cast<char*>( g_matWorldToScreen[ 2 ] ), sizeof( g_matWorldToScreen[ 0 ][ 0 ] ) * 4 );
            ofile.write( reinterpret_cast<char*>( g_matWorldToScreen[ 3 ] ), sizeof( g_matWorldToScreen[ 0 ][ 0 ] ) * 4 );

            // Now output the depth values
            ofile.write( reinterpret_cast<char*>( g_floatData ), sizeof( TqFloat ) * ( XRes * YRes ) );
            ofile.close();
        }
		return;
    }
	
	TIFF* pOut = TIFFOpen( filename.c_str(), "w" );

    if ( pOut )
    {
        // Write the image to a tiff file.
        char version[ 80 ];

        int ExtraSamplesTypes[ 1 ] = {EXTRASAMPLE_ASSOCALPHA};

#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
        sprintf( version, "%s %s", STRNAME, VERSION_STR );
#else
        sprintf( version, "%s %s", STRNAME, VERSION );
#endif

        bool use_logluv = false;

        TIFFSetField( pOut, TIFFTAG_SOFTWARE, ( uint32 ) version );
        TIFFSetField( pOut, TIFFTAG_IMAGEWIDTH, ( uint32 ) XRes );
        TIFFSetField( pOut, TIFFTAG_IMAGELENGTH, ( uint32 ) YRes );
        TIFFSetField( pOut, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
        TIFFSetField( pOut, TIFFTAG_SAMPLESPERPIXEL, g_Channels );

        // Write out an 8 bits per pixel integer image.
        if ( g_appliedQuantizeOneVal == 255 )
        {
            TIFFSetField( pOut, TIFFTAG_BITSPERSAMPLE, 8 );
            TIFFSetField( pOut, TIFFTAG_PLANARCONFIG, config );
            TIFFSetField( pOut, TIFFTAG_COMPRESSION, g_Compression );
            if ( g_Compression == COMPRESSION_JPEG )
                TIFFSetField( pOut, TIFFTAG_JPEGQUALITY, g_Quality );
            //if (description != "")
            //TIFFSetField(TIFFTAG_IMAGEDESCRIPTION, description);
            TIFFSetField( pOut, TIFFTAG_PHOTOMETRIC, photometric );
            TIFFSetField( pOut, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize( pOut, 0 ) );

            if ( g_Channels == 4 )
                TIFFSetField( pOut, TIFFTAG_EXTRASAMPLES, 1, ExtraSamplesTypes );

            // Set the position tages in case we aer dealing with a cropped image.
            TIFFSetField( pOut, TIFFTAG_XPOSITION, ( float ) g_CWXmin );
            TIFFSetField( pOut, TIFFTAG_YPOSITION, ( float ) g_CWYmin );

            TqInt	linelen = g_ImageWidth * g_Channels;
            TqInt row;
            for ( row = 0; row < YRes; row++ )
            {
                if ( TIFFWriteScanline( pOut, g_byteData + ( row * linelen ), row, 0 ) < 0 )
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
                TIFFSetField( pOut, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_INT );
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
                TIFFSetField( pOut, TIFFTAG_COMPRESSION, g_Compression );
            }

            TIFFSetField( pOut, TIFFTAG_SAMPLESPERPIXEL, g_Channels );

            if ( g_Channels == 4 )
                TIFFSetField( pOut, TIFFTAG_EXTRASAMPLES, 1, ExtraSamplesTypes );
            // Set the position tages in case we aer dealing with a cropped image.
            TIFFSetField( pOut, TIFFTAG_XPOSITION, ( float ) g_CWXmin );
            TIFFSetField( pOut, TIFFTAG_YPOSITION, ( float ) g_CWYmin );
            TIFFSetField( pOut, TIFFTAG_PLANARCONFIG, config );

            TqInt	linelen = g_ImageWidth * g_Channels;
            TqInt row = 0;
            for ( row = 0; row < YRes; row++ )
            {
                if ( TIFFWriteScanline( pOut, g_floatData + ( row * linelen ), row, 0 ) < 0 )
                    break;
            }
            TIFFClose( pOut );
        }
    }
}


SOCKET g_Socket;


void BucketFunction()
{
    TqInt	linelen = g_ImageWidth * g_Channels ;
    TqInt	flinelen = g_ImageWidth * 3;
	TqInt	dataLenMax = g_BucketWidthMax * g_BucketHeightMax * g_ElementSize;
	std::vector<TqFloat> dataBin(dataLenMax);

	TqInt iBucket;
	for(iBucket = 0; iBucket < (g_BucketsPerCol * g_BucketsPerRow); iBucket++)
//	for(iBucket = (g_BucketsPerCol * g_BucketsPerRow ) -1; iBucket >= 0 ; iBucket--)
	{
		// Request the next bucket.
		TiXmlDocument doc;
		TiXmlElement root("aqsis:request");
		TiXmlElement bucket("aqsis:bucket");
		bucket.SetAttribute("index", ToString(iBucket).c_str());
		root.InsertEndChild(bucket);
		doc.InsertEndChild(root);
		std::ostringstream strReq;
		strReq << doc;

		sendXMLMessage(g_Socket, strReq.str().c_str());

		// Wait for a response.
		char* resp = receiveXMLMessage(g_Socket);

		// Parse the response
		TiXmlDocument docResp;
		docResp.Parse(resp);
		//free(resp);

		// Extract the format attributes.
		TiXmlHandle respHandle(&docResp);
		TiXmlElement* bucketElement = respHandle.FirstChild("aqsis:response").FirstChild("aqsis:bucket").Element();
		if( bucketElement )
		{
			TqInt xmin, ymin, xmaxp1, ymaxp1;
			if( (bucketElement->QueryIntAttribute("xmin", &xmin) == TIXML_SUCCESS) && 
				(bucketElement->QueryIntAttribute("ymin", &ymin) == TIXML_SUCCESS) && 
				(bucketElement->QueryIntAttribute("xmaxp1", &xmaxp1) == TIXML_SUCCESS) && 
				(bucketElement->QueryIntAttribute("ymaxp1", &ymaxp1) == TIXML_SUCCESS) )
			{
				TiXmlNode* dataNode = bucketElement->FirstChild();
				TiXmlText* data = dataNode->ToText();
				if(data)
				{
					b64_decode(reinterpret_cast<char*>(&dataBin[0]), data->Value());
					TqInt dataOffset=0;
					TqInt t;
					TqFloat alpha = 255.0f;
					TqInt y;
					// Choose the start/end coordinates depending on if in "file" mode or not,
					// If rendering to a framebuffer, show whole frame and render into crop window.
					TqInt ystart = (g_RenderWholeFrame)? ymin : ymin - g_CWYmin;
					TqInt yend = (g_RenderWholeFrame)? ymaxp1 : ymaxp1 - g_CWYmin;
					TqBool use_alpha = g_mode.compare("rgba")==0;
					for ( y = ystart; y < yend; y++ )
					{
						TqInt x;
						// Choose the start/end coordinates depending on if in "file" mode or not,
						// If rendering to a framebuffer, show whole frame and render into crop window.
						TqInt xstart = (g_RenderWholeFrame)? xmin : xmin - g_CWXmin;
						TqInt xend = (g_RenderWholeFrame)? xmaxp1 : xmaxp1 - g_CWXmin;
						for ( x = xstart; x < xend; x++ )
						{
							if ( x >= 0 && y >= 0 && x < g_ImageWidth && y < g_ImageHeight )
							{
								TqInt so = ( y * linelen ) + ( x * g_Channels );

								TqInt i = 0;
								if(use_alpha)
									alpha = dataBin[3 + g_offset + dataOffset];
								while ( i < g_Channels )
								{
									TqFloat value = dataBin[i + g_offset + dataOffset];

									if( !( g_QuantizeZeroVal == 0.0f &&
										   g_QuantizeOneVal  == 0.0f &&
										   g_QuantizeMinVal  == 0.0f &&
										   g_QuantizeMaxVal  == 0.0f ) )
									{
										value = ROUND(g_QuantizeZeroVal + value * (g_QuantizeOneVal - g_QuantizeZeroVal) + g_QuantizeDitherVal );
										value = CLAMP(value, g_QuantizeMinVal, g_QuantizeMaxVal) ;
									}

									// If rendering 8bit, or working as a framebuffer, update the byte array.
									if( g_appliedQuantizeOneVal == 255 || g_ImageType == Type_Framebuffer || g_ImageType == Type_ZFramebuffer)
									{
										if( NULL != g_byteData )
										{
											if( g_ImageType != Type_ZFramebuffer )
											{
												// C’ = INT_PRELERP( A’, B’, b, t )
												if( alpha > 0 )
												{
													int A = static_cast<int>(INT_PRELERP( g_byteData[ so + 0 ], value, alpha, t ));
													g_byteData[ so ] = CLAMP( A, 0, 255 );
												}
											}
											else
											{
												// Fill in the framebuffer data for a zframebuffer.
												// Initially, any surface gives white, then we quantize at the end.
												TqInt fso = ( y * flinelen ) + ( x * 3 );
												g_byteData[ fso + 0 ] = 
												g_byteData[ fso + 1 ] = 
												g_byteData[ fso + 2 ] = value < FLT_MAX ? 255 : 0;
											}
										}
										// If a depth based framebuffer, then we need to update the float data too.
										if( g_ImageType == Type_ZFramebuffer )
											if( NULL != g_floatData )
												g_floatData[ so ] = value;
									}
									else
									{
										if( NULL != g_floatData )
											g_floatData[ so ] = value;
									}
									so++;
									i++;
								}
							}
							dataOffset += g_ElementSize;
						}
					}
				}
			}
			else
			{
				std::cerr << "Error: Invalid response from Aqsis1" << std::endl;
				exit(-1);
			}

			if(g_ImageType == Type_Framebuffer || g_ImageType == Type_ZFramebuffer)
			{
				g_uiImageWidget->damage(1, xmin, ymin, xmaxp1-xmin, ymaxp1-ymin);
				Fl::check();
			}
		}
		else
		{
			std::cerr << "Error: Invalid response from Aqsis2" << std::endl;
			exit(-1);
		}
		free(resp);
	}
	if(g_ImageType == Type_File || g_ImageType == Type_ZFile || g_ImageType == Type_Shadowmap )
	{
		WriteTIFF(g_filename);
	}

	// If we are in depth framebuffer mode, quantize the final data now to give a better visual
	// representation of the depth buffer.
    // Normalize the depth values to the range [0, 1] and regenerate our image ..
	if(g_ImageType == Type_ZFramebuffer )
	{
		// Now that we have all of our data, calculate some handy statistics ...
		TqFloat mindepth = FLT_MAX;
		TqFloat maxdepth = -FLT_MAX;
		TqUint totalsamples = 0;
		TqUint samples = 0;
		TqFloat totaldepth = 0;
		for ( TqInt i = 0; i < g_ImageWidth * g_ImageHeight; i++ )
		{
			totalsamples++;

			// Skip background pixels ...
			if ( g_floatData[ i ] >= FLT_MAX )
				continue;

			mindepth = std::min( mindepth, g_floatData[ i ] );
			maxdepth = std::max( maxdepth, g_floatData[ i ] );

			totaldepth += g_floatData[ i ];
			samples++;
		}

		const TqFloat dynamicrange = maxdepth - mindepth;

/*		std::cerr << info << g_Filename << " total samples: " << totalsamples << std::endl;
		std::cerr << info << g_Filename << " depth samples: " << samples << std::endl;
		std::cerr << info << g_Filename << " coverage: " << static_cast<TqFloat>( samples ) / static_cast<TqFloat>( totalsamples ) << std::endl;
		std::cerr << info << g_Filename << " minimum depth: " << mindepth << std::endl;
		std::cerr << info << g_Filename << " maximum depth: " << maxdepth << std::endl;
		std::cerr << info << g_Filename << " dynamic range: " << dynamicrange << std::endl;
		std::cerr << info << g_Filename << " average depth: " << totaldepth / static_cast<TqFloat>( samples ) << std::endl;
*/
		const TqInt linelength = g_ImageWidth * 3;
		for ( TqInt y = 0; y < g_ImageHeight; y++ )
		{
			for ( TqInt x = 0; x < g_ImageWidth; x++ )
			{
				const TqInt imageindex = ( y * linelength ) + ( x * 3 );
				const TqInt dataindex = ( y * g_ImageWidth ) + x;

				if ( g_floatData[ dataindex ] == FLT_MAX )
				{
					g_byteData[ imageindex + 0 ] = g_byteData[ imageindex + 1 ] = g_byteData[ imageindex + 2 ] = 0;
				}
				else
				{
					const TqFloat normalized = ( g_floatData[ dataindex ] - mindepth ) / dynamicrange;
					g_byteData[ imageindex + 0 ] = static_cast<char>( 255 * ( 1.0 - normalized ) );
					g_byteData[ imageindex + 1 ] = static_cast<char>( 255 * ( 1.0 - normalized ) );
					g_byteData[ imageindex + 2 ] = 255;
				}
			}
		}
		g_uiImageWidget->damage(1);
		Fl::check();
	}

	CloseSocket(g_Socket);
}


void ReadXMLMatrix(TiXmlElement* pmatrixElem, TqFloat mat[4][4])
{
	// Read the row elements one at a time, processing each column element, translating its text to a float
	// entry in the matrix.
	TqInt row, col;
	TiXmlHandle rowHandle = pmatrixElem->FirstChildElement("aqsis:matr");
	TiXmlElement* rowElem = rowHandle.Element();
	for( row = 0; row < 4 && rowElem; row++, rowElem=rowElem->NextSiblingElement("aqsis:matr") )
	{
		TiXmlHandle colHandle = rowElem->FirstChildElement("aqsis:matc");
		TiXmlElement* colElem = colHandle.Element();
		for( col = 0; col < 4 && colElem; col++, colElem=colElem->NextSiblingElement("aqsis:matc") )
		{
			TiXmlHandle valHandle = colElem->FirstChild();
			TiXmlText* val = valHandle.Text();
			if( val )
				mat[row][col] = atof(val->Value());
			else
				mat[row][col] = 0.0f;
		}
	}
}


void ProcessFormat()
{
	// Request the image format.
	TiXmlDocument doc, respDoc;
	TiXmlElement root("aqsis:request");
	TiXmlElement format("aqsis:format");
	root.InsertEndChild(format);
	doc.InsertEndChild(root);
	std::ostringstream strReq;
	strReq << doc;

	sendXMLMessage(g_Socket, strReq.str().c_str());

	// Wait for a response.
	char* resp = receiveXMLMessage(g_Socket);

	// Parse the response
	respDoc.Parse(resp);
	
	free(resp);
	
	// Extract the format attributes.
	TiXmlHandle respHandle(&respDoc);
	TiXmlHandle formatHandle = respHandle.FirstChildElement("aqsis:response").FirstChildElement("aqsis:format");
	TiXmlElement* formatElement = formatHandle.Element();
	if( formatElement )
	{
		TiXmlAttribute* pAttr = formatElement->FirstAttribute();
		formatElement->QueryIntAttribute("xres", &g_ImageWidth);
		formatElement->QueryIntAttribute("yres", &g_ImageHeight);
		formatElement->QueryIntAttribute("cropxmin", &g_CWXmin);
		formatElement->QueryIntAttribute("cropymin", &g_CWYmin);
		formatElement->QueryIntAttribute("cropxmax", &g_CWXmax);
		formatElement->QueryIntAttribute("cropymax", &g_CWYmax);
		formatElement->QueryIntAttribute("bucketsperrow", &g_BucketsPerRow);
		formatElement->QueryIntAttribute("bucketspercol", &g_BucketsPerCol);
		formatElement->QueryIntAttribute("bucketwidthmax", &g_BucketWidthMax);
		formatElement->QueryIntAttribute("bucketheightmax", &g_BucketHeightMax);
		formatElement->QueryIntAttribute("elementsize", &g_ElementSize);

		// Find the appropriate data if using AOV.
		std::string dataName("rgba");
		if(g_mode.compare("rgb")!=0 && g_mode.compare("rgba")!=0 && g_mode.compare("a")!=0)
			dataName = g_mode;
		else
		{
			// Choose out of "rgb", "rgba", and "a"
			if(g_mode.find("rgb")!=g_mode.npos)
			{
				g_Channels=3;
				if(g_mode.find("a")!=g_mode.npos)
					g_Channels=4;
			}
			else
			{
				g_offset = 3;
				g_Channels = 1;
			}
		}

		TiXmlHandle dataListHandle = formatHandle.FirstChildElement("aqsis:datalist");
		TiXmlElement* pdataelem = dataListHandle.FirstChildElement("aqsis:dataelement").Element();
		int offset = 0;
		TqBool found = TqFalse;
		// Loop through available data, looking for the chosen data.
		for( pdataelem; pdataelem; pdataelem=pdataelem->NextSiblingElement("aqsis:dataelement") )
		{
			const char* pname = pdataelem->Attribute("name");
			int size;
			pdataelem->QueryIntAttribute("size", &size);
			if(pname!=0 && dataName.compare(pname)==0)
			{
				found = TqTrue;
				g_offset = offset;
				g_Channels = size;

				// Read quantization data if specified to determine the output format.
				TiXmlNode* pappliedquant_node = pdataelem->FirstChildElement("aqsis:appliedquant");
				if( pappliedquant_node )
				{
					TiXmlElement* pappliedquant = pappliedquant_node->ToElement();

					double temp;
					pappliedquant->QueryDoubleAttribute("one", &temp);
					g_appliedQuantizeOneVal = temp;
					pappliedquant->QueryDoubleAttribute("min", &temp);
					g_appliedQuantizeMinVal = temp;
					pappliedquant->QueryDoubleAttribute("max", &temp);
					g_appliedQuantizeMaxVal = temp;
					pappliedquant->QueryDoubleAttribute("dither", &temp);
					g_appliedQuantizeDitherVal = temp;
				}

				break;
			}
			offset += size;
		}
		if(!found)
			std::cout << "Could not find element " << g_mode.c_str() << std::endl;

		// Read out the matrices for world to camera and  world to screen
		TiXmlHandle matricesListHandle = formatHandle.FirstChildElement("aqsis:matrixlist");
		TiXmlElement* pmatrixElem = matricesListHandle.FirstChildElement("aqsis:matrix").Element();
		for( pmatrixElem; pmatrixElem; pmatrixElem=pmatrixElem->NextSiblingElement("aqsis:matrix") )
		{
			const char* type = pmatrixElem->Attribute("type");
			// Check the type
			if(type && strcmp(type, "worldtocamera")==0)
				ReadXMLMatrix(pmatrixElem, g_matWorldToCamera);
			else if(type && strcmp(type, "worldtoscreen")==0)
				ReadXMLMatrix(pmatrixElem, g_matWorldToScreen);
		}

		g_PixelsProcessed = 0;

		// Create and initialise a byte array if rendering 8bit image, or we are in framebuffer mode
		if(g_appliedQuantizeOneVal == 255 || g_ImageType == Type_Framebuffer || g_ImageType == Type_ZFramebuffer)
		{
			// For a normal image, allocate the same size as the number of channels.
	        if(g_ImageType != Type_ZFramebuffer)
			{
				g_byteData = new unsigned char[ g_ImageWidth * g_ImageHeight * g_Channels ];
				memset(g_byteData, 0, g_ImageWidth * g_ImageHeight * g_Channels);
			}
			else
			// For a depth framebuffer, always allocate 3 channels, for rgb.
			{
				g_byteData = new unsigned char[ g_ImageWidth * g_ImageHeight * 3 ];
				memset(g_byteData, 0, g_ImageWidth * g_ImageHeight * 3);
			}

			// If working as a framebuffer, initialise the display to a checkerboard to show alpha
			if(g_ImageType == Type_Framebuffer)
			{
				for (TqInt i = g_CWYmin; i < g_CWYmax; i ++) 
				{
					for (TqInt j=g_CWXmin; j < g_CWXmax; j++)
					{
						int     t       = 0;
						unsigned char d = 255;

						if ( ( (g_ImageHeight - 1 - i) & 31 ) < 16 ) t ^= 1;
						if ( ( j & 31 ) < 16 ) t ^= 1;

						if ( t )
						{
							d      = 128;
						}
						g_byteData[g_Channels * (i*g_ImageWidth + j) ] = d;
						g_byteData[g_Channels * (i*g_ImageWidth + j) + 1] = d;
						g_byteData[g_Channels * (i*g_ImageWidth + j) + 2] = d;
					}
				}
			}
			// If rendering a depth framebuffer, we will need a float buffer too.
			else if(g_ImageType == Type_ZFramebuffer)
	            g_floatData = new float[ g_ImageWidth * g_ImageHeight * g_Channels ];
		}
        else
            g_floatData = new float[ g_ImageWidth * g_ImageHeight * g_Channels ];
	}
	else
	{
		std::cerr << "Error: Invalid response from Aqsis" << std::endl;
	}
}


END_NAMESPACE( Aqsis )

int main( int argc, char** argv )
{
    ArgParse ap;
    ap.usageHeader( ArgParse::apstring( "Usage: " ) + argv[ 0 ] + " [options]" );
    ap.argFlag( "help", "\aprint this help and exit", &g_help );
    ap.argString( "type", "=string\adefine the type of display", &g_type );
    ap.argString( "mode", "=string\adefine the data that the display will process", &g_mode );
    ap.argString( "name", "=string\athe name of the file to save", &g_filename );
    ap.argString( "hostname", "=string\ahostname of the machine to connect to", &g_hostname );
    ap.argString( "port", "=string\aport to connect to", &g_port );
    ap.argStrings( "paramnames", "=string array\n\aarray of custom parameter names, separated with ','", &g_paramNames, ',' );
    ap.argInts( "paramcounts", "=int array\acustom parameter counts. \n\aThree per parameter, number of ints, \n\anumber of floats and \n\anumber of strings, separated with ','", &g_paramCounts, ',' );
    ap.argInts( "paramints", "=int array\acustom parameter integer values, separated with ','", &g_paramInts, ',' );
    ap.argFloats( "paramfloats", "=float array\acustom parameter float values, separated with ','", &g_paramFloats, ',' );
    ap.argStrings( "paramstrings", "=string array\acustom parameter string values, separated with ','", &g_paramStrings, ',' );
    ap.allowUnrecognizedOptions();

    if ( argc > 1 && !ap.parse( argc - 1, const_cast<const char**>(argv + 1) ) )
    {
        std::cerr << ap.errmsg() << std::endl << ap.usagemsg();
        exit( 1 );
    }

    if ( g_help )
    {
        std::cout << ap.usagemsg();
        exit( 0 );
    }

	// Determine the display type from the list that we support.
	if(g_type.compare("file")==0 || g_type.compare("tiff")==0)
		g_ImageType = Type_File;
	else if(g_type.compare("framebuffer")==0)
		g_ImageType = Type_Framebuffer;
	else if(g_type.compare("zfile")==0)
		g_ImageType = Type_ZFile;
	else if(g_type.compare("zframebuffer")==0)
		g_ImageType = Type_ZFramebuffer;
	else if(g_type.compare("shadow")==0)
		g_ImageType = Type_Shadowmap;
	
	// If rendering to a framebuffer, default to showing the whole frame and render to the clip window.
	if(g_ImageType == Type_Framebuffer || g_ImageType == Type_Framebuffer)
		g_RenderWholeFrame = TqTrue;

	// Extract the recognised custom parameters
	if( g_paramNames.size() > 0 )
	{
		// Must have three counts per parameter.
		assert(g_paramCounts.size() >= g_paramNames.size()*3);

		ArgParse::apstringvec::iterator i;
		TqInt countsindex = 0;
		TqInt intsindex = 0;
		TqInt floatsindex = 0;
		TqInt stringsindex = 0;
		for(i = g_paramNames.begin(); i!=g_paramNames.end(); i++)
		{
			if( i->compare("quantize") == 0 )
			{
				assert( g_paramCounts[countsindex] == 0 && g_paramCounts[countsindex+1] == 4 && g_paramCounts[countsindex+2] == 0 );
				g_QuantizeZeroVal = g_paramFloats[floatsindex];
				g_QuantizeOneVal = g_paramFloats[floatsindex+1];
				g_QuantizeMinVal = g_paramFloats[floatsindex+2];
				g_QuantizeMaxVal = g_paramFloats[floatsindex+3];
			}
			else if( i->compare("dither") == 0 )
			{
				assert( g_paramCounts[countsindex] == 0 && g_paramCounts[countsindex+1] == 1 && g_paramCounts[countsindex+2] == 0 );
				g_QuantizeDitherVal = g_paramFloats[floatsindex];
			}
			else if( i->compare("compression") == 0 )
			{
				assert( g_paramCounts[countsindex] == 0 && g_paramCounts[countsindex+1] == 0 && g_paramCounts[countsindex+2] == 1 );
                ArgParse::apstring comp = g_paramStrings[stringsindex];
				if ( comp.compare("none") == 0 )
                    g_Compression = COMPRESSION_NONE;
				else if ( comp.compare("lzw") == 0 )
                    g_Compression = COMPRESSION_LZW;
				else if ( comp.compare("deflate") == 0 )
                    g_Compression = COMPRESSION_DEFLATE;
				else if ( comp.compare("jpeg") == 0 )
                    g_Compression = COMPRESSION_JPEG;
				else if ( comp.compare("packbits") == 0 )
                    g_Compression = COMPRESSION_PACKBITS;
            }
            else if( i->compare("quality") == 0 )
            {
				assert( g_paramCounts[countsindex] == 1 && g_paramCounts[countsindex+1] == 0 && g_paramCounts[countsindex+2] == 0 );
                g_Quality = g_paramInts[intsindex];
                g_Quality = CLAMP(g_Quality, 0, 100);
			}
            else if( i->compare("append") == 0 )
            {
				assert( g_paramCounts[countsindex] == 1 && g_paramCounts[countsindex+1] == 0 && g_paramCounts[countsindex+2] == 0 );
                g_append = g_paramInts[intsindex];
            }
			intsindex += g_paramCounts[countsindex++];
			floatsindex += g_paramCounts[countsindex++];
			stringsindex += g_paramCounts[countsindex++];
		}
	}

    /// Port is defined as the value passed into the -port= command line argument.
	/// if that is empty, then the value stored in the environment variable AQSIS_DD_PORT,
	/// if that is not available, then the fallback is 2774 ('A', 'Q', 'S', 'I' 'S' on phone keypad)
	int port = 27747;
	if(g_port.empty())
	{
	    const char* port_string_env = getenv( "AQSIS_DD_PORT" );
		if( port_string_env )
			port = atoi(port_string_env);
	}
	else
		port = atoi(g_port.c_str());

	InitializeSockets();

	// Open a socket.
	g_Socket = socket( AF_INET, SOCK_STREAM, 0 );
	if ( g_Socket == INVALID_SOCKET )
	{
		std::cerr << "Aqsis display: Error creating socket" << std::endl;
		return -1;
	}

    /// Host name is the value passed into the -host= command line argument,
	/// or the local host.
	if(g_hostname.empty())
	{
		g_hostname.resize(256);
		gethostname( &g_hostname[0], g_hostname.size() );
	}
		
    hostent* const pHost = GetHostByName(g_hostname);

    SOCKADDR_IN saTemp;
    memset( &saTemp, 0, sizeof( saTemp ) );
    saTemp.sin_family = AF_INET;
        
    saTemp.sin_port = htons( port );
    memcpy( &saTemp.sin_addr, pHost->h_addr, pHost->h_length );

	if(SOCKET_ERROR == connect( g_Socket, PSOCKADDR(&saTemp), sizeof(saTemp)))
	{
		int error = WSAGetLastError();
		std::cerr << "Connecting to " << g_hostname.c_str() << ":" << port << " ... " << error << std::endl;
		CloseSocket(g_Socket);
		return -1;
	}

	// Request and process image format information.
	ProcessFormat();

	if(g_ImageType == Type_Framebuffer || g_ImageType == Type_ZFramebuffer)
	{
		g_theWindow = new Fl_Window(g_ImageWidth,g_ImageHeight);
		g_uiImageWidget = new Fl_FrameBuffer_Widget(0,0, g_ImageWidth, g_ImageHeight, (g_ImageType == Type_ZFramebuffer)?3:g_Channels, g_byteData);
		g_theWindow->resizable(g_uiImageWidget);
		g_theWindow->end();
		g_theWindow->show();
	}

	// Create a thread to request buckets from Aqsis
	boost::thread thrd(&BucketFunction);

	if(g_ImageType == Type_Framebuffer || g_ImageType == Type_ZFramebuffer)
	{
		// Open and run the window.
		Fl::run();
	}

	thrd.join();

	delete[] ( g_byteData );
	delete[] ( g_floatData );

    return 0;
}

