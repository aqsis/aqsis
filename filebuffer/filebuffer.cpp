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
	if ( DDInitialise( NULL, -1 ) == 0 )
	{
		DDProcessMessages();
	}
	return 0;
}


TqInt	XRes, YRes;
TqInt	SamplesPerElement;
unsigned char* pByteData;
TIFF*	pOut;
TqInt	CWXMin, CWYMin;
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
	CWXMin = pMsg->m_CropWindowXMin;
	CWYMin = pMsg->m_CropWindowYMin;
	SamplesPerElement = pMsg->m_SamplesPerElement;

	// Create a buffer big enough to hold a row of buckets.
	pByteData = new unsigned char[ XRes * YRes * SamplesPerElement ];
	return ( 0 );
}


TqInt Data( SOCKET s, SqDDMessageBase* pMsgB )
{
	SqDDMessageData * pMsg = static_cast<SqDDMessageData*>( pMsgB );

	TqInt	linelen = XRes * SamplesPerElement;
	char* pBucket = reinterpret_cast<char*>( &pMsg->m_Data );

	TqInt y;
	for ( y = pMsg->m_YMin; y < pMsg->m_YMaxPlus1; y++ )
	{
		TqInt x;
		for ( x = pMsg->m_XMin; x < pMsg->m_XMaxPlus1; x++ )
		{
			if ( x >= 0 && y >= 0 && x < XRes && y < YRes )
			{
				TqInt so = ( y * linelen ) + ( x * SamplesPerElement );

				TqInt i = 0;
				while ( i < SamplesPerElement )
				{
					pByteData[ so++ ] = static_cast<char>( reinterpret_cast<TqFloat*>( pBucket ) [ i ] );
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
		TIFFSetField( pOut, TIFFTAG_SOFTWARE, ( uint32 ) version );
		TIFFSetField( pOut, TIFFTAG_IMAGEWIDTH, ( uint32 ) XRes );
		TIFFSetField( pOut, TIFFTAG_IMAGELENGTH, ( uint32 ) YRes );
		TIFFSetField( pOut, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
		TIFFSetField( pOut, TIFFTAG_SAMPLESPERPIXEL, SamplesPerElement );
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
		TIFFSetField( pOut, TIFFTAG_XPOSITION, ( float ) CWXMin );
		TIFFSetField( pOut, TIFFTAG_YPOSITION, ( float ) CWYMin );

		TqInt	linelen = XRes * SamplesPerElement;
		TqInt row;
		for ( row = 0; row < YRes; row++ )
		{
			if ( TIFFWriteScanline( pOut, pByteData + ( row * linelen ), row, 0 ) < 0 )
				break;
		}
		TIFFClose( pOut );
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
