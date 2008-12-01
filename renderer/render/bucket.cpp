// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
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
		\brief Implements the CqBucket class responsible for bookkeeping the primitives and storing the results.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/


#include	"aqsis.h"

#ifdef WIN32
#include    <windows.h>
#endif
#include	<cstring>
#include	<algorithm>
#include	<valarray>

#include	"aqsismath.h"
#include	"surface.h"
#include	"imagepixel.h"
#include	"occlusion.h"
#include	"renderer.h"
#include	"micropolygon.h"
#include	"imagebuffer.h"

#include	"bucket.h"
#include	"stats.h"

#include	<algorithm>
#include	<valarray>
#include	<cmath>


namespace Aqsis {


//----------------------------------------------------------------------
CqBucket::CqBucket() : m_bProcessed(false), m_bucketData(0)
{
}

//----------------------------------------------------------------------
/** Add an MP to the list of deferred MPs.
 */
void CqBucket::AddMP( boost::shared_ptr<CqMicroPolygon>& pMP )
{
	m_micropolygons.push_back( pMP );
}


//----------------------------------------------------------------------
TqInt CqBucket::getCol() const
{
	return m_col;
}

//----------------------------------------------------------------------
void CqBucket::setCol(TqInt value)
{
	m_col = value;
}

//----------------------------------------------------------------------
TqInt CqBucket::getRow() const
{
	return m_row;
}

//----------------------------------------------------------------------
void CqBucket::setRow(TqInt value)
{
	m_row = value;
}

//----------------------------------------------------------------------
/** Mark this bucket as processed
 */
void CqBucket::SetProcessed( bool bProc )
{
	assert( !bProc || (bProc && IsEmpty()) );
	m_bProcessed = bProc;
}


//----------------------------------------------------------------------
/** Initialise the static image storage area.
 *  Clear,Allocate, Init. the m_bucketData->m_aieImage samples
 */

void CqBucket::PrepareBucket( const CqVector2D& bucketPos, const CqVector2D& bucketSize,
			      TqInt pixelXSamples, TqInt pixelYSamples, TqFloat filterXWidth, TqFloat filterYWidth,
			      TqInt viewRangeXMin, TqInt viewRangeXMax, TqInt viewRangeYMin, TqInt viewRangeYMax,
			      TqFloat clippingNear, TqFloat clippingFar,
			      bool fJitter)
{
	m_bucketData->m_XOrigin = static_cast<TqInt>( bucketPos.x() );
	m_bucketData->m_YOrigin = static_cast<TqInt>( bucketPos.y() );
	m_bucketData->m_XSize = static_cast<TqInt>( bucketSize.x() );
	m_bucketData->m_YSize = static_cast<TqInt>( bucketSize.y() );
	m_bucketData->m_PixelXSamples = pixelXSamples;
	m_bucketData->m_PixelYSamples = pixelYSamples;
	m_bucketData->m_FilterXWidth = filterXWidth;
	m_bucketData->m_FilterYWidth = filterYWidth;
	m_bucketData->m_DiscreteShiftX = lfloor(m_bucketData->m_FilterXWidth/2.0f);
	m_bucketData->m_DiscreteShiftY = lfloor(m_bucketData->m_FilterYWidth/2.0f);
	m_bucketData->m_RealWidth = m_bucketData->m_XSize + (m_bucketData->m_DiscreteShiftX*2);
	m_bucketData->m_RealHeight = m_bucketData->m_YSize + (m_bucketData->m_DiscreteShiftY*2);
	m_bucketData->m_realXOrigin = lfloor(static_cast<TqFloat>(m_bucketData->m_XOrigin) - m_bucketData->m_DiscreteShiftX);
	m_bucketData->m_realYOrigin = lfloor(static_cast<TqFloat>(m_bucketData->m_YOrigin) - m_bucketData->m_DiscreteShiftY);

	m_bucketData->m_viewRangeXMin = viewRangeXMin;
	m_bucketData->m_viewRangeXMax = viewRangeXMax;
	m_bucketData->m_viewRangeYMin = viewRangeYMin;
	m_bucketData->m_viewRangeYMax = viewRangeYMax;
	m_bucketData->m_clippingNear = clippingNear;
	m_bucketData->m_clippingFar = clippingFar;

        TqFloat opentime = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Shutter" ) [ 0 ];
        TqFloat closetime = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Shutter" ) [ 1 ];

	// Allocate the image element storage if this is the first bucket
	if(m_bucketData->m_aieImage.empty())
	{
		SqImageSample::SetSampleSize(QGetRenderContext() ->GetOutputDataTotalSize());

		m_bucketData->m_aieImage.resize( m_bucketData->m_RealWidth * m_bucketData->m_RealHeight );
		m_bucketData->m_aSamplePositions.resize( m_bucketData->m_RealWidth * m_bucketData->m_RealHeight );
		m_bucketData->m_SamplePoints.resize( m_bucketData->m_RealWidth * m_bucketData->m_RealHeight * m_bucketData->m_PixelXSamples * m_bucketData->m_PixelYSamples );
		m_bucketData->m_NextSamplePoint = 0;

		CalculateDofBounds();

		// Initialise the samples for this bucket.
		TqInt which = 0;
		for ( TqInt i = 0; i < m_bucketData->m_RealHeight; i++ )
		{
			for ( TqInt j = 0; j < m_bucketData->m_RealWidth; j++ )
			{
				m_bucketData->m_aieImage[which].Clear( m_bucketData->m_SamplePoints );
				m_bucketData->m_aieImage[which].AllocateSamples( this,
										 m_bucketData->m_PixelXSamples,
										 m_bucketData->m_PixelYSamples );
				m_bucketData->m_aieImage[which].InitialiseSamples( m_bucketData->m_SamplePoints,
										   m_bucketData->m_aSamplePositions[which] );
				//if(fJitter)
				m_bucketData->m_aieImage[which].JitterSamples( m_bucketData->m_SamplePoints,
									       m_bucketData->m_aSamplePositions[which],
									       opentime, closetime);

				which++;
			}
		}
	}

	// Shuffle the Sample and DOD positions 
	std::vector<CqImagePixel>::iterator itPix;
	TqUint size = m_bucketData->m_aieImage.size();  
	TqUint i = 0;
	if (size > 1)
	{
		CqRandom rand(19);
		for( itPix = m_bucketData->m_aieImage.begin(), i=0 ; itPix <= m_bucketData->m_aieImage.end(), i < size - 1; itPix++, i++)
		{
			TqUint other = i + rand.RandomInt(size - i);
			if (other >= size) other = size - 1;
			(*itPix).m_SampleIndices.swap(m_bucketData->m_aieImage[other].m_SampleIndices);  
			(*itPix).m_DofOffsetIndices.swap(m_bucketData->m_aieImage[other].m_DofOffsetIndices); 
		}
	}

	// Jitter the samplepoints and adjust them for the new bucket position.
	TqInt which = 0;
	for ( TqInt ii = 0; ii < m_bucketData->m_RealHeight; ii++ )
	{
		for ( TqInt j = 0; j < m_bucketData->m_RealWidth; j++ )
		{
			CqVector2D bPos2( m_bucketData->m_XOrigin, m_bucketData->m_YOrigin );
			bPos2 += CqVector2D( ( j - m_bucketData->m_DiscreteShiftX ), ( ii - m_bucketData->m_DiscreteShiftY ) );

			m_bucketData->m_aieImage[which].Clear( m_bucketData->m_SamplePoints );

			//if(fJitter)
			m_bucketData->m_aieImage[which].JitterSamples( m_bucketData->m_SamplePoints,
								       m_bucketData->m_aSamplePositions[which],
								       opentime, closetime);
			m_bucketData->m_aieImage[which].OffsetSamples( m_bucketData->m_SamplePoints,
								       bPos2,
								       m_bucketData->m_aSamplePositions[which] );

			which++;
		}
	}

	InitialiseFilterValues();
}


void CqBucket::CalculateDofBounds()
{
	m_bucketData->m_NumDofBounds = m_bucketData->m_PixelXSamples * m_bucketData->m_PixelYSamples;
	m_bucketData->m_DofBounds.resize(m_bucketData->m_NumDofBounds);

	TqFloat dx = 2.0 / m_bucketData->m_PixelXSamples;
	TqFloat dy = 2.0 / m_bucketData->m_PixelYSamples;

	// I know this is far from an optimal way of calculating this,
	// but it's only done once so I don't care.
	// Calculate the bounding boxes that the dof offset positions fall into.
	TqFloat minX = -1.0;
	TqFloat minY = -1.0;
	TqInt which = 0;
	for(int j = 0; j < m_bucketData->m_PixelYSamples; ++j)
	{
		for(int i = 0; i < m_bucketData->m_PixelXSamples; ++i)
		{
			CqVector2D topLeft(minX, minY);
			CqVector2D topRight(minX + dx, minY);
			CqVector2D bottomLeft(minX, minY + dy);
			CqVector2D bottomRight(minX + dx, minY + dy);

			CqImagePixel::ProjectToCircle(topLeft);
			CqImagePixel::ProjectToCircle(topRight);
			CqImagePixel::ProjectToCircle(bottomLeft);
			CqImagePixel::ProjectToCircle(bottomRight);

			// if the bound straddles x=0 or y=0 then just using the corners
			// will give too small a bound, so we enlarge it by including the
			// non-projected coords.
			if((topLeft.y() > 0.0 && bottomLeft.y() < 0.0) ||
			        (topLeft.y() < 0.0 && bottomLeft.y() > 0.0))
			{
				topLeft.x(minX);
				bottomLeft.x(minX);
				topRight.x(minX + dx);
				bottomRight.x(minX + dx);
			}
			if((topLeft.x() > 0.0 && topRight.x() < 0.0) ||
			        (topLeft.x() < 0.0 && topRight.x() > 0.0))
			{
				topLeft.y(minY);
				bottomLeft.y(minY + dy);
				topRight.y(minY);
				bottomRight.y(minY + dy);
			}

			m_bucketData->m_DofBounds[which].vecMin() = topLeft;
			m_bucketData->m_DofBounds[which].vecMax() = topLeft;
			m_bucketData->m_DofBounds[which].Encapsulate(topRight);
			m_bucketData->m_DofBounds[which].Encapsulate(bottomLeft);
			m_bucketData->m_DofBounds[which].Encapsulate(bottomRight);

			which++;
			minX += dx;
		}
		minX = -1.0;
		minY += dy;
	}
}

//----------------------------------------------------------------------
/** Initialise the static filter values.
 */

void CqBucket::InitialiseFilterValues()
{
	if( !m_bucketData->m_aFilterValues.empty() )
		return;

	// Allocate and fill in the filter values array for each pixel.
	TqInt numsubpixels = ( PixelXSamples() * PixelYSamples() );
	TqInt numperpixel = numsubpixels * numsubpixels;

	TqUint numvalues = static_cast<TqUint>( ( (lceil(FilterXWidth()) + 1) * (lceil(FilterYWidth()) + 1) ) * numperpixel );

	m_bucketData->m_aFilterValues.resize( numvalues );

	RtFilterFunc pFilter;
	pFilter = QGetRenderContext() ->poptCurrent()->funcFilter();

	// Sanity check
	if( NULL == pFilter )
		pFilter = RiBoxFilter;

	TqFloat xmax = m_bucketData->m_DiscreteShiftX;
	TqFloat ymax = m_bucketData->m_DiscreteShiftY;
	TqFloat xfwo2 = std::ceil(FilterXWidth()) * 0.5f;
	TqFloat yfwo2 = std::ceil(FilterYWidth()) * 0.5f;
	TqFloat xfw = std::ceil(FilterXWidth());

	TqFloat subcellwidth = 1.0f / numsubpixels;
	TqFloat subcellcentre = subcellwidth * 0.5f;

	// Go over every pixel touched by the filter
	TqInt px, py;
	for ( py = static_cast<TqInt>( -ymax ); py <= static_cast<TqInt>( ymax ); py++ )
	{
		for( px = static_cast<TqInt>( -xmax ); px <= static_cast<TqInt>( xmax ); px++ )
		{
			// Get the index of the pixel in the array.
			TqInt index = static_cast<TqInt>
			              ( ( ( ( py + ymax ) * xfw ) + ( px + xmax ) ) * numperpixel );
			TqFloat pfx = px - 0.5f;
			TqFloat pfy = py - 0.5f;
			// Go over every subpixel in the pixel.
			TqInt sx, sy;
			for ( sy = 0; sy < PixelYSamples(); sy++ )
			{
				for ( sx = 0; sx < PixelXSamples(); sx++ )
				{
					// Get the index of the subpixel in the array
					TqInt sindex = index + ( ( ( sy * PixelXSamples() ) + sx ) * numsubpixels );
					TqFloat sfx = static_cast<TqFloat>( sx ) / PixelXSamples();
					TqFloat sfy = static_cast<TqFloat>( sy ) / PixelYSamples();
					// Go over each subcell in the subpixel
					TqInt cx, cy;
					for ( cy = 0; cy < PixelXSamples(); cy++ )
					{
						for ( cx = 0; cx < PixelYSamples(); cx++ )
						{
							// Get the index of the subpixel in the array
							TqInt cindex = sindex + ( ( cy * PixelYSamples() ) + cx );
							TqFloat fx = ( cx * subcellwidth ) + sfx + pfx + subcellcentre;
							TqFloat fy = ( cy * subcellwidth ) + sfy + pfy + subcellcentre;
							TqFloat w = 0.0f;
							if ( fx >= -xfwo2 && fy >= -yfwo2 && fx <= xfwo2 && fy <= yfwo2 )
								w = ( *pFilter ) ( fx, fy, std::ceil(FilterXWidth()), std::ceil(FilterYWidth()) );
							m_bucketData->m_aFilterValues[ cindex ] = w;
						}
					}
				}
			}
		}
	}

}


//----------------------------------------------------------------------
void CqBucket::ImageElement( TqInt iXPos, TqInt iYPos, CqImagePixel*& pie ) const
{
	iXPos -= m_bucketData->m_XOrigin;
	iYPos -= m_bucketData->m_YOrigin;
	
	// Check within renderable range
	//assert( iXPos < -m_XMax && iXPos < m_XSize + m_XMax &&
	//	iYPos < -m_YMax && iYPos < m_YSize + m_YMax );

	TqInt i = ( ( iYPos + m_bucketData->m_DiscreteShiftY ) * ( m_bucketData->m_RealWidth ) ) + ( iXPos + m_bucketData->m_DiscreteShiftX );
	if (i < m_bucketData->m_aieImage.size())
		pie = &m_bucketData->m_aieImage[ i ];
}


//----------------------------------------------------------------------
CqImagePixel& CqBucket::ImageElement(TqUint index) const
{
	assert(index < m_bucketData->m_aieImage.size());
	return m_bucketData->m_aieImage[index];
}

//----------------------------------------------------------------------
/** Combine the subsamples into single pixel samples and coverage information.
 */

void CqBucket::CombineElements(enum EqFilterDepth filterdepth, CqColor zThreshold)
{
	std::vector<CqImagePixel>::iterator end = m_bucketData->m_aieImage.end();
	for ( std::vector<CqImagePixel>::iterator i = m_bucketData->m_aieImage.begin(); i != end ; i++ )
		i->Combine(m_bucketData->m_SamplePoints, filterdepth, zThreshold);
}


//----------------------------------------------------------------------
/** Get the sample color for the specified screen position.
 * If position is outside bucket, returns black.
 * \param iXPos Screen position of sample.
 * \param iYPos Screen position of sample.
 */

CqColor CqBucket::Color( TqInt iXPos, TqInt iYPos ) const
{
	CqImagePixel * pie;
	ImageElement( iXPos, iYPos, pie );
	if( NULL != pie )
		return ( pie->Color() );
	else
		return ( gColBlack);
}

//----------------------------------------------------------------------
/** Get the sample opacity for the specified screen position.
 * If position is outside bucket, returns black.
 * \param iXPos Screen position of sample.
 * \param iYPos Screen position of sample.
 */

CqColor CqBucket::Opacity( TqInt iXPos, TqInt iYPos ) const
{
	CqImagePixel * pie;
	ImageElement( iXPos, iYPos, pie );
	if( NULL != pie )
		return ( pie->Opacity() );
	else
		return ( gColBlack);
}


//----------------------------------------------------------------------
/** Get the sample coverage for the specified screen position.
 * If position is outside bucket, returns 0.
 * \param iXPos Screen position of sample.
 * \param iYPos Screen position of sample.
 */

TqFloat CqBucket::Coverage( TqInt iXPos, TqInt iYPos ) const
{
	CqImagePixel * pie;
	ImageElement( iXPos, iYPos, pie );
	if( NULL != pie )
		return ( pie->Coverage() );
	else
		return ( 0.0f );
}


//----------------------------------------------------------------------
/** Get the sample depth for the specified screen position.
 * If position is outside bucket, returns FLT_MAX.
 * \param iXPos Screen position of sample.
 * \param iYPos Screen position of sample.
 */

TqFloat CqBucket::Depth( TqInt iXPos, TqInt iYPos ) const
{
	CqImagePixel * pie;
	ImageElement( iXPos, iYPos, pie );
	if( NULL != pie )
		return ( pie->Depth() );
	else
		return ( FLT_MAX );
}


//----------------------------------------------------------------------
/** Get a pointer to the samples for a given pixel.
 * If position is outside bucket, returns NULL.
 * \param iXPos Screen position of sample.
 * \param iYPos Screen position of sample.
 */

const TqFloat* CqBucket::Data( TqInt iXPos, TqInt iYPos ) const
{
	CqImagePixel * pie;
	ImageElement( iXPos, iYPos, pie );
	if( NULL != pie )
		return ( pie->Data() );
	else
		return ( NULL );
}


//----------------------------------------------------------------------
/** Filter the samples in this bucket according to type and filter widths.
 */

void CqBucket::FilterBucket(bool fImager)
{
	CqImagePixel * pie;

	TqInt datasize = QGetRenderContext()->GetOutputDataTotalSize();
	m_bucketData->m_aDatas.resize( datasize * RealWidth() * RealHeight() );
	m_bucketData->m_aCoverages.resize( RealWidth() * RealHeight() );

	TqInt xmax = m_bucketData->m_DiscreteShiftX;
	TqInt ymax = m_bucketData->m_DiscreteShiftY;
	TqFloat xfwo2 = std::ceil(FilterXWidth()) * 0.5f;
	TqFloat yfwo2 = std::ceil(FilterYWidth()) * 0.5f;
	TqInt numsubpixels = ( PixelXSamples() * PixelYSamples() );

	TqInt numperpixel = numsubpixels * numsubpixels;
	TqInt	xlen = RealWidth();

	TqInt SampleCount = 0;
	CqColor imager;

	TqInt x, y;
	TqInt i = 0;

	TqInt endy = YOrigin() + Height();
	TqInt endx = XOrigin() + Width();

	bool useSeperable = true;

	if(m_bucketData->hasValidSamples())
	{
		// non-seperable is faster for very small filter widths.
		if(FilterXWidth() <= 16.0 || FilterYWidth() <= 16.0)
			useSeperable = false;

		if(useSeperable)
		{
			// seperable filter. filtering by fx,fy is equivalent to filtering
			// by fx,1 followed by 1,fy.

			TqInt size = Width() * RealHeight() * PixelYSamples();
			std::valarray<TqFloat> intermediateSamples( 0.0f, size * datasize);
			std::valarray<TqInt> sampleCounts(0, size);
			for ( y = YOrigin() - ymax; y < endy + ymax ; y++ )
			{
				TqFloat ycent = y + 0.5f;
				TqInt pixelRow = (y-(YOrigin()-ymax)) * PixelYSamples();
				for ( x = XOrigin(); x < endx ; x++ )
				{
					TqFloat xcent = x + 0.5f;

					// Get the element at the left side of the filter area.
					ImageElement( x - xmax, y, pie );

					TqInt pixelIndex = pixelRow*Width() + x-XOrigin();

					// filter just in x first
					for ( TqInt sy = 0; sy < PixelYSamples(); sy++ )
					{
						TqFloat gTot = 0.0;
						SampleCount = 0;
						std::valarray<TqFloat> samples( 0.0f, datasize);

						CqImagePixel* pie2 = pie;
						for ( TqInt fx = -xmax; fx <= xmax; fx++ )
						{
							TqInt index = ( ( ymax * lceil(FilterXWidth()) ) + ( fx + xmax ) ) * numperpixel;
							// Now go over each subsample within the pixel
							TqInt sampleIndex = sy * PixelXSamples();
							TqInt sindex = index + ( sy * PixelXSamples() * numsubpixels );

							for ( TqInt sx = 0; sx < PixelXSamples(); sx++ )
							{
								const SqSampleData& sampleData = pie2->SampleData( m_bucketData->m_SamplePoints,
														   sampleIndex );
								CqVector2D vecS = sampleData.m_Position;
								vecS -= CqVector2D( xcent, ycent );
								if ( vecS.x() >= -xfwo2 && vecS.y() >= -yfwo2 && vecS.x() <= xfwo2 && vecS.y() <= yfwo2 )
								{
									TqInt cindex = sindex + sampleData.m_SubCellIndex;
									TqFloat g = m_bucketData->m_aFilterValues[ cindex ];
									gTot += g;
									if ( pie2->OpaqueValues( m_bucketData->m_SamplePoints, sampleIndex ).isValid() )
									{
										SqImageSample& pSample = pie2->OpaqueValues( m_bucketData->m_SamplePoints,
															     sampleIndex );
										for ( TqInt k = 0; k < datasize; ++k )
											samples[k] += pSample.Data()[k] * g;
										sampleCounts[pixelIndex]++;
									}
								}
								sampleIndex++;
								sindex += numsubpixels;
							}
							pie2++;
						}

						// store the intermediate result
						for ( TqInt k = 0; k < datasize; k ++)
							intermediateSamples[pixelIndex*datasize + k] = samples[k] / gTot;

						pixelIndex += Width();
					}
				}
			}

			// now filter in y.
			for ( y = YOrigin(); y < endy ; y++ )
			{
				TqFloat ycent = y + 0.5f;
				for ( x = XOrigin(); x < endx ; x++ )
				{
					TqFloat xcent = x + 0.5f;
					TqFloat gTot = 0.0;
					SampleCount = 0;
					std::valarray<TqFloat> samples( 0.0f, datasize);

					TqInt fy;
					// Get the element at the top of the filter area.
					ImageElement( x, y - ymax, pie );
					for ( fy = -ymax; fy <= ymax; fy++ )
					{
						CqImagePixel* pie2 = pie;

						TqInt index = ( ( ( fy + ymax ) * lceil(FilterXWidth()) ) + xmax ) * numperpixel;
						// Now go over each y subsample within the pixel
						TqInt sx = PixelXSamples() / 2; // use the samples in the centre of the pixel.
						TqInt sy = 0;
						TqInt sampleIndex = sx;
						TqInt pixelRow = (y + fy - (YOrigin()-ymax)) * PixelYSamples();
						TqInt pixelIndex = pixelRow*Width() + x-XOrigin();

						for ( sy = 0; sy < PixelYSamples(); sy++ )
						{
							TqInt sindex = index + ( ( ( sy * PixelXSamples() ) + sx ) * numsubpixels );
							const SqSampleData& sampleData = pie2->SampleData( m_bucketData->m_SamplePoints,
													   sampleIndex );
							CqVector2D vecS = sampleData.m_Position;
							vecS -= CqVector2D( xcent, ycent );
							if ( vecS.x() >= -xfwo2 && vecS.y() >= -yfwo2 && vecS.x() <= xfwo2 && vecS.y() <= yfwo2 )
							{
								TqInt cindex = sindex + sampleData.m_SubCellIndex;
								TqFloat g = m_bucketData->m_aFilterValues[ cindex ];
								gTot += g;
								if(sampleCounts[pixelIndex] > 0)
								{
									SampleCount += sampleCounts[pixelIndex];
									for ( TqInt k = 0; k < datasize; k++)
										samples[k] += intermediateSamples[pixelIndex * datasize + k] * g;
								}
							}
							sampleIndex += PixelXSamples();
							pixelIndex += Width();
						}

						pie += xlen;
					}

					// Set depth to infinity if no samples.
					if ( SampleCount == 0 )
					{
						memset(&m_bucketData->m_aDatas[i*datasize], 0, datasize * sizeof(float));
						m_bucketData->m_aDatas[ i*datasize+6 ] = FLT_MAX;
						m_bucketData->m_aCoverages[i] = 0.0;
					}
					else
					{
						float oneOverGTot = 1.0 / gTot;
						for ( TqInt k = 0; k < datasize; k ++)
							m_bucketData->m_aDatas[ i*datasize + k ] = samples[k] * oneOverGTot;

						if ( SampleCount >= numsubpixels)
							m_bucketData->m_aCoverages[ i ] = 1.0;
						else
							m_bucketData->m_aCoverages[ i ] = ( TqFloat ) SampleCount / ( TqFloat ) (numsubpixels );
					}

					i++;
				}
			}
		}
		else
		{
			// non-seperable filter
			for ( y = YOrigin(); y < endy ; y++ )
			{
				TqFloat ycent = y + 0.5f;
				for ( x = XOrigin(); x < endx ; x++ )
				{
					TqFloat xcent = x + 0.5f;
					TqFloat gTot = 0.0;
					SampleCount = 0;
					std::valarray<TqFloat> samples( 0.0f, datasize);

					TqInt fx, fy;
					// Get the element at the upper left corner of the filter area.
					ImageElement( x - xmax, y - ymax, pie );
					for ( fy = -ymax; fy <= ymax; fy++ )
					{
						CqImagePixel* pie2 = pie;
						for ( fx = -xmax; fx <= xmax; fx++ )
						{
							TqInt index = ( ( ( fy + ymax ) * lceil(FilterXWidth()) ) + ( fx + xmax ) ) * numperpixel;
							// Now go over each subsample within the pixel
							TqInt sx, sy;
							TqInt sampleIndex = 0;
							for ( sy = 0; sy < PixelYSamples(); sy++ )
							{
								for ( sx = 0; sx < PixelXSamples(); sx++ )
								{
									TqInt sindex = index + ( ( ( sy * PixelXSamples() ) + sx ) * numsubpixels );
									const SqSampleData& sampleData = pie2->SampleData( m_bucketData->m_SamplePoints,
															   sampleIndex );
									CqVector2D vecS = sampleData.m_Position;
									vecS -= CqVector2D( xcent, ycent );
									if ( vecS.x() >= -xfwo2 && vecS.y() >= -yfwo2 && vecS.x() <= xfwo2 && vecS.y() <= yfwo2 )
									{
										TqInt cindex = sindex + sampleData.m_SubCellIndex;
										TqFloat g = m_bucketData->m_aFilterValues[ cindex ];
										gTot += g;
										if ( pie2->OpaqueValues( m_bucketData->m_SamplePoints, sampleIndex ).isValid() )
										{
											SqImageSample& pSample = pie2->OpaqueValues( m_bucketData->m_SamplePoints,
																     sampleIndex );
											for ( TqInt k = 0; k < datasize; ++k )
												samples[k] += pSample.Data()[k] * g;
											SampleCount++;
										}
									}
									sampleIndex++;
								}
							}
							pie2++;
						}
						pie += xlen;
					}


					// Set depth to infinity if no samples.
					if ( SampleCount == 0 )
					{
						memset(&m_bucketData->m_aDatas[i*datasize], 0, datasize * sizeof(float));
						m_bucketData->m_aDatas[ i*datasize+6 ] = FLT_MAX;
						m_bucketData->m_aCoverages[i] = 0.0;
					}
					else
					{
						float oneOverGTot = 1.0 / gTot;
						for ( TqInt k = 0; k < datasize; k ++)
							m_bucketData->m_aDatas[ i*datasize + k ] = samples[k] * oneOverGTot;

						if ( SampleCount >= numsubpixels)
							m_bucketData->m_aCoverages[ i ] = 1.0;
						else
							m_bucketData->m_aCoverages[ i ] = ( TqFloat ) SampleCount / ( TqFloat ) (numsubpixels );
					}

					i++;
				}
			}
		}
	}
	else
	{
		// empty bucket.
		TqInt size = Width()*Height();
		memset(&m_bucketData->m_aDatas[0], 0, size * datasize * sizeof(float));
		memset(&m_bucketData->m_aCoverages[0], 0, size * sizeof(float));
		for(i = 0; i<size; ++i)
		{
			// Set the depth to infinity.
			m_bucketData->m_aDatas[ i*datasize+6 ] = FLT_MAX;
		}
	}

	i = 0;
	ImageElement( XOrigin(), YOrigin(), pie );
	endy = Height();
	endx = Width();

	// Set the coverage and alpha values for the pixel.
	for ( y = 0; y < endy; y++ )
	{
		CqImagePixel* pie2 = pie;
		for ( x = 0; x < endx; x++ )
		{
			SqImageSample& spl = pie2->GetPixelSample();
			for (TqInt k=0; k < datasize; k++)
				spl.Data()[k] = m_bucketData->m_aDatas[ i * datasize + k ];
			TqFloat* sample_data = spl.Data();
			sample_data[Sample_Coverage] = m_bucketData->m_aCoverages[ i++ ];

			// Calculate the alpha as the combination of the opacity and the coverage.
			TqFloat a = ( sample_data[Sample_ORed] + sample_data[Sample_OGreen] + sample_data[Sample_OBlue] ) / 3.0f;
			pie2->SetAlpha(a * sample_data[Sample_Coverage]);

			pie2++;
		}
		pie += xlen;
	}

	endy = YOrigin() + Height();
	endx = XOrigin() + Width();

	if ( QGetRenderContext() ->poptCurrent()->pshadImager() )
	{
		// Init & Execute the imager shader

		QGetRenderContext() ->poptCurrent()->InitialiseColorImager( this );
		AQSIS_TIME_SCOPE(Imager_shading);

		if ( fImager )
		{
			i = 0;
			ImageElement( XOrigin(), YOrigin(), pie );
			for ( y = YOrigin(); y < endy ; y++ )
			{
				CqImagePixel* pie2 = pie;
				for ( x = XOrigin(); x < endx ; x++ )
				{
					imager = QGetRenderContext() ->poptCurrent()->GetColorImager( x , y );
					// Normal case will be to poke the alpha from the image shader and
					// multiply imager color with it... but after investigation alpha is always
					// == 1 after a call to imager shader in 3delight and BMRT.
					// Therefore I did not ask for alpha value and set directly the pCols[i]
					// with imager value. see imagers.cpp
					pie2->SetColor( imager );
					imager = QGetRenderContext() ->poptCurrent()->GetOpacityImager( x , y );
					pie2->SetOpacity( imager );
					TqFloat a = ( imager[0] + imager[1] + imager[2] ) / 3.0f;
					pie2->SetAlpha( a );
					pie2++;
					i++;
				}
				pie += xlen;
			}
		}
	}
}


//----------------------------------------------------------------------
/** Expose the samples in this bucket according to specified gain and gamma settings.
 */

void CqBucket::ExposeBucket()
{
	if(m_bucketData->hasValidSamples())
	{
		if ( QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Exposure" ) [ 0 ] == 1.0 &&
				QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Exposure" ) [ 1 ] == 1.0 )
			return ;
		else
		{
			CqImagePixel* pie;
			ImageElement( XOrigin(), YOrigin(), pie );
			TqInt x, y;
			TqFloat exposegain = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Exposure" ) [ 0 ];
			TqFloat exposegamma = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Exposure" ) [ 1 ];
			TqFloat oneovergamma = 1.0f / exposegamma;
			TqFloat endx, endy;
			TqInt   nextx;
			endy = Height();
			endx = Width();
			nextx = RealWidth();

			for ( y = 0; y < endy; y++ )
			{
				CqImagePixel* pie2 = pie;
				for ( x = 0; x < endx; x++ )
				{
					// color=(color*gain)^1/gamma
					if ( exposegain != 1.0 )
						pie2->SetColor( pie2->Color() * exposegain );

					if ( exposegamma != 1.0 )
					{
						CqColor col = pie2->Color();
						col.SetfRed ( pow( col.fRed (), oneovergamma ) );
						col.SetfGreen( pow( col.fGreen(), oneovergamma ) );
						col.SetfBlue ( pow( col.fBlue (), oneovergamma ) );
						pie2->SetColor( col );
					}
					pie2++;
				}
				pie += nextx;
			}
		}
	}
}


//----------------------------------------------------------------------
/** Quantize the samples in this bucket according to type.
 */

void CqBucket::QuantizeBucket()
{
	if(m_bucketData->hasValidSamples())
	{
		// Initiliaze the random with a value based on the X,Y coordinate
		static CqRandom random( 61 );
		TqFloat endx, endy;
		TqInt   nextx;
		endy = Height();
		endx = Width();
		nextx = RealWidth();


		if ( QGetRenderContext() ->poptCurrent()->GetIntegerOption( "System", "DisplayMode" ) [ 0 ] & ModeRGB )
		{
			const TqFloat* pQuant = QGetRenderContext() ->poptCurrent()->GetFloatOption( "Quantize", "Color" );
			TqInt one = static_cast<TqInt>( pQuant [ 0 ] );
			TqInt min = static_cast<TqInt>( pQuant [ 1 ] );
			TqInt max = static_cast<TqInt>( pQuant [ 2 ] );
			double ditheramplitude = pQuant [ 3 ];

			// If settings are 0,0,0,0 then leave as floating point and we will save an FP tiff.
			if ( one == 0 && min == 0 && max == 0 )
				return ;

			CqImagePixel* pie;
			ImageElement( XOrigin(), YOrigin(), pie );
			TqInt x, y;

			for ( y = 0; y < endy; y++ )
			{
				CqImagePixel* pie2 = pie;
				for ( x = 0; x < endx; x++ )
				{
					double r, g, b, a;
					double _or, _og, _ob;
					double s = random.RandomFloat();
					CqColor col = pie2->Color();
					CqColor opa = pie2->Opacity();
					TqFloat alpha = pie2->Alpha();
					if ( modf( one * col.fRed () + ditheramplitude * s, &r ) > 0.5 )
						r += 1;
					if ( modf( one * col.fGreen() + ditheramplitude * s, &g ) > 0.5 )
						g += 1;
					if ( modf( one * col.fBlue () + ditheramplitude * s, &b ) > 0.5 )
						b += 1;
					if ( modf( one * opa.fRed () + ditheramplitude * s, &_or ) > 0.5 )
						_or += 1;
					if ( modf( one * opa.fGreen() + ditheramplitude * s, &_og ) > 0.5 )
						_og += 1;
					if ( modf( one * opa.fBlue () + ditheramplitude * s, &_ob ) > 0.5 )
						_ob += 1;
					if ( modf( one * alpha + ditheramplitude * s, &a ) > 0.5 )
						a += 1;
					r = clamp<double>(r, min, max);
					g = clamp<double>(g, min, max);
					b = clamp<double>(b, min, max);
					_or = clamp<double>(_or, min, max);
					_og = clamp<double>(_og, min, max);
					_ob = clamp<double>(_ob, min, max);
					a = clamp<double>(a, min, max);
					col.SetfRed ( r );
					col.SetfGreen( g );
					col.SetfBlue ( b );
					opa.SetfRed ( _or );
					opa.SetfGreen( _og );
					opa.SetfBlue ( _ob );
					pie2->SetColor( col );
					pie2->SetOpacity( opa );
					pie2->SetAlpha( a );
					pie2++;
				}
				pie += nextx;
			}
		}

		if ( QGetRenderContext() ->poptCurrent()->GetIntegerOption( "System", "DisplayMode" ) [ 0 ] & ModeZ )
		{
			const TqFloat* pQuant = QGetRenderContext() ->poptCurrent()->GetFloatOption( "Quantize", "Depth" );
			TqInt one = static_cast<TqInt>( pQuant [ 0 ] );
			TqInt min = static_cast<TqInt>( pQuant [ 1 ] );
			TqInt max = static_cast<TqInt>( pQuant [ 2 ] );
			double ditheramplitude = pQuant [ 3 ];
			if( ditheramplitude == 0.0f && one == 0 && min == 0 && max == 0 )
				return;

			CqImagePixel* pie;
			ImageElement( XOrigin(), YOrigin(), pie );
			TqInt x, y;
			for ( y = 0; y < endy; y++ )
			{
				CqImagePixel* pie2 = pie;
				for ( x = 0; x < endx; x++ )
				{
					double d;
					if ( modf( one * pie2->Depth() + ditheramplitude * random.RandomFloat(), &d ) > 0.5 )
						d += 1;
					d = clamp<double>( d, min, max );
					pie2->SetDepth( d );
					pie2++;
				}
				pie += nextx;
			}
		}

		// Now go through the other AOV's and quantize those if necessary.
		std::map<std::string, CqRenderer::SqOutputDataEntry>& DataMap = QGetRenderContext()->GetMapOfOutputDataEntries();
		std::map<std::string, CqRenderer::SqOutputDataEntry>::iterator entry;
		for( entry = DataMap.begin(); entry != DataMap.end(); entry++ )
		{
			const TqFloat* pQuant = QGetRenderContext() ->poptCurrent()->GetFloatOption( "Quantize", entry->first.c_str() );
			if( NULL != pQuant )
			{
				TqInt startindex = entry->second.m_Offset;
				TqInt endindex = startindex + entry->second.m_NumSamples;
				TqInt one = static_cast<TqInt>( pQuant [ 0 ] );
				TqInt min = static_cast<TqInt>( pQuant [ 1 ] );
				TqInt max = static_cast<TqInt>( pQuant [ 2 ] );
				double ditheramplitude = pQuant [ 3 ];

				CqImagePixel* pie;
				ImageElement( XOrigin(), YOrigin(), pie );
				TqInt x, y;
				for ( y = 0; y < endy; y++ )
				{
					CqImagePixel* pie2 = pie;
					for ( x = 0; x < endx; x++ )
					{
						TqInt sampleindex;
						for( sampleindex = startindex; sampleindex < endindex; sampleindex++ )
						{
							double d;
							if ( modf( one * pie2->GetPixelSample().Data()[sampleindex] + ditheramplitude * random.RandomFloat(), &d ) > 0.5 )
								d += 1.0f;
							d = clamp<double>( d, min, max );
							pie2->GetPixelSample().Data()[sampleindex] = d;
						}
						pie2++;
					}
					pie += nextx;
				}
			}
		}
	}
}

//----------------------------------------------------------------------
/** Get the flag that indicates whether the bucket is empty.  It is
 * empty only when this bucket doesn't contain any surface,
 * micropolygon or grids.
 */
bool CqBucket::IsEmpty()
{
	return !pTopSurface() && m_micropolygons.empty();
}


//----------------------------------------------------------------------
bool CqBucket::hasPendingSurfaces() const
{
	return ! m_gPrims.empty();
}


//----------------------------------------------------------------------
bool CqBucket::hasPendingMPs() const
{
	return ! m_micropolygons.empty();
}

//----------------------------------------------------------------------
/** Render any waiting MPs.
 
    Render ready micro polygons waiting to be processed, so that we
    have as few as possible MPs waiting and using memory at any given
    moment
 */

void CqBucket::RenderWaitingMPs()
{
	bool mpgsRendered = false;
	{
		for ( std::vector<boost::shared_ptr<CqMicroPolygon> >::iterator itMP = m_micropolygons.begin();
			  itMP != m_micropolygons.end();
			  itMP++ )
		{
			CqMicroPolygon* mp = (*itMP).get();
			RenderMicroPoly( mp );
			mpgsRendered = true;
		}

		m_micropolygons.clear();
	}
	if(mpgsRendered)
		m_bucketData->m_OcclusionTree.updateDepths();
}


//----------------------------------------------------------------------
/** Render the given Surface
 */
void CqBucket::RenderSurface( boost::shared_ptr<CqSurface>& surface )
{
	// Cull surface if it's hidden
	if ( !( QGetRenderContext() ->poptCurrent()->GetIntegerOption( "System", "DisplayMode" )[0] & ModeZ ) && !surface->pCSGNode() )
	{
		AQSIS_TIME_SCOPE(Occlusion_culling);
		if ( surface->fCachedBound() &&
			 occlusionCullSurface( surface ) )
		{
			return;
		}
	}

	// If the epsilon check has deemed this surface to be undiceable, don't bother asking.
	bool fDiceable = false;
	{
		AQSIS_TIME_SCOPE(Dicable_check);
		fDiceable = surface->Diceable();
	}

	// Dice & shade the surface if it's small enough...
	if ( fDiceable )
	{
		CqMicroPolyGridBase* pGrid = 0;
		{
			AQSIS_TIME_SCOPE(Dicing);
			pGrid = surface->Dice();
		}

		if ( NULL != pGrid )
		{
			ADDREF( pGrid );
			// Only shade in all cases since the Displacement could be called in the shadow map creation too.
			// \note Timings for shading are broken down into component parts within this function.
			pGrid->Shade();
			pGrid->TransferOutputVariables();

			if ( pGrid->vfCulled() == false )
			{
				AQSIS_TIME_SCOPE(Bust_grids);
				// Split any grids in this bucket waiting to be processed.
				pGrid->Split( QGetRenderContext()->pImage(), realXOrigin(), realXOrigin()+RealWidth(), realYOrigin(), realYOrigin()+RealHeight());
			}

			RELEASEREF( pGrid );
		}
	}
	// The surface is not small enough, so split it...
	else if ( !surface->fDiscard() )
	{
		// Decrease the total gprim count since this gprim is replaced by other gprims
		STATS_DEC( GPR_created_total );

		// Split it
		{
			AQSIS_TIME_SCOPE(Splitting);
			std::vector<boost::shared_ptr<CqSurface> > aSplits;
			TqInt cSplits = surface->Split( aSplits );
			for ( TqInt i = 0; i < cSplits; i++ )
			{
				QGetRenderContext()->pImage()->PostSurface( aSplits[ i ] );
			}
		}
	}
}

//----------------------------------------------------------------------
/** Render a particular micropolygon.
 
 * \param pMP Pointer to the micropolygon to process.
   \see CqBucket, CqImagePixel
 */

void CqBucket::RenderMicroPoly( CqMicroPolygon* pMP )
{
	const CqBound& Bound = pMP->GetTotalBound();

	// if bounding box is outside our viewing range, then cull it.
	if ( Bound.vecMax().x() < m_bucketData->m_viewRangeXMin ||
	     Bound.vecMax().y() < m_bucketData->m_viewRangeYMin ||
	     Bound.vecMin().x() > m_bucketData->m_viewRangeXMax ||
	     Bound.vecMin().y() > m_bucketData->m_viewRangeYMax ||
	     Bound.vecMin().z() > m_bucketData->m_clippingFar ||
	     Bound.vecMax().z() < m_bucketData->m_clippingNear )
	{
		STATS_INC( MPG_culled );
		return;
	}

	bool UsingDof = QGetRenderContext()->UsingDepthOfField();
	bool IsMoving = pMP->IsMoving();

	// Cache the shading interpolation type.  Ideally this should really be
	// done by the CacheOutputInterpCoeffs(), or possibly once per grid...
	const TqInt* interpType = pMP->pGrid()->pAttributes()
		->GetIntegerAttribute("System", "ShadingInterpolation");
	// At this stage, only use smooth shading interpolation for stationary
	// grids without DoF.
	/// \todo Allow smooth shading with MB or DoF.
	m_bucketData->m_CurrentMpgSampleInfo.smoothInterpolation
		= !(UsingDof || IsMoving) && (*interpType == ShadingInterp_Smooth);

	// Cache output sample info for this mpg so we don't have to keep fetching
	// it for each sample.
	pMP->CacheOutputInterpCoeffs(m_bucketData->m_CurrentMpgSampleInfo);

	// use the single imagesample rather than the list if possible.
	// transparent, matte or csg samples, or if we need more than the first
	// depth value have to use the (slower) list.
	m_bucketData->m_CurrentMpgSampleInfo.isOpaque =
		m_bucketData->m_CurrentMpgSampleInfo.occludes
		&& !pMP->pGrid()->pCSGNode()
		&& !pMP->pGrid()->GetCachedGridInfo().m_IsMatte
		&& !(QGetRenderContext()->poptCurrent()->
				GetIntegerOption("System", "DisplayMode")[0] & ModeZ);

	if(IsMoving || UsingDof)
		RenderMPG_MBOrDof( pMP, IsMoving, UsingDof );
	else
		RenderMPG_Static( pMP );
}



// this function assumes that neither dof or mb are being used. it is much
// simpler than the general case dealt with above.
void CqBucket::RenderMPG_Static( CqMicroPolygon* pMPG)
{
	const SqGridInfo& currentGridInfo = pMPG->pGrid()->GetCachedGridInfo();
    const TqFloat* LodBounds = currentGridInfo.m_LodBounds;
    bool UsingLevelOfDetail = LodBounds[ 0 ] >= 0.0f;

    TqInt sample_hits = 0;
    //TqFloat shd_rate = m_CurrentGridInfo.m_ShadingRate;

	CqHitTestCache hitTestCache;
	bool cachedHitData = false;

	//bool mustDraw = !m_CurrentGridInfo.m_IsCullable;

    CqBound Bound = pMPG->GetTotalBound();

	TqFloat bminx = Bound.vecMin().x();
	TqFloat bmaxx = Bound.vecMax().x();
	TqFloat bminy = Bound.vecMin().y();
	TqFloat bmaxy = Bound.vecMax().y();
	//TqFloat bminz = Bound.vecMin().z();

	// Now go across all pixels touched by the micropolygon bound.
	// The first pixel position is at (sX, sY), the last one
	// at (eX, eY).
	TqInt eX = lceil( bmaxx );
	TqInt eY = lceil( bmaxy );
	if ( eX > m_bucketData->m_viewRangeXMax ) eX = m_bucketData->m_viewRangeXMax;
	if ( eY > m_bucketData->m_viewRangeYMax ) eY = m_bucketData->m_viewRangeYMax;

	TqInt sX = static_cast<TqInt>(std::floor( bminx ));
	TqInt sY = static_cast<TqInt>(std::floor( bminy ));
	if ( sY < m_bucketData->m_viewRangeYMin ) sY = m_bucketData->m_viewRangeYMin;
	if ( sX < m_bucketData->m_viewRangeXMin ) sX = m_bucketData->m_viewRangeXMin;

	CqImagePixel* pie, *pie2;

	TqInt iXSamples = PixelXSamples();
	TqInt iYSamples = PixelYSamples();

	TqInt im = ( bminx < sX ) ? 0 : static_cast<TqInt>(std::floor( ( bminx - sX ) * iXSamples ));
	TqInt in = ( bminy < sY ) ? 0 : static_cast<TqInt>(std::floor( ( bminy - sY ) * iYSamples ));
	TqInt em = ( bmaxx > eX ) ? iXSamples : lceil( ( bmaxx - ( eX - 1 ) ) * iXSamples );
	TqInt en = ( bmaxy > eY ) ? iYSamples : lceil( ( bmaxy - ( eY - 1 ) ) * iYSamples );

	TqInt nextx = RealWidth();
	ImageElement( sX, sY, pie );

	for( int iY = sY; iY < eY; ++iY)
	{
		pie2 = pie;
		pie += nextx;

		for(int iX = sX; iX < eX; ++iX, ++pie2)
		{
			// only bother sampling if the mpg is not occluded in this pixel.
			//if(mustDraw || bminz <= pie2->SampleData(index).m_occlusionBox->MaxOpaqueZ())
			{
				if(!cachedHitData)
				{
					pMPG->CacheHitTestValues(&hitTestCache);
					cachedHitData = true;
				}

				// Now sample the micropolygon at several subsample positions
				// within the pixel. The subsample indices range from (start_m, n)
				// to (end_m-1, end_n-1).
				register int m, n;
				n = ( iY == sY ) ? in : 0;
				int end_n = ( iY == ( eY - 1 ) ) ? en : iYSamples;
				int start_m = ( iX == sX ) ? im : 0;
				int end_m = ( iX == ( eX - 1 ) ) ? em : iXSamples;
				int index_start = n*iXSamples + start_m;

				for ( ; n < end_n; n++ )
				{
					int index = index_start;
					for ( m = start_m; m < end_m; m++, index++ )
					{
						const SqSampleData& sampleData = pie2->SampleData( m_bucketData->m_SamplePoints, index );
						//if(mustDraw || bminz <= pie2->SampleData(index).m_occlusionBox->MaxOpaqueZ())
						{
							const CqVector2D& vecP = sampleData.m_Position;
							const TqFloat time = 0.0;

							CqStats::IncI( CqStats::SPL_count );

							if(!Bound.Contains2D( vecP ))
								continue;

							// Check to see if the sample is within the sample's level of detail
							if ( UsingLevelOfDetail)
							{
								TqFloat LevelOfDetail = sampleData.m_DetailLevel;
								if ( LodBounds[ 0 ] > LevelOfDetail || LevelOfDetail >= LodBounds[ 1 ] )
								{
									continue;
								}
							}

							CqStats::IncI( CqStats::SPL_bound_hits );

							// Now check if the subsample hits the micropoly
							bool SampleHit;
							TqFloat D;

							SampleHit = pMPG->Sample( hitTestCache, sampleData, D, time );

							if ( SampleHit )
							{
								sample_hits++;
								StoreSample( pMPG, pie2, index, D );
							}
						}
					}
					index_start += iXSamples;
				}
			}
	/*        // Now compute the % of samples that hit...
			TqInt scount = iXSamples * iYSamples;
			TqFloat max_hits = scount * shd_rate;
			TqInt hit_rate = ( sample_hits / max_hits ) / 0.125;
			STATS_INC( MPG_sample_coverage0_125 + CLAMP( hit_rate - 1 , 0, 7 ) );
	*/  }
	}
}

// this function assumes that either dof or mb or both are being used.
void CqBucket::RenderMPG_MBOrDof( CqMicroPolygon* pMPG, bool IsMoving, bool UsingDof )
{
	const SqGridInfo& currentGridInfo = pMPG->pGrid()->GetCachedGridInfo();

    const TqFloat* LodBounds = currentGridInfo.m_LodBounds;
    bool UsingLevelOfDetail = LodBounds[ 0 ] >= 0.0f;

    TqInt sample_hits = 0;
    //TqFloat shd_rate = m_CurrentGridInfo.m_ShadingRate;

	CqHitTestCache hitTestCache;
	bool cachedHitData = false;

	TqInt iXSamples = PixelXSamples();
    TqInt iYSamples = PixelYSamples();

	TqFloat opentime = currentGridInfo.m_ShutterOpenTime;
	TqFloat closetime = currentGridInfo.m_ShutterCloseTime;
	TqFloat timePerSample = 0;
	if(IsMoving)
	{
		TqInt numSamples = iXSamples * iYSamples;
		timePerSample = (float)numSamples / ( closetime - opentime );
	}

	const TqUint timeRanges = std::max(4, m_bucketData->m_PixelXSamples * m_bucketData->m_PixelYSamples);
	TqInt bound_maxMB = pMPG->cSubBounds( timeRanges );
    TqInt bound_maxMB_1 = bound_maxMB - 1;
	//TqInt currentIndex = 0;
    for ( TqInt bound_numMB = 0; bound_numMB < bound_maxMB; bound_numMB++ )
    {
        TqFloat time0;
        TqFloat time1;
        const CqBound& Bound = pMPG->SubBound( bound_numMB, time0 );

		// get the index of the first and last samples that can fall inside
		// the time range of this bound
		TqInt indexT0 = 0;
		TqInt indexT1 = 0;
		if(IsMoving)
		{
			if ( bound_numMB != bound_maxMB_1 )
				pMPG->SubBound( bound_numMB + 1, time1 );
			else
				time1 = closetime;//QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "Shutter" ) [ 1 ];
	
			indexT0 = static_cast<TqInt>(std::floor((time0 - opentime) * timePerSample));
			indexT1 = static_cast<TqInt>(lceil((time1 - opentime) * timePerSample));
		}

		TqFloat maxCocX = 0;
		TqFloat maxCocY = 0;

		TqFloat bminx = 0;
		TqFloat bmaxx = 0;
		TqFloat bminy = 0;
		TqFloat bmaxy = 0;
		TqFloat bminz = 0;
		TqFloat bmaxz = 0;
		// these values are the bound of the mpg not including dof extension.
		// reduce the mpg bound so it doesn't include the coc.
		TqFloat mpgbminx = 0;
		TqFloat mpgbmaxx = 0;
		TqFloat mpgbminy = 0;
		TqFloat mpgbmaxy = 0;
		TqInt bound_maxDof = 1;
		if(UsingDof)
		{
			const CqVector2D& minZCoc = QGetRenderContext()->GetCircleOfConfusion( Bound.vecMin().z() );
			const CqVector2D& maxZCoc = QGetRenderContext()->GetCircleOfConfusion( Bound.vecMax().z() );
			maxCocX = max( minZCoc.x(), maxZCoc.x() );
			maxCocY = max( minZCoc.y(), maxZCoc.y() );

			mpgbminx = Bound.vecMin().x() + maxCocX;
			mpgbmaxx = Bound.vecMax().x() - maxCocX;
			mpgbminy = Bound.vecMin().y() + maxCocY;
			mpgbmaxy = Bound.vecMax().y() - maxCocY;
			bminz = Bound.vecMin().z();
			bmaxz = Bound.vecMax().z();

			bound_maxDof = m_bucketData->m_NumDofBounds;
		}
		else
		{
			bminx = Bound.vecMin().x();
			bmaxx = Bound.vecMax().x();
			bminy = Bound.vecMin().y();
			bmaxy = Bound.vecMax().y();
			bminz = Bound.vecMin().z();
			bmaxz = Bound.vecMax().z();

			bound_maxDof = 1;
		}

		for ( TqInt bound_numDof = 0; bound_numDof < bound_maxDof; bound_numDof++ )
		{
			if(UsingDof)
			{
				// now shift the bounding box to cover only a given range of
				// lens positions.
				const CqBound DofBound = DofSubBound( bound_numDof );
				TqFloat leftOffset = DofBound.vecMax().x() * maxCocX;
				TqFloat rightOffset = DofBound.vecMin().x() * maxCocX;
				TqFloat topOffset = DofBound.vecMax().y() * maxCocY;
				TqFloat bottomOffset = DofBound.vecMin().y() * maxCocY;

				bminx = mpgbminx - leftOffset;
				bmaxx = mpgbmaxx - rightOffset;
				bminy = mpgbminy - topOffset;
				bmaxy = mpgbmaxy - bottomOffset;
			}

			// if bounding box is outside our viewing range, then cull it.
			if ( bmaxx < m_bucketData->m_viewRangeXMin ||
			     bmaxy < m_bucketData->m_viewRangeYMin ||
			     bminx > m_bucketData->m_viewRangeXMax ||
			     bminy > m_bucketData->m_viewRangeYMax ||
			     bminz > m_bucketData->m_clippingFar ||
			     bmaxz < m_bucketData->m_clippingNear )
			{
				continue;
			}

			// Now go across all pixels touched by the micropolygon bound.
			// The first pixel position is at (sX, sY), the last one
			// at (eX, eY).
			TqInt eX = lceil( bmaxx );
			TqInt eY = lceil( bmaxy );
			if ( eX > m_bucketData->m_viewRangeXMax ) eX = m_bucketData->m_viewRangeXMax;
			if ( eY > m_bucketData->m_viewRangeYMax ) eY = m_bucketData->m_viewRangeYMax;

			TqInt sX = static_cast<TqInt>(std::floor( bminx ));
			TqInt sY = static_cast<TqInt>(std::floor( bminy ));
			if ( sY < m_bucketData->m_viewRangeYMin ) sY = m_bucketData->m_viewRangeYMin;
			if ( sX < m_bucketData->m_viewRangeXMin ) sX = m_bucketData->m_viewRangeXMin;

			CqImagePixel* pie, *pie2;

			TqInt nextx = RealWidth();
			ImageElement( sX, sY, pie );

			for( int iY = sY; iY < eY; ++iY)
			{
				pie2 = pie;
				pie += nextx;

				for(int iX = sX; iX < eX; ++iX, ++pie2)
				{
					TqInt index;
					if(UsingDof)
					{
						// when using dof only one sample per pixel can
						// possibbly hit (the one corresponding to the
						// current bounding box).
						index = pie2->GetDofOffsetIndex(bound_numDof);
					}
					else
					{
						// when using mb without dof, a range of samples
						// may have times within the current mb bounding box.
						index = indexT0;
					}
					// only bother sampling if the mpg is not occluded in this pixel.
					//if(mustDraw || bminz <= pie2->SampleData(index).m_occlusionBox->MaxOpaqueZ())
					{

						// loop over potential samples
						do
						{
							const SqSampleData& sampleData = pie2->SampleData( m_bucketData->m_SamplePoints, index );
							const CqVector2D& vecP = sampleData.m_Position;
							const TqFloat time = sampleData.m_Time;

							index++;

							CqStats::IncI( CqStats::SPL_count );

							if(IsMoving && (time < time0 || time > time1))
							{
								continue;
							}

							// check if sample lies inside mpg bounding box.
							if ( UsingDof )
							{
								CqBound DofBound(bminx, bminy, bminz, bmaxx, bmaxy, bmaxz);

								if(!DofBound.Contains2D( vecP ))
									continue;

								// Check to see if the sample is within the sample's level of detail
								if ( UsingLevelOfDetail)
								{
									TqFloat LevelOfDetail = sampleData.m_DetailLevel;
									if ( LodBounds[ 0 ] > LevelOfDetail || LevelOfDetail >= LodBounds[ 1 ] )
									{
										continue;
									}
								}


								CqStats::IncI( CqStats::SPL_bound_hits );

								// Now check if the subsample hits the micropoly
								bool SampleHit;
								TqFloat D;

								SampleHit = pMPG->Sample( hitTestCache, sampleData, D, time, UsingDof );
								if ( SampleHit )
								{
									sample_hits++;
									// note index has already been incremented, so we use the previous value.
									StoreSample( pMPG, pie2, index-1, D );
								}
							}
							else
							{
								if(!Bound.Contains2D( vecP ))
									continue;
								// Check to see if the sample is within the sample's level of detail
								if ( UsingLevelOfDetail)
								{
									TqFloat LevelOfDetail = sampleData.m_DetailLevel;
									if ( LodBounds[ 0 ] > LevelOfDetail || LevelOfDetail >= LodBounds[ 1 ] )
									{
										continue;
									}
								}


								CqStats::IncI( CqStats::SPL_bound_hits );

								// Now check if the subsample hits the micropoly
								bool SampleHit;
								TqFloat D;

								pMPG->CacheHitTestValues(&hitTestCache);
								cachedHitData = true;

								SampleHit = pMPG->Sample( hitTestCache, sampleData, D, time, UsingDof );
								if ( SampleHit )
								{
									sample_hits++;
									// note index has already been incremented, so we use the previous value.
									StoreSample( pMPG, pie2, index-1, D );
								}
							}
						} while (!UsingDof && index < indexT1);
					}
				}
			}
		}
    }
}

void CqBucket::StoreSample( CqMicroPolygon* pMPG, CqImagePixel* pie2, TqInt index, TqFloat D )
{
	const SqGridInfo& currentGridInfo = pMPG->pGrid()->GetCachedGridInfo();

    bool Occludes = m_bucketData->m_CurrentMpgSampleInfo.occludes;
	bool opaque =  m_bucketData->m_CurrentMpgSampleInfo.isOpaque;

	SqImageSample& currentOpaqueSample = pie2->OpaqueValues(m_bucketData->m_SamplePoints, index);
	//static SqImageSample localImageVal( QGetRenderContext() ->GetOutputDataTotalSize() );
	SqImageSample localImageVal;

	SqImageSample& ImageVal = opaque ? currentOpaqueSample : localImageVal;

	std::deque<SqImageSample>& aValues = pie2->Values( m_bucketData->m_SamplePoints, index );
	std::deque<SqImageSample>::iterator sample = aValues.begin();
	std::deque<SqImageSample>::iterator end = aValues.end();

	// return if the sample is occluded and can be culled.
	if(opaque)
	{
		if((currentOpaqueSample.isValid()) &&
			currentOpaqueSample.Data()[Sample_Depth] <= D)
		{
			return;
		}
	}
	else
	{
		// Sort the color/opacity into the visible point list
		// return if the sample is occluded and can be culled.
		while( sample != end )
		{
			if((*sample).Data()[Sample_Depth] >= D)
				break;

			if(((*sample).isOccludes()) &&
				!(*sample).m_pCSGNode && currentGridInfo.m_IsCullable)
				return;

			++sample;
		}
	}

    ImageVal.Data()[Sample_Depth] = D ;

	CqStats::IncI( CqStats::SPL_hits );
	pMPG->MarkHit();
	// Record the fact that we have valid samples in the bucket.
	m_bucketData->setHasValidSamples();

    TqFloat* val = ImageVal.Data();
	CqColor col;
	CqColor opa;
	const SqSampleData& sampleData = pie2->SampleData( m_bucketData->m_SamplePoints, index );
	const CqVector2D& vecP = sampleData.m_Position;
	pMPG->InterpolateOutputs(m_bucketData->m_CurrentMpgSampleInfo, vecP, col, opa);

    val[ Sample_Red ] = col[0];
    val[ Sample_Green ] = col[1];
    val[ Sample_Blue ] = col[2];
    val[ Sample_ORed ] = opa[0];
    val[ Sample_OGreen ] = opa[1];
    val[ Sample_OBlue ] = opa[2];
    val[ Sample_Depth ] = D;

    // Now store any other data types that have been registered.
	if(currentGridInfo.m_UsesDataMap)
	{
		StoreExtraData(pMPG, ImageVal);
	}

	if(!opaque)
	{
		// If depth is exactly the same as previous sample, chances are we've
		// hit a MPG grid line.
		// \note: Cannot do this if there is CSG involved, as all samples must be taken and kept the same.
		if ( sample != end && (*sample).Data()[Sample_Depth] == ImageVal.Data()[Sample_Depth] && !(*sample).m_pCSGNode )
		{
			//(*sample).m_Data = ( (*sample).m_Data + val ) * 0.5f;
			return;
		}
	}

    ImageVal.m_pCSGNode = pMPG->pGrid() ->pCSGNode();

    ImageVal.resetFlags();
    if ( Occludes )
    {
        ImageVal.setOccludes();
    }
    if( currentGridInfo.m_IsMatte )
    {
        ImageVal.setMatte();
    }

	if(!opaque)
	{
		aValues.insert( sample, ImageVal );
	}
	else
	{
		// mark this sample as having been written into.
		ImageVal.setValid();
	}
}



void CqBucket::StoreExtraData( CqMicroPolygon* pMPG, SqImageSample& sample)
{
	std::map<std::string, CqRenderer::SqOutputDataEntry>& DataMap = QGetRenderContext() ->GetMapOfOutputDataEntries();
	std::map<std::string, CqRenderer::SqOutputDataEntry>::iterator entry;
	for ( entry = DataMap.begin(); entry != DataMap.end(); ++entry )
	{
		IqShaderData* pData;
		if ( ( pData = pMPG->pGrid() ->FindStandardVar( entry->first.c_str() ) ) != NULL )
		{
			switch ( pData->Type() )
			{
					case type_float:
					case type_integer:
					{
						TqFloat f;
						pData->GetFloat( f, pMPG->GetIndex() );
						sample.Data()[ entry->second.m_Offset ] = f;
						break;
					}
					case type_point:
					case type_normal:
					case type_vector:
					case type_hpoint:
					{
						CqVector3D v;
						pData->GetPoint( v, pMPG->GetIndex() );
						sample.Data()[ entry->second.m_Offset ] = v.x();
						sample.Data()[ entry->second.m_Offset + 1 ] = v.y();
						sample.Data()[ entry->second.m_Offset + 2 ] = v.z();
						break;
					}
					case type_color:
					{
						CqColor c;
						pData->GetColor( c, pMPG->GetIndex() );
						sample.Data()[ entry->second.m_Offset ] = c.fRed();
						sample.Data()[ entry->second.m_Offset + 1 ] = c.fGreen();
						sample.Data()[ entry->second.m_Offset + 2 ] = c.fBlue();
						break;
					}
					case type_matrix:
					{
						CqMatrix m;
						pData->GetMatrix( m, pMPG->GetIndex() );
						TqFloat* pElements = m.pElements();
						sample.Data()[ entry->second.m_Offset ] = pElements[ 0 ];
						sample.Data()[ entry->second.m_Offset + 1 ] = pElements[ 1 ];
						sample.Data()[ entry->second.m_Offset + 2 ] = pElements[ 2 ];
						sample.Data()[ entry->second.m_Offset + 3 ] = pElements[ 3 ];
						sample.Data()[ entry->second.m_Offset + 4 ] = pElements[ 4 ];
						sample.Data()[ entry->second.m_Offset + 5 ] = pElements[ 5 ];
						sample.Data()[ entry->second.m_Offset + 6 ] = pElements[ 6 ];
						sample.Data()[ entry->second.m_Offset + 7 ] = pElements[ 7 ];
						sample.Data()[ entry->second.m_Offset + 8 ] = pElements[ 8 ];
						sample.Data()[ entry->second.m_Offset + 9 ] = pElements[ 9 ];
						sample.Data()[ entry->second.m_Offset + 10 ] = pElements[ 10 ];
						sample.Data()[ entry->second.m_Offset + 11 ] = pElements[ 11 ];
						sample.Data()[ entry->second.m_Offset + 12 ] = pElements[ 12 ];
						sample.Data()[ entry->second.m_Offset + 13 ] = pElements[ 13 ];
						sample.Data()[ entry->second.m_Offset + 14 ] = pElements[ 14 ];
						sample.Data()[ entry->second.m_Offset + 15 ] = pElements[ 15 ];
						break;
					}
					default:
					// left blank to avoid compiler warnings about unhandled
					//  types
					break;
			}
		}
	}
}

//----------------------------------------------------------------------
/** Test if this surface can be occlusion culled. If it can then
 * transfer surface to the next bucket it covers, or delete it if it
 * covers no more.
 * \param pSurface A pointer to a CqSurface derived class.
*/

bool CqBucket::occlusionCullSurface( const boost::shared_ptr<CqSurface>& surface )
{
	const CqBound RasterBound( surface->GetCachedRasterBound() );

	if ( m_bucketData->canCull( RasterBound ) )
	{
		// Surface is behind everying in this bucket but it may be
		// visible in other buckets it overlaps.
		// bucket to the right
		TqInt nextBucket = getCol() + 1;
		CqVector2D pos = QGetRenderContext()->pImage()->BucketPosition( nextBucket, getRow() );
		if ( ( nextBucket < QGetRenderContext()->pImage()->cXBuckets() ) &&
			 ( RasterBound.vecMax().x() >= pos.x() ) )
		{
			QGetRenderContext()->pImage()->Bucket( nextBucket, getRow() ).AddGPrim( surface );
			return true;
		}

		// next row
		nextBucket = getRow() + 1;
		// find bucket containing left side of bound
		TqInt nextBucketX = static_cast<TqInt>( RasterBound.vecMin().x() ) / QGetRenderContext()->pImage()->XBucketSize();
		nextBucketX = max( nextBucketX, 0 );
		pos = QGetRenderContext()->pImage()->BucketPosition( nextBucketX, nextBucket );

		if ( ( nextBucketX < QGetRenderContext()->pImage()->cXBuckets() ) &&
			 ( nextBucket  < QGetRenderContext()->pImage()->cYBuckets() ) &&
			 ( RasterBound.vecMax().y() >= pos.y() ) )
		{
			QGetRenderContext()->pImage()->Bucket( nextBucketX, nextBucket ).AddGPrim( surface );
			return true;
		}

		// Bound covers no more buckets therefore we can delete the surface completely.
		CqString objname( "unnamed" );
		const CqString* pattrName = surface->pAttributes() ->GetStringAttribute( "identifier", "name" );
		if( pattrName )
			objname = *pattrName;
		Aqsis::log() << info << "GPrim: \"" << objname << "\" occlusion culled" << std::endl;
		STATS_INC( GPR_occlusion_culled );
		return true;
	}
	else
	{
		return false;
	}
}


//---------------------------------------------------------------------
/* Pure virtual destructor for CqBucket
 */
CqBucket::~CqBucket()
{
}

} // namespace Aqsis


