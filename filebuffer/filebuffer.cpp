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
		\brief Implements a TIFF based display driver.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<iostream>
#include	<stdlib.h>

#include	"aqsis.h"

#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
#include	<version.h>
#endif

#ifdef AQSIS_SYSTEM_WIN32

#include	<process.h>

#else // AQSIS_SYSTEM_WIN32

typedef int SOCKET;

#endif // !AQSIS_SYSTEM_WIN32

#include	"displaydriver.h"
#include	"dd.h"
#include	"tiffio.h"
#include	"sstring.h"

using namespace Aqsis;

/// Static response message for a format query.
SqDDMessageFormatResponse frmt( 2 );
/// Static response message close message.
SqDDMessageCloseAcknowledge closeack;

/** Handle a query message from the manager.
 */
TqInt Query( SOCKET s, SqDDMessageBase* pMsg );
/** Handle an open message from the handler.
 */
TqInt Open( SOCKET s, SqDDMessageBase* pMsg );
/** Handle a data message from the manager.
 */
TqInt Data( SOCKET s, SqDDMessageBase* pMsg );
/** Handle a close message from the manager.
 */
TqInt Close( SOCKET s, SqDDMessageBase* pMsg );


/// Main loop,, just cycle handling any recieved messages.
int main( int argc, char* argv[] )
{
        int port = -1;
	char *portStr = getenv("AQSIS_DD_PORT");

	if (portStr != NULL)
	{
		port = atoi(portStr);
	}
	
	if ( DDInitialise( NULL, port ) == 0 )
	{
		DDProcessMessages();
	}
	return 0;
}


TqInt	XRes, YRes;
TqInt	SamplesPerElement,BitsPerSample;
unsigned char* pByteData;
float*	pFloatData;
TIFF*	pOut;
TqInt	g_CWXmin, g_CWYmin;
TqInt	g_CWXmax, g_CWYmax;


/// Storage for the output file name.
std::string	strFilename( "output.tif" );

TqInt Query( SOCKET s, SqDDMessageBase* pMsgB )
{
	switch ( pMsgB->m_MessageID )
	{
			case MessageID_FormatQuery:
			{
				if ( DDSendMsg( s, &frmt ) <= 0 )
					return ( -1 );
			}
			break;
	}
	return ( 0 );
}

TqInt Open( SOCKET s, SqDDMessageBase* pMsgB )
{
	SqDDMessageOpen * pMsg = static_cast<SqDDMessageOpen*>( pMsgB );

	XRes = ( pMsg->m_CropWindowXMax - pMsg->m_CropWindowXMin );
	YRes = ( pMsg->m_CropWindowYMax - pMsg->m_CropWindowYMin );
	SamplesPerElement = pMsg->m_SamplesPerElement;
	BitsPerSample = pMsg->m_BitsPerSample;

	g_CWXmin = pMsg->m_CropWindowXMin;
	g_CWYmin = pMsg->m_CropWindowYMin;
	g_CWXmax = pMsg->m_CropWindowXMax;
	g_CWYmax = pMsg->m_CropWindowYMax;

	// Create a buffer big enough to hold a row of buckets.
	if(BitsPerSample == 8)
		pByteData = new unsigned char[ XRes * YRes * SamplesPerElement ];
	else
		pFloatData = new float[ XRes * YRes * SamplesPerElement ];
	return ( 0 );
}


TqInt Data( SOCKET s, SqDDMessageBase* pMsgB )
{
	SqDDMessageData * pMsg = static_cast<SqDDMessageData*>( pMsgB );

	TqInt	linelen = XRes * SamplesPerElement;
	char* pBucket = reinterpret_cast<char*>( &pMsg->m_Data );

	SqDDMessageData * const message = static_cast<SqDDMessageData*>( pMsgB );

	// CHeck if the beck is not at all within the crop window.
	if( message->m_XMin > g_CWXmax || message->m_XMaxPlus1 < g_CWXmin ||
	    message->m_YMin > g_CWYmax || message->m_YMaxPlus1 < g_CWYmin )
		return( 0 );

	TqInt y;
	for ( y = pMsg->m_YMin - g_CWYmin; y < pMsg->m_YMaxPlus1 - g_CWYmin; y++ )
	{
		TqInt x;
		for ( x = pMsg->m_XMin - g_CWXmin; x < pMsg->m_XMaxPlus1 - g_CWXmin; x++ )
		{
			if ( x >= 0 && y >= 0 && x < XRes && y < YRes )
			{
				TqInt so = ( y * linelen ) + ( x * SamplesPerElement );

				TqInt i = 0;
				while ( i < SamplesPerElement )
				{
					if(BitsPerSample == 8)
						pByteData[ so++ ] = static_cast<char>( reinterpret_cast<TqFloat*>( pBucket ) [ i ] );
					else
						pFloatData[ so++ ] = reinterpret_cast<TqFloat*>( pBucket ) [ i ];
					i++;
				}
			}
			pBucket += pMsg->m_ElementSize;
		}
	}
	return ( 0 );
}


