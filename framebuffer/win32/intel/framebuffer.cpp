// framebuffer.cpp : Defines the entry point for the application.
//

#include	"stdafx.h"

#include	<winsock.h>
#include	<process.h>

#include	"aqsis.h"
#include	"displaydriver.h"
#include	"dd.h"
#include	"sstring.h"

#include	"resource.h"

#define MAX_LOADSTRING 100

using namespace Aqsis;

HINSTANCE	hInst;
TCHAR szWindowClass[MAX_LOADSTRING];			// The title bar text

BITMAPINFO			bmInfo;
HPALETTE			hPal;
HBITMAP				hBitmap;
HWND				hWnd;
HDC					hDCMem;
DIBSECTION			DIB;
unsigned char*		pData;
CqString			strName;
TqInt				XImageRes;
TqInt				YImageRes;
TqBool				fOkToDelete;

ATOM				MyRegisterClass(HINSTANCE hInstance);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
void DisplayImage();
void MessageLoop(void* p);

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	LoadString(hInst, IDS_FRAMEBUFFER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInst);

	hWnd=CreateWindow(szWindowClass, "Framebuffer", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInst, NULL);
	if(hWnd)
	{
		HDC	hDC=GetDC(hWnd);
		hDCMem=::CreateCompatibleDC(hDC);
		ReleaseDC(hWnd,hDC);
	}


	DDInitialise(NULL,-1);
	// Start a thread to manage the display driver messages.
	_beginthread(MessageLoop,0,NULL);

	MSG msg;
	while( GetMessage( &msg, NULL, 0, 0 ) )
	{
		TranslateMessage( &msg);
		DispatchMessage( &msg );
	}

	::DeleteObject(hPal);
	::DeleteDC(hDCMem);
	::DeleteObject(hBitmap);

	return 0;
}

void MessageLoop(void* p)
{
	DDProcessMessages();	
	_endthread();
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
/*					if(pImage)
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
*/
				case IDM_EXIT:
					break;
				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			DisplayImage();
			EndPaint(hWnd, &ps);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}


void DisplayImage()
{
	BITMAPINFOHEADER &bmInfoHdr = DIB.dsBmih;
	HGDIOBJ hBmpOld=::SelectObject(hDCMem, hBitmap);

	HDC		hDC=GetDC(hWnd);

	if(::GetDeviceCaps(hDC, RASTERCAPS) & RC_PALETTE )
	{
		HPALETTE hPalOld=::SelectPalette(hDC,hPal,FALSE);
		::RealizePalette(hDC);
		SetDIBitsToDevice(hDC,0,0,bmInfoHdr.biWidth, bmInfoHdr.biHeight,0,0,0,bmInfoHdr.biHeight,pData,&bmInfo,DIB_RGB_COLORS);
		::SelectPalette(hDC,hPalOld,FALSE);
		// delete GDI objects
	}
	else
//		BitBlt(hDC,0,0,bmInfo.biWidth,bmInfo.biHeight,m_hDCMem,0,0,SRCCOPY);
		SetDIBitsToDevice(hDC,0,0,bmInfoHdr.biWidth, bmInfoHdr.biHeight,0,0,0,bmInfoHdr.biHeight,pData,&bmInfo,DIB_RGB_COLORS);

	::SelectObject(hDCMem, hBmpOld);
	ReleaseDC(hWnd, hDC);
}


//----------------------------------------------------------------------------
// Functions required by libdd.

TqInt	XRes,YRes;
TqInt	SamplesPerElement;
TqInt	CWXMin,CWYMin;
SqDDMessageFormatResponse frmt(2);
std::string	strFilename("output.tif");

TqInt Query(SOCKET s,SqDDMessageBase* pMsgB)
{
	switch(pMsgB->m_MessageID)
	{
		case MessageID_FormatQuery:
		{
			if(DDSendMsg(s,&frmt)<=0)
				return(-1);
		}
		break;
	}
	return(0);
}


