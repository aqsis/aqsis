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
		\brief Implements the CqImageBuffer class responsible for rendering the primitives and storing the results.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<strstream>

#include	<math.h>

#include	"aqsis.h"
#include	"stats.h"
#include	"options.h"
#include	"renderer.h"
#include	"surface.h"
#include	"shadervm.h"
#include	"micropolygon.h"
#include	"imagebuffer.h"

#include	<map>

START_NAMESPACE(Aqsis)


//----------------------------------------------------------------------
/** Constructor
 */

CqImageElement::CqImageElement() : 
				m_XSamples(0),
				m_YSamples(0),
//				m_aValues(0), 
//				m_avecSamples(0),
				m_colColor(0,0,0)
{
}


//----------------------------------------------------------------------
/** Destructor
 */

CqImageElement::~CqImageElement()
{
}


//----------------------------------------------------------------------
/** Copy constructor
 */

CqImageElement::CqImageElement(const CqImageElement& ieFrom) : m_aValues(0), m_avecSamples(0)
{
	*this=ieFrom;
}


//----------------------------------------------------------------------
/** Allocate the subpixel samples array.
 * \param XSamples Integer samples count in X.
 * \param YSamples Integer samples count in Y.
 */

void CqImageElement::AllocateSamples(TqInt XSamples, TqInt YSamples)
{
	m_XSamples=XSamples;
	m_YSamples=YSamples;

	if(XSamples>0 && YSamples>0)
	{
		m_aValues.resize(m_XSamples*m_YSamples);
		m_avecSamples.resize(m_XSamples*m_YSamples);
		m_aTimes.resize(m_XSamples*m_YSamples);
	}
}


//----------------------------------------------------------------------
/** Fill in the sample array usig the multijitter function from GG IV.
 * \param vecPixel Cq2DVector pixel coordinate of this image element, used to make sure sample points are absolute, not relative.
 * \param fJitter Flag indicating whether to apply jittering to the sample points or not.
 */

void CqImageElement::InitialiseSamples(CqVector2D& vecPixel, TqBool fJitter)
{
	TqFloat subcell_width=1.0f/(m_XSamples*m_YSamples);
	CqRandom random;
	TqInt m=m_XSamples;
	TqInt n=m_YSamples;

	if(!fJitter)
	{
		// Initialise the samples to the centre points.
		TqFloat XInc=(1.0f/m_XSamples)/2.0f;
		TqFloat YInc=(1.0f/m_YSamples)/2.0f;
		TqInt y;
		for(y=0; y<m_YSamples; y++)
		{
			TqFloat YSam=YInc+(YInc*y);
			TqInt x;
			for(x=0; x<m_XSamples; x++)
				m_avecSamples[(y*m_XSamples)+x]=CqVector2D(XInc+(XInc*x),YSam)+vecPixel;
		}
	}
	else
	{
		// Initialize points to the "canonical" multi-jittered pattern.
		TqInt i,j;
		for (i = 0; i < n; i++) 
		{
			for (j = 0; j < m; j++) 
			{
				m_avecSamples[i*m + j].x(j*n*subcell_width + i*subcell_width + random.RandomFloat(subcell_width));
				m_avecSamples[i*m + j].y(i*m*subcell_width + j*subcell_width + random.RandomFloat(subcell_width));
			}
		}

		// Shuffle y coordinates within each row of cells.
		for (i = 0; i < n; i++) 
		{
			for (j = 0; j < m; j++) 
			{
				double t;
				int k;

				k = random.RandomInt(n - 1 - i) + i;
				t = m_avecSamples[i*m + j].y();
				m_avecSamples[i*m + j].y(m_avecSamples[i*m + k].y());
				m_avecSamples[i*m + k].y(t);
			}
		}

		// Shuffle x coordinates within each column of cells.
		for (i = 0; i < m; i++) 
		{
			for (j = 0; j < n; j++) 
			{
				double t;
				int k;

				k = random.RandomInt(n - 1 - j) + j;
				t = m_avecSamples[j*m + i].x();
				m_avecSamples[j*m + i].x(m_avecSamples[k*m + i].x());
				m_avecSamples[k*m + i].x(t);
			}
		}

		// finally add in the pixel offset
		for (i = 0; i < n; i++) 
		{
			for (j = 0; j < m; j++) 
				m_avecSamples[i*m + j]+=vecPixel;
		}
	}

	// Fill in the sample times for motion blur.
	TqFloat time=0;
	TqFloat dtime=1.0f/((m_XSamples*m_YSamples));
	TqInt i;
	for(i=0; i<m_XSamples*m_YSamples; i++)
	{
		m_aTimes[i]=time+random.RandomFloat(dtime);
		time+=dtime;
	}
}


//----------------------------------------------------------------------
/** Clear the relevant data from the image element preparing it for the next usage.
 */

void CqImageElement::Clear()
{
	TqInt i;
	for(i=(m_XSamples*m_YSamples)-1; i>=0; i--)	
		m_aValues[i].resize(0);
}


//----------------------------------------------------------------------
/** Get the color at the specified sample point by blending the colors that appear at that point.
 */

