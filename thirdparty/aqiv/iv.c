// Aqsis
// Copyright (C) 1997 - 2003, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
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
		\brief Implements a IV GLUT based viewer for Aqsis.
			It is based on framebuffer_glut and
				sgigt from libtiff source/contribs
		\author Michel Joron joron@sympatico.ca
*/

#include "tiffio.h"
#include <stdio.h>
#include <aqsis/config.h>

#ifndef AQSIS_SYSTEM_WIN32
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>


#ifdef AQSIS_SYSTEM_MACOSX
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#include <GL/gl.h>
#endif //!AQSIS_SYSTEM_MACOSX


#include "sgigt.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif


GLint g_ImageWidth = 100;
GLint g_ImageHeight = 100;
extern GLubyte* g_Image;

TIFFErrorHandler oerror;
TIFFErrorHandler owarning;

void *font = GLUT_BITMAP_9_BY_15;

unsigned short iv_which = ALL;  /* (0xF eg. RGBA enable) */
char iv_direct[ 1024 ]; /* Current working directory */

int argc;
char **argv;

static GLfloat zoom= 1.0f;
static GLint gx =0, gy = 0;

static GLubyte *todisplay = NULL;
static void output(int x, int y, char *string);

//----------------------------------------------------------------------------
// Draw a current TIFF image to the OpenGL context
//----------------------------------------------------------------------------
void display( void )
{
	char * filename = sgigt_get_filename();
	int i, j;
	static int hugesize = 0;

	glDisable( GL_SCISSOR_TEST );
	glClear( GL_COLOR_BUFFER_BIT );
	
	glRasterPos2i(0, 0);

	/* Keep track of the largest buffer as possible */
	if ( hugesize == 0 )
	{
		todisplay = (GLubyte *) calloc( g_ImageWidth * g_ImageHeight, 4 );
		hugesize = g_ImageWidth * g_ImageHeight;
	}
	else if ( hugesize < g_ImageWidth * g_ImageHeight )
	{
		todisplay = (GLubyte *) realloc( todisplay, g_ImageWidth * g_ImageHeight * 4 );
		hugesize = g_ImageWidth * g_ImageHeight;
	}
	else
	{
		memset( todisplay, '\0', hugesize * 4 );
	}

	/* Compute R, G, B, A components if required */
	for ( i = 0; i < g_ImageHeight; i++ ) for ( j = 0; j < g_ImageWidth; j++ )
		{
#ifdef AQSIS_SYSTEM_MACOSX
			if ( iv_which & RED )   /* red */
				todisplay[ i * g_ImageWidth * 4 + j * 4 ] = ( ( GLubyte * ) g_Image ) [ i * g_ImageWidth * 4 + 4 * j + 3 ];
			if ( iv_which & GREEN )   /* green */
				todisplay[ i * g_ImageWidth * 4 + j * 4 + 1 ] = ( ( GLubyte * ) g_Image ) [ i * g_ImageWidth * 4 + 4 * j + 2 ];
			if ( iv_which & BLUE )   /* blue */
				todisplay[ i * g_ImageWidth * 4 + j * 4 + 2 ] = ( ( GLubyte * ) g_Image ) [ i * g_ImageWidth * 4 + 4 * j + 1 ];
			if ( iv_which & ALPHA )   /* alpha */
				todisplay[ i * g_ImageWidth * 4 + j * 4 + 3 ] = ( ( GLubyte * ) g_Image ) [ i * g_ImageWidth * 4 + 4 * j ];
#else
			/* AQSIS_SYSTEM_WIN32 */
			if ( iv_which & RED )   /* red */
				todisplay[ i * g_ImageWidth * 4 + j * 4 ] = ( ( GLubyte * ) g_Image ) [ i * g_ImageWidth * 4 + 4 * j ];
			if ( iv_which & GREEN )   /* green */
				todisplay[ i * g_ImageWidth * 4 + j * 4 + 1 ] = ( ( GLubyte * ) g_Image ) [ i * g_ImageWidth * 4 + 4 * j + 1 ];
			if ( iv_which & BLUE )   /* blue */
				todisplay[ i * g_ImageWidth * 4 + j * 4 + 2 ] = ( ( GLubyte * ) g_Image ) [ i * g_ImageWidth * 4 + 4 * j + 2 ];
			if ( iv_which & ALPHA )   /* alpha */
				todisplay[ i * g_ImageWidth * 4 + j * 4 + 3 ] = ( ( GLubyte * ) g_Image ) [ i * g_ImageWidth * 4 + 4 * j + 3 ];
#endif

		}

	/* Send the Pixels to the screen */
       
	glPixelZoom(zoom, zoom);
	glDrawPixels( g_ImageWidth, g_ImageHeight, GL_RGBA, GL_UNSIGNED_BYTE, todisplay);
	glutSetWindowTitle( filename );
	glFlush();
}

