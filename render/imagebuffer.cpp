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
		m_aSubCellIndex.resize(m_XSamples*m_YSamples);
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
				m_avecSamples[i*m + j].x(i);
				m_avecSamples[i*m + j].y(j);
			}
		}

		// Shuffle y coordinates within each row of cells.
		for (i = 0; i < n; i++) 
		{
			for (j = 0; j < m; j++) 
			{
				TqFloat t;
				TqInt k;
				
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
				TqFloat t;
				TqInt k;

				k = random.RandomInt(n - 1 - j) + j;
				t = m_avecSamples[j*m + i].x();
				m_avecSamples[j*m + i].x(m_avecSamples[k*m + i].x());
				m_avecSamples[k*m + i].x(t);

			}
		}

		
		TqFloat subpixelheight=1.0f/m_YSamples;
		TqFloat subpixelwidth=1.0f/m_XSamples;

		// finally add in the pixel offset
		for (i = 0; i < n; i++) 
		{
			TqFloat sy=i*subpixelheight;
			for (j = 0; j < m; j++)
			{ 
				TqFloat sx=j*subpixelwidth;
				TqFloat xindex=m_avecSamples[i*m + j].x();
				TqFloat yindex=m_avecSamples[i*m + j].y();
				m_avecSamples[i*m + j].x(xindex*subcell_width + (subcell_width*0.5f) + sx);
				m_avecSamples[i*m + j].y(yindex*subcell_width + (subcell_width*0.5f) + sy);
				m_avecSamples[i*m + j]+=vecPixel;
				m_aSubCellIndex[i*m+j]=(yindex*m_YSamples)+xindex;
			}
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
			}
		}
	}
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
TqInt	CqBucket::m_XPixelSamples;
TqInt	CqBucket::m_YPixelSamples;
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

	TqInt fxo2=ceil((m_XFWidth-1)*0.5f);
	TqInt fyo2=ceil((m_YFWidth-1)*0.5f);
	
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
	m_XPixelSamples=xsamples;
	m_YPixelSamples=ysamples;
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

	TqFloat xmax=ceil((m_XFWidth-1)*0.5f);
	TqFloat ymax=ceil((m_YFWidth-1)*0.5f);
	TqFloat xfwo2=m_XFWidth*0.5f;
	TqFloat yfwo2=m_YFWidth*0.5f;
	TqFloat xfw=m_XFWidth;
	TqFloat yfw=m_YFWidth;
	TqInt numsubpixels=(m_XPixelSamples*m_YPixelSamples);
	TqInt numsubcells=numsubpixels;
	TqFloat subcellwidth=1.0f/numsubcells;
	TqFloat subcellcentre=subcellwidth*0.5f;
	TqInt numperpixel=numsubpixels*numsubcells;
	TqInt numvalues=((xfw+1)*(yfw+1))*(numperpixel);
	
	m_aFilterValues.resize(numvalues);

	// Go over every pixel touched by the filter
	TqInt px,py;
	for(py=-ymax; py<=ymax; py++)
	{
		for(px=-xmax; px<=xmax; px++)
		{
			// Get the index of the pixel in the array.
			TqInt index=(((py+ymax)*xfw)+(px+xmax))*numperpixel;
			TqFloat pfx=px-0.5f;
			TqFloat pfy=py-0.5f;
			// Go over every subpixel in the pixel.
			TqInt sx,sy;
			for(sy=0; sy<m_YPixelSamples; sy++)
			{
				for(sx=0; sx<m_XPixelSamples; sx++)
				{
					// Get the index of the subpixel in the array
					TqInt sindex=index+(((sy*m_XPixelSamples)+sx)*numsubcells);
					TqFloat sfx=static_cast<TqFloat>(sx)/m_XPixelSamples;
					TqFloat sfy=static_cast<TqFloat>(sy)/m_YPixelSamples;
					// Go over each subcell in the subpixel
					TqInt cx,cy;
					for(cy=0; cy<m_XPixelSamples; cy++)
					{
						for(cx=0; cx<m_YPixelSamples; cx++)
						{
							// Get the index of the subpixel in the array
							TqInt cindex=sindex+((cy*m_YPixelSamples)+cx);
							TqFloat fx=(cx*subcellwidth)+sfx+pfx+subcellcentre;
							TqFloat fy=(cy*subcellwidth)+sfy+pfy+subcellcentre;
							TqFloat w=0.0f;
							if(fx>=-xfwo2 && fy>=-yfwo2 && fx<=xfwo2 && fy<=yfwo2)
								w=(*pFilter)(fx,fy,m_XFWidth,m_YFWidth);
							m_aFilterValues[cindex]=w;
						}
					}
				}
			}
		}
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
	CqImageElement* pie;

	CqColor* pCols=new CqColor[XSize()*YSize()];
	TqInt xmax=ceil((XFWidth()-1)*0.5f);
	TqInt ymax=ceil((YFWidth()-1)*0.5f);
	TqFloat xfwo2=XFWidth()*0.5f;
	TqFloat yfwo2=YFWidth()*0.5f;
	TqInt numsubpixels=(m_XPixelSamples*m_YPixelSamples);
	TqInt numsubcells=numsubpixels;
	TqFloat subcellwidth=1.0f/numsubcells;
	TqInt numperpixel=numsubpixels*numsubcells;
	TqInt	xlen=XSize()+XFWidth();

	TqInt x,y;
	TqInt i=0;
	for(y=YOrigin(); y<YOrigin()+YSize(); y++)
	{
		TqFloat ycent=y+0.5f;
		for(x=XOrigin(); x<XOrigin()+XSize(); x++)
		{
			TqFloat xcent=x+0.5f;
			CqColor c(0,0,0);
			TqFloat gTot=0.0;
			
			TqInt fx,fy;
			// Get the element at the upper left corner of the filter area.
			ImageElement(x-xmax, y-ymax, pie);
			for(fy=-ymax; fy<=ymax; fy++)
			{
				CqImageElement* pie2=pie;
				for(fx=-xmax; fx<=xmax; fx++)
				{
					TqInt index=(((fy+ymax)*XFWidth())+(fx+xmax))*numperpixel;
					// Now go over each subsample within the pixel
					TqInt sx,sy;
					for(sy=0; sy<m_YPixelSamples; sy++)
					{
						for(sx=0; sx<m_XPixelSamples; sx++)
						{
							TqInt sindex=index+(((sy*m_XPixelSamples)+sx)*numsubcells);
							CqVector2D vecS=pie2->SamplePoint(sx,sy);
							vecS-=CqVector2D(xcent,ycent);
							if(vecS.x()>=-xfwo2 && vecS.y()>=-yfwo2 && vecS.x()<=xfwo2 && vecS.y()<=yfwo2)
							{
								TqInt cindex=sindex+pie2->SubCellIndex(sx,sy);
								TqFloat g=m_aFilterValues[cindex];
								gTot+=g;
								if(pie2->Values(sx,sy).size()>0)
									c+=pie2->Values(sx,sy)[0].m_colColor*g;
							}	
						}
					}
					pie2++;
				}
				pie+=xlen;
				fy++;
			}
			pCols[i++]=c/gTot;
		}
	}
	
	i=0;
	ImageElement(XOrigin(),YOrigin(),pie);
	for(y=0; y<YSize(); y++)
	{
		CqImageElement* pie2=pie;
		for(x=0; x<XSize(); x++)
		{
			pie2->Color()=pCols[i++];
			pie2++;
		}
		pie+=xlen;
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
		CqImageElement* pie;
		ImageElement(XOrigin(), YOrigin(), pie);
		TqInt x,y;
		for(y=0; y<YSize(); y++)
		{
			CqImageElement* pie2=pie;
			for(x=0; x<XSize(); x++)
			{
				// color=(color*gain)^1/gamma
				if(QGetRenderContext()->optCurrent().fExposureGain()!=1.0)
					pie2->Color()*=QGetRenderContext()->optCurrent().fExposureGain();

				if(QGetRenderContext()->optCurrent().fExposureGamma()!=1.0)
				{
					TqFloat oneovergamma=1.0f/QGetRenderContext()->optCurrent().fExposureGamma();
					pie2->Color().SetfRed  (pow(pie2->Color().fRed  (),oneovergamma));
					pie2->Color().SetfGreen(pow(pie2->Color().fGreen(),oneovergamma));
					pie2->Color().SetfBlue (pow(pie2->Color().fBlue (),oneovergamma));
				}
				pie2++;
			}
			pie+=XSize()+XFWidth();
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
		CqImageElement* pie;
		ImageElement(XOrigin(),YOrigin(),pie);
		TqInt x,y;
		for(y=0; y<YSize(); y++)
		{
			CqImageElement* pie2=pie;
			for(x=0; x<XSize(); x++)
			{
				double r,g,b,a;
				double s=random.RandomFloat();
				if(modf(one*pie2->Color().fRed  ()+ditheramplitude*s,&r)>0.5)	r+=1;
				if(modf(one*pie2->Color().fGreen()+ditheramplitude*s,&g)>0.5)	g+=1;
				if(modf(one*pie2->Color().fBlue ()+ditheramplitude*s,&b)>0.5)	b+=1;
				if(modf(one*pie2->Coverage()	  +ditheramplitude*s,&a)>0.5)	a+=1;
				r=CLAMP(r,min,max);
				g=CLAMP(g,min,max);
				b=CLAMP(b,min,max);
				a=CLAMP(a,min,max);
				pie2->Color().SetfRed  (r);
				pie2->Color().SetfGreen(g);
				pie2->Color().SetfBlue (b);
				pie2->SetCoverage(a);
				
				pie2++;
			}
			pie+=XSize()+XFWidth();
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
		CqImageElement* pie;
		ImageElement(XOrigin(), YOrigin(), pie);
		TqInt x,y;
		for(y=0; y<YSize(); y++)
		{
			CqImageElement* pie2=pie;
			for(x=0; x<XSize(); x++)
			{
				double d;
				if(modf(one*pie2->Depth()+ditheramplitude*random.RandomFloat(),&d)>0.5)	d+=1;
				d=CLAMP(d,min,max);
				pie2->SetDepth(d);
				pie2++;
			}
			pie+=XSize()+XFWidth();
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
 * \param iBucket Integer bucket index (0-based).
 * \return Bucket position as 2d vector (xpos, ypos).
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

    Usually the return value is just (XBucketSize, YBucketSize) except
    for the buckets on the right and bottom side of the image where the
    size can be smaller. The crop window is not taken into account.

 * \param iBucket Integer bucket index.
 * \return Bucket size as 2d vector (xsize, ysize).
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
/** Check if a surface can be culled and transform bound.

    This method checks if the surface lies outside the viewing volume
    and returns true if it does.
    Additionally it checks if the surface spans the eye plane and marks it
    as undiceable if it does (it might even be marked as discarded).
    It also grows the bound by half the filter width and transforms it
    into raster space.

 * \param Bound CqBound containing the geometric bound in camera space.
 * \param pSurface Pointer to the CqBasicSurface derived class being processed.
 * \return Boolean indicating that the GPrim can be culled.

  \bug If the gprim spans the eye plane the bound is not transformed into raster
   space (how could it anyway), but PostSurface() relies on this behaviour and
   inserts EVERY gprim into buckets (using a bound that is still in camera space).
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

	// Convert the bounds to raster space.
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
  // Count the number of total gprims
  QGetRenderContext()->Stats().IncTotalGPrims();

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
		QGetRenderContext()->Stats().IncCulledGPrims();
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
/** Render any waiting MPGs.

    All micro polygon grids in the specified bucket are bust into
    individual micro polygons which are assigned to their appropriate
    bucket. Then RenderMicroPoly() is called for each micro polygon in
    the current bucket.

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
/** Render a particular micropolygon.

 * \param pMPG Pointer to the micropolygon to process.
 * \param iBucket Integer index of bucket being processed.
 * \param xmin Integer minimum extend of the image part being rendered, takes into account buckets and clipping.
 * \param xmax Integer maximum extend of the image part being rendered, takes into account buckets and clipping.
 * \param ymin Integer minimum extend of the image part being rendered, takes into account buckets and clipping.
 * \param ymax Integer maximum extend of the image part being rendered, takes into account buckets and clipping.

   \see CqBucket, CqImageElement
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
	{
	  QGetRenderContext()->Stats().IncCulledMPGs();
		return;
	}

	// If the micropolygon is outside the hither-yon range, cull it.
	if(Bound.vecMin().z()>QGetRenderContext()->optCurrent().fClippingPlaneFar() ||
	   Bound.vecMax().z()<QGetRenderContext()->optCurrent().fClippingPlaneNear())
	{
	  QGetRenderContext()->Stats().IncCulledMPGs();
		return;
	}

	// Now go across all pixels touched by the micropolygon bound.
	// The first pixel position is at (initX, initY), the last one
	// at (eX, eY).
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
			// Now sample the micropolygon at several subsample positions
			// within the pixel. The subsample indices range from (start_m, n)
			// to (end_m-1, end_n-1).
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
					QGetRenderContext()->Stats().IncSamples();
					// First, check if the subsample point lies within the micropoly bound
					if(Bound.Contains2D(vecP))
					{
						QGetRenderContext()->Stats().IncSampleBoundHits();

						TqFloat t=pie->SampleTime(m,n);
						// Now check if the subsample hits the micropoly
						if(pMPG->Sample(vecP,t,ImageVal.m_Depth))
						{									
							QGetRenderContext()->Stats().IncSampleHits();
							// Sort the color/opacity into the visible point list
							std::vector<SqImageValue>& aValues=pie->Values(m,n);
							int i=0;
							int c=aValues.size();
							if(c>0 && aValues[0].m_Depth<ImageVal.m_Depth)
							{
								SqImageValue* p=&aValues[0];
								while(i<c && p[i].m_Depth<ImageVal.m_Depth)	i++;
								// If it is exactly the same, chances are we've hit a MPG grid line.
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

    This method loops through all the gprims stored in the specified bucket
    and checks if the gprim can be diced and turned into a grid of micro
    polygons or if it is still too large and has to be split (this check 
    is done in CqBasicSurface::Diceable()).

    The dicing is done by the gprim in CqBasicSurface::Dice(). After that
    the entire grid is shaded by calling CqMicroPolyGridBase::Shade().
    The shaded grid is then stored in the current bucket and will eventually 
    be further processed by RenderMPGs().

    If the gprim could not yet be diced, it is split into a number of
    smaller gprims (CqBasicSurface::Split()) which are again assigned to
    buckets (this doesn't necessarily have to be the current one again)
    by calling PostSurface() (just as if it were regular gprims).

    Finally, when all the gprims are diced and the resulting micro polygons
    are rendered, the individual subpixel samples are combined into one
    pixel color and opacity which is then exposed and quantized.
    After that the method BucketComplete() and IqDDManager::DisplayBucket()
    is called which can be used to display the bucket inside a window or
    save it to disk.

 * \param iBucket Integer index of bucket being processed (0-based).
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

    // Dice & shade the surface if it's small enough...
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
		// The surface is not small enough, so split it...
   	else if(!pSurface->fDiscard())
		{
			std::vector<CqBasicSurface*> aSplits;
			// Decrease the total gprim count since this gprim is replaced by other gprims
			QGetRenderContext()->Stats().DecTotalGPrims();
			// Split it
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

    Starting from the upper left corner of the image every bucket is
    processed by computing its extent and calling RenderSurfaces().
    After the image is complete ImageComplete() is called.
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