void CqImageElement::Combine()
{
	static CqColor	opaque(1,1,1);
	static CqColor	colWhite(1,1,1);
	CqColor col_total(0,0,0);
	TqFloat	depth_total=0;
	TqFloat cov=0;

	TqInt n;
	for(n=0; n<m_YSamples; n++)
	{
		TqInt m;
		for(m=0; m<m_XSamples; m++)
		{
			std::vector<SqImageValue>& vals=m_aValues[n*m_XSamples+m];
			CqColor	col(0,0,0);

			if(vals.size()>0)
			{
				TqInt i=vals.size();
				// Now mix the colors using the transparency as a filter.
				while(i-->0)
					col=(vals[i].m_colColor)+(col*(colWhite-vals[i].m_colOpacity));
				col.Clamp();
				vals[0].m_colColor=col;
				col_total+=col;
				depth_total+=vals[i].m_Depth;
				cov++;
			}
		}
	}
	// Store the totals in the image element ready for filtering.
	TqFloat smul=1.0f/(m_XSamples*m_YSamples);
	col_total*=smul;
	m_colColor=col_total;
	depth_total*=smul;
	m_Depth=depth_total;
	cov*=smul;
	m_Coverage=cov;
}


//----------------------------------------------------------------------
/** Static data on CqBucket
 */ 

TqInt	CqBucket::m_XSize;
TqInt	CqBucket::m_YSize;
TqInt	CqBucket::m_XFWidth;
TqInt	CqBucket::m_YFWidth;
TqInt	CqBucket::m_XOrigin;
TqInt	CqBucket::m_YOrigin;
std::vector<CqImageElement>	CqBucket::m_aieImage;
std::vector<TqFloat> CqBucket::m_aFilterValues;

//----------------------------------------------------------------------
/** Get a reference to pixel data.
 * \param iXPos Integer pixel coordinate.
 * \param iYPos Integer pixel coordinate.
 * \param iBucket Integer bucket index.
 * \param pie Pointer to CqImageElement to fill in.
 * \return Boolean indicating success, could fail if the specified pixel is not within the specified bucket.
 */

TqBool CqBucket::ImageElement(TqInt iXPos, TqInt iYPos, CqImageElement*& pie)
{
	iXPos-=m_XOrigin;
	iYPos-=m_YOrigin;

	int fxo2=static_cast<int>(m_XFWidth*0.5f);
	int fyo2=static_cast<int>(m_YFWidth*0.5f);
	
	// Check within renderable range
	if(iXPos>=-fxo2 && iXPos<=m_XSize+fxo2 &&
	   iYPos>=-fyo2 && iYPos<=m_YSize+fyo2)
	{
		TqInt i=((iYPos+fyo2)*(m_XSize+m_XFWidth))+(iXPos+fxo2);
		pie=&m_aieImage[i];
		return(TqTrue);
	}
	else
		return(TqFalse);
}


//----------------------------------------------------------------------
/** Clear the image data storage area.
 */

void CqBucket::Clear()
{
	// Call the clear function on each element in the bucket.
	for(std::vector<CqImageElement>::iterator iElement=m_aieImage.begin(); iElement!=m_aieImage.end(); iElement++)
		iElement->Clear();
}


//----------------------------------------------------------------------
/** Initialise the static image storage area.
 */

void CqBucket::InitialiseBucket(TqInt xorigin, TqInt yorigin, TqInt xsize, TqInt ysize, TqInt xfwidth, TqInt yfwidth, TqInt xsamples, TqInt ysamples, TqBool fJitter)
{
	m_XOrigin=xorigin;
	m_YOrigin=yorigin;
	m_XSize=xsize;
	m_YSize=ysize;
	m_XFWidth=xfwidth;
	m_YFWidth=yfwidth;
	// Allocate the image element storage for a single bucket
	m_aieImage.resize((xsize+xfwidth)*(ysize+yfwidth));

	// Initialise the samples for this bucket.
	TqInt i;
	for(i=0; i<(m_YSize+m_YFWidth); i++)
	{
		TqInt j;
		for(j=0; j<(m_XSize+m_XFWidth); j++)
		{
			CqVector2D bPos2(m_XOrigin,m_YOrigin);
			bPos2+=CqVector2D((j-m_XFWidth/2),(i-m_YFWidth/2));
			m_aieImage[(i*(m_XSize+m_XFWidth))+j].AllocateSamples(xsamples,ysamples);
			m_aieImage[(i*(m_XSize+m_XFWidth))+j].InitialiseSamples(bPos2,fJitter);
		}
	}
}


//----------------------------------------------------------------------
/** Initialise the static filter values.
 */

void CqBucket::InitialiseFilterValues()
{
	// Allocate and fill in the filter values array for each pixel.
	RtFilterFunc pFilter;
	pFilter=QGetRenderContext()->optCurrent().funcFilter();
	TqInt NumFilterValues=((m_YFWidth+1)*(m_XFWidth+1));//*(m_PixelXSamples*m_PixelYSamples);
	m_aFilterValues.resize(NumFilterValues);
	TqFloat* pFilterValues=&m_aFilterValues[0];
	TqInt fy=-m_YFWidth*0.5f;
	while(fy<=m_YFWidth*0.5f)
	{
		TqInt fx=-m_XFWidth*0.5f;
		while(fx<=m_YFWidth*0.5f)
		{
			TqFloat g=(*pFilter)(fx, fy, m_XFWidth, m_YFWidth);
			*pFilterValues++=g;

			fx++;
		}
		fy++;
	}
}


