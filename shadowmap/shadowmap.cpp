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
		\brief Implements a display driver for saving shadow maps.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<iostream>
#include	<fstream>

#include	"aqsis.h"

#ifdef AQSIS_SYSTEM_WIN32

#include	<process.h>

#else

typedef int SOCKET;

#endif

#include	"ri.h"
#include	"displaydriver.h"
#include	"dd.h"
#include	"tiffio.h"
#include	"sstring.h"
#include	"texturemap.h"

#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
#include	"version.h"
#endif

using namespace Aqsis;

SqDDMessageFormatResponse frmt( 2 );
SqDDMessageCloseAcknowledge closeack;

void SaveAsShadowMap();

int main( int argc, char* argv[] )
{
	int port = -1;
	char *portStr = getenv( "AQSIS_DD_PORT" );

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

#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
#define	ZFILE_HEADER		"Aqsis ZFile" VERSION_STR
#else // AQSIS_SYSTEM_WIN32
#define ZFILE_HEADER		"Aqsis ZFile" VERSION
#endif // !AQSIS_SYSTEM_WIN32


TqInt	XRes, YRes;
TqInt	SamplesPerElement;
TqFloat* pData;
TIFF*	pOut;
TqInt	CWXMin, CWYMin;
std::string	strFilename( "output.tif" );
std::string	strType( "zfile" );
TqFloat	matWorldToCamera[ 4 ][ 4 ];
TqFloat	matWorldToScreen[ 4 ][ 4 ];

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

	if ( SamplesPerElement > 1 ) return ( -1 );

	// Create a buffer big enough to hold a row of buckets.
	pData = new TqFloat[ XRes * YRes ];
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
					pData[ so++ ] = reinterpret_cast<TqFloat*>( pBucket ) [ i ];
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
	// Save the shadowmap to a binary file.
	if ( strFilename != "" )
	{
		if( strType.compare( "shadow" ) == 0 )
		{
			SaveAsShadowMap();
		}
		else
		{
			std::ofstream ofile( strFilename.c_str(), std::ios::out | std::ios::binary );
			if ( ofile.is_open() )
			{
				// Save a file type and version marker
				ofile << ZFILE_HEADER;

				// Save the xres and yres.
				ofile.write( reinterpret_cast<char* >( &XRes ), sizeof( XRes ) );
				ofile.write( reinterpret_cast<char* >( &YRes ), sizeof( XRes ) );

				// Save the transformation matrices.
				ofile.write( reinterpret_cast<char*>( matWorldToCamera[ 0 ] ), sizeof( matWorldToCamera[ 0 ][ 0 ] ) * 4 );
				ofile.write( reinterpret_cast<char*>( matWorldToCamera[ 1 ] ), sizeof( matWorldToCamera[ 0 ][ 0 ] ) * 4 );
				ofile.write( reinterpret_cast<char*>( matWorldToCamera[ 2 ] ), sizeof( matWorldToCamera[ 0 ][ 0 ] ) * 4 );
				ofile.write( reinterpret_cast<char*>( matWorldToCamera[ 3 ] ), sizeof( matWorldToCamera[ 0 ][ 0 ] ) * 4 );

				ofile.write( reinterpret_cast<char*>( matWorldToScreen[ 0 ] ), sizeof( matWorldToScreen[ 0 ][ 0 ] ) * 4 );
				ofile.write( reinterpret_cast<char*>( matWorldToScreen[ 1 ] ), sizeof( matWorldToScreen[ 0 ][ 0 ] ) * 4 );
				ofile.write( reinterpret_cast<char*>( matWorldToScreen[ 2 ] ), sizeof( matWorldToScreen[ 0 ][ 0 ] ) * 4 );
				ofile.write( reinterpret_cast<char*>( matWorldToScreen[ 3 ] ), sizeof( matWorldToScreen[ 0 ][ 0 ] ) * 4 );

				// Now output the depth values
				ofile.write( reinterpret_cast<char*>( pData ), sizeof( TqFloat ) * ( XRes * YRes ) );
				ofile.close();
			}
		}
	}
	if ( DDSendMsg( s, &closeack ) <= 0 )
		return ( -1 );
	else
		return ( 1 );
}


