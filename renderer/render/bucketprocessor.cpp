/* Aqsis - bucketprocessor.cpp
 *
 * Copyright (C) 2007 Manuel A. Fernadez Montecelo <mafm@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include	"bucket.h"
#include	"multitimer.h"
#include	"imagebuffer.h"
#include	"aqsismath.h"

#include	"bucketprocessor.h"


namespace Aqsis {


CqBucketProcessor::CqBucketProcessor() :
	m_bucket(0),
	m_hasValidSamples(false)
{
}

CqBucketProcessor::~CqBucketProcessor()
{
}

void CqBucketProcessor::setBucket(CqBucket* bucket)
{
	assert(m_bucket == 0);

	m_bucket = bucket;
	m_hasValidSamples = false;
}

const CqBucket* CqBucketProcessor::getBucket() const
{
	return m_bucket;
}

void CqBucketProcessor::reset()
{
	if (!m_bucket)
		return;

	assert(m_bucket && m_bucket->IsProcessed());

	m_bucket = 0;
	m_hasValidSamples = false;
}

void CqBucketProcessor::preProcess(TqInt xMin, TqInt yMin, TqInt xMax, TqInt yMax,
				   TqInt pixelXSamples, TqInt pixelYSamples, TqFloat filterXWidth, TqFloat filterYWidth,
				   TqInt viewRangeXMin, TqInt viewRangeXMax, TqInt viewRangeYMin, TqInt viewRangeYMax,
				   TqFloat clippingNear, TqFloat clippingFar)
{
	assert(m_bucket);

	{
		AQSIS_TIME_SCOPE(Prepare_bucket);

		m_PixelXSamples = pixelXSamples;
		m_PixelYSamples = pixelYSamples;
		m_FilterXWidth = filterXWidth;
		m_FilterYWidth = filterYWidth;

		m_DRegion = CqRegion( xMin, yMin, xMax, yMax );

		m_DiscreteShiftX = lfloor(m_FilterXWidth/2.0f);
		m_DiscreteShiftY = lfloor(m_FilterYWidth/2.0f);

		TqInt sminx = xMin - m_DiscreteShiftX;
		TqInt sminy = yMin - m_DiscreteShiftY;
		TqInt smaxx = xMax + m_DiscreteShiftX;
		TqInt smaxy = yMax + m_DiscreteShiftY;

		if ( sminx < QGetRenderContext()->cropWindowXMin() - m_DiscreteShiftX )
			sminx = static_cast<TqInt>(QGetRenderContext()->cropWindowXMin() - m_DiscreteShiftX );
		if ( sminy < QGetRenderContext()->cropWindowYMin() - m_DiscreteShiftY )
			sminy = static_cast<TqInt>(QGetRenderContext()->cropWindowYMin() - m_DiscreteShiftY );
		if ( smaxx > QGetRenderContext()->cropWindowXMax() + m_DiscreteShiftX )
			smaxx = static_cast<TqInt>(QGetRenderContext()->cropWindowXMax() + m_DiscreteShiftX);
		if ( smaxy > QGetRenderContext()->cropWindowYMax() + m_DiscreteShiftY )
			smaxy = static_cast<TqInt>(QGetRenderContext()->cropWindowYMax() + m_DiscreteShiftY );
		m_SRegion = CqRegion( sminx, sminy, smaxx, smaxy );

		m_clippingNear = clippingNear;
		m_clippingFar = clippingFar;

		TqFloat opentime = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Shutter" ) [ 0 ];
		TqFloat closetime = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Shutter" ) [ 1 ];

		// Allocate the image element storage if this is the first bucket
		if(m_aieImage.empty())
		{
			SqImageSample::SetSampleSize(QGetRenderContext() ->GetOutputDataTotalSize());

			m_aieImage.resize( SRegion().area() );
			m_aSamplePositions.resize( SRegion().area() );
			m_SamplePoints.resize( SRegion().area() * PixelXSamples() * PixelYSamples() );
			m_NextSamplePoint = 0;

			CalculateDofBounds();

			// Initialise the samples for this bucket.
			TqInt which = 0;
			TqInt maxY = SRegion().height();
			TqInt maxX = SRegion().width();
			for ( TqInt i = 0; i < maxY; i++ )
			{
				for ( TqInt j = 0; j < maxX; j++ )
				{
					m_aieImage[which].Clear( m_SamplePoints );
					m_aieImage[which].AllocateSamples( this,
											 PixelXSamples(),
											 PixelYSamples() );
					m_aieImage[which].InitialiseSamples( m_SamplePoints,
											   m_aSamplePositions[which] );
					//if(fJitter)
					m_aieImage[which].JitterSamples( m_SamplePoints,
											   m_aSamplePositions[which],
											   opentime, closetime);

					which++;
				}
			}
		}

		// Shuffle the Sample and DOF positions 
		std::vector<CqImagePixel>::iterator itPix;
		TqInt size = m_aieImage.size();  
		TqInt i = 0;
		if (size > 1)
		{
			CqRandom rand(19);
			for( itPix = m_aieImage.begin(), i=0 ; itPix <= m_aieImage.end(), i < size - 1; itPix++, i++)
			{
				TqInt other = i + rand.RandomInt(size - i);
				if (other >= size) other = size - 1;
				(*itPix).m_SampleIndices.swap(m_aieImage[other].m_SampleIndices);  
				(*itPix).m_DofOffsetIndices.swap(m_aieImage[other].m_DofOffsetIndices); 
			}
		}

		// Jitter the samplepoints and adjust them for the new bucket position.
		TqInt which = 0;
		TqInt maxY = SRegion().height();
		TqInt maxX = SRegion().width();
		for ( TqInt ii = 0; ii < maxY; ii++ )
		{
			for ( TqInt j = 0; j < maxX; j++ )
			{
				CqVector2D bPos2( DRegion().xMin(), DRegion().yMin() );
				bPos2 += CqVector2D( ( j - m_DiscreteShiftX ), ( ii - m_DiscreteShiftY ) );

				m_aieImage[which].Clear( m_SamplePoints );

				//if(fJitter)
				m_aieImage[which].JitterSamples( m_SamplePoints,
										   m_aSamplePositions[which],
										   opentime, closetime);
				m_aieImage[which].OffsetSamples( m_SamplePoints,
										   bPos2,
										   m_aSamplePositions[which] );

				which++;
			}
		}
		InitialiseFilterValues();
	}

	{
		AQSIS_TIME_SCOPE(Occlusion_culling_initialisation);
		m_OcclusionTree.setupTree(this, SRegion().xMin(), SRegion().yMin(), SRegion().xMax(), SRegion().yMax());
	}
}

void CqBucketProcessor::process()
{
	if (!m_bucket)
		return;

	{
		AQSIS_TIME_SCOPE(Render_MPGs);
		RenderWaitingMPs();
	}

	// Render any waiting subsurfaces.
	// \todo Need to refine the exit condition, to ensure that all previous buckets have been
	// duly processed.
	while ( m_bucket->hasPendingSurfaces() )
	{
		boost::shared_ptr<CqSurface> surface = m_bucket->pTopSurface();
		if (surface)
		{
			// Advance to next surface
			m_bucket->popSurface();
			RenderSurface( surface );
			{
				AQSIS_TIME_SCOPE(Render_MPGs);
				RenderWaitingMPs();
			}
		}
	}
	{
		AQSIS_TIME_SCOPE(Render_MPGs);
		RenderWaitingMPs();
	}
}

void CqBucketProcessor::postProcess( bool imager, EqFilterDepth depthfilter, const CqColor& zThreshold )
{
	if (!m_bucket)
		return;

	// Combine the colors at each pixel sample for any
	// micropolygons rendered to that pixel.
	{
		AQSIS_TIME_SCOPE(Combine_samples);
		CombineElements(depthfilter, zThreshold);
	}

	{
		AQSIS_TIME_SCOPE(Filter_samples);
		FilterBucket(imager);
		ExposeBucket();
		// \note: Used to quantize here too, but not any more, as it is handled by
		//	  ddmanager in a specific way for each display.
	}

	assert(!m_bucket->IsProcessed());
	m_bucket->SetProcessed();
}

//----------------------------------------------------------------------
/** Combine the subsamples into single pixel samples and coverage information.
 */

