// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
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
		\brief Implements the CqBucket class responsible for bookkeeping the primitives and storing the results.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"MultiTimer.h"

#include	"aqsis.h"

#ifdef WIN32
#include    <windows.h>
#endif
#include	<math.h>

#include	"surface.h"
#include	"imagepixel.h"
#include	"bucket.h"

#include	"imagers.h"

#include	<algorithm>
#include	<valarray>




START_NAMESPACE( Aqsis )


//----------------------------------------------------------------------
/** Static data on CqBucket
 */

TqInt	CqBucket::m_XSize;
TqInt	CqBucket::m_YSize;
TqInt	CqBucket::m_RealWidth;
TqInt	CqBucket::m_RealHeight;
TqInt	CqBucket::m_DiscreteShiftX;
TqInt	CqBucket::m_DiscreteShiftY;
TqInt	CqBucket::m_XOrigin;
TqInt	CqBucket::m_YOrigin;
TqInt	CqBucket::m_PixelXSamples;
TqInt	CqBucket::m_PixelYSamples;
TqFloat	CqBucket::m_FilterXWidth;
TqFloat	CqBucket::m_FilterYWidth;
TqInt	CqBucket::m_NumTimeRanges;
TqInt	CqBucket::m_NumDofBounds;
CqImageBuffer* CqBucket::m_ImageBuffer;
std::vector<CqBound>		CqBucket::m_DofBounds;
std::vector<CqImagePixel>	CqBucket::m_aieImage;
std::vector<SqSampleData>	CqBucket::m_SamplePoints;
TqInt	CqBucket::m_NextSamplePoint = 0;
std::vector<std::vector<CqVector2D> >	CqBucket::m_aSamplePositions;
std::vector<TqFloat> CqBucket::m_aFilterValues;
std::vector<TqFloat> CqBucket::m_aDatas;
std::vector<TqFloat> CqBucket::m_aCoverages;


//----------------------------------------------------------------------
/** Initialise the static image storage area.
 *  Clear,Allocate, Init. the m_aieImage samples
 */

void CqBucket::PrepareBucket( TqInt xorigin, TqInt yorigin, TqInt xsize, TqInt ysize, TqBool fJitter, TqBool empty )
{
	m_XOrigin = xorigin;
	m_YOrigin = yorigin;
	m_XSize = xsize;
	m_YSize = ysize;
	m_PixelXSamples = m_ImageBuffer->PixelXSamples();
	m_PixelYSamples = m_ImageBuffer->PixelYSamples();
	m_FilterXWidth = m_ImageBuffer->FilterXWidth();
	m_FilterYWidth = m_ImageBuffer->FilterYWidth();
	m_DiscreteShiftX = FLOOR(m_FilterXWidth/2.0f);
	m_DiscreteShiftY = FLOOR(m_FilterYWidth/2.0f);
	m_RealWidth = m_XSize + (m_DiscreteShiftX*2);
	m_RealHeight = m_YSize + (m_DiscreteShiftY*2);

	m_NumTimeRanges = MAX(4, m_PixelXSamples * m_PixelYSamples);

	// Allocate the image element storage if this is the first bucket
	if(m_aieImage.empty())
	{
		SqImageSample::SetSampleSize(QGetRenderContext() ->GetOutputDataTotalSize());

		m_aieImage.resize( m_RealWidth * m_RealHeight );
		m_aSamplePositions.resize( m_RealWidth * m_RealHeight );
		m_SamplePoints.resize( m_RealWidth * m_RealHeight * m_PixelXSamples * m_PixelYSamples );
		m_NextSamplePoint = 0;

		CalculateDofBounds();

		// Initialise the samples for this bucket.
		TqInt which = 0;
		for ( TqInt i = 0; i < m_RealHeight; i++ )
		{
			for ( TqInt j = 0; j < m_RealWidth; j++ )
			{
				m_aieImage[which].Clear();
				m_aieImage[which].AllocateSamples( m_PixelXSamples, m_PixelYSamples );
				m_aieImage[which].InitialiseSamples( m_aSamplePositions[which] );
				if(fJitter)
					m_aieImage[which].JitterSamples(m_aSamplePositions[which]);

				which++;
			}
		}
	}

	std::random_shuffle(m_aieImage.begin(), m_aieImage.end());

	// Jitter the samplepoints and adjust them for the new bucket position.
	TqInt which = 0;
	TqInt numPixels = m_RealWidth*m_RealHeight;
	for ( TqInt i = 0; i < m_RealHeight; i++ )
	{
		for ( TqInt j = 0; j < m_RealWidth; j++ )
		{
			CqVector2D bPos2( m_XOrigin, m_YOrigin );
			bPos2 += CqVector2D( ( j - m_DiscreteShiftX ), ( i - m_DiscreteShiftY ) );

			if(!empty)
				m_aieImage[which].Clear();

			//if(fJitter)
			//	m_aieImage[which].JitterSamples(m_aSamplePositions[which]);
			m_aieImage[which].OffsetSamples( bPos2, m_aSamplePositions[which] );

			which++;
		}
	}
}