//----------------------------------------------------------------------
/** Combine the subsamples into single pixel samples and coverage information.
 */

void CqBucket::CombineElements()
{
	for(std::vector<CqImageElement>::iterator i=m_aieImage.begin(); i!=m_aieImage.end(); i++)
		i->Combine();
}


//----------------------------------------------------------------------
/** Get the sample color for the specified screen position.
 * If position is outside bucket, returns black.
 * \param iXPos Screen position of sample.
 * \param iYPos Screen position of sample.
 */

CqColor CqBucket::Color(TqInt iXPos, TqInt iYPos)
{
	CqImageElement* pie;
	if(ImageElement(iXPos,iYPos,pie))
		return(pie->Color());
	else
		return(CqColor(0.0f,0.0f,0.0f));
}


//----------------------------------------------------------------------
/** Get the sample coverage for the specified screen position.
 * If position is outside bucket, returns 0.
 * \param iXPos Screen position of sample.
 * \param iYPos Screen position of sample.
 */

TqFloat CqBucket::Coverage(TqInt iXPos, TqInt iYPos)
{
	CqImageElement* pie;
	if(ImageElement(iXPos,iYPos,pie))
		return(pie->Coverage());
	else
		return(0.0f);
}


//----------------------------------------------------------------------
/** Get the sample depth for the specified screen position.
 * If position is outside bucket, returns FLT_MAX.
 * \param iXPos Screen position of sample.
 * \param iYPos Screen position of sample.
 */

TqFloat CqBucket::Depth(TqInt iXPos, TqInt iYPos)
{
	CqImageElement* pie;
	if(ImageElement(iXPos,iYPos,pie))
		return(pie->Depth());
	else
		return(FLT_MAX);
}


//----------------------------------------------------------------------
/** Filter the samples in this bucket according to type and filter widths.
 */

void CqBucket::FilterBucket()
{
	CqImageElement* pie=&m_aieImage[0];;

	CqColor* pCols=new CqColor[XSize()*YSize()];
	TqFloat xmax=XFWidth();
	TqFloat ymax=YFWidth();
	TqInt	xlen=XSize()+XFWidth();

	TqInt x,y;
	TqInt i=0;
	for(y=0; y<YSize(); y++)
	{
		for(x=0; x<XSize(); x++)
		{
			CqColor c(0,0,0);
			TqFloat gTot=0.0;
			TqInt fy=0;
			TqFloat* pFilterValues=&m_aFilterValues[0];
			while(fy<=ymax)
			{
				TqInt fx=0;
				while(fx<=xmax)
				{
					TqInt ieoff=(fy*xlen)+fx;
					TqFloat g=*pFilterValues++;
					c+=pie[ieoff].Color()*g;
					gTot+=g;
					fx++;
				}
				fy++;
			}
			pie++;
			pCols[i++]=c/gTot;
		}
		pie+=XFWidth();
	}
	
	i=0;
	pie=&m_aieImage[((YFWidth()*0.5f)*xlen)+(XFWidth()*0.5f)];
	for(y=0; y<YSize(); y++)
	{
		for(x=0; x<XSize(); x++)
		{
			pie->Color()=pCols[i++];
			pie++;
		}
		pie+=XFWidth();
	}
	delete[](pCols);
}


//----------------------------------------------------------------------
/** Expose the samples in this bucket according to specified gain and gamma settings.
 */

void CqBucket::ExposeBucket()
{
	if(QGetRenderContext()->optCurrent().fExposureGain()==1.0 &&
	   QGetRenderContext()->optCurrent().fExposureGamma()==1.0)
		return;
	else
	{
		TqInt	xlen=XSize()+XFWidth();
		CqImageElement* pie=&m_aieImage[((YFWidth()*0.5f)*xlen)+(XFWidth()*0.5f)];
		TqInt x,y;
		for(y=0; y<YSize(); y++)
		{
			for(x=0; x<XSize(); x++)
			{
				// color=(color*gain)^1/gamma
				if(QGetRenderContext()->optCurrent().fExposureGain()!=1.0)
					pie->Color()*=QGetRenderContext()->optCurrent().fExposureGain();

				if(QGetRenderContext()->optCurrent().fExposureGamma()!=1.0)
				{
					TqFloat oneovergamma=1.0f/QGetRenderContext()->optCurrent().fExposureGamma();
					pie->Color().SetfRed  (pow(pie->Color().fRed  (),oneovergamma));
					pie->Color().SetfGreen(pow(pie->Color().fGreen(),oneovergamma));
					pie->Color().SetfBlue (pow(pie->Color().fBlue (),oneovergamma));
				}
				pie++;
			}
			pie+=XFWidth();
		}
	}
}


//----------------------------------------------------------------------
/** Quantize the samples in this bucket according to type.
 */

