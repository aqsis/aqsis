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

#include <aqsis.h>
#include <dd.h>
#include <displaydriver.h>
#include <logging.h>
#include <logging_streambufs.h>

#include <tiffio.h>

using namespace Aqsis;

#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

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

#ifdef AQSIS_SYSTEM_MACOSX

	#include <GLUT/glut.h>
	#include <GLUT/macxglut_utilities.h>
	#include <ApplicationServices/ApplicationServices.h>
	
#else // AQSIS_SYSTEM_MACOSX

	#include <GL/glut.h>

#endif //!AQSIS_SYSTEM_MACOSX

namespace
{

bool g_DoubleBuffer = true;
std::string g_Filename( "output.tif" );
TqInt g_ImageWidth = 0;
TqInt g_ImageHeight = 0;
TqInt g_PixelsProcessed = 0;
TqInt g_Channels = 0;
TqInt g_Format = 0;
int g_Window = 0;
GLubyte* g_Image = 0;
TqInt g_CWXmin, g_CWYmin;
TqInt g_CWXmax, g_CWYmax;
TqFloat g_QuantizeZeroVal = 0.0f;
TqFloat g_QuantizeOneVal = 0.0f;
TqFloat g_QuantizeMinVal = 0.0f;
TqFloat g_QuantizeMaxVal = 0.0f;
TqFloat g_QuantizeDitherVal = 0.0f;

std::string g_TextPrompt;
std::string g_TextInput;
typedef void (TextCallback)(const std::string&);
TextCallback* g_TextOKCallback = 0;

const std::string GetWindowTitle()
{
    std::ostringstream buffer;
    buffer << g_Filename << ": " << std::fixed << std::setprecision(1) << 100.0 * static_cast<double>(g_PixelsProcessed) / static_cast<double>(g_ImageWidth * g_ImageHeight) << "% complete";

    return buffer.str();
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

void TextPrompt(const std::string& Prompt, const std::string& DefaultValue, TextCallback* Callback)
{
    g_TextPrompt = Prompt;
    g_TextInput = DefaultValue;
    g_TextOKCallback = Callback;

    glutPostRedisplay();
}

void CancelTextPrompt()
{
    g_TextPrompt.erase();
    g_TextInput.erase();
    g_TextOKCallback = 0;

    glutPostRedisplay();
}

void DrawText(const std::string& Text)
{
    for(std::string::const_iterator c = Text.begin(); c != Text.end(); ++c)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
}

void GetImageOffset(GLint& X, GLint& Y)
{
    double viewport[4];
    glGetDoublev(GL_VIEWPORT, viewport);

    X = static_cast<GLint>(MAX(0.0, (viewport[2] - g_ImageWidth) / 2));
    Y = static_cast<GLint>(MAX(0.0, (viewport[3] - g_ImageHeight) / 2));
}

void Display()
{
    GLint imagex, imagey = 0;
    GetImageOffset(imagex, imagey);

    glDisable(GL_BLEND);
    glRasterPos2i( imagex, imagey );
    glDrawPixels( g_ImageWidth, g_ImageHeight, GL_RGB, GL_UNSIGNED_BYTE, g_Image );

    // Prompt the user for input ...
    if(!g_TextPrompt.empty())
    {
        double viewport[4];
        glGetDoublev(GL_VIEWPORT, viewport);

        glEnable(GL_BLEND);
        glColor4d(0, 0, 0.5, 0.5);
        glRectd(viewport[0], viewport[3] / 2 + 20, viewport[2], viewport[3] / 2 - 10);

        glColor4d(1, 1, 1, 1);
        glRasterPos2d(viewport[0] + 10, viewport[3] / 2);
        DrawText(g_TextPrompt);
        DrawText(g_TextInput);
        DrawText("_");
    }

    // Note ... calling glutSwapBuffers() implicitly calls glFlush(),
    // and is safe for single-buffered windows
    glutSwapBuffers();
}

void BucketDisplay(const GLint X, const GLint Y, const GLsizei Width, const GLsizei Height)
{
    GLint imagex, imagey = 0;
    GetImageOffset(imagex, imagey);

    glScissor( X + imagex, Y + imagey, Width, Height );
    glEnable( GL_SCISSOR_TEST );

    Display();
}

void FullDisplay()
{
    glDisable( GL_SCISSOR_TEST );
    glClear( GL_COLOR_BUFFER_BIT );

    Display();
}

void OnReshape( int w, int h )
{
    glViewport( 0, 0, ( GLsizei ) w, ( GLsizei ) h );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluOrtho2D( 0.0, ( GLdouble ) w, 0.0, ( GLdouble ) h );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
}

void OnIdle( void )
{
    if ( !DDProcessMessageAsync( 0, 1000 ) )
        glutIdleFunc( 0 );
}

void OnKeyboard( unsigned char key, int x, int y )
{
    // If the text prompt is active, it consumes all input ...
    if(g_TextPrompt.size())
    {
        switch(key)
        {
        case 8: // Backspace
            if(g_TextInput.size())
	    {
				std::string::iterator i = g_TextInput.end();
				g_TextInput.erase(--i);
                glutPostRedisplay();
	    }
            break;
        case 3: // CTRL-C
        case 27: // ESC
	    CancelTextPrompt();
            break;

        case 13: // ENTER
            g_TextOKCallback(g_TextInput);
	    CancelTextPrompt();
	    break;
        default:
            if(isprint(key))
            {
                g_TextInput += key;
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
        TextPrompt("Save TIFF: ", AppendPath(GetCurrentWorkingDirectory(), g_Filename), &WriteTIFF);
        break;
    case 3: // CTRL-C
    case 27: // ESC
    case 'q':
        exit( 0 );
        break;
    default:
        break;
    }
}

void OnMenu(int value)
{
    switch (value)
    {
    case 1:
        TextPrompt("Save TIFF: ", AppendPath(GetCurrentWorkingDirectory(), g_Filename), &WriteTIFF);
        break;
    }
}

} // namespace

int main( int argc, char** argv )
{
    // Setup logging output options (write all log messages to std::cerr)
    std::auto_ptr<std::streambuf> reset_level( new reset_level_buf(std::cerr) );
    std::auto_ptr<std::streambuf> show_timestamps( new timestamp_buf(std::cerr) );
    std::auto_ptr<std::streambuf> fold_duplicates( new fold_duplicates_buf(std::cerr) );
    std::auto_ptr<std::streambuf> show_level( new show_level_buf(std::cerr) );
    std::auto_ptr<std::streambuf> filter_level( new filter_by_level_buf(INFO, std::cerr) );

    // Connect to Aqsis ...
    const char* port_string = getenv( "AQSIS_DD_PORT" );
    int port = port_string ? atoi(port_string) : -1;

    if ( -1 == DDInitialise( NULL, port ) )
    {
        std::cerr << error << "Could not open communications channel to Aqsis" << std::endl;
        return 1;
    }

    // Initialize GLUT so we can create a window ...
    glutInit( &argc, argv );

    // Process incoming messages from Aqsis until we have enough data to create a window ...
    while ( 0 == g_Window )
    {
        if ( !DDProcessMessage() )
        {
            std::cerr << "Premature end of messages" << std::endl;
            return 2;
        }
    }

    // Start the glut message loop ...
    glutDisplayFunc( FullDisplay );
    glutReshapeFunc( OnReshape );
    glutKeyboardFunc( OnKeyboard );
    glutIdleFunc( OnIdle );
    glutCreateMenu(OnMenu);
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

    return 0;
}

////////////////////////////////////////////////////////////////////////////
// Functions required by libdd.

TqInt Query( SOCKET s, SqDDMessageBase* pMsgB )
{
    static SqDDMessageFormatResponse frmt( DataFormat_Signed32 );

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


    glutInitDisplayMode( ( g_DoubleBuffer ? GLUT_DOUBLE : GLUT_SINGLE) | GLUT_RGBA);
    glutInitWindowSize( g_ImageWidth, g_ImageHeight );
    g_Window = glutCreateWindow( GetWindowTitle().c_str() );

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

                if( !( g_QuantizeZeroVal == 0.0f &&
                        g_QuantizeOneVal  == 0.0f &&
                        g_QuantizeMinVal  == 0.0f &&
                        g_QuantizeMaxVal  == 0.0f ) )
                {
                    value0 = ROUND(g_QuantizeZeroVal + value0 * (g_QuantizeOneVal - g_QuantizeZeroVal) + g_QuantizeDitherVal );
                    value0 = CLAMP(value0, g_QuantizeMinVal, g_QuantizeMaxVal) ;
                    value1 = ROUND(g_QuantizeZeroVal + value1 * (g_QuantizeOneVal - g_QuantizeZeroVal) + g_QuantizeDitherVal );
                    value1 = CLAMP(value1, g_QuantizeMinVal, g_QuantizeMaxVal) ;
                    value2 = ROUND(g_QuantizeZeroVal + value2 * (g_QuantizeOneVal - g_QuantizeZeroVal) + g_QuantizeDitherVal );
                    value2 = CLAMP(value2, g_QuantizeMinVal, g_QuantizeMaxVal) ;
                    alpha  = ROUND(g_QuantizeZeroVal + alpha * (g_QuantizeOneVal - g_QuantizeZeroVal) + g_QuantizeDitherVal );
                    alpha  = CLAMP(alpha, g_QuantizeMinVal, g_QuantizeMaxVal) ;
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
                    int A = static_cast<int>(INT_PRELERP( g_Image[ so + 0 ], value0, alpha, t ));
                    int B = static_cast<int>(INT_PRELERP( g_Image[ so + 1 ], value1, alpha, t ));
                    int C = static_cast<int>(INT_PRELERP( g_Image[ so + 2 ], value2, alpha, t ));
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

    BucketDisplay( BucketX, BucketY, BucketW, BucketH );
    
    g_PixelsProcessed += (BucketW * BucketH);
    glutSetWindowTitle(GetWindowTitle().c_str());

    return ( 0 );
}

TqInt Close( SOCKET s, SqDDMessageBase* pMsgB )
{
    glutPostRedisplay();

    static SqDDMessageCloseAcknowledge closeack;

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
                g_QuantizeZeroVal = quantize[0];
                g_QuantizeOneVal  = quantize[1];
                g_QuantizeMinVal  = quantize[2];
                g_QuantizeMaxVal  = quantize[3];
            }
            else if( strncmp( pMsg->m_NameAndData, "doublebuffer", pMsg->m_NameLength ) == 0 )
            {
                g_DoubleBuffer = *reinterpret_cast<TqInt*>( &pMsg->m_NameAndData[ pMsg->m_NameLength + 1 ] ) != 0;
            }
        }
        break;
    }
    return ( 0 );
}

