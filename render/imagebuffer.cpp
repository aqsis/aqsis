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

#include	<math.h>

#include	"aqsis.h"
#include	"stats.h"
#include	"imagebuffer.h"
#include	"options.h"
#include	"irenderer.h"
#include	"surface.h"
#include	"shadervm.h"
#include	"micropolygon.h"

START_NAMESPACE(Aqsis)


//----------------------------------------------------------------------
/** Constructor
 */

CqImageElement::CqImageElement() : 
				m_XSamples(0),
				m_YSamples(0),
				m_aValues(0), 
				m_avecSamples(0),
				m_colColor(0,0,0)
{
}


//----------------------------------------------------------------------
/** Destructor
 */

CqImageElement::~CqImageElement()
{
	delete[](m_aValues);
	delete[](m_avecSamples);
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
	delete[](m_aValues);
	delete[](m_avecSamples);
	m_aValues=0;
	m_avecSamples=0;

	m_XSamples=XSamples;
	m_YSamples=YSamples;

	if(XSamples>0 && YSamples>0)
	{
		m_aValues=new std::vector<SqImageValue>[m_XSamples*m_YSamples];
		m_avecSamples=new CqVector2D[m_XSamples*m_YSamples];
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
/** Copy function.
 */

CqImageElement& CqImageElement::operator=(const CqImageElement& ieFrom)
{
	if(ieFrom.m_XSamples*ieFrom.m_YSamples != m_XSamples*m_YSamples)
	{
		// Delete any existing arrays.
		delete[](m_aValues);
		delete[](m_avecSamples);

		// Now allocate new arrays
		m_aValues=new std::vector<SqImageValue>[ieFrom.m_XSamples*ieFrom.m_YSamples];
		m_avecSamples=new CqVector2D[ieFrom.m_XSamples*ieFrom.m_YSamples];
	}
	m_XSamples=ieFrom.m_XSamples;
	m_YSamples=ieFrom.m_YSamples;

	TqInt i;
	for(i=(m_XSamples*m_YSamples)-1; i>=0; i--)
	{
		m_aValues[i]=ieFrom.m_aValues[i];
		m_avecSamples[i]=ieFrom.m_avecSamples[i];
	}

	m_colColor=ieFrom.m_colColor;
	return(*this);	
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
	const TqInt* poptBucketSize=pCurrentRenderer()->optCurrent().GetIntegerOption("limits","bucketsize");
	if(poptBucketSize!=0)
	{
		m_XBucketSize=poptBucketSize[0];
		m_YBucketSize=poptBucketSize[1];
	}

	m_iXRes=pCurrentRenderer()->optCurrent().iXResolution();
	m_iYRes=pCurrentRenderer()->optCurrent().iYResolution();
	m_CropWindowXMin=static_cast<TqInt>(CLAMP(ceil(m_iXRes*pCurrentRenderer()->optCurrent().fCropWindowXMin()  ),0,m_iXRes));
	m_CropWindowXMax=static_cast<TqInt>(CLAMP(ceil(m_iXRes*pCurrentRenderer()->optCurrent().fCropWindowXMax()  ),0,m_iXRes));
	m_CropWindowYMin=static_cast<TqInt>(CLAMP(ceil(m_iYRes*pCurrentRenderer()->optCurrent().fCropWindowYMin()  ),0,m_iYRes));
	m_CropWindowYMax=static_cast<TqInt>(CLAMP(ceil(m_iYRes*pCurrentRenderer()->optCurrent().fCropWindowYMax()  ),0,m_iYRes));
	m_cXBuckets=(m_iXRes/m_XBucketSize)+1;
	m_cYBuckets=(m_iYRes/m_YBucketSize)+1;
	m_PixelXSamples=static_cast<TqInt>(pCurrentRenderer()->optCurrent().fPixelXSamples());
	m_PixelYSamples=static_cast<TqInt>(pCurrentRenderer()->optCurrent().fPixelYSamples());
	m_FilterXWidth=static_cast<TqInt>(pCurrentRenderer()->optCurrent().fFilterXWidth());
	m_FilterYWidth=static_cast<TqInt>(pCurrentRenderer()->optCurrent().fFilterYWidth());
	m_DisplayMode=pCurrentRenderer()->optCurrent().iDisplayMode();

	// Allocate the image element storage for a single bucket
	m_pieImage=new CqImageElement[(m_XBucketSize+m_FilterXWidth)*(m_YBucketSize+m_FilterYWidth)];

	AllocateSamples();
	InitialiseSamples(0);

	// Allocate the array of surface and micropolygrid lists for all buckets
	m_aampgWaiting.resize(m_cXBuckets*m_cYBuckets);
	m_aagridWaiting.resize(m_cXBuckets*m_cYBuckets);
	m_aSurfaces=new CqList<CqBasicSurface>[m_cXBuckets*m_cYBuckets];
	m_aScene=new std::vector<CqBasicSurface*>[m_cXBuckets*m_cYBuckets];
	ClearBucket();

	// Allocate and fill in the filter values array for each pixel.
	RtFilterFunc pFilter;
	pFilter=pCurrentRenderer()->optCurrent().funcFilter();
	m_aaFilterValues.resize(m_XBucketSize*m_YBucketSize);
	CqImageElement* pie;
	TqInt NumFilterValues=((m_FilterYWidth+1)*(m_FilterXWidth+1));//*(m_PixelXSamples*m_PixelYSamples);
	TqInt y;
	for(y=0; y<m_YBucketSize; y++)
	{
		TqInt x;
		for(x=0; x<m_XBucketSize; x++)
		{
			Pixel(x,y,0,pie);
			// Allocate enough entries
			m_aaFilterValues[(y*m_XBucketSize)+x].resize(NumFilterValues);
			TqFloat* pFilterValues=&m_aaFilterValues[(y*m_XBucketSize)+x][0];
			TqInt fy=-m_FilterYWidth/2;
			while(fy<=m_FilterYWidth/2)
			{
				TqInt fx=-m_FilterXWidth/2;
				while(fx<=m_FilterXWidth/2)
				{
					TqFloat g=(*pFilter)(fx, fy, m_FilterXWidth, m_FilterYWidth);
					*pFilterValues++=g;

					fx++;
				}
				fy++;
			}
		}
	}

	// Clear state ready for render
	m_CurrBucket=0;
}


//----------------------------------------------------------------------
/** Allocate the storage for the pixel samples in the bucket, should be called only once.
 */

void CqImageBuffer::AllocateSamples()
{
	TqInt i;
	for(i=0; i<(m_XBucketSize+m_FilterXWidth)*(m_YBucketSize+m_FilterYWidth); i++)
	{
		m_pieImage[i].AllocateSamples(m_PixelXSamples,m_PixelYSamples);
	}
}


//----------------------------------------------------------------------
/** Initialise the sample information for the specified bucket.
 * \param iBucket Integer bucket index.
 */

void CqImageBuffer::InitialiseSamples(TqInt iBucket)
{
	TqBool fJitter=(pCurrentRenderer()->optCurrent().iDisplayMode()&ModeRGB);

	CqVector2D	bPos=Position(iBucket);
	TqInt i;
	for(i=0; i<(m_YBucketSize+m_FilterYWidth); i++)
	{
		TqInt j;
		for(j=0; j<(m_XBucketSize+m_FilterXWidth); j++)
		{
			CqVector2D bPos2(bPos);
			bPos2+=CqVector2D((j-m_FilterXWidth/2),(i-m_FilterYWidth/2));
			m_pieImage[(i*(m_XBucketSize+m_FilterXWidth))+j].InitialiseSamples(bPos2,fJitter);
		}
	}
}

//----------------------------------------------------------------------
/** Reset the image buffer to default values.
 */

void	CqImageBuffer::ClearBucket()
{
	// Call the clear function on each element in the bucket.
	if(m_pieImage!=0)
	{
		TqInt iElement;
		for(iElement=0; iElement<(m_XBucketSize+m_FilterXWidth)*(m_YBucketSize+m_FilterYWidth); iElement++)
			m_pieImage[iElement].Clear();
	}
}


//----------------------------------------------------------------------
/** Delete the allocated memory for the image buffer.
 */

void	CqImageBuffer::DeleteImage()
{
	delete[](m_pieImage);
	delete[](m_aSurfaces);
	delete[](m_aScene);

	m_pieImage=0;
	m_aSurfaces=0;
	m_aampgWaiting.clear();
	m_aagridWaiting.clear();
	m_aScene=0;

	m_aaFilterValues.clear();

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
/** Get a reference to pixel data.
 * \param iXPos Integer pixel coordinate.
 * \param iYPos Integer pixel coordinate.
 * \param iBucket Integer bucket index.
 * \param pie Pointer to CqImageElement to fill in.
 * \return Boolean indicating success, could fail if the specified pixel is not within the specified bucket.
 */

TqBool CqImageBuffer::Pixel(TqInt iXPos, TqInt iYPos, TqInt iBucket, CqImageElement*& pie)
{
	TqInt iYBucket=iBucket/m_cXBuckets;
	TqInt iXBucket=iBucket%m_cXBuckets;
	
	iXPos-=(iXBucket*m_XBucketSize);
	iYPos-=(iYBucket*m_YBucketSize);

	int fyo2=m_FilterYWidth*0.5f;
	int fxo2=m_FilterXWidth*0.5f;
	
	// Check within renderable range
	if(iXPos>=-fxo2 && iXPos<=m_XBucketSize+fxo2 &&
	   iYPos>=-fyo2 && iYPos<=m_YBucketSize+fyo2)
	{
		TqInt i=((iYPos+fyo2)*(m_XBucketSize+m_FilterXWidth))+(iXPos+fxo2);
		pie=&m_pieImage[i];
		return(TqTrue);
	}
	else
		return(TqFalse);
}


//----------------------------------------------------------------------
/** Get the value of the specified pixel using the current filter function.
 * \param iXPos Integer pixel coordinate.
 * \param iYPos Integer pixel coordinate.
 * \param iBucket Integer bucket index.
 * \param Val SqImageValue structure to fill in.
 */

void CqImageBuffer::FilterPixel(TqInt X, TqInt Y, TqInt iBucket, SqImageValue& Val)
{
	TqInt iYBucket=iBucket/m_cXBuckets;
	TqInt iXBucket=iBucket-(iYBucket*m_cXBuckets);
	
	CqImageElement* pie;
	if(!Pixel(X-(m_FilterXWidth/2),Y-(m_FilterYWidth/2),iBucket,pie))
	{
		CqBasicError(ErrorID_InvalidPixel,Severity_Fatal,"Invalid Pixel filter request");
		return;
	}

	TqFloat* pFilterValues=&m_aaFilterValues[((Y%m_YBucketSize)*m_XBucketSize)+(X%m_XBucketSize)][0];

	CqColor c(0,0,0);
	TqFloat d=0.0f;
	TqFloat gTot=0.0;
	TqFloat xmax=m_FilterXWidth*0.5f;
	TqFloat ymax=m_FilterYWidth*0.5f;
	TqInt fy=-ymax;
	while(fy<=ymax)
	{
		TqInt fx=-xmax;
		CqImageElement* pie2=pie;
		while(fx<=xmax)
		{
			TqFloat g=*pFilterValues++;
			c+=pie2->Color()*g;
			d+=pie2->Depth()*g;

			gTot+=g;

			pie2++;
			fx++;
		}
		pie+=(m_XBucketSize+m_FilterXWidth);
		fy++;
	}

	Val.m_colColor=c/gTot;
	Val.m_colOpacity=(CqColor(1,1,1)); // TODO: Work out the A value properly.
	Val.m_Depth=d/gTot;
}


//----------------------------------------------------------------------
/** Perform standard exposure correction on the specified pixel value.
 * \param Pixel SqImageElement structure to modify.
 */

void CqImageBuffer::ExposePixel(SqImageValue& Pixel)
{
	if(pCurrentRenderer()->optCurrent().fExposureGain()==1.0 &&
	   pCurrentRenderer()->optCurrent().fExposureGamma()==1.0)
		return;
	else
	{
		// color=(color*gain)^1/gamma
		if(pCurrentRenderer()->optCurrent().fExposureGain()!=1.0)
			Pixel.m_colColor*=pCurrentRenderer()->optCurrent().fExposureGain();

		if(pCurrentRenderer()->optCurrent().fExposureGamma()!=1.0)
		{
			TqFloat oneovergamma=1.0f/pCurrentRenderer()->optCurrent().fExposureGamma();
			Pixel.m_colColor.SetfRed  (pow(Pixel.m_colColor.fRed  (),oneovergamma));
			Pixel.m_colColor.SetfGreen(pow(Pixel.m_colColor.fGreen(),oneovergamma));
			Pixel.m_colColor.SetfBlue (pow(Pixel.m_colColor.fBlue (),oneovergamma));
		}
	}
}


//----------------------------------------------------------------------
/** Perform standard color quantisation on the specified pixel value.
 * \param Pixel SqImageElement to modify.
 */

void CqImageBuffer::QuantizePixel(SqImageValue& Pixel)
{
	static CqRandom random;

	if(pCurrentRenderer()->optCurrent().iDisplayMode()&ModeRGB)
	{
		double ditheramplitude=pCurrentRenderer()->optCurrent().fColorQuantizeDitherAmplitude();
		if(ditheramplitude==0)	return;
		TqInt one=pCurrentRenderer()->optCurrent().iColorQuantizeOne();
		TqInt min=pCurrentRenderer()->optCurrent().iColorQuantizeMin();
		TqInt max=pCurrentRenderer()->optCurrent().iColorQuantizeMax();

		double r,g,b,a;
		double s=random.RandomFloat();
		if(modf(one*Pixel.m_colColor.fRed  ()+ditheramplitude*s,&r)>0.5)	r+=1;
		if(modf(one*Pixel.m_colColor.fGreen()+ditheramplitude*s,&g)>0.5)	g+=1;
		if(modf(one*Pixel.m_colColor.fBlue ()+ditheramplitude*s,&b)>0.5)	b+=1;
		if(modf(one*Pixel.m_Coverage		  +ditheramplitude*s,&a)>0.5)	a+=1;
		r=CLAMP(r,min,max);
		g=CLAMP(g,min,max);
		b=CLAMP(b,min,max);
		a=CLAMP(a,min,max);
		Pixel.m_colColor.SetfRed  (r);
		Pixel.m_colColor.SetfGreen(g);
		Pixel.m_colColor.SetfBlue (b);
		Pixel.m_Coverage=a;
	}
	else
	{
		double ditheramplitude=pCurrentRenderer()->optCurrent().fDepthQuantizeDitherAmplitude();
		if(ditheramplitude==0)	return;
		TqInt one=pCurrentRenderer()->optCurrent().iDepthQuantizeOne();
		TqInt min=pCurrentRenderer()->optCurrent().iDepthQuantizeMin();
		TqInt max=pCurrentRenderer()->optCurrent().iDepthQuantizeMax();

		double d;
		if(modf(one*Pixel.m_Depth+ditheramplitude*random.RandomFloat(),&d)>0.5)	d+=1;
		d=CLAMP(d,min,max);
		Pixel.m_Depth=d;
	}
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
	if(Bound.vecMin().z()>=pCurrentRenderer()->optCurrent().fClippingPlaneFar() ||
	   Bound.vecMax().z()<=pCurrentRenderer()->optCurrent().fClippingPlaneNear())
		return(TqTrue);

	// If the primitive spans the epsilon plane and the hither plane and can be split,
	if(Bound.vecMin().z()<=FLT_EPSILON && Bound.vecMax().z()>=FLT_EPSILON)
	{
		// Mark the primitive as not dicable.
		pSurface->ForceUndiceable();
		TqInt MaxEyeSplits=10;
		const TqInt* poptEyeSplits=pCurrentRenderer()->optCurrent().GetIntegerOption("limits","eyesplits");
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
	Bound.Transform(pCurrentRenderer()->matSpaceToSpace("camera","raster"));
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
/** Add a pointer to the a top level scene surface to the list, in the correct bucket.
 * \param  pSurface A pointer to a CqBasicSurface derived class, surface should at this point be in camera space.
 */

void CqImageBuffer::AddSurfacePointer(CqBasicSurface* pSurface)
{
	// Bound the primitive in camera space taking into account any motion specification.
	CqBound Bound(pSurface->Bound());
	pSurface->ExpandBoundForMotion(Bound);

	// Take into account the displacement bound extension.
	TqFloat db=0;
	CqString strCoordinateSystem("object");
	const TqFloat* pattrDisplacementBound=pSurface->pAttributes()->GetFloatAttribute("displacementbound", "sphere");
	const CqString* pattrCoordinateSystem=pSurface->pAttributes()->GetStringAttribute("displacementbound", "coordinatesystem");
	if(pattrDisplacementBound!=0)	db=pattrDisplacementBound[0];
	if(pattrCoordinateSystem!=0)	strCoordinateSystem=pattrCoordinateSystem[0];

	// Do a quick check if there is a Displacement shader specified, but no displacement bound.
	// This will not catch the case where a surface shader does displacement, but helps anyway.
	if(pattrDisplacementBound==0 && pSurface->pAttributes()->pshadDisplacement()!=0)
		CqAttributeError(WarningID_NoDisplacementBound, Severity_Normal,"Using displacement shader without specifying displacementbound option, may cause rendering problems", pSurface->pAttributes(), TqTrue);

	CqVector3D	vecDB(db,0,0);
	vecDB=pCurrentRenderer()->matVSpaceToSpace(strCoordinateSystem.c_str(),"camera",pSurface->pAttributes()->pshadSurface()->matCurrent(),pSurface->pTransform()->matObjectToWorld())*vecDB;
	db=vecDB.Magnitude();

	Bound.vecMax()+=db;
	Bound.vecMin()-=db;
	
	// Check if the surface can be culled. (also converts Bound to raster space).
	if(CullSurface(Bound,pSurface))
		return;

	// Find out which bucket(s) the surface belongs to.
	TqInt XMinb,YMinb;
	TqInt iBucket=Bucket(static_cast<TqInt>(Bound.vecMin().x()), static_cast<TqInt>(Bound.vecMin().y()),XMinb,YMinb);
	
	if(XMinb>=m_cXBuckets || YMinb>=m_cYBuckets)	return;
	
	if(XMinb<0 || YMinb<0)
	{
		if(XMinb<0)	XMinb=0;
		if(YMinb<0)	YMinb=0;
		iBucket=(YMinb*m_cXBuckets)+XMinb;
	}
	assert(iBucket>=m_CurrBucket);
	m_aScene[iBucket].push_back(pSurface);
}


//----------------------------------------------------------------------
/** Add a new surface to the front of the list of waiting ones.
 * \param pSurface A pointer to a CqBasicSurface derived class, surface should at this point be in camera space.
 */

void CqImageBuffer::PostSurface(CqBasicSurface* pSurface)
{
	// Bound the primitive in its current space (camera) space taking into account any motion specification.
	CqBound Bound(pSurface->Bound());
	pSurface->ExpandBoundForMotion(Bound);

	// Take into account the displacement bound extension.
	TqFloat db=0;
	CqString strCoordinateSystem("object");
	const TqFloat* pattrDispclacementBound=pSurface->pAttributes()->GetFloatAttribute("displacementbound", "sphere");
	const CqString* pattrCoordinateSystem=pSurface->pAttributes()->GetStringAttribute("displacementbound", "coordinatesystem");
	if(pattrDispclacementBound!=0)	db=pattrDispclacementBound[0];
	if(pattrCoordinateSystem!=0)	strCoordinateSystem=pattrCoordinateSystem[0];

	CqVector3D	vecDB(db,0,0);
	vecDB=pCurrentRenderer()->matVSpaceToSpace(strCoordinateSystem.c_str(),"camera",pSurface->pAttributes()->pshadSurface()->matCurrent(),pSurface->pTransform()->matObjectToWorld())*vecDB;
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
	assert(iBucket>=m_CurrBucket);
	m_aSurfaces[iBucket].LinkFirst(pSurface);
}


//----------------------------------------------------------------------
/** Add a new shaded grid to the front of the list of waiting ones.
 * \param pGrid A pointer to a CqMicroPolyGridBase derived class.
 */

void CqImageBuffer::AddGrid(CqMicroPolyGridBase* pGrid)
{
	// Bound the primitive in its current space (camera) space taking into account any motion specification.
	CqBound Bound(pGrid->Bound());
	pGrid->pSurface()->ExpandBoundForMotion(Bound);

	Bound.vecMin().x(Bound.vecMin().x()-m_FilterXWidth/2);
	Bound.vecMin().y(Bound.vecMin().y()-m_FilterYWidth/2);
	Bound.vecMax().x(Bound.vecMax().x()+m_FilterXWidth/2);
	Bound.vecMax().y(Bound.vecMax().y()+m_FilterYWidth/2);

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
	assert(iBucket>=m_CurrBucket);
	m_aagridWaiting[iBucket].push_back(pGrid);
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
	// Check if bucket has already been rendered, error situation.
	assert((iYBa*m_cXBuckets)+iXBa>=m_CurrBucket);

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
					m_aampgWaiting[(iYB*m_cXBuckets)+iXB].push_back(pmpgNew);
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
	if(!m_aagridWaiting[iBucket].empty())
	{
		for(std::vector<CqMicroPolyGridBase*>::iterator i=m_aagridWaiting[iBucket].begin(); i!=m_aagridWaiting[iBucket].end(); i++)
			(*i)->Split(this,iBucket,xmin,xmax,ymin,ymax);
	}
	m_aagridWaiting[iBucket].clear();

	// Render any waiting MPGs
	static	CqColor	colWhite(1,1,1);
	static	CqVector2D	vecP;
	
	if(m_aampgWaiting[iBucket].empty())	return;

	TqFloat farplane=pCurrentRenderer()->optCurrent().fClippingPlaneFar();
	TqFloat nearplane=pCurrentRenderer()->optCurrent().fClippingPlaneNear();
	
	register long iY=ymin;

	TqInt i;
	for(i=m_aampgWaiting[iBucket].size()-1; i>=0; i--)
	{
		CqMicroPolygonBase* pMPG=m_aampgWaiting[iBucket][i];
		RenderMicroPoly(pMPG,iBucket,xmin,xmax,ymin,ymax);
		pMPG->Release();
	}

	m_aampgWaiting[iBucket].clear();
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

	// Bound the microplygon in hybrid camera/raster space
	CqBound Bound(pMPG->Bound());

	TqFloat bminx=Bound.vecMin().x();
	TqFloat bmaxx=Bound.vecMax().x();
	TqFloat bminy=Bound.vecMin().y();
	TqFloat bmaxy=Bound.vecMax().y();

	if(bmaxx<xmin || bmaxy<ymin || bminx>xmax || bminy>ymax)
		return;

	// If the micropolygon is outside the hither-yon range, cull it.
	if(Bound.vecMin().z()>pCurrentRenderer()->optCurrent().fClippingPlaneFar() ||
	   Bound.vecMax().z()<pCurrentRenderer()->optCurrent().fClippingPlaneNear())
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

	TqInt iXSamples=pCurrentRenderer()->optCurrent().fPixelXSamples();
	TqInt iYSamples=pCurrentRenderer()->optCurrent().fPixelYSamples();
	
	TqInt im=(bminx<initX)?0:floor((bminx-initX)*iXSamples);
	TqInt in=(bminy<initY)?0:floor((bminy-initY)*iYSamples);
	TqInt em=(bmaxx>eX)?iXSamples:ceil((bmaxx-eX)*iXSamples);
	TqInt en=(bmaxy>eY)?iYSamples:ceil((bmaxy-eY)*iYSamples);

	register long iY=initY;

	while(iY<=eY)
	{
		register long iX=initX;

		bool valid=Pixel(iX,iY,iBucket,pie);

		while(iX<=eX)
		{
			register int m,n;
			n=(iY==initY)?in:0;
			int end_n=(iY==eY)?en:iYSamples;
			int start_m=(iX==initX)?im:0;
			int end_m=(iX==eX)?em:iXSamples;
			for(; n<end_n; n++)
			{
				for(m=start_m; m<end_m; m++)
				{
					pCurrentRenderer()->Stats().cSamples()++;
					CqVector2D vecP(pie->SamplePoint(m,n));

					if(Bound.Contains2D(vecP))
					{
						pCurrentRenderer()->Stats().cSampleBoundHits()++;

						TqFloat t=pie->SampleTime(m,n);
						if(pMPG->Sample(vecP,t,ImageVal.m_Depth))
						{									
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

	// Render any waiting subsurfaces.
	CqBasicSurface* pSurface=m_aSurfaces[iBucket].pFirst();
	while(pSurface!=0)
	{
		if(m_fQuit)	return;

		if(pSurface->Diceable())
		{
			CqMicroPolyGridBase* pGrid;
			if((pGrid=pSurface->Dice())!=0)
			{
				//pGrid->Split(this,iBucket,xmin,xmax,ymin,ymax);
				pGrid->Project();
				AddGrid(pGrid);
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
		pSurface=m_aSurfaces[iBucket].pFirst();
		// Render any waiting micro polygon grids.
		RenderMPGs(iBucket, xmin, xmax, ymin, ymax);
	}


	// Render the first top level scene surface.
	TqInt iSurface;
	for(iSurface=0; iSurface<m_aScene[iBucket].size(); iSurface++)
	{
		// The surfaces in the scene store are in world coordinates, so take a copy and convert it.
		CqBasicSurface* pTopSurface;
		pTopSurface=m_aScene[iBucket][iSurface];

		if(pTopSurface->Diceable())
		{
			CqMicroPolyGridBase* pGrid;
			if((pGrid=pTopSurface->Dice())!=0)
			{
				//pGrid->Split(this,iBucket,xmin,xmax,ymin,ymax);
				pGrid->Project();
				AddGrid(pGrid);
			}
		}
		else if(!pTopSurface->fDiscard())
		{
			std::vector<CqBasicSurface*> aSplits;
			TqInt cSplits=pTopSurface->Split(aSplits);
			TqInt i;
			for(i=0; i<cSplits; i++)
				PostSurface(aSplits[i]);
		}

		
		delete(pTopSurface);

		// Now render any split surfaces generated.
		CqBasicSurface* pSurface=m_aSurfaces[iBucket].pFirst();
		while(pSurface!=0)
		{
			if(m_fQuit)	return;

			if(pSurface->Diceable())
			{
				CqMicroPolyGridBase* pGrid;
				if((pGrid=pSurface->Dice())!=0)
				{
					//pGrid->Split(this,iBucket,xmin,xmax,ymin,ymax);
					pGrid->Project();
					AddGrid(pGrid);
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
			pSurface=m_aSurfaces[iBucket].pFirst();
			// Render any waiting micro polygon grids.
			RenderMPGs(iBucket, xmin, xmax, ymin, ymax);
		}
		// Render any waiting micro polygon grids.
		RenderMPGs(iBucket, xmin, xmax, ymin, ymax);
	}
	
	// Now combine the colors at each pixel sample for any micropolygons rendered to that pixel.
	if(m_fQuit)	return;
	if(pCurrentRenderer()->optCurrent().iDisplayMode()&ModeRGB)
	{
		CqImageElement* ie=m_pieImage;
		TqInt i;
		for(i=0; i<(m_XBucketSize+m_FilterXWidth)*(m_YBucketSize+m_FilterYWidth); i++)
		{
			ie->Combine();
			ie++;
		}
	}
	BucketComplete(iBucket);
	// Clear the MPG waiting array
	m_aampgWaiting[iBucket].clear();
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
		// Set up some bounds for the bucket.
		CqVector2D vecMin=Position(iBucket);
		CqVector2D vecMax=Size(iBucket)+vecMin;
		vecMin-=CqVector2D(m_FilterXWidth/2,m_FilterYWidth/2);
		vecMax+=CqVector2D(m_FilterXWidth/2,m_FilterYWidth/2);

		long xmin=vecMin.x();
		long ymin=vecMin.y();
		long xmax=vecMax.x();
		long ymax=vecMax.y();

		if(xmin<CropWindowXMin()-m_FilterXWidth/2)	xmin=CropWindowXMin()-m_FilterXWidth/2;
		if(ymin<CropWindowYMin()-m_FilterYWidth/2)	ymin=CropWindowYMin()-m_FilterYWidth/2;
		if(xmax>CropWindowXMax()+m_FilterXWidth/2)	xmax=CropWindowXMax()+m_FilterXWidth/2;
		if(ymax>CropWindowYMax()+m_FilterYWidth/2)	ymax=CropWindowYMax()+m_FilterYWidth/2;

		// Inform the status class how far we have got, and update UI.
		m_CurrBucket=iBucket;
		float Complete=(m_cXBuckets*m_cYBuckets);
		Complete/=iBucket;
		Complete=100.0f/Complete;
		pCurrentRenderer()->Stats().SetComplete(Complete);

		RenderSurfaces(iBucket,xmin,xmax,ymin,ymax);
		if(m_fQuit)
		{
			m_fDone=TqTrue;
			pCurrentRenderer()->SignalDisplayDriverFinished(-1);
			return;
		}

		// Rejitter the sample locations to avoid bucket level interference patterns.
		InitialiseSamples(iBucket+1);
		ClearBucket();
	}
	ImageComplete();
	pCurrentRenderer()->SignalDisplayDriverFinished(0);
	m_fDone=TqTrue;
}


//----------------------------------------------------------------------
/** Stop rendering.
 */

void CqImageBuffer::Quit()
{
	m_fQuit=TqTrue;
}


//----------------------------------------------------------------------
/** Initialise the main surface list from the entities stored in a CqScene object.
 * \param Scene CqScene class containing all the GPrims to render.
 */

void CqImageBuffer::InitialiseSurfaces(CqScene& Scene)
{
	// Clear the surfaces first.
	TqInt i;
	for(i=0; i<m_cXBuckets*m_cYBuckets; i++)
		m_aScene[i].clear();
	// Add every surface to the image buffer, allocating them to the correct bucket
	CqBasicSurface* pSurface=Scene.lSurfaces().pFirst();
	while(pSurface)
	{
		AddSurfacePointer(pSurface);
		pSurface->Reset();
		pSurface=pSurface->pNext();
	}
}



//---------------------------------------------------------------------
 
END_NAMESPACE(Aqsis)