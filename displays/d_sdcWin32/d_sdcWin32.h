/******************************************************************************/
/* COPYRIGHT                                                                  */
/*                                                                            */
/* Copyright 2000 by Schroff Development Corporation, Shawnee-Mission,        */
/* Kansas, United States of America. All rights reserved.                     */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* This Display Driver is distributed as "freeware". There are no             */
/* restrictions on its' usage.                                                */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* DISCLAIMER OF ALL WARRANTIES AND LIABILITY                                 */
/*                                                                            */
/* Schroff Development Corporation makes no warranties, either expressed      */
/* or implied, with respect to the programs contained within this file, or    */
/* with respect to software used in conjunction with these programs. The      */
/* programs are distributed 'as is'.  The entire risk as to its quality and   */
/* performance is assumed by the purchaser.  Schroff  Development Corporation */
/* does not guarantee, warrant or make any representation regarding the       */
/* use of, or the results of the use of the programs in terms of correctness, */
/* accuracy, reliability, or performance. Schroff Development Corporation     */
/* assumes no liability for any direct, indirect, or consquential, special    */
/* or exemplary damages, regardless of its having been advised of the         */
/* possibility of such damage.                                                */
/*                                                                            */
/******************************************************************************/


//
// This is a Display Driver that was written to comply with the PhotoRealistic
// RenderMan Display Driver Implementation Guide (on the web at:
// www.pixar.com/products/rendermandocs/toolkit/Toolkit/dspy.html).
//
// This driver displays image data to a Win32 window which it creates and
// manages.
//

// This was modified slight by me (joron@sympatico.ca) to have ORDERLINE 
// set as environment variable where we could see in action the line 
// by line rendering

// -----------------------------------------------------------------------------
// Local structures
// -----------------------------------------------------------------------------

typedef struct
{
	// Application data

	HINSTANCE   hInstance;
	WNDCLASS    WindowClass;


	// App window data

	HWND        hWnd;
	HDC         hDC;


	// Bitmap data

	HBITMAP     hDIB;
	BITMAPINFO  bmi;
	char        *ImageData;
	int         Channels;
	int         RowSize;
	int         PixelBytes;
}
AppData;

static const int     DEFAULT_IMAGEWIDTH         = 512;   // Tiff display driver defaults are good enough for me.
static const int     DEFAULT_IMAGEHEIGHT        = 384;
static const float   DEFAULT_PIXELASPECTRATIO   = 1.0f;

//
// Set SHOW_CALLSTACK to 1 if you wish to see trace messages
// from the driver written to stderr.
//

#define SHOW_CALLSTACK 0

//
// Set SHOW_LINEORDER to 1 if you wish to see bucket/pixels messages
// draw rows by rows if LINEORDER environment is set
//

#define SHOW_ORDERLINE 1