void CqBucket::QuantizeBucket()
{
	static CqRandom random;

	if(QGetRenderContext()->optCurrent().iDisplayMode()&ModeRGB)
	{
		double ditheramplitude=QGetRenderContext()->optCurrent().fColorQuantizeDitherAmplitude();
		if(ditheramplitude==0)	return;
		TqInt one=QGetRenderContext()->optCurrent().iColorQuantizeOne();
		TqInt min=QGetRenderContext()->optCurrent().iColorQuantizeMin();
		TqInt max=QGetRenderContext()->optCurrent().iColorQuantizeMax();

		TqInt	xlen=XSize()+XFWidth();
		CqImageElement* pie=&m_aieImage[((YFWidth()*0.5f)*xlen)+(XFWidth()*0.5f)];
		TqInt x,y;
		for(y=0; y<YSize(); y++)
		{
			for(x=0; x<XSize(); x++)
			{
				double r,g,b,a;
				double s=random.RandomFloat();
				if(modf(one*pie->Color().fRed  ()+ditheramplitude*s,&r)>0.5)	r+=1;
				if(modf(one*pie->Color().fGreen()+ditheramplitude*s,&g)>0.5)	g+=1;
				if(modf(one*pie->Color().fBlue ()+ditheramplitude*s,&b)>0.5)	b+=1;
				if(modf(one*pie->Coverage()		 +ditheramplitude*s,&a)>0.5)	a+=1;
				r=CLAMP(r,min,max);
				g=CLAMP(g,min,max);
				b=CLAMP(b,min,max);
				a=CLAMP(a,min,max);
				pie->Color().SetfRed  (r);
				pie->Color().SetfGreen(g);
				pie->Color().SetfBlue (b);
				pie->SetCoverage(a);
				
				pie++;
			}
			pie+=XFWidth();
		}
	}
	else
	{
		double ditheramplitude=QGetRenderContext()->optCurrent().fDepthQuantizeDitherAmplitude();
		if(ditheramplitude==0)	return;
		TqInt one=QGetRenderContext()->optCurrent().iDepthQuantizeOne();
		TqInt min=QGetRenderContext()->optCurrent().iDepthQuantizeMin();
		TqInt max=QGetRenderContext()->optCurrent().iDepthQuantizeMax();

		TqInt	xlen=XSize()+XFWidth();
		CqImageElement* pie=&m_aieImage[((YFWidth()*0.5f)*xlen)+(XFWidth()*0.5f)];
		TqInt x,y;
		for(y=0; y<YSize(); y++)
		{
			for(x=0; x<XSize(); x++)
			{
				double d;
				if(modf(one*pie->Depth()+ditheramplitude*random.RandomFloat(),&d)>0.5)	d+=1;
				d=CLAMP(d,min,max);
				pie->SetDepth(d);
				pie++;
			}
			pie+=XFWidth();
		}
	}
}


//----------------------------------------------------------------------
/** Destructor
 */

CqImageBuffer::~CqImageBuffer()
{
	DeleteImage();
}


//----------------------------------------------------------------------
/** Get the bucket index for the specified image coordinates.
 * \param X Integer pixel coordinate in X.
 * \param Y Integer pixel coordinate in Y.
 * \param Xb Storage for integer bucket index in X.
 * \param Yb Storage for integer bucket index in Y.
 * \return Integer bucket index.
 */

TqInt CqImageBuffer::Bucket(TqInt X, TqInt Y, TqInt& Xb, TqInt& Yb) const
{
	Xb=static_cast<TqInt>((X/m_XBucketSize));
	Yb=static_cast<TqInt>((Y/m_YBucketSize));
	
	return((Yb*m_cXBuckets)+Xb);
}


//----------------------------------------------------------------------
/** Get the bucket index for the specified image coordinates.
 * \param X Integer pixel coordinate in X.
 * \param Y Integer pixel coordinate in Y.
 * \return Integer bucket index.
 */

TqInt CqImageBuffer::Bucket(TqInt X, TqInt Y) const
{
	static TqInt Xb,Yb;
	return(Bucket(X,Y,Xb,Yb));
}


//----------------------------------------------------------------------
/** Get the screen position for the specified bucket index.
 * \param iBucket Integer bucket index.
 */

CqVector2D	CqImageBuffer::Position(TqInt iBucket) const
{
	CqVector2D	vecA;
	vecA.y(iBucket/m_cXBuckets);
	vecA.x(iBucket%m_cXBuckets);
	vecA.x(vecA.x()*XBucketSize());
	vecA.y(vecA.y()*YBucketSize());

	return(vecA);
}

//----------------------------------------------------------------------
/** Get the bucket size for the specified index.
 * \param iBucket Integer bucket index.
 */

CqVector2D CqImageBuffer::Size(TqInt iBucket) const
{
	CqVector2D	vecA=Position(iBucket);
	vecA.x(m_iXRes-vecA.x());
	if(vecA.x()>m_XBucketSize)	vecA.x(m_XBucketSize);
	vecA.y(m_iYRes-vecA.y());
	if(vecA.y()>m_YBucketSize)	vecA.y(m_YBucketSize);
	
	return(vecA);
}


//----------------------------------------------------------------------
/** Construct the image buffer to an initial state using the current options.
 */

