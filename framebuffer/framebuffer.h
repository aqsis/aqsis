////---------------------------------------------------------------------
////    Associated header file: FRAMEBUFFER.H
////    Class definition file:  FRAMEBUFFER.CPP
////
////    Author:					Paul C. Gregory
////    Creation date:			29/03/99
////---------------------------------------------------------------------

//? Is .h included already?
#ifndef IBUFFER_H_INCLUDED
#define IBUFFER_H_INCLUDED 1

#include	"aqsis.h"

#include	"imageBuffer.h"

#define		_qShareName	FRAMEBUFFER
#include	"share.h"

START_NAMESPACE(Aqsis)

///----------------------------------------------------------------------
/// CqDisplayImageBuffer

class  _qShareC CqFrameBuffer : public CqImageBuffer
{
	public:
	_qShareM					CqFrameBuffer(const char* strName);
	_qShareM	virtual			~CqFrameBuffer();

	_qShareM	virtual	void	SetImage();
	_qShareM	virtual	void	GridRendered();
	_qShareM	virtual	void	SurfaceRendered();
	_qShareM	virtual	void	BucketComplete(TqInt iBucket);
	_qShareM	virtual	void	ImageComplete();
	_qShareM	virtual	void	Release();

				const CqString& strName()	{return(m_strName);}

						void	DisplayImage();
						void	SaveBitmap();
						TqBool	fOkToDelete() const	{return(m_fOkToDelete);}

	private:
	BITMAPINFO			m_bmInfo;
	HPALETTE			m_hPal;
	HBITMAP				m_hBitmap;
	HWND				m_hWnd;
	HDC					m_hDCMem;
	DIBSECTION			m_DIB;
	unsigned char*		m_pData;
	CqString			m_strName;
	TqInt				m_XImageRes;
	TqInt				m_YImageRes;
	TqBool			m_fOkToDelete;
};

//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif	// !FRAMEBUFFER_H_INCLUDED
