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
		\brief Implements a display drive that links Aqsis' sockets based system to the standard dspy system.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<iostream>
#include	<stdlib.h>

#include	"aqsis.h"
#include	"plugins.h"
#include	"ri.h"

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
#include	"dspy.h"

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
/** Handle an abandon message from the manager.
 */
TqInt Abandon( SOCKET s, SqDDMessageBase* pMsg );

static	TqInt	XRes, YRes;
static	TqInt	g_Channels;
static	TqInt	g_Format;
static	TIFF*	pOut;
static	TqInt	g_CWXmin, g_CWYmin;
static	TqInt	g_CWXmax, g_CWYmax;
static	uint16	compression = COMPRESSION_NONE, quality = 0;
static	TqFloat	quantize_zeroval = 0.0f;
static	TqFloat	quantize_oneval  = 0.0f;
static	TqFloat	quantize_minval  = 0.0f;
static	TqFloat	quantize_maxval  = 0.0f;
static	TqFloat dither_val       = 0.0f;

CqString g_strOpenMethod("DspyImageOpen");
CqString g_strQueryMethod("DspyImageQuery");
CqString g_strDataMethod("DspyImageData");
CqString g_strCloseMethod("DspyImageClose");
CqString g_strDelayCloseMethod("DspyImageDelayClose");

DspyImageOpenMethod			g_OpenMethod = NULL;
DspyImageQueryMethod		g_QueryMethod = NULL;
DspyImageDataMethod			g_DataMethod = NULL;
DspyImageCloseMethod		g_CloseMethod = NULL;
DspyImageDelayCloseMethod	g_DelayCloseMethod = NULL;

void* g_DriverHandle = NULL;
PtDspyImageHandle g_ImageHandle;
PtFlagStuff g_Flags;
CqSimplePlugin g_DspyDriver;
PtDspyDevFormat	g_Formats[4];
CqString g_strName( "" );
/// Storage for the output file name.
std::string	strFilename( "output.tif" );


/// Main loop,, just cycle handling any recieved messages.
int main( int argc, char* argv[] )
{
	int port = -1;
	char *portStr = getenv( "AQSIS_DD_PORT" );

	if( argc <= 1 )
		return(-1);

	// store the name of the dspy driver requested.
	g_strName = argv[1];

	if ( portStr != NULL )
	{
		port = atoi( portStr );
	}

	if ( DDInitialise( NULL, port ) == 0 )
	{
		DDProcessMessages();
	}
	return 0;
}