void	CqImageBuffer::SetImage()
{
	DeleteImage();

	m_XBucketSize=16;
	m_YBucketSize=16;
	const TqInt* poptBucketSize=QGetRenderContext()->optCurrent().GetIntegerOption("limits","bucketsize");
	if(poptBucketSize!=0)
	{
		m_XBucketSize=poptBucketSize[0];
		m_YBucketSize=poptBucketSize[1];
	}

	m_iXRes=QGetRenderContext()->optCurrent().iXResolution();
	m_iYRes=QGetRenderContext()->optCurrent().iYResolution();
	m_CropWindowXMin=static_cast<TqInt>(CLAMP(ceil(m_iXRes*QGetRenderContext()->optCurrent().fCropWindowXMin()  ),0,m_iXRes));
	m_CropWindowXMax=static_cast<TqInt>(CLAMP(ceil(m_iXRes*QGetRenderContext()->optCurrent().fCropWindowXMax()  ),0,m_iXRes));
	m_CropWindowYMin=static_cast<TqInt>(CLAMP(ceil(m_iYRes*QGetRenderContext()->optCurrent().fCropWindowYMin()  ),0,m_iYRes));
	m_CropWindowYMax=static_cast<TqInt>(CLAMP(ceil(m_iYRes*QGetRenderContext()->optCurrent().fCropWindowYMax()  ),0,m_iYRes));
	m_cXBuckets=(m_iXRes/m_XBucketSize)+1;
	m_cYBuckets=(m_iYRes/m_YBucketSize)+1;
	m_PixelXSamples=static_cast<TqInt>(QGetRenderContext()->optCurrent().fPixelXSamples());
	m_PixelYSamples=static_cast<TqInt>(QGetRenderContext()->optCurrent().fPixelYSamples());
	m_FilterXWidth=static_cast<TqInt>(QGetRenderContext()->optCurrent().fFilterXWidth());
	m_FilterYWidth=static_cast<TqInt>(QGetRenderContext()->optCurrent().fFilterYWidth());
	m_DisplayMode=QGetRenderContext()->optCurrent().iDisplayMode();

	m_aBuckets.resize(m_cXBuckets*m_cYBuckets);

	CqBucket::InitialiseBucket(0,0,m_XBucketSize,m_YBucketSize,m_FilterXWidth,m_FilterYWidth,m_PixelXSamples,m_PixelXSamples);
	CqBucket::InitialiseFilterValues();
}


//----------------------------------------------------------------------
/** Delete the allocated memory for the image buffer.
 */

void	CqImageBuffer::DeleteImage()
{
	m_iXRes=0;
	m_iYRes=0;
}


//----------------------------------------------------------------------
/** This is called by the renderer to inform an image buffer it is no longer needed.
 */

void	CqImageBuffer::Release()
{
	delete(this);
}


//----------------------------------------------------------------------
/** Check if a surface can be culled.
 * \param Bound CqBound containing the geometric bound in camera space.
 * \param pSurface Pointer to the CqBasicSurface derived class being processed.
 * \return Boolean indicating that the GPrim can be culled.
 */

TqBool CqImageBuffer::CullSurface(CqBound& Bound, CqBasicSurface* pSurface)
{
	// If the primitive is completely outside of the hither-yon z range, cull it.
	if(Bound.vecMin().z()>=QGetRenderContext()->optCurrent().fClippingPlaneFar() ||
	   Bound.vecMax().z()<=QGetRenderContext()->optCurrent().fClippingPlaneNear())
		return(TqTrue);

	// If the primitive spans the epsilon plane and the hither plane and can be split,
	if(Bound.vecMin().z()<=FLT_EPSILON && Bound.vecMax().z()>=FLT_EPSILON)
	{
		// Mark the primitive as not dicable.
		pSurface->ForceUndiceable();
		TqInt MaxEyeSplits=10;
		const TqInt* poptEyeSplits=QGetRenderContext()->optCurrent().GetIntegerOption("limits","eyesplits");
		if(poptEyeSplits!=0)
			MaxEyeSplits=poptEyeSplits[0];

		if(pSurface->EyeSplitCount()>MaxEyeSplits)	
		{
			CqAttributeError(ErrorID_MaxEyeSplits, Severity_Normal,"Max eyesplits exceeded", pSurface->pAttributes(), TqTrue);
			pSurface->Discard();
		}
		return(TqFalse);
	}

	// Convert the bounds to screen space.
	Bound.Transform(QGetRenderContext()->matSpaceToSpace("camera","raster"));
	Bound.vecMin().x(Bound.vecMin().x()-m_FilterXWidth/2);
	Bound.vecMin().y(Bound.vecMin().y()-m_FilterYWidth/2);
	Bound.vecMax().x(Bound.vecMax().x()+m_FilterXWidth/2);
	Bound.vecMax().y(Bound.vecMax().y()+m_FilterYWidth/2);

	// If the bounds are completely outside the viewing frustum, cull the primitive.
	if(Bound.vecMin().x()>CropWindowXMax()+m_FilterXWidth/2 || 
	   Bound.vecMin().y()>CropWindowYMax()+m_FilterYWidth/2 ||
	   Bound.vecMax().x()<CropWindowXMin()-m_FilterXWidth/2 || 
	   Bound.vecMax().y()<CropWindowYMin()-m_FilterYWidth/2)
		return(TqTrue);

	return(TqFalse);
}


//----------------------------------------------------------------------
/** Add a new surface to the front of the list of waiting ones.
 * \param pSurface A pointer to a CqBasicSurface derived class, surface should at this point be in camera space.
 */

