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
		\brief Implements a GLUT based framebuffer display driver for Aqsis
		\author Timothy M. Shead (tshead@k-3d.com)
*/

#include <iostream>
#include <string>

#include "aqsis.h"

#ifdef AQSIS_SYSTEM_WIN32

#include	<winsock2.h>

#else // AQSIS_SYSTEM_WIN32

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

#endif // !AQSIS_SYSTEM_WIN32

#include "displaydriver.h"
#include "dd.h"

using namespace Aqsis;

#ifdef AQSIS_SYSTEM_MACOSX
#include <GLUT/glut.h>
#include <GLUT/macxglut_utilities.h>
#include <ApplicationServices/ApplicationServices.h>
#else
#include <GL/glut.h>
#endif //!AQSIS_SYSTEM_MACOSX

#ifndef AQSIS_SYSTEM_WIN32
typedef int SOCKET;
#endif // !AQSIS_SYSTEM_WIN32

static std::string	g_Filename( "output.tif" );
static TqInt g_ImageWidth = 0;
static TqInt g_ImageHeight = 0;
static TqInt g_Channels = 0;
static TqInt g_Format = 0;
static int g_Window = 0;
static GLubyte* g_Image = 0;
static TqInt g_CWXmin, g_CWYmin;
static TqInt g_CWXmax, g_CWYmax;
static TqFloat	quantize_zeroval = 0.0f;
static TqFloat	quantize_oneval  = 0.0f;
static TqFloat	quantize_minval  = 0.0f;
static TqFloat	quantize_maxval  = 0.0f;
static TqFloat dither_val       = 0.0f;

void display( void )
{
	glDisable( GL_SCISSOR_TEST );
	glClear( GL_COLOR_BUFFER_BIT );
	glRasterPos2i( 0, 0 );
	glDrawPixels( g_ImageWidth, g_ImageHeight, GL_RGB, GL_UNSIGNED_BYTE, g_Image );
	glFlush();
}

void reshape( int w, int h )
{
	glViewport( 0, 0, ( GLsizei ) w, ( GLsizei ) h );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluOrtho2D( 0.0, ( GLdouble ) w, 0.0, ( GLdouble ) h );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
}

void idle( void )
{
	if ( !DDProcessMessageAsync( 0, 1000 ) )
		glutIdleFunc( 0 );
}

void keyboard( unsigned char key, int x, int y )
{
	switch ( key )
	{
			case 27:
			case 'q':
			exit( 0 );
			break;
			default:
			break;
	}
}

int main( int argc, char** argv )
{
	int port = -1;
	char *portStr = getenv( "AQSIS_DD_PORT" );

	if ( portStr != NULL )
	{
		port = atoi( portStr );
	}

	if ( -1 == DDInitialise( NULL, port ) )
	{
		std::cerr << "Unable to open socket" << std::endl;
		return 1;
	}

	glutInit( &argc, argv );

	// Process messages until we have enough data to create our window ...
	while ( 0 == g_Window )
	{
		if ( !DDProcessMessage() )
		{
			std::cerr << "Premature end of messages" << std::endl;
			return 2;
		}
	}

	// Start the glut message loop ...
	glutDisplayFunc( display );
	glutReshapeFunc( reshape );
	glutKeyboardFunc( keyboard );
	glutIdleFunc( idle );

	// Setup GL context.
	glClearColor( 0.0, 0.0, 0.0, 0.0 );
	glDisable( GL_SCISSOR_TEST );
	glDisable( GL_DEPTH_TEST );
	glShadeModel( GL_FLAT );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glClear( GL_COLOR_BUFFER_BIT );

	// Start up.
	glutMainLoop();

	// Lose our image buffer ...
	delete g_Image;

	return 0;
}

//----------------------------------------------------------------------------
// Functions required by libdd.

