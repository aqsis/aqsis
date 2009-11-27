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

#define NOMINMAX
#include <windows.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>

extern "C"
{
#include <aqsis/ri/ndspy.h>
#include "d_sdcWin32.h"
}




// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------



static const DWORD   APP_WINDOW_STYLE =
    WS_VISIBLE    |   // Make this window visible

    WS_OVERLAPPED |   // Creates an overlapped window. An overlapped window
    // has a title bar and a border. Same as the WS_TILED
    // style.

    WS_CAPTION    |   // Creates a window that has a title bar (includes the
    // WS_BORDER style).

    WS_SYSMENU    |   // Creates a window that has a window-menu on its title
    // bar. The WS_CAPTION style must also be specified.

    WS_MINIMIZEBOX;   // Creates a window that has a Minimize button. Cannot
// be combined with the WS_EX_CONTEXTHELP style. The
// WS_SYSMENU style must also be specified.



// -----------------------------------------------------------------------------
// Function Prototypes
// -----------------------------------------------------------------------------


static LRESULT CALLBACK MoeWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static bool             InitApplication(HINSTANCE hInstance);
static bool             ExitApplication();



// -----------------------------------------------------------------------------
// Global Data
// -----------------------------------------------------------------------------

static int     AppActive = false;
static AppData g_Data;

//******************************************************************************
// DllMain
//
// This is the main entry point into the DLL.
//******************************************************************************

BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD  Reason,
                      LPVOID lpReserved)
{
	switch ( Reason )
	{
			case DLL_PROCESS_ATTACH:
			case DLL_THREAD_ATTACH:

#if SHOW_CALLSTACK

			fprintf(stderr, "sdcWin32_ATTACH\n");
#endif

			return InitApplication((HINSTANCE) hModule);


			case DLL_PROCESS_DETACH:
			case DLL_THREAD_DETACH:

#if SHOW_CALLSTACK

			fprintf(stderr, "sdcWin32_DETACH\n");
#endif

			if ( g_Data.hWnd && ::IsWindow(g_Data.hWnd) && ::IsWindowVisible(g_Data.hWnd) )
			{
				//
				// This message pump will keep the window around until the user
				// closes it.
				//
				// I originally thought that this code should be placed in
				// DspyImageDelayClose, but that is called for every frame
				// of an animation.
				//

				MSG  msg;

				while ( GetMessage(&msg, NULL, 0, 0) )
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}


			return ExitApplication();
	}

	return TRUE;
}

//******************************************************************************
// DWORDALIGN
//******************************************************************************

static int DWORDALIGN(int bits)
{
	return ((bits + 31) >> 5) << 2;
}



//******************************************************************************
// MainWndProc
//
// Message processing function for the display driver's window
//******************************************************************************

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch ( msg )
	{
       	/*********************/
       	/* Special key input */
       	/*********************/
         case WM_CHAR : {
         		if (wParam == VK_ESCAPE )
				      PostQuitMessage(0);
			} break;
      	
         case WM_KEYDOWN : {
         		if (wParam == VK_DELETE )
				      PostQuitMessage(0);
			} break;
	
      	case WM_LBUTTONUP:
			case WM_DESTROY:
				PostQuitMessage(0);
				break;

			case WM_PAINT:

			if ( g_Data.hDC && g_Data.hDIB )
			{
				GdiFlush();

				::SetDIBitsToDevice(g_Data.hDC,
				                    0,
				                    0,
				                    g_Data.bmi.bmiHeader.biWidth,
				                    g_Data.bmi.bmiHeader.biHeight,
				                    0,
				                    0,
				                    0,
				                    g_Data.bmi.bmiHeader.biHeight,
				                    g_Data.ImageData,
				                    &g_Data.bmi,
				                    DIB_RGB_COLORS);
			}
			break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}



//******************************************************************************
// InitApplication
//******************************************************************************

static bool InitApplication(HINSTANCE hInstance)
{
#if SHOW_CALLSTACK
	fprintf(stderr, "sdcWin32_InitApplication called.\n");
#endif

	// If the user hits Ctrl+C while aqsis is rendering a multi-frame
	// rib file, the display driver can be left in limbo (the DLL might
	// not be unloaded properly). The following test attempts to correct
	// for that situation.

	if ( AppActive )
	{
		ExitApplication();
		return false;
	}


	// Initialize our global application data

	memset(&g_Data, 0, sizeof(AppData));


	// Save our instance handle.
	g_Data.hInstance = hInstance;


	// Register a window class for the application

	g_Data.WindowClass.style         = CS_OWNDC;
	g_Data.WindowClass.lpfnWndProc   = (WNDPROC)MainWndProc;
	g_Data.WindowClass.hInstance     = hInstance;
	g_Data.WindowClass.hIcon         = LoadIcon(NULL, IDI_WARNING);
	g_Data.WindowClass.hCursor       = LoadCursor(hInstance, IDC_ARROW);
	g_Data.WindowClass.lpszClassName = "sdcWin32";
	g_Data.WindowClass.hbrBackground = NULL;


	if ( ! RegisterClass(&g_Data.WindowClass) )
	{
		fprintf(stderr, "sdcWin32_InitApplication: Class registration failed\n");
		return false;
	}

	AppActive = true; // Elvis has entered the building.

	return true;
}