void CqImageBuffer::PostSurface(CqBasicSurface* pSurface)
{
	// Bound the primitive in its current space (camera) space taking into account any motion specification.
	CqBound Bound(pSurface->Bound());

	// Take into account the displacement bound extension.
	TqFloat db=0;
	CqString strCoordinateSystem("object");
	const TqFloat* pattrDispclacementBound=pSurface->pAttributes()->GetFloatAttribute("displacementbound", "sphere");
	const CqString* pattrCoordinateSystem=pSurface->pAttributes()->GetStringAttribute("displacementbound", "coordinatesystem");
	if(pattrDispclacementBound!=0)	db=pattrDispclacementBound[0];
	if(pattrCoordinateSystem!=0)	strCoordinateSystem=pattrCoordinateSystem[0];

	CqVector3D	vecDB(db,0,0);
	vecDB=QGetRenderContext()->matVSpaceToSpace(strCoordinateSystem.c_str(),"camera",pSurface->pAttributes()->pshadSurface()->matCurrent(),pSurface->pTransform()->matObjectToWorld())*vecDB;
	db=vecDB.Magnitude();

	Bound.vecMax()+=db;
	Bound.vecMin()-=db;

	// Check if the surface can be culled. (also converts Bound to raster space).
	if(CullSurface(Bound,pSurface))
	{
		delete(pSurface);
		return;
	}

	// Find out which bucket(s) the surface belongs to.
	TqInt XMinb,YMinb;
	if(Bound.vecMin().x()<0)	Bound.vecMin().x(0.0f);
	if(Bound.vecMin().y()<0)	Bound.vecMin().y(0.0f);
	TqInt iBucket=Bucket(static_cast<TqInt>(Bound.vecMin().x()), static_cast<TqInt>(Bound.vecMin().y()),XMinb,YMinb);

	if(XMinb>=m_cXBuckets || YMinb>=m_cYBuckets)	return;
	
	if(XMinb<0 || YMinb<0)
	{
		if(XMinb<0)	XMinb=0;
		if(YMinb<0)	YMinb=0;
		iBucket=(YMinb*m_cXBuckets)+XMinb;
	}
	m_aBuckets[iBucket].AddGPrim(pSurface);
}


//----------------------------------------------------------------------
/** Add a new micro polygon to the list of waiting ones.
 * \param pmpgNew Pointer to a CqMicroPolygonBase derived class.
 */

void CqImageBuffer::AddMPG(CqMicroPolygonBase* pmpgNew)
{
	// Find out which bucket(s) the mpg belongs to.
	CqBound	B(pmpgNew->Bound());

	if(B.vecMax().x()<m_CropWindowXMin-m_FilterXWidth/2 || B.vecMax().y()<m_CropWindowYMin-m_FilterYWidth/2 ||
	   B.vecMin().x()>m_CropWindowXMax+m_FilterXWidth/2 || B.vecMin().y()>m_CropWindowYMax+m_FilterYWidth/2)
	{
		pmpgNew->Release();
		return;
	}

	B.vecMin().x(B.vecMin().x()-m_FilterXWidth/2);
	B.vecMin().y(B.vecMin().y()-m_FilterYWidth/2);
	B.vecMax().x(B.vecMax().x()+m_FilterXWidth/2);
	B.vecMax().y(B.vecMax().y()+m_FilterYWidth/2);

	TqInt iXBa=static_cast<TqInt>(B.vecMin().x()/(m_XBucketSize));
	TqInt iXBb=static_cast<TqInt>(B.vecMax().x()/(m_XBucketSize));
	TqInt iYBa=static_cast<TqInt>(B.vecMin().y()/(m_YBucketSize));
	TqInt iYBb=static_cast<TqInt>(B.vecMax().y()/(m_YBucketSize));
	// Now duplicate and link into any buckets it crosses.

	TqInt iXB=iXBa,iYB=iYBa;
	do
	{
		if(iYB>=0 && iYB<m_cYBuckets)
		{
			iXB=iXBa;
			do
			{
				if(iXB>=0 && iXB<m_cXBuckets)
				{
					m_aBuckets[(iYB*m_cXBuckets)+iXB].AddMPG(pmpgNew);
					pmpgNew->AddRef();
				}
				iXB+=1;
			}while(iXB<=iXBb);
		}
		iYB+=1;
	}while(iYB<=iYBb);
}


//----------------------------------------------------------------------
/** Render any waiting MPGs
 * \param iBucket Integer index of bucket being processed.
 * \param xmin Integer minimum extend of the image part being rendered, takes into account buckets and clipping.
 * \param xmax Integer maximum extend of the image part being rendered, takes into account buckets and clipping.
 * \param ymin Integer minimum extend of the image part being rendered, takes into account buckets and clipping.
 * \param ymax Integer maximum extend of the image part being rendered, takes into account buckets and clipping.
 */

void CqImageBuffer::RenderMPGs(TqInt iBucket, long xmin, long xmax, long ymin, long ymax)
{
	// First of all split any grids in this bucket waiting to be processed.
	if(!m_aBuckets[iBucket].aGrids().empty())
	{
		for(std::vector<CqMicroPolyGridBase*>::iterator i=m_aBuckets[iBucket].aGrids().begin(); i!=m_aBuckets[iBucket].aGrids().end(); i++)
			(*i)->Split(this,iBucket,xmin,xmax,ymin,ymax);
	}
	m_aBuckets[iBucket].aGrids().clear();

	// Render any waiting MPGs
	static	CqColor	colWhite(1,1,1);
	static	CqVector2D	vecP;
	
	if(m_aBuckets[iBucket].aMPGs().empty())	return;

	TqFloat farplane=QGetRenderContext()->optCurrent().fClippingPlaneFar();
	TqFloat nearplane=QGetRenderContext()->optCurrent().fClippingPlaneNear();
	
	register long iY=ymin;

	for(std::vector<CqMicroPolygonBase*>::iterator i=m_aBuckets[iBucket].aMPGs().begin(); i!=m_aBuckets[iBucket].aMPGs().end(); i++)
	{
		RenderMicroPoly(*i,iBucket,xmin,xmax,ymin,ymax);
		(*i)->Release();
	}

	m_aBuckets[iBucket].aMPGs().clear();
}


