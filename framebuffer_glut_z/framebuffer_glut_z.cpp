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

#include <algorithm>
#include <iostream>
#include <string>
#include <algorithm>

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

#include <float.h>

#ifndef AQSIS_SYSTEM_WIN32
typedef int SOCKET;
#endif // !AQSIS_SYSTEM_WIN32

static std::string	g_Filename( "output.tif" );
static TqInt g_ImageWidth = 0;
static TqInt g_ImageHeight = 0;
static TqInt g_SamplesPerElement = 0;
static int g_Window = 0;
static GLubyte* g_Image = 0;
static TqFloat* g_Data = 0;
static TqInt g_CWXmin, g_CWYmin;
static TqInt g_CWXmax, g_CWYmax;

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
	if ( DDProcessMessageAsync( 0, 1000 ) )
		return ;

	// If we get this far, there aren't any more messages to process,
	// so disable this idle function
	glutIdleFunc( 0 );

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
		if ( g_Data[ i ] == FLT_MAX )
			continue;

#ifdef	AQSIS_SYSTEM_WIN32
		mindepth = min( mindepth, g_Data[ i ] );
		maxdepth = max( maxdepth, g_Data[ i ] );
#else // AQSIS_SYSTEM_WIN32
		mindepth = std::min( mindepth, g_Data[ i ] );
		maxdepth = std::max( maxdepth, g_Data[ i ] );
#endif // AQSIS_SYSTEM_WIN32
		totaldepth += g_Data[ i ];
		samples++;
	}

	const TqFloat dynamicrange = maxdepth - mindepth;

#ifdef AQSIS_SYSTEM_WIN32
	Sleep( 200 );
#else
	sleep( 2 );
#endif

	std::cout << std::endl;
	std::cout << "Total Samples: " << totalsamples << std::endl;
	std::cout << "Depth Samples: " << samples << std::endl;
	std::cout << "Coverage: " << static_cast<TqFloat>( samples ) / static_cast<TqFloat>( totalsamples ) << std::endl;
	std::cout << "Minimum Depth: " << mindepth << std::endl;
	std::cout << "Maximum Depth: " << maxdepth << std::endl;
	std::cout << "Dynamic Range: " << dynamicrange << std::endl;
	std::cout << "Average Depth: " << totaldepth / static_cast<TqFloat>( samples ) << std::endl;
	std::cout << std::endl;

	// Normalize the depth values to the range [0, 1] and regenerate our image ..
	const TqInt linelength = g_ImageWidth * 3;
	for ( TqInt y = 0; y < g_ImageHeight; y++ )
	{
		for ( TqInt x = 0; x < g_ImageWidth; x++ )
		{
			const TqInt imageindex = ( ( g_ImageHeight - y - 1 ) * linelength ) + ( x * 3 );
			const TqInt dataindex = ( y * g_ImageWidth ) + x;

			if ( g_Data[ dataindex ] == FLT_MAX )
			{
				g_Image[ imageindex + 0 ] = g_Image[ imageindex + 1 ] = g_Image[ imageindex + 2 ] = 0;
			}
			else
			{
				const TqFloat normalized = ( g_Data[ dataindex ] - mindepth ) / dynamicrange;
				g_Image[ imageindex + 0 ] = static_cast<GLubyte>( 255 * ( 1.0 - normalized ) );
				g_Image[ imageindex + 1 ] = static_cast<GLubyte>( 255 * ( 1.0 - normalized ) );
				g_Image[ imageindex + 2 ] = 255;
			}
		}
	}

	// Update the display
	glutPostRedisplay();
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

	// Lose our image & data buffers ...
	delete g_Image;
	delete g_Data;

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
	g_SamplesPerElement = message->m_SamplesPerElement;

	g_CWXmin = message->m_CropWindowXMin;
	g_CWYmin = message->m_CropWindowYMin;
	g_CWXmax = message->m_CropWindowXMax;
	g_CWYmax = message->m_CropWindowYMax;

	if ( g_SamplesPerElement > 1 )
		return -1;

	g_Image = new GLubyte[ g_ImageWidth * g_ImageHeight * 3 ];
	memset( g_Image, 128, g_ImageWidth * g_ImageHeight * 3 );

	g_Data = new TqFloat[ g_ImageWidth * g_ImageHeight ];
	memset( g_Data, 0, g_ImageWidth * g_ImageHeight );

	glutInitDisplayMode( GLUT_SINGLE | GLUT_RGBA );
	glutInitWindowSize( g_ImageWidth, g_ImageHeight );
	g_Window = glutCreateWindow( g_Filename.c_str() );

	return ( 0 );
}

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
				const TqFloat value = *reinterpret_cast<TqFloat*>( bucket );
				const TqInt imageindex = ( ( g_ImageHeight - y - 1 ) * linelength ) + ( x * 3 );
				const TqInt dataindex = ( y * g_ImageWidth ) + x;

				g_Image[ imageindex + 0 ] = g_Image[ imageindex + 1 ] = g_Image[ imageindex + 2 ] = value < FLT_MAX ? 255 : 0;
				g_Data[ dataindex ] = value;
			}
			bucket += message->m_ElementSize;
		}
	}

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
	if ( DDSendMsg( s, &closeack ) <= 0 )
		return ( -1 );
	else
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
	}
	return ( 0 );
}