SqDDMessageFormatResponse frmt( DataFormat_Signed32 );
SqDDMessageCloseAcknowledge closeack;

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
	SqDDMessageOpen * const message = static_cast<SqDDMessageOpen*>( pMsgB );

	g_ImageWidth = ( message->m_CropWindowXMax - message->m_CropWindowXMin );
	g_ImageHeight = ( message->m_CropWindowYMax - message->m_CropWindowYMin );
	g_Channels = message->m_Channels;

	g_CWXmin = message->m_CropWindowXMin;
	g_CWYmin = message->m_CropWindowYMin;
	g_CWXmax = message->m_CropWindowXMax;
	g_CWYmax = message->m_CropWindowYMax;

	g_Image = new GLubyte[ g_ImageWidth * g_ImageHeight * 3 ];
	//memset( g_Image, 128, g_ImageWidth * g_ImageHeight * 3 );
	for (TqInt i = 0; i < g_ImageHeight; i ++) {
		for (TqInt j=0; j < g_ImageWidth; j++)
		{
			int     t       = 0;
			GLubyte d = 255;
			
			if ( ( i & 31 ) < 16 ) t ^= 1;
			if ( ( j & 31 ) < 16 ) t ^= 1;
			
			if ( t )
			{
				d      = 128;
			}
			g_Image[3 * (i*g_ImageWidth + j) ] = d;
			g_Image[3 * (i*g_ImageWidth + j) + 1] = d;
			g_Image[3 * (i*g_ImageWidth + j) + 2] = d;
		}
	}


	//	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitDisplayMode( GLUT_SINGLE | GLUT_RGBA );
	glutInitWindowSize( g_ImageWidth, g_ImageHeight );
	g_Window = glutCreateWindow( g_Filename.c_str() );

	return ( 0 );
}

#define INT_MULT(a,b,t) ( (t) = (a) * (b) + 0x80, ( ( ( (t)>>8 ) + (t) )>>8 ) )
#define INT_PRELERP(p, q, a, t) ( (p) + (q) - INT_MULT( a, p, t) )

