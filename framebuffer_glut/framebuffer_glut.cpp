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

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "aqsis.h"
#include <logging.h>
#include <logging_streambufs.h>

#ifdef AQSIS_SYSTEM_WIN32

#include <winsock2.h>
#include <locale>
#include <direct.h>

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

#include <tiffio.h>

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
static TqInt g_PixelsProcessed = 0;
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

typedef void (text_callback)(const std::string&);
static std::string g_text_prompt;
static std::string g_text_input;
static text_callback* g_text_ok_callback = 0;

const std::string get_window_title()
{
    std::ostringstream buffer;
    buffer << g_Filename << ": " << std::fixed << std::setprecision(1) << 100.0 * static_cast<double>(g_PixelsProcessed) / static_cast<double>(g_ImageWidth * g_ImageHeight) << "% complete";

    return buffer.str();
}

const std::string get_current_working_directory()
{
    std::string result(1024, '\0');
    getcwd(const_cast<char*>(result.c_str()), result.size());
    result.resize(strlen(result.c_str()));

    return result;
}

const std::string append_path(const std::string& LHS, const std::string& RHS)
{
    return LHS + PATH_SEPARATOR + RHS;
}

void write_tiff(const std::string& filename)
{
    TIFF* const file = TIFFOpen(filename.c_str(), "w");
    if(!file)
    {
        std::cerr << error << "Could not open [" << filename << "] for TIFF output" << std::endl;
        return;
    }

    TIFFSetField(file, TIFFTAG_IMAGEWIDTH, g_ImageWidth);
    TIFFSetField(file, TIFFTAG_IMAGELENGTH, g_ImageHeight);
    TIFFSetField(file, TIFFTAG_BITSPERSAMPLE, 8);
    TIFFSetField(file, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
    TIFFSetField(file, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    TIFFSetField(file, TIFFTAG_SAMPLESPERPIXEL, 3);
    TIFFSetField(file, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(file, TIFFTAG_ROWSPERSTRIP, 1);
    TIFFSetField(file, TIFFTAG_IMAGEDESCRIPTION, "Image rendered with Aqsis, http://www.aqsis.com");

    GLubyte* p = g_Image;
    for(int i = g_ImageHeight - 1; i >= 0; i--)
    {
        if(TIFFWriteScanline(file, p, i, 0) < 0)
        {
            TIFFClose(file);
            std::cerr << error << "Could not write data to [" << filename << "] for TIFF output" << std::endl;
            return;
        }
        p += g_ImageWidth * sizeof(GLubyte) * 3;
    }

    TIFFClose(file);
}

void text_prompt(const std::string& Prompt, const std::string& DefaultValue, text_callback* Callback)
{
    g_text_prompt = Prompt;
    g_text_input = DefaultValue;
    g_text_ok_callback = Callback;

    glutPostRedisplay();
}

void menu(int value)
{
    switch (value)
    {
    case 1:
        text_prompt("Save TIFF: ", append_path(get_current_working_directory(), g_Filename), &write_tiff);
        break;
    }
}

void draw_text(const std::string& Text)
{
    for(std::string::const_iterator c = Text.begin(); c != Text.end(); ++c)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
}

void display()
{
    glDisable(GL_BLEND);
    glRasterPos2i( 0, 0 );
    glDrawPixels( g_ImageWidth, g_ImageHeight, GL_RGB, GL_UNSIGNED_BYTE, g_Image );

    // Prompt the user for input ...
    if(g_text_prompt.size())
    {
        double viewport[4];
        glGetDoublev(GL_VIEWPORT, viewport);

        glEnable(GL_BLEND);
        glColor4d(0, 0, 0.5, 0.5);
        glRectd(viewport[0], viewport[3] / 2 + 20, viewport[2], viewport[3] / 2 - 10);

        glColor4d(1, 1, 1, 1);
        glRasterPos2d(viewport[0] + 10, viewport[3] / 2);
        draw_text(g_text_prompt);
        draw_text(g_text_input);
        draw_text("_");
    }

    // We're done ...
    glFlush();
}

void full_display()
{
    glDisable( GL_SCISSOR_TEST );
    glClear( GL_COLOR_BUFFER_BIT );

    display();
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
    // If the text prompt is active, it consumes all input ...
    if(g_text_prompt.size())
    {
        switch(key)
        {
        case 8: // Backspace
            if(g_text_input.size())
            {
                std::string::iterator i = g_text_input.end();
                g_text_input.erase(--i);
            }
            glutPostRedisplay();
            break;
        case 3: // CTRL-C
        case 27: // ESC
            g_text_prompt = "";
            g_text_input = "";
            g_text_ok_callback = 0;
            glutPostRedisplay();
            break;

        case 13: // ENTER
            g_text_ok_callback(g_text_input);
            g_text_prompt= "";
            g_text_input = "";
            g_text_ok_callback = 0;
            glutPostRedisplay();
        default:
            if(isprint(key))
            {
                g_text_input += key;
                glutPostRedisplay();
            }
            break;
        }
        return;
    }

    // No text prompt ...
    switch ( key )
    {
    case 'w':
        text_prompt("Save TIFF: ", append_path(get_current_working_directory(), g_Filename), &write_tiff);
        break;
    case 27: // ESC
    case 'q':
        exit( 0 );
        break;
    default:
        break;
    }
}

int main( int argc, char** argv )
{
    std::auto_ptr<std::streambuf> reset_level( new Aqsis::reset_level_buf(std::cerr) );
    std::auto_ptr<std::streambuf> show_timestamps( new Aqsis::timestamp_buf(std::cerr) );
    std::auto_ptr<std::streambuf> fold_duplicates( new Aqsis::fold_duplicates_buf(std::cerr) );
    std::auto_ptr<std::streambuf> show_level( new Aqsis::show_level_buf(std::cerr) );
    std::auto_ptr<std::streambuf> filter_level( new Aqsis::filter_by_level_buf(Aqsis::WARNING, std::cerr) );

    int port = -1;
    char *portStr = getenv( "AQSIS_DD_PORT" );

    if ( portStr != NULL )
    {
        port = atoi( portStr );
    }

    if ( -1 == DDInitialise( NULL, port ) )
    {
        std::cerr << error << "Could not open communications channel to Aqsis" << std::endl;
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
    glutDisplayFunc( full_display );
    glutReshapeFunc( reshape );
    glutKeyboardFunc( keyboard );
    glutIdleFunc( idle );
    glutCreateMenu(menu);
    glutAddMenuEntry("Write TIFF file", 1);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    // Setup GL context.
    glClearColor( 0.0, 0.0, 0.0, 0.0 );
    glDisable( GL_DEPTH_TEST );
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
    g_PixelsProcessed = 0;

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

            if ( ( (g_ImageHeight - 1 - i) & 31 ) < 16 ) t ^= 1;
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
    g_Window = glutCreateWindow( get_window_title().c_str() );

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
    display();

    g_PixelsProcessed += (BucketW * BucketH);
    glutSetWindowTitle(get_window_title().c_str());

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