//----------------------------------------------------------------------
/** Render a particular micopolygon.
 * \param pMPG Pointer to the micrpolygon to process.
 * \param iBucket Integer index of bucket being processed.
 * \param xmin Integer minimum extend of the image part being rendered, takes into account buckets and clipping.
 * \param xmax Integer maximum extend of the image part being rendered, takes into account buckets and clipping.
 * \param ymin Integer minimum extend of the image part being rendered, takes into account buckets and clipping.
 * \param ymax Integer maximum extend of the image part being rendered, takes into account buckets and clipping.
 */

inline void CqImageBuffer::RenderMicroPoly(CqMicroPolygonBase* pMPG, TqInt iBucket, long xmin, long xmax, long ymin, long ymax)
{
	static	SqImageValue	ImageVal;

	CqBucket& Bucket=m_aBuckets[iBucket];

	// Bound the microplygon in hybrid camera/raster space
	CqBound Bound(pMPG->Bound());

	TqFloat bminx=Bound.vecMin().x();
	TqFloat bmaxx=Bound.vecMax().x();
	TqFloat bminy=Bound.vecMin().y();
	TqFloat bmaxy=Bound.vecMax().y();

	if(bmaxx<xmin || bmaxy<ymin || bminx>xmax || bminy>ymax)
		return;

	// If the micropolygon is outside the hither-yon range, cull it.
	if(Bound.vecMin().z()>QGetRenderContext()->optCurrent().fClippingPlaneFar() ||
	   Bound.vecMax().z()<QGetRenderContext()->optCurrent().fClippingPlaneNear())
		return;

	// Now go across all pixels touched by the micropolygon bound.
	long eX=static_cast<TqInt>(ceil(bmaxx));
	long eY=static_cast<TqInt>(ceil(bmaxy));
	if(eX>=xmax)	eX=xmax-1;
	if(eY>=ymax)	eY=ymax-1;

	CqImageElement* pie;

	long initY=static_cast<long>(floor(Bound.vecMin().y()));
	if(initY<ymin)	initY=ymin;

	long initX=static_cast<long>(floor(Bound.vecMin().x()));
	if(initX<xmin)	initX=xmin;

	TqInt iXSamples=static_cast<TqInt>(QGetRenderContext()->optCurrent().fPixelXSamples());
	TqInt iYSamples=static_cast<TqInt>(QGetRenderContext()->optCurrent().fPixelYSamples());
	
	TqInt im=static_cast<TqInt>((bminx<initX)?0:floor((bminx-initX)*iXSamples));
	TqInt in=static_cast<TqInt>((bminy<initY)?0:floor((bminy-initY)*iYSamples));
	TqInt em=static_cast<TqInt>((bmaxx>eX)?iXSamples:ceil((bmaxx-eX)*iXSamples));
	TqInt en=static_cast<TqInt>((bmaxy>eY)?iYSamples:ceil((bmaxy-eY)*iYSamples));

	register long iY=initY;

	while(iY<=eY)
	{
		register long iX=initX;

		bool valid=Bucket.ImageElement(iX,iY,pie);

		while(iX<=eX)
		{
			register int m,n;
			n=(iY==initY)?in:0;
			int end_n=(iY==eY)?en:iYSamples;
			int start_m=(iX==initX)?im:0;
			int end_m=(iX==eX)?em:iXSamples;
			TqBool brkVert=TqFalse;
			for(; n<end_n && !brkVert; n++)
			{
				TqBool brkHoriz=TqFalse;
				for(m=start_m; m<end_m && !brkHoriz; m++)
				{
					CqVector2D vecP(pie->SamplePoint(m,n));
					QGetRenderContext()->Stats().cSamples()++;
					if(Bound.Contains2D(vecP))
					{
						QGetRenderContext()->Stats().cSampleBoundHits()++;

						TqFloat t=pie->SampleTime(m,n);
						if(pMPG->Sample(vecP,t,ImageVal.m_Depth))
						{									
							QGetRenderContext()->Stats().cSampleHits()++;
							std::vector<SqImageValue>& aValues=pie->Values(m,n);
							int i=0;
							int c=aValues.size();
							if(c>0 && aValues[0].m_Depth<ImageVal.m_Depth)
							{
								SqImageValue* p=&aValues[0];
								while(i<c && p[i].m_Depth<ImageVal.m_Depth)	i++;
								// If it is exaclty the same, chances are we've hit a MPG grid line.
								if(i<c && p[i].m_Depth==ImageVal.m_Depth)
								{
									p[i].m_colColor=(p[i].m_colColor+pMPG->colColor())*0.5f;
									p[i].m_colOpacity=(p[i].m_colOpacity+pMPG->colOpacity())*0.5f;
									continue;
								}
							}
							ImageVal.m_colColor=pMPG->colColor();
							ImageVal.m_colOpacity=pMPG->colOpacity();
							aValues.insert(aValues.begin()+i,ImageVal);
						}
					}
					else
					{
						if(vecP.x()>Bound.vecMax().x())
							brkHoriz=TqTrue;
						if(vecP.y()>Bound.vecMax().y())
							brkVert=TqTrue;
					}
				}
			}
			iX++;
			pie++;
		}
		iY++;
	}
}

