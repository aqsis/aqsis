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
#include	<version.h>
#endif

#include "display.h"

START_NAMESPACE( Aqsis )

TqInt g_ImageWidth = 0;
TqInt g_ImageHeight = 0;
TqInt g_PixelsProcessed = 0;
TqInt g_Channels = 0;
TqInt g_Format = 1;
TqInt g_ElementSize = 0;
TqInt g_CWXmin, g_CWYmin;
TqInt g_CWXmax, g_CWYmax;
TqFloat g_QuantizeZeroVal = 0.0f;
TqFloat g_QuantizeOneVal = 0.0f;
TqFloat g_QuantizeMinVal = 0.0f;
TqFloat g_QuantizeMaxVal = 0.0f;
TqFloat g_QuantizeDitherVal = 0.0f;
uint16	g_Compression = COMPRESSION_NONE, g_Quality = 0;

TqInt	g_BucketsPerCol, g_BucketsPerRow;

unsigned char* g_byteData;
float*	g_floatData;

Fl_Window *g_theWindow;
Fl_FrameBuffer_Widget *g_uiImageWidget;
Fl_RGB_Image* g_uiImage;

// Command line arguments 
ArgParse::apstring g_type;
ArgParse::apstring g_filename = "output.tif";
ArgParse::apstring g_hostname;
ArgParse::apstring g_port;
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
    close( Socket );

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

void WriteTIFF(const std::string& filename)
{
    uint16 photometric = PHOTOMETRIC_RGB;
    uint16 config = PLANARCONFIG_CONTIG;

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

        // Set common tags
        TIFFSetField( pOut, TIFFTAG_SOFTWARE, ( uint32 ) version );
        TIFFSetField( pOut, TIFFTAG_IMAGEWIDTH, ( uint32 ) g_ImageWidth );
        TIFFSetField( pOut, TIFFTAG_IMAGELENGTH, ( uint32 ) g_ImageHeight );
        TIFFSetField( pOut, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
        TIFFSetField( pOut, TIFFTAG_SAMPLESPERPIXEL, g_Channels );

        // Write out an 8 bits per pixel integer image.
        if ( g_Format == 1 /*DataFormat_Unsigned8*/ )
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
            for ( row = 0; row < g_ImageHeight; row++ )
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
            for ( row = 0; row < g_ImageHeight; row++ )
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
    TqInt	linelen = g_ImageWidth * g_Channels;

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
		free(resp);

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
					TqInt dataLen = strlen(data->Value());
					dataLen = (dataLen/4)*3;
					char* dataBin = new char[dataLen];
					TqFloat* dataPtr = reinterpret_cast<TqFloat*>(dataBin);
					b64_decode(reinterpret_cast<char*>(dataBin), data->Value());
					TqInt y;
					for ( y = ymin - g_CWYmin; y < ymaxp1 - g_CWYmin; y++ )
					{
						TqInt x;
						for ( x = xmin - g_CWXmin; x < xmaxp1 - g_CWXmin; x++ )
						{
							if ( x >= 0 && y >= 0 && x < g_ImageWidth && y < g_ImageHeight )
							{
								TqInt so = ( y * linelen ) + ( x * g_Channels );

								TqInt i = 0;
								while ( i < g_Channels )
								{
									TqFloat value = dataPtr[i];

									if( !( g_QuantizeZeroVal == 0.0f &&
										   g_QuantizeOneVal  == 0.0f &&
										   g_QuantizeMinVal  == 0.0f &&
										   g_QuantizeMaxVal  == 0.0f ) )
									{
										value = ROUND(g_QuantizeZeroVal + value * (g_QuantizeOneVal - g_QuantizeZeroVal) + g_QuantizeDitherVal );
										value = CLAMP(value, g_QuantizeMinVal, g_QuantizeMaxVal) ;
									}

									if ( g_Format == 1 )
									{
										if( NULL != g_byteData )
											g_byteData[ so ] = static_cast<char>( value );
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
							dataPtr += g_ElementSize;
						}
					}
					delete[](dataBin);
				}
			}
			else
			{
				std::cerr << "Error: Invalid response from Aqsis1" << std::endl;
				exit(-1);
			}

			if(g_type.compare("framebuffer")==0)
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
	}
	if(g_type.compare("file")==0)
	{
		WriteTIFF(g_filename);
	}

	CloseSocket(g_Socket);
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
	TiXmlElement* formatElement = respHandle.FirstChildElement("aqsis:response").FirstChildElement("aqsis:format").Element();
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
		formatElement->QueryIntAttribute("elementsize", &g_ElementSize);
		g_PixelsProcessed = 0;
		g_Channels = 3;

        if ( g_Format == 1 )
		{
            g_byteData = new unsigned char[ g_ImageWidth * g_ImageHeight * g_Channels ];
			for (TqInt i = 0; i < g_ImageHeight; i ++) 
			{
				for (TqInt j=0; j < g_ImageWidth; j++)
				{
					int     t       = 0;
					unsigned char d = 255;

					if ( ( (g_ImageHeight - 1 - i) & 31 ) < 16 ) t ^= 1;
					if ( ( j & 31 ) < 16 ) t ^= 1;

					if ( t )
					{
						d      = 128;
					}
					g_byteData[3 * (i*g_ImageWidth + j) ] = d;
					g_byteData[3 * (i*g_ImageWidth + j) + 1] = d;
					g_byteData[3 * (i*g_ImageWidth + j) + 2] = d;
				}
			}
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
    ap.argString( "name", "=string\athe name of the file to save", &g_filename );
    ap.argString( "hostname", "=string\ahostname of the machine to connect to", &g_hostname );
    ap.argString( "port", "=string\aport to connect to", &g_port );
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

    /// Port is defined as the value passed into the -port= command line argument.
	/// if that is empty, then the value stored in the environment variable AQSIS_DD_PORT,
	/// if that is not available, then the fallback is 27742 ('A', 'Q', 'S', 'I' 'S', '2' on phone keypad)
	int port = 277472;
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
		return -1;

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
		std::cerr << "Connecting to " << g_hostname.c_str() << ":" << port << " ... " << strerror(errno) << std::endl;
		CloseSocket(g_Socket);
		return -1;
	}

	// Request and process image format information.
	ProcessFormat();

	if(g_type.compare("framebuffer")==0)
	{
		g_theWindow = new Fl_Window(g_ImageWidth,g_ImageHeight);
		g_uiImageWidget = new Fl_FrameBuffer_Widget(0,0, g_ImageWidth, g_ImageHeight, g_byteData);
		g_theWindow->resizable(g_uiImageWidget);
		g_theWindow->end();
		g_theWindow->show();
	}

	// Create a thread to request buckets from Aqsis
	boost::thread thrd(&BucketFunction);

	if(g_type.compare("framebuffer")==0)
	{
		// Open and run the window.
		Fl::run();
	}

	thrd.join();

	delete[] ( g_byteData );
	delete[] ( g_floatData );

    return 0;
}