void CqBucket::CalculateDofBounds()
{
	m_NumDofBounds = m_PixelXSamples * m_PixelYSamples;
	m_DofBounds.resize(m_NumDofBounds);

	TqFloat dx = 2.0 / m_PixelXSamples;
	TqFloat dy = 2.0 / m_PixelYSamples;

	// I know this is far from an optimal way of calculating this,
	// but it's only done once so I don't care.
	// Calculate the bounding boxes that the dof offset positions fall into.
	TqFloat minX = -1.0;
	TqFloat minY = -1.0;
	TqInt which = 0;
	for(int j = 0; j < m_PixelYSamples; ++j)
	{
		for(int i = 0; i < m_PixelXSamples; ++i)
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

			m_DofBounds[which].vecMin() = topLeft;
			m_DofBounds[which].vecMax() = topLeft;
			m_DofBounds[which].Encapsulate(topRight);
			m_DofBounds[which].Encapsulate(bottomLeft);
			m_DofBounds[which].Encapsulate(bottomRight);

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
	if( !m_aFilterValues.empty() )
		return;

	// Allocate and fill in the filter values array for each pixel.
	TqInt numsubpixels = ( PixelXSamples() * PixelYSamples() );
	TqInt numperpixel = numsubpixels * numsubpixels;

	TqUint numvalues = static_cast<TqUint>( ( ( CEIL(FilterXWidth()) + 1 ) * ( CEIL(FilterYWidth()) + 1 ) ) * ( numperpixel ) );

	m_aFilterValues.resize( numvalues );

	RtFilterFunc pFilter;
	pFilter = QGetRenderContext() ->optCurrent().funcFilter();

	// Sanity check
	if( NULL == pFilter )
		pFilter = RiBoxFilter;

	TqFloat xmax = m_DiscreteShiftX;
	TqFloat ymax = m_DiscreteShiftY;
	TqFloat xfwo2 = CEIL(FilterXWidth()) * 0.5f;
	TqFloat yfwo2 = CEIL(FilterYWidth()) * 0.5f;
	TqFloat xfw = CEIL(FilterXWidth());

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
								w = ( *pFilter ) ( fx, fy, CEIL(FilterXWidth()), CEIL(FilterYWidth()) );
							m_aFilterValues[ cindex ] = w;
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

void CqBucket::CombineElements(enum EqFilterDepth filterdepth, CqColor zThreshold)
{
	std::vector<CqImagePixel>::iterator end = m_aieImage.end();
	for ( std::vector<CqImagePixel>::iterator i = m_aieImage.begin(); i != end ; i++ )
		i->Combine(filterdepth, zThreshold);
}


//----------------------------------------------------------------------
/** Get the sample color for the specified screen position.
 * If position is outside bucket, returns black.
 * \param iXPos Screen position of sample.
 * \param iYPos Screen position of sample.
 */

CqColor CqBucket::Color( TqInt iXPos, TqInt iYPos )
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

CqColor CqBucket::Opacity( TqInt iXPos, TqInt iYPos )
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

TqFloat CqBucket::Coverage( TqInt iXPos, TqInt iYPos )
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

TqFloat CqBucket::Depth( TqInt iXPos, TqInt iYPos )
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

const TqFloat* CqBucket::Data( TqInt iXPos, TqInt iYPos )
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

void CqBucket::FilterBucket(TqBool empty)
{
	CqImagePixel * pie;

	TqInt datasize = QGetRenderContext()->GetOutputDataTotalSize();
	m_aDatas.resize( datasize * RealWidth() * RealHeight() );
	m_aCoverages.resize( RealWidth() * RealHeight() );

	TqInt xmax = m_DiscreteShiftX;
	TqInt ymax = m_DiscreteShiftY;
	TqFloat xfwo2 = CEIL(FilterXWidth()) * 0.5f;
	TqFloat yfwo2 = CEIL(FilterYWidth()) * 0.5f;
	TqInt numsubpixels = ( PixelXSamples() * PixelYSamples() );

	TqInt numperpixel = numsubpixels * numsubpixels;
	TqInt	xlen = RealWidth();

	TqInt SampleCount = 0;
	CqColor imager;

	TqInt x, y;
	TqInt i = 0;

	TqBool fImager = TqFalse;
	const CqString* systemOptions;
	if( ( systemOptions = QGetRenderContext() ->optCurrent().GetStringOption( "System", "Imager" ) ) != 0 )
		if( systemOptions[ 0 ].compare("null") != 0 )
			fImager = TqTrue;

	TqInt endy = YOrigin() + Height();
	TqInt endx = XOrigin() + Width();

	bool useSeperable = true;


	if(!empty)
	{
		// non-seperable is faster for very small filter widths.
		if(FilterXWidth() <= 1.0 || FilterYWidth() <= 1.0)
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
							TqInt index = ( ( ymax * CEIL(FilterXWidth()) ) + ( fx + xmax ) ) * numperpixel;
							// Now go over each subsample within the pixel
							TqInt sampleIndex = sy * PixelXSamples();
							TqInt sindex = index + ( sy * PixelXSamples() * numsubpixels );

							for ( TqInt sx = 0; sx < PixelXSamples(); sx++ )
							{
								const SqSampleData& sampleData = pie2->SampleData( sampleIndex );
								CqVector2D vecS = sampleData.m_Position;
								vecS -= CqVector2D( xcent, ycent );
								if ( vecS.x() >= -xfwo2 && vecS.y() >= -yfwo2 && vecS.x() <= xfwo2 && vecS.y() <= yfwo2 )
								{
									TqInt cindex = sindex + sampleData.m_SubCellIndex;
									TqFloat g = m_aFilterValues[ cindex ];
									gTot += g;
									if ( pie2->OpaqueValues( sampleIndex ).m_flags & SqImageSample::Flag_Valid )
									{
										SqImageSample& pSample = pie2->OpaqueValues( sampleIndex );
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

						TqInt index = ( ( ( fy + ymax ) * CEIL(FilterXWidth()) ) + xmax ) * numperpixel;
						// Now go over each y subsample within the pixel
						TqInt sx = PixelXSamples() / 2; // use the samples in the centre of the pixel.
						TqInt sy = 0;
						TqInt sampleIndex = sx;
						TqInt pixelRow = (y + fy - (YOrigin()-ymax)) * PixelYSamples();
						TqInt pixelIndex = pixelRow*Width() + x-XOrigin();

						for ( sy = 0; sy < PixelYSamples(); sy++ )
						{
							TqInt sindex = index + ( ( ( sy * PixelXSamples() ) + sx ) * numsubpixels );
							const SqSampleData& sampleData = pie2->SampleData( sampleIndex );
							CqVector2D vecS = sampleData.m_Position;
							vecS -= CqVector2D( xcent, ycent );
							if ( vecS.x() >= -xfwo2 && vecS.y() >= -yfwo2 && vecS.x() <= xfwo2 && vecS.y() <= yfwo2 )
							{
								TqInt cindex = sindex + sampleData.m_SubCellIndex;
								TqFloat g = m_aFilterValues[ cindex ];
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
						memset(&m_aDatas[i*datasize], 0, datasize * sizeof(float));
						m_aDatas[ i*datasize+6 ] = FLT_MAX;
						m_aCoverages[i] = 0.0;
					}
					else
					{
						float oneOverGTot = 1.0 / gTot;
						for ( TqInt k = 0; k < datasize; k ++)
							m_aDatas[ i*datasize + k ] = samples[k] * oneOverGTot;

						if ( SampleCount >= numsubpixels)
							m_aCoverages[ i ] = 1.0;
						else
							m_aCoverages[ i ] = ( TqFloat ) SampleCount / ( TqFloat ) (numsubpixels );
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
							TqInt index = ( ( ( fy + ymax ) * CEIL(FilterXWidth()) ) + ( fx + xmax ) ) * numperpixel;
							// Now go over each subsample within the pixel
							TqInt sx, sy;
							TqInt sampleIndex = 0;
							for ( sy = 0; sy < PixelYSamples(); sy++ )
							{
								for ( sx = 0; sx < PixelXSamples(); sx++ )
								{
									TqInt sindex = index + ( ( ( sy * PixelXSamples() ) + sx ) * numsubpixels );
									const SqSampleData& sampleData = pie2->SampleData( sampleIndex );
									CqVector2D vecS = sampleData.m_Position;
									vecS -= CqVector2D( xcent, ycent );
									if ( vecS.x() >= -xfwo2 && vecS.y() >= -yfwo2 && vecS.x() <= xfwo2 && vecS.y() <= yfwo2 )
									{
										TqInt cindex = sindex + sampleData.m_SubCellIndex;
										TqFloat g = m_aFilterValues[ cindex ];
										gTot += g;
										if ( pie2->OpaqueValues( sampleIndex ).m_flags & SqImageSample::Flag_Valid )
										{
											SqImageSample& pSample = pie2->OpaqueValues( sampleIndex );
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
						memset(&m_aDatas[i*datasize], 0, datasize * sizeof(float));
						m_aDatas[ i*datasize+6 ] = FLT_MAX;
						m_aCoverages[i] = 0.0;
					}
					else
					{
						float oneOverGTot = 1.0 / gTot;
						for ( TqInt k = 0; k < datasize; k ++)
							m_aDatas[ i*datasize + k ] = samples[k] * oneOverGTot;

						if ( SampleCount >= numsubpixels)
							m_aCoverages[ i ] = 1.0;
						else
							m_aCoverages[ i ] = ( TqFloat ) SampleCount / ( TqFloat ) (numsubpixels );
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
		memset(&m_aDatas[0], 0, size * datasize * sizeof(float));
		memset(&m_aCoverages[0], 0, size * sizeof(float));
		for(i = 0; i<size; ++i)
		{
			// Set the depth to infinity.
			m_aDatas[ i*datasize+6 ] = FLT_MAX;
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
				spl.Data()[k] = m_aDatas[ i * datasize + k ];
			TqFloat* sample_data = spl.Data();
			sample_data[Sample_Coverage] = m_aCoverages[ i++ ];

			// Calculate the alpha as the combination of the opacity and the coverage.
			TqFloat a = ( sample_data[Sample_ORed] + sample_data[Sample_OGreen] + sample_data[Sample_OBlue] ) / 3.0f;
			pie2->SetAlpha(a * sample_data[Sample_Coverage]);

			pie2++;
		}
		pie += xlen;
	}

	endy = YOrigin() + Height();
	endx = XOrigin() + Width();

	if ( NULL != QGetRenderContext() ->optCurrent().pshadImager() && QGetRenderContext() ->optCurrent().pshadImager() ->pShader() )
	{
		// Init & Execute the imager shader

		QGetRenderContext() ->optCurrent().InitialiseColorImager( this );
		TIME_SCOPE("Imager shading")

		if ( fImager )
		{
			i = 0;
			ImageElement( XOrigin(), YOrigin(), pie );
			for ( y = YOrigin(); y < endy ; y++ )
			{
				CqImagePixel* pie2 = pie;
				for ( x = XOrigin(); x < endx ; x++ )
				{
					imager = QGetRenderContext() ->optCurrent().GetColorImager( x , y );
					// Normal case will be to poke the alpha from the image shader and
					// multiply imager color with it... but after investigation alpha is always
					// == 1 after a call to imager shader in 3delight and BMRT.
					// Therefore I did not ask for alpha value and set directly the pCols[i]
					// with imager value. see imagers.cpp
					pie2->SetColor( imager );
					imager = QGetRenderContext() ->optCurrent().GetOpacityImager( x , y );
					pie2->SetOpacity( imager );
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
	if ( QGetRenderContext() ->optCurrent().GetFloatOption( "System", "Exposure" ) [ 0 ] == 1.0 &&
	        QGetRenderContext() ->optCurrent().GetFloatOption( "System", "Exposure" ) [ 1 ] == 1.0 )
		return ;
	else
	{
		CqImagePixel* pie;
		ImageElement( XOrigin(), YOrigin(), pie );
		TqInt x, y;
		TqFloat exposegain = QGetRenderContext() ->optCurrent().GetFloatOption( "System", "Exposure" ) [ 0 ];
		TqFloat exposegamma = QGetRenderContext() ->optCurrent().GetFloatOption( "System", "Exposure" ) [ 1 ];
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


//----------------------------------------------------------------------
/** Quantize the samples in this bucket according to type.
 */

void CqBucket::QuantizeBucket()
{
	// Initiliaze the random with a value based on the X,Y coordinate
	static CqRandom random( 61 );
	TqFloat endx, endy;
	TqInt   nextx;
	endy = Height();
	endx = Width();
	nextx = RealWidth();


	if ( QGetRenderContext() ->optCurrent().GetIntegerOption( "System", "DisplayMode" ) [ 0 ] & ModeRGB )
	{
		const TqFloat* pQuant = QGetRenderContext() ->optCurrent().GetFloatOption( "Quantize", "Color" );
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
				r = CLAMP( r, min, max );
				g = CLAMP( g, min, max );
				b = CLAMP( b, min, max );
				_or = CLAMP( _or, min, max );
				_og = CLAMP( _og, min, max );
				_ob = CLAMP( _ob, min, max );
				a = CLAMP( a, min, max );
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

	if ( QGetRenderContext() ->optCurrent().GetIntegerOption( "System", "DisplayMode" ) [ 0 ] & ModeZ )
	{
		const TqFloat* pQuant = QGetRenderContext() ->optCurrent().GetFloatOption( "Quantize", "Depth" );
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
				d = CLAMP( d, min, max );
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
		const TqFloat* pQuant = QGetRenderContext() ->optCurrent().GetFloatOption( "Quantize", entry->first.c_str() );
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
						d = CLAMP( d, min, max );
						pie2->GetPixelSample().Data()[sampleindex] = d;
					}
					pie2++;
				}
				pie += nextx;
			}
		}
	}
}

//----------------------------------------------------------------------
/** Clear any data on the bucket
 */
void CqBucket::ShutdownBucket()
{
	m_aieImage.clear();
	m_aFilterValues.clear();
	m_aCoverages.clear();
	m_aDatas.clear();
	std::vector<std::vector<CqVector2D> >::iterator i;
	for( i=m_aSamplePositions.begin(); i!=m_aSamplePositions.end(); i++ )
		(*i).clear();
	m_aSamplePositions.clear();
	m_SamplePoints.clear();
}


//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )

