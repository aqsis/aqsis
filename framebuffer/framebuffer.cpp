////---------------------------------------------------------------------
////    Class definition file:  IBUFFER.CPP
////    Associated header file: IBUFFER.H
////
////    Author:					Paul C. Gregory
////    Creation date:			29/03/99
////---------------------------------------------------------------------

#include	<stdio.h>
#include	<windows.h>
#include	<COMMDLG.H>

#include	"aqsis.h"
#include	"resource.h"
#include	"framebuffer.h"
#include	"renderer.h"
#include	"sstring.h"

#define MAX_LOADSTRING 100

using namespace Aqsis;

HINSTANCE	hInst;
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// The title bar text
Aqsis::CqFrameBuffer* pImage;

ATOM				MyRegisterClass(HINSTANCE hInstance);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

BOOL APIENTRY DllMain( HINSTANCE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    hInst=hModule;
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    
	LoadString(hInst, IDS_WINDOW_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInst, IDS_FRAMEBUFFER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInst);

	return TRUE;
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_FRAMEBUFFER);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDR_FRAMEBUFFER;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}


//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) 
	{
		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			// Parse the menu selections:
			switch (wmId)
			{
				case ID_FILE_SAVEASBMP:
					// TODO: Should check if its finished rendering yet.
					// actually should really disable this menu item until finished.
					if(pImage)
					{
						OPENFILENAME	ofn;
						char FileName[100];
						strcpy(FileName, pImage->strName().c_str());
						ofn.lStructSize=sizeof(OPENFILENAME);
						ofn.hwndOwner=hWnd;
						ofn.hInstance=hInst;
						ofn.lpstrFilter="Bitmap Files\x0*.bmp";
						ofn.lpstrCustomFilter=0;
						ofn.nMaxCustFilter=0;
						ofn.nFilterIndex=0;
						ofn.lpstrFile=FileName;
						ofn.nMaxFile=100;
						ofn.lpstrFileTitle=0;
						ofn.nMaxFileTitle=0;
						ofn.lpstrInitialDir=0;
						ofn.lpstrTitle=0;
						ofn.Flags=0;
						ofn.nFileOffset=0;
						ofn.nFileExtension=0;
						ofn.lpstrDefExt="bmp";
						ofn.lCustData=0;
						ofn.lpfnHook=0;
						ofn.lpTemplateName=0;
						GetSaveFileName(&ofn);

						//pImage->strName()=FileName;
						pImage->SaveBitmap();
					}
					break;

				case IDM_EXIT:
					break;
				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		case WM_PAINT:
			OutputDebugString("WM_PAINT\n");
			hdc = BeginPaint(hWnd, &ps);
			if(pImage)
				pImage->DisplayImage();
			EndPaint(hWnd, &ps);
			break;
		case WM_DESTROY:
			if(pImage && pImage->fOkToDelete())
				delete(pImage);
			PostQuitMessage(0);
			OutputDebugString("WM_DESTROY\n");
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

extern "C"
_qShareM Aqsis::CqImageBuffer* CreateImage(const char* strName)
{
	pImage=new Aqsis::CqFrameBuffer(strName);
	return(pImage);
}

extern "C"
_qShareM void MessageLoop(void)
{
	MSG msg;
	while( GetMessage( &msg, NULL, 0, 0 ) )
	{
		TranslateMessage( &msg);
		DispatchMessage( &msg );
	}
	OutputDebugString("Message Loop Exiting\n");
}


START_NAMESPACE(Aqsis)

///---------------------------------------------------------------------
/// CqFrameBuffer::CqFrameBuffer
///
/// Constructor

CqFrameBuffer::CqFrameBuffer(const char* strName) :
										CqImageBuffer(),
										m_strName(strName),
										m_hPal(0),
										m_hDCMem(0),
										m_hBitmap(0),
										m_XImageRes(0),
										m_YImageRes(0),
										m_fOkToDelete(TqFalse)
{
	m_hWnd=CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
							CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInst, NULL);

	if (m_hWnd)
	{
		HDC	hDC=GetDC(m_hWnd);
		m_hDCMem=::CreateCompatibleDC(hDC);
		ReleaseDC(m_hWnd,hDC);
	}
}


///---------------------------------------------------------------------
/// CqFrameBuffer::~CqFrameBuffer
///
/// Destructor

CqFrameBuffer::~CqFrameBuffer()
{
	::DeleteObject(m_hPal);
	::DeleteDC(m_hDCMem);
	::DeleteObject(m_hBitmap);
}


///---------------------------------------------------------------------
/// CqFrameBuffer::SetImage
///
/// Set the reolution of the image buffer.

void CqFrameBuffer::SetImage()
{
	// Call through to the standard image buffer function.
	CqImageBuffer::SetImage();

	m_XImageRes=(CropWindowXMax()-CropWindowXMin());
	m_YImageRes=(CropWindowYMax()-CropWindowYMin());

	// Create a DIBSection to draw into.
	m_bmInfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	m_bmInfo.bmiHeader.biWidth=m_XImageRes;
	m_bmInfo.bmiHeader.biHeight=m_YImageRes;
	m_bmInfo.bmiHeader.biPlanes=1;
	m_bmInfo.bmiHeader.biBitCount=24;
	m_bmInfo.bmiHeader.biCompression=BI_RGB;
	m_bmInfo.bmiHeader.biSizeImage=0;

	HDC		hDC=GetDC(m_hWnd);

	m_hBitmap=CreateDIBSection(hDC, &m_bmInfo, DIB_RGB_COLORS, reinterpret_cast<void**>(&m_pData), NULL,0);
	::GetObject(m_hBitmap, sizeof(m_DIB), &m_DIB );

	int nColors=m_bmInfo.bmiHeader.biClrUsed ? m_bmInfo.bmiHeader.biClrUsed : 1 << m_DIB.dsBm.bmBitsPixel;	
	
	if(::GetDeviceCaps(hDC, RASTERCAPS) & RC_PALETTE )
	{
		// Create a halftone palette if colors > 256.
		if( nColors > 256 )	m_hPal=::CreateHalftonePalette(hDC);
		else		
		{			// Create the palette
			RGBQUAD *pRGB = new RGBQUAD[nColors];			
			::GetDIBColorTable(m_hDCMem, 0, nColors, pRGB);			
			UINT nSize = sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * nColors);
			LOGPALETTE *pLP = reinterpret_cast<LOGPALETTE *>(new BYTE[nSize]);			
			pLP->palVersion = 0x300;
			pLP->palNumEntries = nColors;			
			int i;
			for(i=0; i < nColors; i++)
			{
				pLP->palPalEntry[i].peRed = pRGB[i].rgbRed;
				pLP->palPalEntry[i].peGreen = pRGB[i].rgbGreen;
				pLP->palPalEntry[i].peBlue = pRGB[i].rgbBlue;
				pLP->palPalEntry[i].peFlags = 0;			
			}
			m_hPal=::CreatePalette( pLP );			
			delete[] pLP;			
			delete[] pRGB;
		}
	}

	// Now resize the display window to the image size.
	RECT rect,rect1;

	GetClientRect(m_hWnd, &rect);    // starting rect comes from client rect
	GetWindowRect(m_hWnd, &rect1);
	rect1.right-=rect1.left;
	rect1.bottom-=rect1.top;
	rect1.left=rect1.top=0;
	rect1.right-=rect.right;
	rect1.left-=rect.left;
	rect1.bottom-=rect.bottom;
	rect1.top-=rect.top;

	SetWindowPos(m_hWnd, 0,0, 0, m_XImageRes+rect1.right+2,m_YImageRes+rect1.bottom+2, SWP_NOMOVE|SWP_NOZORDER);
	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);

	ReleaseDC(m_hWnd,hDC);
}


///---------------------------------------------------------------------
/// CqFrameBuffer::GridRendered
///
/// Grid rendered callback

void CqFrameBuffer::GridRendered()
{
//	DisplayImage();
}


///---------------------------------------------------------------------
/// CqFrameBuffer::SurfaceRendered
///
/// Surface rendered callback

void CqFrameBuffer::SurfaceRendered()
{
//	DisplayImage();
}


///---------------------------------------------------------------------
/// CqFrameBuffer::BucketComplete
///
/// Bucket is complete so display the image.

void CqFrameBuffer::BucketComplete(TqInt iBucket)
{
	// Copy the bucket to the display buffer.
	CqVector2D	vecA=Position(iBucket);
	CqVector2D	vecB=Size(iBucket);
	TqInt linelen=m_XImageRes*3;
	linelen=(linelen+3)&0xfffffffc;

	// Check if this bucket is outside the crop window.
	if((vecA.x()+vecB.x())<=CropWindowXMin() ||
	   (vecA.y()+vecB.y())<=CropWindowYMin() ||
	   (vecA.x())>CropWindowXMax() ||
	   (vecA.y())>CropWindowYMax())
		return;

	// Check if the bucket spans the cropwindow.
	if((vecA.x())<CropWindowXMin())	
	{
		vecB.x(vecB.x()-(CropWindowXMin()-vecA.x()));
		vecA.x(CropWindowXMin());
	}
	if((vecA.y())<CropWindowYMin())	
	{
		vecB.y(vecB.y()-(CropWindowYMin()-vecA.y()));
		vecA.y(CropWindowYMin());
	}
	if((vecA.x()+vecB.x())>=CropWindowXMax())	vecB.x(vecB.x()-(vecA.x()+vecB.x()-CropWindowXMax()));
	if((vecA.y()+vecB.y())>=CropWindowYMax())	vecB.y(vecB.y()-(vecA.y()+vecB.y()-CropWindowYMax()));

	SqImageValue val;
	TqInt y;
	for(y=0; y<vecB.y(); y++)
	{
		TqInt sy=y+static_cast<TqInt>(vecA.y());
		TqInt x;
		for(x=0; x<vecB.x(); x++)
		{
			TqInt sx=x+static_cast<TqInt>(vecA.x());
			TqInt isx=sx-CropWindowXMin();
			TqInt isy=sy-CropWindowYMin();
			//if(isx<0 || isy<0 || isx>m_XImageRes || isy>m_YImageRes)
			//	continue;

			TqInt so=((m_YImageRes-isy-1)*linelen)+(isx*3);
			
			FilterPixel(sx,sy,iBucket,val);
			ExposePixel(val);
			QuantizePixel(val);
	
			m_pData[so+0]=static_cast<unsigned char>(val.m_colColor.fBlue());
			m_pData[so+1]=static_cast<unsigned char>(val.m_colColor.fGreen());
			m_pData[so+2]=static_cast<unsigned char>(val.m_colColor.fRed());
		}
	}
	DisplayImage();
}