//----------------------------------------------------------------------------
// Resize a GLUT window
//----------------------------------------------------------------------------
void reshape( int w, int h )
{
	glViewport( 0, 0, ( GLsizei ) w, ( GLsizei ) h );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluOrtho2D( 0.0, ( GLdouble ) w, 0.0, ( GLdouble ) h );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
}

//----------------------------------------------------------------------------
// Idle function for GLUT window
//----------------------------------------------------------------------------
void idle( void )
{
	glutIdleFunc( 0 );
}

//----------------------------------------------------------------------------
// Left mouse button does '<', right mouse '>', and the middle print RGB values
//----------------------------------------------------------------------------
void mouse( int which, int state, int x, int y )
{
	extern	int optind;
	int nix, ix;
	GLubyte rgba[ 4 ];
	char buffer[100];

	if ( !state ) return ;


	ix = sgigt_get_ix();
	switch ( which )
	{
			case GLUT_MIDDLE_BUTTON:
			{

				/* This is dump the RGB value of the framebuffer when
				 * it should used Img->Map information instead
				 * I plan here to start an new window GLUT which will
				 * enhance the pixel area of the X,Y pixels.
				 */
				glReadPixels(
				    x,
				    g_ImageHeight - y,
				    zoom,
				    zoom,
				    GL_RGBA,
				    GL_UNSIGNED_BYTE,
				    &rgba
				);

				sprintf( buffer, "(%d,%d) RGB( 0x%02X, 0x%02X, 0x%02X )",
				         x, y,
				         rgba[ 0 ],
				         rgba[ 1 ],
				         rgba[ 2 ] );
				if (zoom != 1.0) {
					char zoomb[50];
					   sprintf(zoomb, " zoom %4.2f", zoom);
					strcat(buffer, zoomb);

				}

				

			}
			break;

			case GLUT_LEFT_BUTTON:

			{

				zoom *= 0.80;
				if (zoom < 1.0) zoom = 1.0;
				gx = x;
				gy = y;
#ifdef MJO
				if ( nix = sgigt_prev_image( argv, ix, optind, argc, FALSE ) )
				{
					ix = nix;
					sgigt_new_file( ix );
				}
#endif

			}
			break;
			case GLUT_RIGHT_BUTTON:

			{
				zoom *= 1.25;
				if (zoom > 16.0) zoom = 1.0;
#ifdef MJO
				if ( nix = sgigt_next_image( argv, ix, optind, argc, FALSE ) )
				{
					ix = nix;
					sgigt_new_file( ix );
				}
#endif
			}
			break;
	}
	display();
	if (which == GLUT_MIDDLE_BUTTON)
		output(0,0,buffer);
}