TqInt Abandon( SOCKET s, SqDDMessageBase* pMsgB )
{
	return ( 1 );
}


void SaveAsShadowMap()
{
	TqChar version[ 80 ];
	TqInt twidth = 32;
	TqInt tlength = 32;

	// Save the shadowmap to a binary file.
	if ( strFilename.compare( "" ) != 0 )
	{
		TIFF * pshadow = TIFFOpen( strFilename.c_str(), "w" );
		TIFFCreateDirectory( pshadow );

#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
		sprintf( version, "%s %s", STRNAME, VERSION_STR );
#else
		sprintf( version, "%s %s", STRNAME, VERSION );
#endif
		TIFFSetField( pshadow, TIFFTAG_SOFTWARE, ( uint32 ) version );
		TIFFSetField( pshadow, TIFFTAG_PIXAR_MATRIX_WORLDTOCAMERA, matWorldToCamera );
		TIFFSetField( pshadow, TIFFTAG_PIXAR_MATRIX_WORLDTOSCREEN, matWorldToScreen );
		TIFFSetField( pshadow, TIFFTAG_PIXAR_TEXTUREFORMAT, SHADOWMAP_HEADER );
		TIFFSetField( pshadow, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK );

		// Write the floating point image to the directory.
		TqFloat *depths = ( TqFloat * ) pData;

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
		TIFFSetField( pshadow, TIFFTAG_SAMPLESPERPIXEL, SamplesPerElement );
		TIFFSetField( pshadow, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
		TIFFSetField( pshadow, TIFFTAG_TILEWIDTH, twidth );
		TIFFSetField( pshadow, TIFFTAG_TILELENGTH, tlength );
		TIFFSetField( pshadow, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP );
		//TIFFSetField( ptex, TIFFTAG_COMPRESSION, compression );


		TqInt tsize = twidth * tlength;
		TqInt tperrow = ( XRes + twidth - 1 ) / twidth;
		TqFloat* ptile = static_cast<TqFloat*>( _TIFFmalloc( tsize * SamplesPerElement * sizeof( TqFloat ) ) );

		if ( ptile != NULL )
		{
			TqInt ctiles = tperrow * ( ( YRes + tlength - 1 ) / tlength );
			TqInt itile;
			for ( itile = 0; itile < ctiles; itile++ )
			{
				TqInt x = ( itile % tperrow ) * twidth;
				TqInt y = ( itile / tperrow ) * tlength;
				TqFloat* ptdata = pData + ( ( y * XRes ) + x ) * SamplesPerElement;
				// Clear the tile to black.
				memset( ptile, 0, tsize * SamplesPerElement * sizeof( TqFloat ) );
				for ( TqUlong i = 0; i < tlength; i++ )
				{
					for ( TqUlong j = 0; j < twidth; j++ )
					{
						if ( ( x + j ) < XRes && ( y + i ) < YRes )
						{
							TqInt ii;
							for ( ii = 0; ii < SamplesPerElement; ii++ )
								ptile[ ( i * twidth * SamplesPerElement ) + ( ( ( j * SamplesPerElement ) + ii ) ) ] = ptdata[ ( ( j * SamplesPerElement ) + ii ) ];
						}
					}
					ptdata += ( XRes * SamplesPerElement );
				}
				TIFFWriteTile( pshadow, ptile, x, y, 0, 0 );
			}
			TIFFWriteDirectory( pshadow );

		}

		TIFFClose( pshadow );
	}
}


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

			case MessageID_DisplayType:
			{
				SqDDMessageDisplayType * pMsg = static_cast<SqDDMessageDisplayType*>( pMsgB );
				strType = pMsg->m_String;
			}
			break;

			case MessageID_Nl:
			{
				SqDDMessageNl* pMsg = static_cast<SqDDMessageNl*>( pMsgB );
				memcpy( matWorldToCamera, pMsg->m_Matrix, sizeof( matWorldToCamera ) );
			}
			break;

			case MessageID_NP:
			{
				SqDDMessageNP* pMsg = static_cast<SqDDMessageNP*>( pMsgB );
				memcpy( matWorldToScreen, pMsg->m_Matrix, sizeof( matWorldToScreen ) );
			}
			break;
	}
	return ( 0 );
}