TqInt Data( SOCKET s, SqDDMessageBase* pMsgB )
{
	SqDDMessageData * const message = static_cast<SqDDMessageData*>( pMsgB );

	const TqInt linelength = g_ImageWidth * 3;
	char* bucket = reinterpret_cast<char*>( &message->m_Data );

	// CHeck if the beck is not at all within the crop window.
	if ( message->m_XMin > g_CWXmax || message->m_XMaxPlus1 < g_CWXmin ||
	     message->m_YMin > g_CWYmax || message->m_YMaxPlus1 < g_CWYmin )
		return ( 0 );

	for ( TqInt y = message->m_YMin - g_CWYmin; y < message->m_YMaxPlus1 - g_CWYmin; y++ )
	{
		for ( TqInt x = message->m_XMin - g_CWXmin; x < message->m_XMaxPlus1 - g_CWXmin; x++ )
		{
			if ( x >= 0 && y >= 0 && x < g_ImageWidth && y < g_ImageHeight )
			{
				const TqInt so = ( ( g_ImageHeight - y - 1 ) * linelength ) + ( x * 3 );

				TqFloat value0, value1, value2;
				TqFloat alpha = 255.0f;
				if ( g_Channels >= 3 )
				{
					value0 = reinterpret_cast<TqFloat*>( bucket ) [ 0 ];
					value1 = reinterpret_cast<TqFloat*>( bucket ) [ 1 ];
					value2 = reinterpret_cast<TqFloat*>( bucket ) [ 2 ];
				}
				else
				{
					value0 = reinterpret_cast<TqFloat*>( bucket ) [ 0 ];
					value1 = reinterpret_cast<TqFloat*>( bucket ) [ 0 ];
					value2 = reinterpret_cast<TqFloat*>( bucket ) [ 0 ];
				}

				if ( g_Channels > 3 ) 
					alpha = (reinterpret_cast<TqFloat*>( bucket ) [ 3 ]);

				if( !( quantize_zeroval == 0.0f &&
					   quantize_oneval  == 0.0f &&
					   quantize_minval  == 0.0f &&
					   quantize_maxval  == 0.0f ) )
				{
					value0 = ROUND(quantize_zeroval + value0 * (quantize_oneval - quantize_zeroval) + dither_val );
					value0 = CLAMP(value0, quantize_minval, quantize_maxval) ;
					value1 = ROUND(quantize_zeroval + value1 * (quantize_oneval - quantize_zeroval) + dither_val );
					value1 = CLAMP(value1, quantize_minval, quantize_maxval) ;
					value2 = ROUND(quantize_zeroval + value2 * (quantize_oneval - quantize_zeroval) + dither_val );
					value2 = CLAMP(value2, quantize_minval, quantize_maxval) ;
					alpha  = ROUND(quantize_zeroval + alpha * (quantize_oneval - quantize_zeroval) + dither_val );
					alpha  = CLAMP(alpha, quantize_minval, quantize_maxval) ;
				}
				else if ( g_Format != DataFormat_Unsigned8 )
				{
					// If we are displaying an FP image we will need to quantize ourselves.
					value0 *= 255;
					value1 *= 255;
					value2 *= 255;
					alpha  *= 255;
				}

				// C’ = INT_PRELERP( A’, B’, b, t )
				TqInt t;
				if( alpha > 0 ) 
				{
					int A = INT_PRELERP( g_Image[ so + 0 ], value0, alpha, t );
					int B = INT_PRELERP( g_Image[ so + 1 ], value1, alpha, t );
					int C = INT_PRELERP( g_Image[ so + 2 ], value2, alpha, t );
					g_Image[ so + 0 ] = CLAMP( A, 0, 255 );
					g_Image[ so + 1 ] = CLAMP( B, 0, 255 );
					g_Image[ so + 2 ] = CLAMP( C, 0, 255 );
				}
			}
			bucket += message->m_ElementSize;
		}
	}

	//std::cerr << message->m_XMin << ", " << message->m_YMin << " - " << message->m_XMaxPlus1 << ", " << message->m_YMaxPlus1 << std::endl;

	const TqInt BucketX = message->m_XMin - g_CWXmin;
	const TqInt BucketY = g_ImageHeight - ( message->m_YMaxPlus1 - g_CWYmin );
	const TqInt BucketW = message->m_XMaxPlus1 - message->m_XMin;
	const TqInt BucketH = message->m_YMaxPlus1 - message->m_YMin;

	glEnable( GL_SCISSOR_TEST );
	glScissor( BucketX, BucketY, BucketW, BucketH );
	glRasterPos2i( 0, 0 );
	glDrawPixels( g_ImageWidth, g_ImageHeight, GL_RGB, GL_UNSIGNED_BYTE, g_Image );
	glFlush();

	return ( 0 );
}

TqInt Close( SOCKET s, SqDDMessageBase* pMsgB )
{
	glutPostRedisplay();
	if ( DDSendMsg( s, &closeack ) <= 0 )
		return ( -1 );
	else
		return ( 1 );
}

TqInt Abandon( SOCKET s, SqDDMessageBase* pMsgB )
{
	return ( 1 );
}

TqInt HandleMessage( SOCKET s, SqDDMessageBase* pMsgB )
{
	switch ( pMsgB->m_MessageID )
	{
			case MessageID_Filename:
			{
				SqDDMessageFilename * message = static_cast<SqDDMessageFilename*>( pMsgB );
				g_Filename = message->m_String;
			}
			break;

			case MessageID_UserParam:
			{
				SqDDMessageUserParam * pMsg = static_cast<SqDDMessageUserParam*>( pMsgB );
				// Check if we understand the parameter.
				if( strncmp( pMsg->m_NameAndData, "quantize", pMsg->m_NameLength ) == 0 )
				{
					TqFloat* quantize = reinterpret_cast<TqFloat*>( &pMsg->m_NameAndData[ pMsg->m_NameLength + 1 ] );
					quantize_zeroval = quantize[0];
					quantize_oneval  = quantize[1];
					quantize_minval  = quantize[2];
					quantize_maxval  = quantize[3];
				}
			}
			break;
	}
	return ( 0 );
}



