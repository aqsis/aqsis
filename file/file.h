////---------------------------------------------------------------------
////    Associated header file: FILE.H
////    Class definition file:  FILE.CPP
////
////    Author:					Paul C. Gregory
////    Creation date:			07/12/99
////---------------------------------------------------------------------

//? Is .h included already?
#ifndef IBUFFER_H_INCLUDED
#define IBUFFER_H_INCLUDED 1

#include	"aqsis.h"

#include	"imagebuffer.h"
#include	"tiffio.h"

#define		_qShareName	FILEBUFFER
#include	"share.h"

START_NAMESPACE(Aqsis)

///----------------------------------------------------------------------
/// CqDisplayImageBuffer

class  _qShareC CqFileBuffer : public CqImageBuffer
{
	public:
								CqFileBuffer(const char* strName);
				virtual			~CqFileBuffer();

				virtual	void	SetImage();
				virtual	void	GridRendered();
				virtual	void	SurfaceRendered();
				virtual	void	BucketComplete(TqInt iBucket);
				virtual	void	ImageComplete();

				const	CqString& strName()	{return(m_strName);}

						void	DisplayImage();
						void	SaveBitmap();

	private:
	float*				m_pData;
	unsigned char*		m_pByteData;
	CqString			m_strName;
	TqInt				m_XImageRes;
	TqInt				m_YImageRes;
	TIFF*				m_pOut;
	TqInt				m_iXBucket;
	TqInt				m_SamplesPerPixel;
};

//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif	// !FRAMEBUFFER_H_INCLUDED