//******************************************************************************
// ExitApplication
//******************************************************************************

static bool ExitApplication()
{
#if SHOW_CALLSTACK
	fprintf(stderr, "sdcWin32_ExitApplication called.\n");
#endif


	if ( g_Data.hDIB )
		::DeleteObject(g_Data.hDIB);
	g_Data.hDIB = NULL;
	g_Data.ImageData = NULL;

	if ( g_Data.hDC )
		DeleteDC(g_Data.hDC);
	g_Data.hDC = NULL;

	if ( g_Data.hWnd )
		DestroyWindow(g_Data.hWnd);
	g_Data.hWnd = NULL;


	if ( ! UnregisterClass(g_Data.WindowClass.lpszClassName, g_Data.hInstance) )
		return false;

	AppActive = false;   // Elvis has left the building.

	return true;
}



//******************************************************************************
// CreateAppWindow
//
// This function creates the application window and also allocates an image
// buffer that accompanies it.
//******************************************************************************

static PtDspyError CreateAppWindow(const char *title, int cx, int cy)
{
	RECT r;


	if (AppActive == false)
		InitApplication(GetModuleHandle(NULL));

	// Create a window for the application

	r.left   = 0;
	r.top    = 0;
	r.right  = cx;
	r.bottom = cy;

	AdjustWindowRect(&r, APP_WINDOW_STYLE, false);

	g_Data.hWnd = CreateWindow(g_Data.WindowClass.lpszClassName,
	                           title[0] ? title : "Win32",
	                           APP_WINDOW_STYLE,
	                           CW_USEDEFAULT,
	                           CW_USEDEFAULT,
	                           r.right - r.left,
	                           r.bottom - r.top,
	                           NULL,
	                           NULL,
	                           g_Data.hInstance,
	                           &g_Data);

	if ( ! g_Data.hWnd )
	{
		fprintf(stderr, "sdcWin32_CreateAppWindow: Cannot create an application window.\n");
		return PkDspyErrorNoResource;
	}

	g_Data.hDC = ::GetDC(g_Data.hWnd);   // We can do this because of CS_OWNDC


	// Can this device support SetDIBitsToDevice calls?

	if ( ! (::GetDeviceCaps(g_Data.hDC, RASTERCAPS) & RC_DIBTODEV) )
		fprintf(stderr, "sdcWin32_CreateAppWindow: Warning, RC_DIBTODEV not supported.\n");


	// Create a DIB for the window.

	g_Data.hDIB = ::CreateDIBSection(g_Data.hDC,
	                                 &g_Data.bmi,
	                                 DIB_RGB_COLORS,
	                                 (void **)&g_Data.ImageData,
	                                 NULL,
	                                 NULL);


	if ( ! g_Data.hDIB || ! g_Data.ImageData )
	{
		fprintf(stderr, "sdcWin32_DspyImageOpen: Insufficient Memory\n");
		return PkDspyErrorNoResource;
	}

	return PkDspyErrorNone;
}

//******************************************************************************
// DspyImageOpen
//
// Initializes the display driver, allocates necessary resources, checks image
// size, specifies format in which incoming data will arrive.
//******************************************************************************