TqInt Query( SOCKET s, SqDDMessageBase* pMsgB )
{
	switch ( pMsgB->m_MessageID )
	{
		case MessageID_FormatQuery:
		{
			SqDDMessageFormatQuery* pMsg = static_cast<SqDDMessageFormatQuery*>(pMsgB);
			g_Format = pMsg->m_Formats[0];
			frmt.m_DataFormat = g_Format;
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
	g_Channels = pMsg->m_Channels;

	g_CWXmin = pMsg->m_CropWindowXMin;
	g_CWYmin = pMsg->m_CropWindowYMin;
	g_CWXmax = pMsg->m_CropWindowXMax;
	g_CWYmax = pMsg->m_CropWindowYMax;

	g_DriverHandle = g_DspyDriver.SimpleDLOpen( &g_strName );
	if( g_DriverHandle != NULL )
	{
		g_OpenMethod = (DspyImageOpenMethod)g_DspyDriver.SimpleDLSym( g_DriverHandle, &g_strOpenMethod );
		g_QueryMethod = (DspyImageQueryMethod)g_DspyDriver.SimpleDLSym( g_DriverHandle, &g_strQueryMethod );
		g_DataMethod = (DspyImageDataMethod)g_DspyDriver.SimpleDLSym( g_DriverHandle, &g_strDataMethod );
		g_CloseMethod = (DspyImageCloseMethod)g_DspyDriver.SimpleDLSym( g_DriverHandle, &g_strCloseMethod );
		g_DelayCloseMethod = (DspyImageDelayCloseMethod)g_DspyDriver.SimpleDLSym( g_DriverHandle, &g_strDelayCloseMethod );
	}

	if( NULL != g_OpenMethod )
	{
		// \todo: need to pass the proper mode string.
		TqInt formatindex = 0;
		if( g_Channels >=4 )
		{
			g_Formats[formatindex  ].name = "a";
			g_Formats[formatindex++].type = PkDspyUnsigned8;
		}
		g_Formats[formatindex  ].name = "r";
		g_Formats[formatindex++].type = PkDspyUnsigned8;
		g_Formats[formatindex  ].name = "g";
		g_Formats[formatindex++].type = PkDspyUnsigned8;
		g_Formats[formatindex  ].name = "b";
		g_Formats[formatindex++].type = PkDspyUnsigned8;
		std::cout << strFilename.c_str() << std::endl;
		PtDspyError err = (*g_OpenMethod)(&g_ImageHandle, "sdchwnd", strFilename.c_str(), XRes, YRes, 0, NULL, 4, g_Formats, &g_Flags);
		PtDspySizeInfo size;
		err = (*g_QueryMethod)(g_ImageHandle, PkSizeQuery, sizeof(size), &size);
		PtDspyOverwriteInfo owinfo;
		err = (*g_QueryMethod)(g_ImageHandle, PkOverwriteQuery, sizeof(owinfo), &owinfo);
	}

	return ( 0 );
}


TqInt Data( SOCKET s, SqDDMessageBase* pMsgB )
{
	SqDDMessageData * pMsg = static_cast<SqDDMessageData*>( pMsgB );
	SqDDMessageData * const message = static_cast<SqDDMessageData*>( pMsgB );

	TqInt xmin = message->m_XMin;
	TqInt ymin = message->m_YMin;
	TqInt xmaxp1 = message->m_XMaxPlus1;
	TqInt ymaxp1 = message->m_YMaxPlus1;
	TqInt xsize = xmaxp1 - xmin;
	TqInt ysize = ymaxp1 - ymin;

	TqInt	linelen = xsize * g_Channels;
	char* pBucket = reinterpret_cast<char*>( &pMsg->m_Data );


	// CHeck if the beck is not at all within the crop window.
	if ( xmin > g_CWXmax || xmaxp1 < g_CWXmin ||
	     xmin > g_CWYmax || xmaxp1 < g_CWYmin )
		return ( 0 );

	unsigned char* pByteData = NULL;
	float*	pFloatData = NULL;

	if ( g_Formats[0].type == PkDspyUnsigned8 )
		pByteData = new unsigned char[ xsize * ysize * g_Channels ];
	else if( g_Formats[0].type == PkDspyFloat32 )
		pFloatData = new float[ xsize * ysize * g_Channels ];
	else
		return(-1);
	
	TqInt y;
	for ( y = ymin; y < ymaxp1; y++ )
	{
		TqInt x;
		for ( x = xmin; x < xmaxp1; x++ )
		{
			TqInt so = ( ( y - ymin ) * linelen ) + ( ( x - xmin ) * g_Channels );

			TqFloat rvalue = reinterpret_cast<TqFloat*>( pBucket ) [ 0 ];
			TqFloat gvalue = reinterpret_cast<TqFloat*>( pBucket ) [ 1 ];
			TqFloat bvalue = reinterpret_cast<TqFloat*>( pBucket ) [ 2 ];

			TqFloat avalue = 0.0f;
			if( g_Channels >= 4 )
				avalue = reinterpret_cast<TqFloat*>( pBucket ) [ 3 ];

			if( !( quantize_zeroval == 0.0f &&
				   quantize_oneval  == 0.0f &&
				   quantize_minval  == 0.0f &&
				   quantize_maxval  == 0.0f ) )
			{
				rvalue = ROUND(quantize_zeroval + rvalue * (quantize_oneval - quantize_zeroval) + dither_val );
				rvalue = CLAMP(rvalue, quantize_minval, quantize_maxval) ;
				gvalue = ROUND(quantize_zeroval + gvalue * (quantize_oneval - quantize_zeroval) + dither_val );
				gvalue = CLAMP(gvalue, quantize_minval, quantize_maxval) ;
				bvalue = ROUND(quantize_zeroval + bvalue * (quantize_oneval - quantize_zeroval) + dither_val );
				bvalue = CLAMP(bvalue, quantize_minval, quantize_maxval) ;
				if( g_Channels >= 4 )
				{
					avalue = ROUND(quantize_zeroval + avalue * (quantize_oneval - quantize_zeroval) + dither_val );
					avalue = CLAMP(avalue, quantize_minval, quantize_maxval) ;
				}
			}

			if ( g_Formats[0].type == PkDspyUnsigned8  )
			{
				if( g_Format == DataFormat_Unsigned8 )
				{
					pByteData[ so + 1 ] = static_cast<char>( rvalue );
					pByteData[ so + 2 ] = static_cast<char>( gvalue );
					pByteData[ so + 3 ] = static_cast<char>( bvalue );
					if( g_Channels >= 4 )
						pByteData[ so + 0 ] = static_cast<char>( avalue );
				}
				else
				{
					pByteData[ so + 1 ] = static_cast<char>( rvalue * 255.0f );
					pByteData[ so + 2 ] = static_cast<char>( gvalue * 255.0f );
					pByteData[ so + 3 ] = static_cast<char>( bvalue * 255.0f );
					if( g_Channels >= 4 )
						pByteData[ so + 0 ] = static_cast<char>( avalue * 255.0f );
				}
			}
			else
			{
				pFloatData[ so + 1 ] = rvalue;
				pFloatData[ so + 2 ] = gvalue;
				pFloatData[ so + 3 ] = bvalue;
				if( g_Channels >= 4 )
					pFloatData[ so + 0 ] = avalue;
			}
			pBucket += pMsg->m_ElementSize;
		}
	}


	// Pass the data onto the dspy driver.
	if( NULL != g_DataMethod )
	{
		PtDspyError err;
		if( g_Formats[0].type == PkDspyUnsigned8 )
			err = (*g_DataMethod)(g_ImageHandle, xmin, xmaxp1, ymin, ymaxp1, sizeof(char) * g_Channels, (unsigned char*)pByteData);
		else
			err = (*g_DataMethod)(g_ImageHandle, xmin, xmaxp1, ymin, ymaxp1, sizeof(float) * g_Channels, (unsigned char*)pFloatData);
	}

	delete[] ( pByteData );
	delete[] ( pFloatData );
	return ( 0 );
}


TqInt Close( SOCKET s, SqDDMessageBase* pMsgB )
{
	SqDDMessageClose *pClose = ( SqDDMessageClose * ) pMsgB;

	// Close the dspy display driver.
	if( NULL != g_DelayCloseMethod )
		(*g_DelayCloseMethod)(g_ImageHandle);
	else if( NULL != g_CloseMethod )
		(*g_CloseMethod)(g_ImageHandle);

	if ( DDSendMsg( s, &closeack ) <= 0 )
		return ( -1 );
	else
		return ( 1 );
}


TqInt Abandon( SOCKET s, SqDDMessageBase* pMsgB )
{
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

			case MessageID_UserParam:
			{
				SqDDMessageUserParam * pMsg = static_cast<SqDDMessageUserParam*>( pMsgB );
				// Need to pass these messages through to the dspy driver.
			}
			break;
	}
	return ( 0 );
}