TqInt Close( SOCKET s, SqDDMessageBase* pMsgB )
{
	uint16 photometric = PHOTOMETRIC_RGB;
	uint16 config = PLANARCONFIG_CONTIG;
	uint16 compression, quality;
	SqDDMessageClose *pClose = (SqDDMessageClose *) pMsgB;
	compression = pClose->m_Compression;
	quality = pClose->m_Quality;
	//description = pClose->m_Description;

	pOut = TIFFOpen( strFilename.c_str(), "w" );

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
		TIFFSetField( pOut, TIFFTAG_IMAGEWIDTH, ( uint32 ) XRes );
		TIFFSetField( pOut, TIFFTAG_IMAGELENGTH, ( uint32 ) YRes );
		TIFFSetField( pOut, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
		TIFFSetField( pOut, TIFFTAG_SAMPLESPERPIXEL, SamplesPerElement );

		// Write out an 8 bits per pixel integer image.
		if(BitsPerSample == 8)
		{
			TIFFSetField( pOut, TIFFTAG_BITSPERSAMPLE, 8 );
			TIFFSetField( pOut, TIFFTAG_PLANARCONFIG, config );
			TIFFSetField( pOut, TIFFTAG_COMPRESSION, compression );
			if (compression == COMPRESSION_JPEG)
				TIFFSetField( pOut, TIFFTAG_JPEGQUALITY, quality );
			//if (description != "")
				//TIFFSetField(TIFFTAG_IMAGEDESCRIPTION, description);
			TIFFSetField( pOut, TIFFTAG_PHOTOMETRIC, photometric );
			TIFFSetField( pOut, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize( pOut, 0 ) );

			if ( SamplesPerElement == 4 )
				TIFFSetField( pOut, TIFFTAG_EXTRASAMPLES, 1, ExtraSamplesTypes );

			// Set the position tages in case we aer dealing with a cropped image.
			TIFFSetField( pOut, TIFFTAG_XPOSITION, ( float ) g_CWXmin );
			TIFFSetField( pOut, TIFFTAG_YPOSITION, ( float ) g_CWYmin );

			TqInt	linelen = XRes * SamplesPerElement;
			TqInt row;
			for ( row = 0; row < YRes; row++ )
			{
				if ( TIFFWriteScanline( pOut, pByteData + ( row * linelen ), row, 0 ) < 0 )
					break;
			}
			TIFFClose( pOut );
		}
		else
		{
			// Write out a floating point image.
			TIFFSetField(pOut, TIFFTAG_STONITS, (double) 1.0);

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
		
			if(use_logluv)
			{
				/* use SGI LogLuv compression */
				TIFFSetField(pOut, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_INT);
				TIFFSetField(pOut, TIFFTAG_BITSPERSAMPLE, 16);
				TIFFSetField(pOut, TIFFTAG_COMPRESSION, COMPRESSION_SGILOG);
				TIFFSetField(pOut, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_LOGLUV);
				TIFFSetField(pOut, TIFFTAG_SGILOGDATAFMT, SGILOGDATAFMT_FLOAT);
			}
			else
			{
				/* use uncompressed IEEEFP pixels */
				TIFFSetField(pOut, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
				TIFFSetField(pOut, TIFFTAG_BITSPERSAMPLE, 32);
				TIFFSetField(pOut, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
				TIFFSetField(pOut, TIFFTAG_COMPRESSION, compression);
			}

			TIFFSetField(pOut, TIFFTAG_SAMPLESPERPIXEL, SamplesPerElement);

			if ( SamplesPerElement == 4 )
				TIFFSetField( pOut, TIFFTAG_EXTRASAMPLES, 1, ExtraSamplesTypes );
			// Set the position tages in case we aer dealing with a cropped image.
			TIFFSetField( pOut, TIFFTAG_XPOSITION, ( float ) g_CWXmin );
			TIFFSetField( pOut, TIFFTAG_YPOSITION, ( float ) g_CWYmin );
			TIFFSetField( pOut, TIFFTAG_PLANARCONFIG, config );

			TqInt	linelen = XRes * SamplesPerElement;
			TqInt row = 0;
			for ( row = 0; row < YRes; row++ )
			{
				if ( TIFFWriteScanline( pOut, pFloatData + ( row * linelen ), row, 0 ) < 0 )
					break;
			}
			TIFFClose( pOut );
		}
	}
	if ( DDSendMsg( s, &closeack ) <= 0 )
		return ( -1 );
	else
		return ( 1 );
}


/** Handle a general message from the manager.
 */
TqInt HandleMessage( SOCKET s, SqDDMessageBase* pMsgB )
{
	switch ( pMsgB->m_MessageID )
	{
			case MessageID_Filename:
			{
				SqDDMessageFilename * pMsg = static_cast<SqDDMessageFilename*>( pMsgB );
				strFilename = pMsg->m_String;
			}
			break;
	}
	return ( 0 );
}