//----------------------------------------------------------------------
/** Render any waiting Surfaces
 * \param iBucket Integer index of bucket being processed.
 * \param xmin Integer minimum extend of the image part being rendered, takes into account buckets and clipping.
 * \param xmax Integer maximum extend of the image part being rendered, takes into account buckets and clipping.
 * \param ymin Integer minimum extend of the image part being rendered, takes into account buckets and clipping.
 * \param ymax Integer maximum extend of the image part being rendered, takes into account buckets and clipping.
 */

void CqImageBuffer::RenderSurfaces(TqInt iBucket,long xmin, long xmax, long ymin, long ymax)
{
	// Render any waiting micro polygon grids.
	RenderMPGs(iBucket, xmin, xmax, ymin, ymax);

	CqBucket& Bucket=m_aBuckets[iBucket];

	// Render any waiting subsurfaces.
	CqBasicSurface* pSurface=Bucket.pTopSurface();
	while(pSurface!=0)
	{
		if(m_fQuit)	return;

		if(pSurface->Diceable())
		{
			CqMicroPolyGridBase* pGrid;
			if((pGrid=pSurface->Dice())!=0)
			{
				// Only shade if the ImageBuffer mode is at least RGB
				if(QGetRenderContext()->optCurrent().iDisplayMode()&ModeRGB)
					pGrid->Shade();

				pGrid->Project();
				Bucket.AddGrid(pGrid);
			}
		}
     	else if(!pSurface->fDiscard())
		{
			std::vector<CqBasicSurface*> aSplits;
			TqInt cSplits=pSurface->Split(aSplits);
			TqInt i;
			for(i=0; i<cSplits; i++)
				PostSurface(aSplits[i]);
		}

		delete(pSurface);
		pSurface=Bucket.pTopSurface();
		// Render any waiting micro polygon grids.
		RenderMPGs(iBucket, xmin, xmax, ymin, ymax);
	}
	
	// Now combine the colors at each pixel sample for any micropolygons rendered to that pixel.
	if(m_fQuit)	return;
	if(QGetRenderContext()->optCurrent().iDisplayMode()&ModeRGB)
		CqBucket::CombineElements();

	Bucket.FilterBucket();
	Bucket.ExposeBucket();
	Bucket.QuantizeBucket();
	
	BucketComplete(iBucket);
	QGetRenderContext()->pDDmanager()->DisplayBucket(&m_aBuckets[iBucket]);
}


//----------------------------------------------------------------------
/** Render any waiting Surfaces
 */

void CqImageBuffer::RenderImage()
{
	// Render the surface at the front of the list.
	m_fDone=TqFalse;

	TqInt iBucket;
	for(iBucket=0; iBucket<m_cXBuckets*m_cYBuckets; iBucket++)
	{
		// Prepare the bucket.
		CqBucket::Clear();
		CqVector2D bPos=Position(iBucket);
		CqVector2D bSize=Size(iBucket);
		CqBucket::InitialiseBucket(bPos.x(), bPos.y(), bSize.x(), bSize.y(), m_FilterXWidth, m_FilterYWidth, m_PixelXSamples, m_PixelYSamples);

		CqBucket& Bucket=m_aBuckets[iBucket];

		// Set up some bounds for the bucket.
		CqVector2D vecMin=bPos;
		CqVector2D vecMax=bPos+bSize;
		vecMin-=CqVector2D(m_FilterXWidth/2,m_FilterYWidth/2);
		vecMax+=CqVector2D(m_FilterXWidth/2,m_FilterYWidth/2);

		long xmin=static_cast<long>(vecMin.x());
		long ymin=static_cast<long>(vecMin.y());
		long xmax=static_cast<long>(vecMax.x());
		long ymax=static_cast<long>(vecMax.y());

		if(xmin<CropWindowXMin()-m_FilterXWidth/2)	xmin=CropWindowXMin()-m_FilterXWidth/2;
		if(ymin<CropWindowYMin()-m_FilterYWidth/2)	ymin=CropWindowYMin()-m_FilterYWidth/2;
		if(xmax>CropWindowXMax()+m_FilterXWidth/2)	xmax=CropWindowXMax()+m_FilterXWidth/2;
		if(ymax>CropWindowYMax()+m_FilterYWidth/2)	ymax=CropWindowYMax()+m_FilterYWidth/2;

		// Inform the status class how far we have got, and update UI.
		float Complete=(m_cXBuckets*m_cYBuckets);
		Complete/=iBucket;
		Complete=100.0f/Complete;
		QGetRenderContext()->Stats().SetComplete(Complete);

		RenderSurfaces(iBucket,xmin,xmax,ymin,ymax);
		if(m_fQuit)
		{
			m_fDone=TqTrue;
			return;
		}
	}

	ImageComplete();
	m_fDone=TqTrue;
}


//----------------------------------------------------------------------
/** Stop rendering.
 */

void CqImageBuffer::Quit()
{
	m_fQuit=TqTrue;
}


//---------------------------------------------------------------------
 
END_NAMESPACE(Aqsis)