///---------------------------------------------------------------------
/// CqFrameBuffer::ImageComplete
///
/// Rendering is finished so display the image.

void CqFrameBuffer::ImageComplete()
{
	OutputDebugString("Image Complete\n");
}


///---------------------------------------------------------------------
/// CqFrameBuffer::DisplayImage
///
/// Display the image in its current state to the window.

void CqFrameBuffer::DisplayImage()
{
	BITMAPINFOHEADER &bmInfo = m_DIB.dsBmih;
	HGDIOBJ hBmpOld=::SelectObject(m_hDCMem, m_hBitmap);

	HDC		hDC=GetDC(m_hWnd);

	if(::GetDeviceCaps(hDC, RASTERCAPS) & RC_PALETTE )
	{
		HPALETTE hPalOld=::SelectPalette(hDC,m_hPal,FALSE);
		::RealizePalette(hDC);
//		BitBlt(hDC,0,0,bmInfo.biWidth,bmInfo.biHeight,m_hDCMem,0,0,SRCCOPY);
		SetDIBitsToDevice(hDC,0,0,bmInfo.biWidth, bmInfo.biHeight,0,0,0,bmInfo.biHeight,m_pData,&m_bmInfo,DIB_RGB_COLORS);
		::SelectPalette(hDC,hPalOld,FALSE);
		// delete GDI objects
	}
	else
//		BitBlt(hDC,0,0,bmInfo.biWidth,bmInfo.biHeight,m_hDCMem,0,0,SRCCOPY);
		SetDIBitsToDevice(hDC,0,0,bmInfo.biWidth, bmInfo.biHeight,0,0,0,bmInfo.biHeight,m_pData,&m_bmInfo,DIB_RGB_COLORS);

	::SelectObject(m_hDCMem, hBmpOld);
	ReleaseDC(m_hWnd, hDC);
}



///---------------------------------------------------------------------
/// CqFrameBuffer::SaveBitmap
///
/// Save the image as a windows bitmap.

#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B') 
#define WIDTHBYTES(i)		((i+31)/32*4) 
void CqFrameBuffer::SaveBitmap() 
{ 
	BITMAPFILEHEADER    bmfHdr;     // Header for Bitmap file 
	HANDLE              fh;         // file handle for opened file 
	DWORD               dwWritten; 

	fh=CreateFile(m_strName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 
				  FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL); 

	if(fh==INVALID_HANDLE_VALUE) 
		return; 

	// Fill in the fields of the file header 
	// Fill in file type (first 2 bytes must be "BM" for a bitmap) 

	bmfHdr.bfType = DIB_HEADER_MARKER;  // "BM" 

	m_bmInfo.bmiHeader.biSizeImage=WIDTHBYTES((m_bmInfo.bmiHeader.biWidth)*((DWORD)m_bmInfo.bmiHeader.biBitCount)) * 
								m_bmInfo.bmiHeader.biHeight; 

	// Calculate the file size by adding the DIB size to sizeof(BITMAPFILEHEADER) 
              
	bmfHdr.bfSize=m_bmInfo.bmiHeader.biSizeImage+sizeof(BITMAPFILEHEADER); 
	bmfHdr.bfReserved1=0; 
	bmfHdr.bfReserved2=0; 

	// Now, calculate the offset the actual bitmap bits will be in 
	// the file -- It's the Bitmap file header plus the DIB header, 
	// plus the size of the color table. 
 
	bmfHdr.bfOffBits=(DWORD)sizeof(BITMAPFILEHEADER)+m_bmInfo.bmiHeader.biSize;

	// Write the file header 
	WriteFile(fh,(LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL); 

	// Write the DIB header and the bits
	WriteFile(fh, (LPSTR)&(m_bmInfo.bmiHeader), sizeof(BITMAPINFOHEADER), &dwWritten, NULL); 
	WriteFile(fh, (LPSTR)m_pData, m_bmInfo.bmiHeader.biSizeImage, &dwWritten, NULL); 

	CloseHandle(fh); 
} 


///---------------------------------------------------------------------
/// CqFrameBuffer::Release
///
/// The renderer has finished with us.

void	CqFrameBuffer::Release()
{
	m_fOkToDelete=TqTrue;
	if(pImage && !IsWindow(m_hWnd))	delete(pImage);
} 

//---------------------------------------------------------------------

END_NAMESPACE(Aqsis)
