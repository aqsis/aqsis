////---------------------------------------------------------------------
////    Class definition file:  FILE.CPP
////    Associated header file: FILE.H
////
////    Author:					Paul C. Gregory
////    Creation date:			07/12/99
////---------------------------------------------------------------------

#include	"aqsis.h"
#include	"file.h"
#include	"irenderer.h"
#include	"sstring.h"

HINSTANCE	hInst;

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
    
	return TRUE;
}


extern "C"
_qShareM Aqsis::CqImageBuffer* CreateImage(const char* strName)
{
	return(new Aqsis::CqFile(strName));
}

START_NAMESPACE(Aqsis)

///---------------------------------------------------------------------
/// CqFile::CqFile
///
/// Constructor

CqFile::CqFile(const char* strName) :
										CqImageBuffer(),
										m_strName(strName),
										m_pData(0),
										m_XImageRes(0),
										m_YImageRes(0)
{
}


///---------------------------------------------------------------------
/// CqFile::~CqFile
///
/// Destructor

CqFile::~CqFile()
{
	// Delate the image buffer
	delete[](m_pData);
}


///---------------------------------------------------------------------
/// CqFile::SetImage
///
/// Set the reolution of the image buffer.

void CqFile::SetImage()
{
	// Call through to the standard image buffer function.
	CqImageBuffer::SetImage();

	m_XImageRes=(CropWindowXMax()-CropWindowXMin());
	m_YImageRes=(CropWindowYMax()-CropWindowYMin());
	m_iXBucket=0;
	m_SamplesPerPixel=DisplayMode()&ModeRGB?3:0;
	m_SamplesPerPixel+=DisplayMode()&ModeA?1:0;

	// Create a buffer big enough to hold a row of buckets.
	m_pData=new float[m_XImageRes*YBucketSize()*m_SamplesPerPixel];
	m_pByteData=new unsigned char[m_XImageRes*m_SamplesPerPixel];

	uint16 photometric = PHOTOMETRIC_RGB;
	uint16 config = PLANARCONFIG_CONTIG;
	uint16 compression = COMPRESSION_NONE;

	m_pOut = TIFFOpen(strName().c_str(), "w");

	if(m_pOut)
	{
		// Write the image to a tiff file.
		
		int ExtraSamplesTypes[1]={EXTRASAMPLE_ASSOCALPHA};
		
		TIFFSetField(m_pOut, TIFFTAG_IMAGEWIDTH, (uint32)m_XImageRes);
		TIFFSetField(m_pOut, TIFFTAG_IMAGELENGTH, (uint32)m_YImageRes);
		TIFFSetField(m_pOut, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
		TIFFSetField(m_pOut, TIFFTAG_SAMPLESPERPIXEL, m_SamplesPerPixel);
		TIFFSetField(m_pOut, TIFFTAG_BITSPERSAMPLE, 8);
		TIFFSetField(m_pOut, TIFFTAG_PLANARCONFIG, config);			
		TIFFSetField(m_pOut, TIFFTAG_COMPRESSION, compression);
		TIFFSetField(m_pOut, TIFFTAG_PHOTOMETRIC, photometric);
		TIFFSetField(m_pOut, TIFFTAG_ROWSPERSTRIP,	TIFFDefaultStripSize(m_pOut, -1));
		
		if(m_SamplesPerPixel==4)
			TIFFSetField(m_pOut, TIFFTAG_EXTRASAMPLES,	1, ExtraSamplesTypes);

		// Set the position tages in case we aer dealing with a cropped image.
		TIFFSetField(m_pOut, TIFFTAG_XPOSITION, (float)CropWindowXMin());
		TIFFSetField(m_pOut, TIFFTAG_YPOSITION, (float)CropWindowYMin());
	}
	else
		CqBasicError(1,Severity_Fatal,"Cannot create TIFF file");
}


///---------------------------------------------------------------------
/// CqFile::GridRendered
///
/// Grid rendered callback

void CqFile::GridRendered()
{
}


///---------------------------------------------------------------------
/// CqFile::SurfaceRendered
///
/// Surface rendered callback

void CqFile::SurfaceRendered()
{
}


///---------------------------------------------------------------------
/// CqFile::BucketComplete
///
/// Bucket is complete so display the image.

void CqFile::BucketComplete(TqInt iBucket)
{
	if(m_pOut == NULL)
		return;

	// Copy the bucket to the display buffer.
	CqVector2D	vecA=Position(iBucket);
	CqVector2D	vecB=Size(iBucket);
	TqInt linelen=m_XImageRes*m_SamplesPerPixel;

	// Check if this bucket is outside the crop window.
	if((vecA.x()+vecB.x())<CropWindowXMin() ||
	   (vecA.y()+vecB.y())<CropWindowYMin() ||
	   (vecA.x())>CropWindowXMax() ||
	   (vecA.y())>CropWindowYMax())
		return;

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
			if(isx<0 || isy<0 || isx>=m_XImageRes || isy>=m_YImageRes)
				continue;

			TqInt so=((isy-vecA.y())*linelen)+(isx*m_SamplesPerPixel);
			
			FilterPixel(sx,sy,iBucket,val);
			ExposePixel(val);
			QuantizePixel(val);
	
			if(m_SamplesPerPixel>=3)
			{
				m_pData[so+0]=val.m_colColor.fRed();
				m_pData[so+1]=val.m_colColor.fGreen();
				m_pData[so+2]=val.m_colColor.fBlue();
				if(m_SamplesPerPixel==4)
					m_pData[so+3]=val.m_Coverage;
			}
			if(m_SamplesPerPixel==1)
					m_pData[so+0]=val.m_Coverage;
		}
	}

	m_iXBucket++;
	// If this is the last bucket in this row, write the row to the image.
	if(m_iXBucket==cXBuckets())
	{
		TqInt row;
		for(row=0; row<YBucketSize(); row++)
		{
			if((row+vecA.y())<m_YImageRes)
			{
				// Convert to bytes per channel.
				TqInt i;
				for(i=0; i<m_XImageRes*m_SamplesPerPixel; i++)
					m_pByteData[i]=static_cast<unsigned char>(m_pData[i+(row*m_XImageRes*m_SamplesPerPixel)]);

				if(TIFFWriteScanline(m_pOut, m_pByteData, row+vecA.y(), 0) < 0)
					break;
			}
		}
		m_iXBucket=0;
	}
}


///---------------------------------------------------------------------
/// CqFile::ImageComplete
///
/// Rendering is finished so display the image.

void CqFile::ImageComplete()
{
	if(m_pOut)
		(void) TIFFClose(m_pOut);
}

//---------------------------------------------------------------------

END_NAMESPACE(Aqsis)