void CqBucketProcessor::CombineElements(enum EqFilterDepth filterdepth, CqColor zThreshold)
{
	for ( std::vector<CqImagePixel>::iterator i = m_aieImage.begin(), end = m_aieImage.end(); i != end ; i++ )
		i->Combine(m_SamplePoints, filterdepth, zThreshold);
}

//----------------------------------------------------------------------
/** Filter the samples in this bucket according to type and filter widths.
 */

void CqBucketProcessor::FilterBucket(bool fImager)
{
	CqImagePixel * pie;

	std::map<TqInt, CqRenderer::SqOutputDataEntry> channelMap;
	// Setup the channel buffer ready to accept the output data.
	// First fill in the default display value r, g, b, a, and z.
	m_channelBuffer.clearChannels();
	channelMap[m_channelBuffer.addChannel("r", 1)] = CqRenderer::SqOutputDataEntry(Sample_Red, 1, type_float);
	channelMap[m_channelBuffer.addChannel("g", 1)] = CqRenderer::SqOutputDataEntry(Sample_Green, 1, type_float);
	channelMap[m_channelBuffer.addChannel("b", 1)] = CqRenderer::SqOutputDataEntry(Sample_Blue, 1, type_float);
	channelMap[m_channelBuffer.addChannel("or", 1)] = CqRenderer::SqOutputDataEntry(Sample_ORed, 1, type_float);
	channelMap[m_channelBuffer.addChannel("og", 1)] = CqRenderer::SqOutputDataEntry(Sample_OGreen, 1, type_float);
	channelMap[m_channelBuffer.addChannel("ob", 1)] = CqRenderer::SqOutputDataEntry(Sample_OBlue, 1, type_float);
	channelMap[m_channelBuffer.addChannel("a", 1)] = CqRenderer::SqOutputDataEntry(Sample_Alpha, 1, type_float);
	channelMap[m_channelBuffer.addChannel("z", 1)] = CqRenderer::SqOutputDataEntry(Sample_Depth, 1, type_float);
	channelMap[m_channelBuffer.addChannel("coverage", 1)] = CqRenderer::SqOutputDataEntry(Sample_Coverage, 1, type_float);
	std::map<std::string, CqRenderer::SqOutputDataEntry>& outputMap = QGetRenderContext()->GetMapOfOutputDataEntries();
	std::map<std::string, CqRenderer::SqOutputDataEntry>::iterator aov_i = outputMap.begin();
	std::map<std::string, CqRenderer::SqOutputDataEntry>::iterator aov_end = outputMap.end();
	for(; aov_i != aov_end; ++aov_i)
		channelMap[m_channelBuffer.addChannel(aov_i->first, aov_i->second.m_NumSamples)] = aov_i->second;

	TqInt depthIndex = m_channelBuffer.getChannelIndex("z");

	m_channelBuffer.allocate(DRegion().width(), DRegion().height());
	TqInt datasize = QGetRenderContext()->GetOutputDataTotalSize();

	m_aDatas.resize( datasize * SRegion().area() );
	m_aCoverages.resize( SRegion().area() );

	TqInt xmax = m_DiscreteShiftX;
	TqInt ymax = m_DiscreteShiftY;
	TqFloat xfwo2 = std::ceil(FilterXWidth()) * 0.5f;
	TqFloat yfwo2 = std::ceil(FilterYWidth()) * 0.5f;
	TqInt numsubpixels = ( PixelXSamples() * PixelYSamples() );

	TqInt numperpixel = numsubpixels * numsubpixels;
	TqInt	xlen = SRegion().width();

	TqInt SampleCount = 0;

	TqInt x, y;
	TqInt i = 0;

	TqInt endy = DRegion().yMax();
	TqInt endx = DRegion().xMax();

	bool useSeperable = true;

	if(m_hasValidSamples)
	{
		// non-seperable is faster for very small filter widths.
		if(FilterXWidth() <= 16.0 || FilterYWidth() <= 16.0)
			useSeperable = false;

		if(useSeperable)
		{
			// seperable filter. filtering by fx,fy is equivalent to filtering
			// by fx,1 followed by 1,fy.

			TqInt size = DRegion().width() * SRegion().height() * PixelYSamples();
			std::valarray<TqFloat> intermediateSamples( 0.0f, size * datasize);
			std::valarray<TqInt> sampleCounts(0, size);
			for ( y = DRegion().yMin() - ymax; y < endy + ymax ; y++ )
			{
				TqFloat ycent = y + 0.5f;
				TqInt pixelRow = (y-(DRegion().yMin()-ymax)) * PixelYSamples();
				for ( x = DRegion().xMin(); x < endx ; x++ )
				{
					TqFloat xcent = x + 0.5f;

					// Get the element at the left side of the filter area.
					ImageElement( x - xmax, y, pie );

					TqInt pixelIndex = pixelRow*DRegion().width() + x-DRegion().xMin();

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
								const SqSampleData& sampleData = pie2->SampleData( m_SamplePoints,
														   sampleIndex );
								CqVector2D vecS = sampleData.m_Position;
								vecS -= CqVector2D( xcent, ycent );
								if ( vecS.x() >= -xfwo2 && vecS.y() >= -yfwo2 && vecS.x() <= xfwo2 && vecS.y() <= yfwo2 )
								{
									TqInt cindex = sindex + sampleData.m_SubCellIndex;
									TqFloat g = m_aFilterValues[ cindex ];
									gTot += g;
									if ( pie2->OpaqueValues( m_SamplePoints, sampleIndex ).isValid() )
									{
										SqImageSample& pSample = pie2->OpaqueValues( m_SamplePoints,
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

						pixelIndex += DRegion().width();
					}
				}
			}

			// now filter in y.
			for ( y = DRegion().yMin(); y < endy ; y++ )
			{
				TqFloat ycent = y + 0.5f;
				for ( x = DRegion().xMin(); x < endx ; x++ )
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
						TqInt pixelRow = (y + fy - (DRegion().yMin()-ymax)) * PixelYSamples();
						TqInt pixelIndex = pixelRow*DRegion().width() + x-DRegion().xMin();

						for ( sy = 0; sy < PixelYSamples(); sy++ )
						{
							TqInt sindex = index + ( ( ( sy * PixelXSamples() ) + sx ) * numsubpixels );
							const SqSampleData& sampleData = pie2->SampleData( m_SamplePoints,
													   sampleIndex );
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
							pixelIndex += DRegion().width();
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

						// Copy the filtered sample data into the channel buffer.
						for( std::map<TqInt, CqRenderer::SqOutputDataEntry>::iterator channel_i = channelMap.begin(); channel_i != channelMap.end(); ++channel_i )
						{
							for(TqInt i = 0; i < channel_i->second.m_NumSamples; ++i)
								m_channelBuffer(x, y, channel_i->first)[i] = samples[channel_i->second.m_Offset + i] * oneOverGTot;
						}

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
			for ( y = DRegion().yMin(); y < endy ; y++ )
			{
				TqFloat ycent = y + 0.5f;
				for ( x = DRegion().xMin(); x < endx ; x++ )
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
									const SqSampleData& sampleData = pie2->SampleData( m_SamplePoints,
															   sampleIndex );
									CqVector2D vecS = sampleData.m_Position;
									vecS -= CqVector2D( xcent, ycent );
									if ( vecS.x() >= -xfwo2 && vecS.y() >= -yfwo2 && vecS.x() <= xfwo2 && vecS.y() <= yfwo2 )
									{
										TqInt cindex = sindex + sampleData.m_SubCellIndex;
										TqFloat g = m_aFilterValues[ cindex ];
										gTot += g;
										if ( pie2->OpaqueValues( m_SamplePoints, sampleIndex ).isValid() )
										{
											SqImageSample& pSample = pie2->OpaqueValues( m_SamplePoints,
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
						for( std::map<TqInt, CqRenderer::SqOutputDataEntry>::iterator channel_i = channelMap.begin(); channel_i != channelMap.end(); ++channel_i )
						{
							for(TqInt i = 0; i < channel_i->second.m_NumSamples; ++i)
								m_channelBuffer(x-DRegion().xMin(), y-DRegion().yMin(), channel_i->first)[i] = 0.0f;
						}
						// Set the depth to infinity.
						m_channelBuffer(x-DRegion().xMin(), y-DRegion().yMin(), depthIndex)[0] = FLT_MAX;
						m_aCoverages[i] = 0.0;
					}
					else
					{
						float oneOverGTot = 1.0 / gTot;
						
						// Copy the filtered sample data into the channel buffer.
						for( std::map<TqInt, CqRenderer::SqOutputDataEntry>::iterator channel_i = channelMap.begin(); channel_i != channelMap.end(); ++channel_i )
						{
							for(TqInt i = 0; i < channel_i->second.m_NumSamples; ++i)
								m_channelBuffer(x-DRegion().xMin(), y-DRegion().yMin(), channel_i->first)[i] = samples[channel_i->second.m_Offset + i] * oneOverGTot;
						}

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
		// Copy the filtered sample data into the channel buffer.
		for(TqInt y = 0; y < DRegion().height(); ++y)
		{
			for(TqInt x = 0; x < DRegion().width(); ++x)
			{
				for( std::map<TqInt, CqRenderer::SqOutputDataEntry>::iterator channel_i = channelMap.begin(); channel_i != channelMap.end(); ++channel_i )
				{
					for(TqInt i = 0; i < channel_i->second.m_NumSamples; ++i)
						m_channelBuffer(x, y, channel_i->first)[i] = 0.0f;
				}
				// Set the depth to infinity.
				m_channelBuffer(x, y, depthIndex)[0] = FLT_MAX;
				m_aCoverages[ i++ ] = 0.0f;
			}
		}
	}

	i = 0;
	endy = DRegion().height();
	endx = DRegion().width();

	// Set the coverage and alpha values for the pixel.
	TqInt redIndex = m_channelBuffer.getChannelIndex("r");
	TqInt greenIndex = m_channelBuffer.getChannelIndex("g");
	TqInt blueIndex = m_channelBuffer.getChannelIndex("b");
	TqInt redOIndex = m_channelBuffer.getChannelIndex("or");
	TqInt greenOIndex = m_channelBuffer.getChannelIndex("og");
	TqInt blueOIndex = m_channelBuffer.getChannelIndex("ob");
	TqInt alphaIndex = m_channelBuffer.getChannelIndex("a");
	TqInt coverageIndex = m_channelBuffer.getChannelIndex("coverage");
	for ( y = 0; y < endy; y++ )
	{
		for ( x = 0; x < endx; x++ )
		{
			TqFloat coverage = m_aCoverages[ i++ ];

			// Calculate the alpha as the combination of the opacity and the coverage.
			TqFloat a = ( m_channelBuffer(x, y, redOIndex)[0] + m_channelBuffer(x, y, greenOIndex)[0] + m_channelBuffer(x, y, blueOIndex)[0] ) / 3.0f;
			m_channelBuffer(x, y, alphaIndex)[0] = a * coverage;
			m_channelBuffer(x, y, coverageIndex)[0] = coverage;
		}
	}

	endy = DRegion().height();
	endx = DRegion().width();

	if ( QGetRenderContext() ->poptCurrent()->pshadImager() )
	{
		// Init & Execute the imager shader

		QGetRenderContext() ->poptCurrent()->InitialiseColorImager( DRegion(), &m_channelBuffer );
		AQSIS_TIME_SCOPE(Imager_shading);

		if ( fImager )
		{
			CqColor imager;
			for ( y = 0; y < endy ; y++ )
			{
				for ( x = 0; x < endx ; x++ )
				{
					imager = QGetRenderContext() ->poptCurrent()->GetColorImager( x+DRegion().xMin() , y+DRegion().yMin() );
					// Normal case will be to poke the alpha from the image shader and
					// multiply imager color with it... but after investigation alpha is always
					// == 1 after a call to imager shader in 3delight and BMRT.
					// Therefore I did not ask for alpha value and set directly the pCols[i]
					// with imager value. see imagers.cpp
					m_channelBuffer(x, y, redIndex)[0] = imager.r();
					m_channelBuffer(x, y, greenIndex)[0] = imager.g();
					m_channelBuffer(x, y, blueIndex)[0] = imager.b();
					imager = QGetRenderContext() ->poptCurrent()->GetOpacityImager( x+DRegion().xMin() , y+DRegion().yMin() );
					m_channelBuffer(x, y, redOIndex)[0] = imager.r();
					m_channelBuffer(x, y, greenOIndex)[0] = imager.g();
					m_channelBuffer(x, y, blueOIndex)[0] = imager.b();
					TqFloat a = ( imager[0] + imager[1] + imager[2] ) / 3.0f;
					m_channelBuffer(x, y, alphaIndex)[0] = a;
				}
			}
		}
	}
}

void CqBucketProcessor::ImageElement( TqInt iXPos, TqInt iYPos, CqImagePixel*& pie )
{
	iXPos -= DRegion().xMin();
	iYPos -= DRegion().yMin();
	
	// Check within renderable range
	//assert( iXPos < -m_XMax && iXPos < m_XSize + m_XMax &&
	//	iYPos < -m_YMax && iYPos < m_YSize + m_YMax );

	TqInt i = (iYPos + m_DiscreteShiftY)*SRegion().width() + iXPos + m_DiscreteShiftX;

	assert(i < static_cast<TqInt>(m_aieImage.size()));
	assert(i >= 0);
	pie = &m_aieImage[ i ];
}

//----------------------------------------------------------------------
/** Expose the samples in this bucket according to specified gain and gamma settings.
 */

void CqBucketProcessor::ExposeBucket()
{
	if(m_hasValidSamples)
	{
		if ( QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Exposure" ) [ 0 ] == 1.0 &&
			 QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Exposure" ) [ 1 ] == 1.0 )
			return ;
		else
		{
			TqFloat exposegain = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Exposure" ) [ 0 ];
			TqFloat exposegamma = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Exposure" ) [ 1 ];
			TqFloat oneovergamma = 1.0f / exposegamma;
			TqFloat endx, endy;
			endy = DRegion().height();
			endx = DRegion().width();
			TqInt r_index = m_channelBuffer.getChannelIndex("r");
			TqInt g_index = m_channelBuffer.getChannelIndex("g");
			TqInt b_index = m_channelBuffer.getChannelIndex("b");

			TqInt x, y;
			for ( y = 0; y < endy; y++ )
			{
				for ( x = 0; x < endx; x++ )
				{
					// color=(color*gain)^1/gamma
					if ( exposegain != 1.0 )
					{
						m_channelBuffer(x, y, r_index)[0] *= exposegain;
						m_channelBuffer(x, y, g_index)[0] *= exposegain;
						m_channelBuffer(x, y, b_index)[0] *= exposegain;
					}

					if ( exposegamma != 1.0 )
					{
						m_channelBuffer(x, y, r_index)[0] = pow(m_channelBuffer(x, y, r_index)[0], oneovergamma);
						m_channelBuffer(x, y, g_index)[0] = pow(m_channelBuffer(x, y, g_index)[0], oneovergamma);
						m_channelBuffer(x, y, b_index)[0] = pow(m_channelBuffer(x, y, b_index)[0], oneovergamma);
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------
/** Initialise the static filter values.
 */

void CqBucketProcessor::InitialiseFilterValues()
{
	if( !m_aFilterValues.empty() )
		return;

	// Allocate and fill in the filter values array for each pixel.
	TqInt numsubpixels = ( PixelXSamples() * PixelYSamples() );
	TqInt numperpixel = numsubpixels * numsubpixels;

	TqInt numvalues = static_cast<TqInt>( ( (lceil(FilterXWidth()) + 1) * (lceil(FilterYWidth()) + 1) ) * numperpixel );

	m_aFilterValues.resize( numvalues );

	RtFilterFunc pFilter;
	pFilter = QGetRenderContext() ->poptCurrent()->funcFilter();

	// Sanity check
	if( NULL == pFilter )
		pFilter = RiBoxFilter;

	TqFloat xmax = m_DiscreteShiftX;
	TqFloat ymax = m_DiscreteShiftY;
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
							m_aFilterValues[ cindex ] = w;
						}
					}
				}
			}
		}
	}
}

void CqBucketProcessor::CalculateDofBounds()
{
	m_NumDofBounds = PixelXSamples() * PixelYSamples();
	m_DofBounds.resize(m_NumDofBounds);

	TqFloat dx = 2.0 / PixelXSamples();
	TqFloat dy = 2.0 / PixelYSamples();

	// I know this is far from an optimal way of calculating this,
	// but it's only done once so I don't care.
	// Calculate the bounding boxes that the dof offset positions fall into.
	TqFloat minX = -1.0;
	TqFloat minY = -1.0;
	TqInt which = 0;
	for(int j = 0; j < PixelYSamples(); ++j)
	{
		for(int i = 0; i < PixelXSamples(); ++i)
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
/** Render any waiting MPs.
 
    Render ready micro polygons waiting to be processed, so that we
    have as few as possible MPs waiting and using memory at any given
    moment
 */

void CqBucketProcessor::RenderWaitingMPs()
{
	bool mpgsRendered = false;
	{
		for ( std::vector<boost::shared_ptr<CqMicroPolygon> >::iterator itMP = m_bucket->micropolygons().begin();
			  itMP != m_bucket->micropolygons().end();
			  itMP++ )
		{
			CqMicroPolygon* mp = (*itMP).get();
			RenderMicroPoly( mp );
			mpgsRendered = true;
		}

		m_bucket->micropolygons().clear();
	}
	if(mpgsRendered)
		m_OcclusionTree.updateDepths();
}


//----------------------------------------------------------------------
/** Render the given Surface
 */
void CqBucketProcessor::RenderSurface( boost::shared_ptr<CqSurface>& surface )
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
				pGrid->Split( SRegion().xMin(), SRegion().xMax(), SRegion().yMin(), SRegion().yMax());
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
void CqBucketProcessor::RenderMicroPoly( CqMicroPolygon* pMP )
{
	bool UsingDof = QGetRenderContext()->UsingDepthOfField();
	bool IsMoving = pMP->IsMoving();

	// Cache the shading interpolation type.  Ideally this should really be
	// done by the CacheOutputInterpCoeffs(), or possibly once per grid...
	const TqInt* interpType = pMP->pGrid()->pAttributes()
		->GetIntegerAttribute("System", "ShadingInterpolation");
	// At this stage, only use smooth shading interpolation for stationary
	// grids without DoF.
	/// \todo Allow smooth shading with MB or DoF.
	m_CurrentMpgSampleInfo.smoothInterpolation
		= !(UsingDof || IsMoving) && (*interpType == ShadingInterp_Smooth);

	// Cache output sample info for this mpg so we don't have to keep fetching
	// it for each sample.
	pMP->CacheOutputInterpCoeffs(m_CurrentMpgSampleInfo);

	// use the single imagesample rather than the list if possible.
	// transparent, matte or csg samples, or if we need more than the first
	// depth value have to use the (slower) list.
	m_CurrentMpgSampleInfo.isOpaque =
		m_CurrentMpgSampleInfo.occludes
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
void CqBucketProcessor::RenderMPG_Static( CqMicroPolygon* pMPG)
{
	const SqGridInfo& currentGridInfo = pMPG->pGrid()->GetCachedGridInfo();
    const TqFloat* LodBounds = currentGridInfo.m_LodBounds;
    bool UsingLevelOfDetail = LodBounds[ 0 ] >= 0.0f;

    TqInt sample_hits = 0;
    //TqFloat shd_rate = m_CurrentGridInfo.m_ShadingRate;

	CqHitTestCache hitTestCache;
	bool cachedHitData = false;

	//bool mustDraw = !m_CurrentGridInfo.m_IsCullable;

    CqBound Bound = pMPG->GetBound();

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
	if ( eX > SRegion().xMax() ) eX = SRegion().xMax();
	if ( eY > SRegion().yMax() ) eY = SRegion().yMax();

	TqInt sX = static_cast<TqInt>(std::floor( bminx ));
	TqInt sY = static_cast<TqInt>(std::floor( bminy ));
	if ( sY < SRegion().yMin() ) sY = SRegion().yMin();
	if ( sX < SRegion().xMin() ) sX = SRegion().xMin();

	CqImagePixel* pie, *pie2;

	TqInt iXSamples = PixelXSamples();
	TqInt iYSamples = PixelYSamples();

	TqInt im = ( bminx < sX ) ? 0 : static_cast<TqInt>(std::floor( ( bminx - sX ) * iXSamples ));
	TqInt in = ( bminy < sY ) ? 0 : static_cast<TqInt>(std::floor( ( bminy - sY ) * iYSamples ));
	TqInt em = ( bmaxx > eX ) ? iXSamples : lceil( ( bmaxx - ( eX - 1 ) ) * iXSamples );
	TqInt en = ( bmaxy > eY ) ? iYSamples : lceil( ( bmaxy - ( eY - 1 ) ) * iYSamples );

	TqInt nextx = SRegion().width();
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
						const SqSampleData& sampleData = pie2->SampleData( m_SamplePoints, index );
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
void CqBucketProcessor::RenderMPG_MBOrDof( CqMicroPolygon* pMPG, bool IsMoving, bool UsingDof )
{
	const SqGridInfo& currentGridInfo = pMPG->pGrid()->GetCachedGridInfo();

    const TqFloat* LodBounds = currentGridInfo.m_LodBounds;
    bool UsingLevelOfDetail = LodBounds[ 0 ] >= 0.0f;

    TqInt sample_hits = 0;
    //TqFloat shd_rate = m_CurrentGridInfo.m_ShadingRate;

	CqHitTestCache hitTestCache;

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

	if(UsingDof)
		pMPG->CacheCocMultipliers(hitTestCache);

	const TqInt timeRanges = std::max(4, PixelXSamples() * PixelYSamples() );
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

		TqFloat bminx = Bound.vecMin().x();
		TqFloat bmaxx = Bound.vecMax().x();
		TqFloat bminy = Bound.vecMin().y();
		TqFloat bmaxy = Bound.vecMax().y();
		TqFloat bminz = Bound.vecMin().z();
		TqFloat bmaxz = Bound.vecMax().z();
		// if bounding box is outside our viewing range, then cull it.
		if ( bminz > m_clippingFar || bmaxz < m_clippingNear )
			continue;
		TqFloat mpgbminx = bminx;
		TqFloat mpgbmaxx = bmaxx;
		TqFloat mpgbminy = bminy;
		TqFloat mpgbmaxy = bmaxy;
		TqInt bound_maxDof = 1;
		if(UsingDof)
		{
			const CqVector2D& minZCoc = QGetRenderContext()->GetCircleOfConfusion( Bound.vecMin().z() );
			const CqVector2D& maxZCoc = QGetRenderContext()->GetCircleOfConfusion( Bound.vecMax().z() );
			maxCocX = max( minZCoc.x(), maxZCoc.x() );
			maxCocY = max( minZCoc.y(), maxZCoc.y() );
			bound_maxDof = m_NumDofBounds;
		}
		else
		{
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

			// Now go across all pixels touched by the micropolygon bound.
			// The first pixel position is at (sX, sY), the last one
			// at (eX, eY).
			TqInt eX = lceil( bmaxx );
			TqInt eY = lceil( bmaxy );
			if ( eX > SRegion().xMax() ) eX = SRegion().xMax();
			if ( eY > SRegion().yMax() ) eY = SRegion().yMax();

			TqInt sX = static_cast<TqInt>(std::floor( bminx ));
			TqInt sY = static_cast<TqInt>(std::floor( bminy ));
			if ( sY < SRegion().yMin() ) sY = SRegion().yMin();
			if ( sX < SRegion().xMin() ) sX = SRegion().xMin();

			CqImagePixel* pie, *pie2;

			TqInt nextx = SRegion().width();
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
							const SqSampleData& sampleData = pie2->SampleData( m_SamplePoints, index );
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

void CqBucketProcessor::StoreSample( CqMicroPolygon* pMPG, CqImagePixel* pie2, TqInt index, TqFloat D )
{
	const SqGridInfo& currentGridInfo = pMPG->pGrid()->GetCachedGridInfo();

    bool Occludes = m_CurrentMpgSampleInfo.occludes;
	bool opaque =  m_CurrentMpgSampleInfo.isOpaque;

	SqImageSample& currentOpaqueSample = pie2->OpaqueValues(m_SamplePoints, index);
	//static SqImageSample localImageVal( QGetRenderContext() ->GetOutputDataTotalSize() );
	SqImageSample localImageVal;

	SqImageSample& ImageVal = opaque ? currentOpaqueSample : localImageVal;

	std::deque<SqImageSample>& aValues = pie2->Values( m_SamplePoints, index );
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
	m_hasValidSamples = true;

    TqFloat* val = ImageVal.Data();
	CqColor col;
	CqColor opa;
	const SqSampleData& sampleData = pie2->SampleData( m_SamplePoints, index );
	const CqVector2D& vecP = sampleData.m_Position;
	pMPG->InterpolateOutputs(m_CurrentMpgSampleInfo, vecP, col, opa);

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



void CqBucketProcessor::StoreExtraData( CqMicroPolygon* pMPG, SqImageSample& sample)
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
						sample.Data()[ entry->second.m_Offset ] = c.r();
						sample.Data()[ entry->second.m_Offset + 1 ] = c.g();
						sample.Data()[ entry->second.m_Offset + 2 ] = c.b();
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

bool CqBucketProcessor::occlusionCullSurface( const boost::shared_ptr<CqSurface>& surface )
{
	const CqBound RasterBound( surface->GetCachedRasterBound() );

	if ( m_OcclusionTree.canCull( RasterBound ) )
	{
		CqString objname( "unnamed" );
		const CqString* pattrName = surface->pAttributes() ->GetStringAttribute( "identifier", "name" );
		if( pattrName )
			objname = *pattrName;

		// Surface is behind everying in this bucket but it may be
		// visible in other buckets it overlaps.
		// bucket to the right
		TqInt nextBucketX = m_bucket->getCol() + 1;
		TqInt xpos, ypos;
		QGetRenderContext()->pImage()->bucketPosition( nextBucketX, m_bucket->getRow(), xpos, ypos );
		if ( ( nextBucketX < QGetRenderContext()->pImage()->cXBuckets() ) &&
			 ( RasterBound.vecMax().x() >= xpos ) )
		{
			Aqsis::log() << info << "GPrim: \"" << objname << 
					"\" occluded in bucket: " << m_bucket->getCol() << ", " << m_bucket->getRow() << 
					" shifted into bucket: " << nextBucketX << ", " << m_bucket->getRow() << std::endl;
			QGetRenderContext()->pImage()->Bucket( nextBucketX, m_bucket->getRow() ).AddGPrim( surface );
			return true;
		}

		// next row
		TqInt nextBucketY = m_bucket->getRow() + 1;
		// find bucket containing left side of bound
		nextBucketX = static_cast<TqInt>( RasterBound.vecMin().x() ) / QGetRenderContext()->pImage()->XBucketSize();
		nextBucketX = max( nextBucketX, 0 );
		QGetRenderContext()->pImage()->bucketPosition( nextBucketX, nextBucketY, xpos, ypos );

		if ( ( nextBucketX < QGetRenderContext()->pImage()->cXBuckets() ) &&
			 ( nextBucketY  < QGetRenderContext()->pImage()->cYBuckets() ) &&
			 ( RasterBound.vecMax().y() >= ypos ) )
		{
			Aqsis::log() << info << "GPrim: \"" << objname << 
				"\" occluded in bucket: " << m_bucket->getCol() << ", " << m_bucket->getRow() << 
				" shifted into bucket: " << nextBucketX << ", " << nextBucketY << std::endl;
			QGetRenderContext()->pImage()->Bucket( nextBucketX, nextBucketY ).AddGPrim( surface );
			return true;
		}

		// Bound covers no more buckets therefore we can delete the surface completely.
		Aqsis::log() << info << "GPrim: \"" << objname << "\" occlusion culled" << std::endl;
		STATS_INC( GPR_occlusion_culled );
		return true;
	}
	else
	{
		return false;
	}
}


} // namespace Aqsis