TqInt Open(SOCKET s, SqDDMessageBase* pMsgB)
{
	SqDDMessageOpen* pMsg=static_cast<SqDDMessageOpen*>(pMsgB);

	SetWindowText(hWnd, strFilename.c_str());

	XRes=(pMsg->m_CropWindowXMax-pMsg->m_CropWindowXMin);
	YRes=(pMsg->m_CropWindowYMax-pMsg->m_CropWindowYMin);
	CWXMin=pMsg->m_CropWindowXMin;
	CWYMin=pMsg->m_CropWindowYMin;
	SamplesPerElement=pMsg->m_SamplesPerElement;

	// Create a DIBSection to draw into.
	bmInfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	bmInfo.bmiHeader.biWidth=XRes;
	bmInfo.bmiHeader.biHeight=YRes;
	bmInfo.bmiHeader.biPlanes=1;
	bmInfo.bmiHeader.biBitCount=24;
	bmInfo.bmiHeader.biCompression=BI_RGB;
	bmInfo.bmiHeader.biSizeImage=0;

	HDC		hDC=GetDC(hWnd);

	hBitmap=CreateDIBSection(hDC, &bmInfo, DIB_RGB_COLORS, reinterpret_cast<void**>(&pData), NULL,0);
	::GetObject(hBitmap, sizeof(DIB), &DIB );

	int nColors=bmInfo.bmiHeader.biClrUsed ? bmInfo.bmiHeader.biClrUsed : 1 << DIB.dsBm.bmBitsPixel;	
	
	if(::GetDeviceCaps(hDC, RASTERCAPS) & RC_PALETTE )
	{
		// Create a halftone palette if colors > 256.
		if( nColors > 256 )	hPal=::CreateHalftonePalette(hDC);
		else		
		{			// Create the palette
			RGBQUAD *pRGB = new RGBQUAD[nColors];			
			::GetDIBColorTable(hDCMem, 0, nColors, pRGB);			
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
			hPal=::CreatePalette( pLP );			
			delete[] pLP;			
			delete[] pRGB;
		}
	}

	// Now resize the display window to the image size.
	RECT rect,rect1;

	GetClientRect(hWnd, &rect);    // starting rect comes from client rect
	GetWindowRect(hWnd, &rect1);
	rect1.right-=rect1.left;
	rect1.bottom-=rect1.top;
	rect1.left=rect1.top=0;
	rect1.right-=rect.right;
	rect1.left-=rect.left;
	rect1.bottom-=rect.bottom;
	rect1.top-=rect.top;

	SetWindowPos(hWnd, 0,0, 0, XRes+rect1.right+2,YRes+rect1.bottom+2, SWP_NOMOVE|SWP_NOZORDER);
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	ReleaseDC(hWnd,hDC);

	return(0);
}


TqInt Data(SOCKET s, SqDDMessageBase* pMsgB)
{
	SqDDMessageData* pMsg=static_cast<SqDDMessageData*>(pMsgB);

	TqInt	linelen=XRes*SamplesPerElement;
	char* pBucket=reinterpret_cast<char*>(&pMsg->m_Data);
	linelen=(linelen+3)&0xfffffffc;

	TqInt y;
	for(y=pMsg->m_YMin; y<pMsg->m_YMaxPlus1; y++)
	{
		TqInt x;
		for(x=pMsg->m_XMin; x<pMsg->m_XMaxPlus1; x++)
		{
			if(x>=0 && y>=0 && x<XRes && y<YRes)
			{
				TqInt so=((YRes-y-1)*linelen)+(x*SamplesPerElement);
				
				if(SamplesPerElement>=3)
				{
					pData[so+0]=static_cast<char>(reinterpret_cast<TqFloat*>(pBucket)[2]);
					pData[so+1]=static_cast<char>(reinterpret_cast<TqFloat*>(pBucket)[1]);
					pData[so+2]=static_cast<char>(reinterpret_cast<TqFloat*>(pBucket)[0]);
				}
				else
				{
					pData[so+0]=static_cast<char>(reinterpret_cast<TqFloat*>(pBucket)[0]);
					pData[so+1]=static_cast<char>(reinterpret_cast<TqFloat*>(pBucket)[0]);
					pData[so+2]=static_cast<char>(reinterpret_cast<TqFloat*>(pBucket)[0]);
				}
			}
			pBucket+=pMsg->m_ElementSize;
		}
	}
	DisplayImage();
	return(0);
}


TqInt Close(SOCKET s, SqDDMessageBase* pMsgB)
{
	return(1);	
}


TqInt HandleMessage(SOCKET s,SqDDMessageBase* pMsgB)
{
	switch(pMsgB->m_MessageID)
	{
		case MessageID_Filename:
		{
			SqDDMessageFilename* pMsg=static_cast<SqDDMessageFilename*>(pMsgB);
			strFilename=pMsg->m_String;
		}
		break;
	}
	return(0);
}