extern "C" PtDspyError DspyImageOpen(PtDspyImageHandle    *image,
                          const char           *drivername,
                          const char           *filename,
                          int                  width,
                          int                  height,
                          int                  paramCount,
                          const UserParameter  *parameters,
                          int                  formatCount,
                          PtDspyDevFormat      *format,
                          PtFlagStuff          *flagstuff)
{
#if SHOW_CALLSTACK
	fprintf(stderr, "sdcWin32_DspyImageOpen called.\n");
#endif

#if SHOW_ORDERLINE

	if (getenv("ORDERLINE"))
		flagstuff->flags = PkDspyFlagsWantsScanLineOrder;
#endif
	//
	// This reuse of hWnd and ImageData is an optimization for the rendering
	// of multi-framed RIBs. This code assumes that the window size will not change
	// willy-nilly and that there is therefore no reason to allocate a new
	// image buffer for each frame.
	//

	if ( g_Data.hWnd )
	{
		if ( width != g_Data.bmi.bmiHeader.biWidth )
		{
			fprintf(stderr, "sdcWin32_DspyImageOpen: Driver doesn't support multiple Format commands\n");
			return PkDspyErrorBadParams;
		}

		if ( height != g_Data.bmi.bmiHeader.biHeight )
		{
			fprintf(stderr, "sdcWin32_DspyImageOpen: Driver doesn't support multiple Format commands\n");
			return PkDspyErrorBadParams;
		}

		memset(g_Data.ImageData, 0, g_Data.bmi.bmiHeader.biSizeImage);

		return PkDspyErrorNone;
	}



	if ( width <= 0 )
		width = DEFAULT_IMAGEWIDTH;

	if ( height <= 0 )
		height = DEFAULT_IMAGEHEIGHT;

	*image = &g_Data;

	g_Data.Channels = formatCount;

	g_Data.PixelBytes = 3; // One byte for red, one for green, and one for blue.

	g_Data.bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	g_Data.bmi.bmiHeader.biWidth       = width;
	g_Data.bmi.bmiHeader.biHeight      = height;
	g_Data.bmi.bmiHeader.biPlanes      = 1;
	g_Data.bmi.bmiHeader.biBitCount    = 24;
	g_Data.bmi.bmiHeader.biCompression = BI_RGB;
	g_Data.RowSize                     = DWORDALIGN(width * g_Data.bmi.bmiHeader.biBitCount);
	g_Data.bmi.bmiHeader.biSizeImage   = g_Data.RowSize * height;

	return CreateAppWindow(filename, width, height);
}

//******************************************************************************
// DspyImageData
//
// Send data to the display driver.
//******************************************************************************
extern "C" PtDspyError DspyImageData(PtDspyImageHandle image,
                          int xmin,
                          int xmax_plusone,
                          int ymin,
                          int ymax_plusone,
                          int entrysize,
                          const unsigned char *data)
{
#if SHOW_CALLSTACK
	fprintf(stderr, "sdcWin32_DspyImageData called.\n");
#endif

	int  x, y;
	int  r, g, b;
	char *spot;
	char msg[80];
	char file[80];
	float percent = 0.0f;


	if ( ! g_Data.hWnd )
		return PkDspyErrorNoResource;

	//
	// When PkDspyFlagsWantsNullEmptyBuckets is turned on in DspyImageOpen, then
	// DspyImageData will be called with 'data' set to NULL.
	//

	for (y = ymin; y < ymax_plusone; y++)
	{
		spot  = g_Data.ImageData + (g_Data.RowSize * (g_Data.bmi.bmiHeader.biHeight - y - 1));
		spot += g_Data.PixelBytes * xmin;

		for (x = xmin; x < xmax_plusone; x++)
		{
			// Extract the r, g, and b values from data

			if ( ! data )
				r = g = b = 0;
			else
				if ( g_Data.Channels == 1 )
					r = g = b = data[0];
				else
					if ( g_Data.Channels >= 3 )
					{
						r = data[g_Data.Channels - 1];
						g = data[g_Data.Channels - 2];
						b = data[g_Data.Channels - 3];
					}


			if ( data )
				data += entrysize;

			// Place the r, g, and b values into our bitmap

			spot[0] = r;
			spot[1] = g;
			spot[2] = b;

			spot   += g_Data.PixelBytes;
		}
	}


	InvalidateRect(g_Data.hWnd, NULL, FALSE);
	UpdateWindow(g_Data.hWnd);
	if ((xmax_plusone - xmin) && (ymax_plusone - ymin) )
	{
		int totalx = g_Data.bmi.bmiHeader.biWidth / (xmax_plusone - xmin);
		int totaly = g_Data.bmi.bmiHeader.biHeight / (ymax_plusone - ymin);
		int nowx = xmin / (xmax_plusone - xmin);
		int nowy = ymin / (ymax_plusone - ymin);
		percent = (float) (nowx + (float)(nowy * totalx)) / (float) (totalx * totaly);
		percent *= 100.0f;
		if ((ymax_plusone == g_Data.bmi.bmiHeader.biHeight)&&
			(xmax_plusone == g_Data.bmi.bmiHeader.biWidth)) 
			percent = 100.0f;
	}

	GetWindowText(g_Data.hWnd, file, 80);
	spot = strstr(file, " ");
	if(spot) *spot = '\0';
	sprintf(msg, "%s [%3.2f%%]", file, percent);
	SetWindowText(g_Data.hWnd, msg);

	return PkDspyErrorNone;
}


//******************************************************************************
// DspyImageClose
//******************************************************************************

extern "C" PtDspyError DspyImageClose(PtDspyImageHandle image)
{
#if SHOW_CALLSTACK
	fprintf(stderr, "sdcWin32_DspyImageClose called.\n");
#endif

	return PkDspyErrorNone;
}