//----------------------------------------------------------------------------
// Keyboard operation right:
// 'r', 'b', 'g', 'a' display red, blue, green and alpha information
// '<', previous file
// '>', next file
// 'D' previous directory (bigger)
// 'd' next directory (smaller)
// '.' reset
// and anything else reset
// ESC, 'q' quite aqiv
// 'E', "W" level of warning for TIFF files error status
//----------------------------------------------------------------------------
void keyboard( unsigned char key, int x, int y )
{
	extern	int optind;
	TIFF * tif;
	int nix, ix;
	ix = sgigt_get_ix();

	tif = sgigt_get_tif();

	switch ( key )
	{
			case 27:
			case 'q':  /* Goodbye cruel world */
			{
				if ( todisplay )
					free( todisplay );
				exit( 0 );
			}
			break;
			case 'a':  /* Alpha information Only */
			{
				if ( iv_which != ALPHA )
					iv_which = ALPHA;
				else
					iv_which = ALL;
			}
			break;
			case 'r':  /* Red information Only */
			{
				if ( iv_which != RED )
					iv_which = RED;
				else
					iv_which = ALL;
			}
			break;
			case 'g':  /* Green information Only */
			{
				if ( iv_which != GREEN )
					iv_which = GREEN;
				else
					iv_which = ALL;
			}
			break;
			case 'b':  /* Blue information Only */
			{
				if ( iv_which != BLUE )
					iv_which = BLUE;
				else
					iv_which = ALL;
			}
			break;
			case 'W':
			{ /* toggle warnings */
				owarning = TIFFSetWarningHandler( owarning );
				sgigt_new_page();
			}
			break;
			case 'E':   			/* toggle errors */
			{
				oerror = TIFFSetErrorHandler( oerror );
				sgigt_new_page();
			}
			break;
			case 'z':   			/* reset to defaults */
			case 'Z':
			{
				iv_which = ALL;
				if ( owarning == NULL )
					owarning = TIFFSetWarningHandler( NULL );
				if ( oerror == NULL )
					oerror = TIFFSetErrorHandler( NULL );
				sgigt_new_page();
				zoom = 1.0;
			}
			break;
			case 'D':   			/* previous logical image */
			{
				if ( TIFFCurrentDirectory( tif ) > 0 )
				{
					if ( TIFFSetDirectory( tif, TIFFCurrentDirectory( tif ) - 1 ) )
						sgigt_new_page();
				}
				else
				{
					if ( TIFFSetDirectory( tif, 0 ) )
						sgigt_new_page();
				}

			}
			break;
			case 'd':   			/* next logical image */
			{
				if ( !TIFFLastDirectory( tif ) )
				{
					if ( TIFFReadDirectory( tif ) )
						sgigt_new_page();
				}
				else
				{
					if ( TIFFSetDirectory( tif, 0 ) )
						sgigt_new_page();
				}
			}
			break;
			case '0':   			/* 1st image in current file */
			{
				if ( TIFFSetDirectory( tif, 0 ) )
					sgigt_new_page();
			}
			break;


			case '<':
			{		/* previous file */

				if ( nix = sgigt_prev_image( argv, ix, optind, argc, FALSE ) )
				{
					ix = nix;
					sgigt_new_file( ix );
					zoom = 1.0;
				}
			}
			break;
			case '>':   			/* next file */
			{

				if ( nix = sgigt_next_image( argv, ix, optind, argc, FALSE ) )
				{
					ix = nix;
					sgigt_new_file( ix );
					zoom = 1.0;
				}


			}
			break;
			case '.':   			/* first file */
			{
				iv_which = ALL;
				if ( nix = sgigt_next_image( argv, optind - 1, optind, argc, FALSE ) )
				{
					ix = nix;
					sgigt_new_file( ix );
					zoom = 1.0;
				}

			}
			break;

			default:
			iv_which = ALL;
			break;

	}
	display();
}


//----------------------------------------------------------------------------
// main() the root of all the problem
//----------------------------------------------------------------------------
int main( int targc, char** targv )
{
	char * filename;
	int fd;
	int g_Window;

	g_Window = 0;

#ifdef AQSIS_SYSTEM_MACOSX
	getwd( iv_direct );
	fd = chdir( "." );
	fchdir( fd );
#else
	getcwd( iv_direct, 1024 );
#endif

	argc = targc;
	argv = targv;
	glutInit( &argc, argv );

	oerror = TIFFSetErrorHandler( NULL );
	owarning = TIFFSetWarningHandler( NULL );

	sgigt_read_images( targc, targv );
	filename = sgigt_get_filename();

	glutInitDisplayMode( GLUT_SINGLE | GLUT_RGBA );
	glutInitWindowSize( g_ImageWidth, g_ImageHeight );
	g_Window = glutCreateWindow( filename );


	reshape( g_ImageWidth, g_ImageHeight );

	// Start the glut message loop ...
	glutDisplayFunc( display );
	glutReshapeFunc( reshape );
	glutKeyboardFunc( keyboard );
	glutMouseFunc( mouse );
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

	// Free our image buffer ...
	if ( todisplay ) free( todisplay );

#ifdef AQSIS_SYSTEM_MACOSX
	close( fd );
#endif
	return 0;
}


static void output(int x, int y, char *string)
{
  int len, i;

  glColor3f(1.0, 1.0, 1.0);
    
  glRasterPos2f(x, y);
  len = (int) strlen(string);
  for (i = 0; i < len; i++) {
    glutBitmapCharacter(font, string[i]);
  }
  glFlush();
}

