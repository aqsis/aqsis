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
		\brief Implements the CqImageBuffer class responsible for rendering the primitives and storing the results.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	"imagepixel.h"

#include	"options.h"
#include	"random.h"
#include	"logging.h"
#include	"bucketprocessor.h"

#include	<algorithm>

#ifdef WIN32
#include	<windows.h>
#endif
#include	<math.h>



namespace Aqsis {

/// \todo Previous code for the sample pool
/// CqSampleDataPool	SqImageSample::m_theSamplePool;
TqUint SqImageSample::sampleSize(9);

//----------------------------------------------------------------------
/** Constructor
 */

CqImagePixel::CqImagePixel(TqInt xSamples, TqInt ySamples)
		: m_XSamples(xSamples),
		m_YSamples(ySamples),
		m_samples(new SqSampleData[xSamples*ySamples]),
		m_DofOffsetIndices(new TqInt[xSamples*ySamples]),
		m_refCount(0)
{
	assert(xSamples > 0);
	assert(ySamples > 0);

	initialiseSamples();
}


//----------------------------------------------------------------------
void CqImagePixel::swap(CqImagePixel& other)
{
	assert(m_XSamples == other.m_XSamples);
	assert(m_YSamples == other.m_YSamples);

	m_samples.swap(other.m_samples);
	m_DofOffsetIndices.swap(other.m_DofOffsetIndices);
}


//----------------------------------------------------------------------
void CqImagePixel::initialiseSamples()
{
	TqInt nSamples = m_XSamples * m_YSamples;

	// Initialise the samples to the centre points.
	TqFloat XInc = ( 1.0f / m_XSamples ) / 2.0f;
	TqFloat YInc = ( 1.0f / m_YSamples ) / 2.0f;
	TqInt y;
	for ( y = 0; y < m_YSamples; y++ )
	{
		TqFloat YSam = YInc*(y + 1);
		TqInt x;
		for ( x = 0; x < m_XSamples; x++ )
			m_samples[y*m_XSamples + x].position = CqVector2D(XInc*(1+x), YSam);
	}


	static CqRandom random(  53 );
	// Fill in the sample times for motion blur, LOD and SubCellIndex entries

	TqFloat time = 0;
	TqFloat dtime = 1.0f / nSamples;

	for (TqInt i = 0; i < nSamples; i++ )
	{
		m_samples[i].subCellIndex = 0;
		m_samples[i].detailLevel = m_samples[i].time = time;
		time += dtime;
	}


	// we calculate dof offsets in a grid inside the unit cube and then
	// project them into the unit circle. This means that the offset
	// positions match the offset bounding boxes calculated in CqBucket.
	// The sample test in RenderMicroPoly then can be split into a number
	// of smaller bounding boxes where we know in advance which samples
	// fall into each. (This is analagous to what we do for mb now as well).
	// note that there is an implied symmetry to the way we number the bounding
	// boxes here and in the bucket code where the bb's are created (it
	// should be left to right, top to bottom).
	TqFloat dx = 2.0 / m_XSamples;
	TqFloat dy = 2.0 / m_YSamples;
	// We use the same random offset for each sample within a pixel.
	// This ensures the best possible coverage whilst still avoiding
	// aliasing. (I reckon). should minimise the noise.
	TqFloat sx = random.RandomFloat(dx);
	TqFloat sy = random.RandomFloat(dy);
	TqFloat xOffset = -1.0 + sx;
	TqFloat yOffset = -1.0 + sy;
	TqInt which = 0;
	std::vector<CqVector2D> tmpDofOffsets(nSamples);
	for (TqInt i = 0; i < m_YSamples; ++i )
	{
		for (TqInt j = 0; j < m_XSamples; ++j )
		{
			tmpDofOffsets[which].x(xOffset);
			tmpDofOffsets[which].y(yOffset);
			ProjectToCircle(tmpDofOffsets[which]);

			m_DofOffsetIndices[which] = which;

			xOffset += dx;
			which++;
		}
		yOffset += dy;
		xOffset = -1.0 + sx;
	}

	// we now shuffle the dof offsets but remember which one went where.
	for(TqInt i = 0; i < nSamples/2; i++)
   	{
      		int k = random.RandomInt(nSamples/2) + nSamples/2;
      		if (k >= nSamples) k = nSamples - 1;
      		int tmp = m_DofOffsetIndices[i];
      		m_DofOffsetIndices[i] = m_DofOffsetIndices[k];
      		m_DofOffsetIndices[k] = tmp;
   	}

	for(TqInt i = 0; i < nSamples; ++i)
	{
		m_samples[m_DofOffsetIndices[i]].dofOffset = tmpDofOffsets[i];
		m_samples[m_DofOffsetIndices[i]].dofOffsetIndex = i;
	}
}


//----------------------------------------------------------------------
/** Shuffle the sample data to avoid repeating patterns in the sampling.
 */

void CqImagePixel::JitterSamples( CqVector2D& offset, TqFloat opentime, TqFloat closetime )
{
	TqInt nSamples = numSamples();
	TqFloat subcell_width = 1.0f / nSamples;
	TqInt m = m_XSamples;
	TqInt n = m_YSamples;
	TqInt i, j;

	static CqRandom random(  53 );

	// Initialize points to the "canonical" multi-jittered pattern.

	if( m == 1 && n == 1)
	{
		m_samples[0].position = offset
			+ CqVector2D(random.RandomFloat(), random.RandomFloat());
	}
	else
	{
		for ( i = 0; i < n; i++ )
		{
			for ( j = 0; j < m; j++ )
			{
				TqInt which = i * m + j;
				m_samples[which].position = CqVector2D(i,j);
			}
		}

		// Shuffle y coordinates within each row of cells.
		for ( i = 0; i < n; i++ )
		{
			for ( j = 0; j < m; j++ )
			{
				TqInt k = random.RandomInt( n - 1 - i ) + i;
				TqInt i1 = i * m + j;
				TqInt i2 = k * m + j;
				assert(i1 < nSamples);
				assert(i2 < nSamples);
				std::swap(m_samples[i1].position.y(), m_samples[i2].position.y());
			}
		}

		// Shuffle x coordinates within each column of cells.
		for ( i = 0; i < m; i++ )
		{
			for ( j = 0; j < n; j++ )
			{
				TqInt k = random.RandomInt( n - 1 - j ) + j;
				TqInt i1 = j * m + i;
				TqInt i2 = k * m + i;
				assert(i1 < nSamples);
				assert(i2 < nSamples);
				std::swap(m_samples[i1].position.x(), m_samples[i2].position.x());
			}
		}


		TqFloat subpixelheight = 1.0f / m_YSamples;
		TqFloat subpixelwidth = 1.0f / m_XSamples;

		TqInt which = 0;
		for ( i = 0; i < n; i++ )
		{
			TqFloat sy = i * subpixelheight;
			for ( j = 0; j < m; j++ )
			{
				TqFloat sx = j * subpixelwidth;
				TqFloat xindex = m_samples[which].position.x();
				TqFloat yindex = m_samples[which].position.y();
				m_samples[which].position.x( xindex * subcell_width + ( subcell_width * 0.5f ) + sx + offset.x() );
				m_samples[which].position.y( yindex * subcell_width + ( subcell_width * 0.5f ) + sy + offset.y());
				m_samples[which].subCellIndex = static_cast<TqInt>( ( yindex * m_YSamples ) + xindex );
				which++;
			}
		}
	}

	// Fill in the sample times for motion blur, detail levels for LOD and DoF.

	TqFloat time = 0;
	TqFloat dtime = 1.0f / nSamples;
	// We use the same random offset for each sample within a pixel.
	// This ensures the best possible coverage whilst still avoiding
	// aliasing. (I reckon). should minimise the noise.
	TqFloat randomTime = random.RandomFloat( dtime );

	TqFloat lod = 0;
	TqFloat dlod = dtime;

	for ( i = 0; i < nSamples; i++ )
	{
		// Scale the value of time to the shutter time.
		TqFloat t = time + randomTime;
		t = ( closetime - opentime ) * t + opentime;
		m_samples[i].time = t;
		time += dtime;

		m_samples[i].detailLevel = lod + random.RandomFloat( dlod );
		lod += dlod;
	}

	std::vector<CqVector2D> tmpDofOffsets(nSamples);
	// Store the DoF offsets in the canonical order to ensure that
	// assumptions made about ordering during sampling still hold.
	for( i = 0; i < nSamples; ++i)
	{
		tmpDofOffsets[i] = m_samples[m_DofOffsetIndices[i]].dofOffset;
		m_DofOffsetIndices[i] = i;
	}

	// we now shuffle the dof offsets but remember which one went where.
	for( i = 0; i < nSamples/2; i++)
   	{
      		int k = random.RandomInt(nSamples/2) + nSamples/2;
      		if (k >= nSamples) k = nSamples - 1;
      		int tmp = m_DofOffsetIndices[i];
      		m_DofOffsetIndices[i] = m_DofOffsetIndices[k];
      		m_DofOffsetIndices[k] = tmp;
   	}

	for( i = 0; i < nSamples; ++i)
	{
		m_samples[m_DofOffsetIndices[i]].dofOffset = tmpDofOffsets[i];
		m_samples[m_DofOffsetIndices[i]].dofOffsetIndex = i;
	}
}

//----------------------------------------------------------------------
/** Clear the relevant data from the image element preparing it for the next usage.
 */

void CqImagePixel::Clear()
{
	for ( TqInt i = ( m_XSamples * m_YSamples ) - 1; i >= 0; i-- )
	{
		if(!m_samples[i].data.empty())
			m_samples[i].data.clear();
		m_samples[i].opaqueSample.flags = 0;
	}
}


//----------------------------------------------------------------------
/** Get the color at the specified sample point by blending the colors that appear at that point.
 */
// Ascending depth sorting function
struct SqAscendingDepthSort
{
     bool operator()(const SqImageSample& splStart, const SqImageSample& splEnd) const
     {
          return splStart.data[Sample_Depth] < splEnd.data[Sample_Depth];
     }
};

void CqImagePixel::Combine( enum EqFilterDepth depthfilter, CqColor zThreshold )
{
	TqUint samplecount = 0;
	TqInt sampleIndex = 0;
	TqInt nSamples = numSamples();
	for(TqInt sampIdx = 0; sampIdx < nSamples; ++sampIdx)
	{
		SqSampleData& sampleData = m_samples[sampIdx];

		SqImageSample& opaqueValue = sampleData.opaqueSample;
		sampleIndex++;

		if(!sampleData.data.empty())
		{
			// Sort the samples by depth.
			std::sort(sampleData.data.begin(), sampleData.data.end(), SqAscendingDepthSort());
			if (opaqueValue.flags & SqImageSample::Flag_Valid)
			{
				//	insert opaqueValue into samples in the right place.
				std::deque<SqImageSample>::iterator isi = sampleData.data.begin();
				std::deque<SqImageSample>::iterator isend = sampleData.data.end();
				while( isi != isend )
				{
					if((*isi).data[Sample_Depth] >= opaqueValue.data[Sample_Depth])
						break;

					++isi;
				}
				sampleData.data.insert(isi, opaqueValue);
			}

			// Find out if any of the samples are in a CSG tree.
			bool bProcessed;
			bool CqCSGRequired = CqCSGTreeNode::IsRequired();
			if (CqCSGRequired)
			{
				do
				{
					bProcessed = false;
					//Warning ProcessTree add or remove elements in samples list
					//We could not optimized the for loop here at all.
					for ( std::deque<SqImageSample>::iterator isample = sampleData.data.begin();
					        isample != sampleData.data.end();
					        ++isample )
					{
						if ( isample->csgNode )
						{
							isample->csgNode->ProcessTree( sampleData.data );
							bProcessed = true;
							break;
						}
					}
				}
				while ( bProcessed );
			}

			CqColor samplecolor = gColBlack;
			CqColor sampleopacity = gColBlack;
			bool samplehit = false;
			TqFloat opaqueDepths[2] = { FLT_MAX, FLT_MAX };
			TqFloat maxOpaqueDepth = FLT_MAX;

			for ( std::deque<SqImageSample>::reverse_iterator sample = sampleData.data.rbegin();
			        sample != sampleData.data.rend();
			        sample++ )
			{
				TqFloat* sample_data = sample->data;
				if ( sample->flags & SqImageSample::Flag_Matte )
				{
					if ( sample->flags & SqImageSample::Flag_Occludes )
					{
						// Optimise common case
						samplecolor = gColBlack;
						sampleopacity = gColBlack;
					}
					else
					{
						samplecolor = CqColor(
						    lerp( sample_data[Sample_ORed], samplecolor.r(), 0.0f ),
						    lerp( sample_data[Sample_OGreen], samplecolor.g(), 0.0f ),
						    lerp( sample_data[Sample_OBlue], samplecolor.b(), 0.0f )
						);
						sampleopacity = CqColor(
						    lerp( sample_data[Sample_Red], sampleopacity.r(), 0.0f ),
						    lerp( sample_data[Sample_Green], sampleopacity.g(), 0.0f ),
						    lerp( sample_data[Sample_Blue], sampleopacity.b(), 0.0f )
						);
					}
				}
				else
				{
					samplecolor = ( samplecolor *
					                ( gColWhite - CqColor(clamp(sample_data[Sample_ORed], 0.0f, 1.0f), clamp(sample_data[Sample_OGreen], 0.0f, 1.0f), clamp(sample_data[Sample_OBlue], 0.0f, 1.0f)) ) ) +
					              CqColor(sample_data[Sample_Red], sample_data[Sample_Green], sample_data[Sample_Blue]);
					sampleopacity = ( ( gColWhite - sampleopacity ) *
					                  CqColor(sample_data[Sample_ORed], sample_data[Sample_OGreen], sample_data[Sample_OBlue]) ) +
					                sampleopacity;
				}

				// Now determine if the sample opacity meets the limit for depth mapping.
				// If so, store the depth in the appropriate nearest opaque sample slot.
				// The test is, if any channel of the opacity color is greater or equal to the threshold.
				if(sample_data[Sample_ORed] >= zThreshold.r() || sample_data[Sample_OGreen] >= zThreshold.g() || sample_data[Sample_OBlue] >= zThreshold.b())
				{
					// Make sure we store the nearest and second nearest depth values.
					opaqueDepths[1] = opaqueDepths[0];
					opaqueDepths[0] = sample->data[Sample_Depth];
					// Store the max opaque depth too, if not already stored.
					if(!(maxOpaqueDepth < FLT_MAX))
						maxOpaqueDepth = sample->data[Sample_Depth];
				}
				samplehit = true;
			}

			if ( samplehit )
			{
				samplecount++;
			}

			// Write the collapsed color values back into the opaque entry.
			if ( !sampleData.data.empty() )
			{
				// Make sure the extra sample data from the top entry is copied
				// to the opaque sample, which is then sent to the display.
				opaqueValue = *sampleData.data.begin();
				// Set the color and opacity.
				opaqueValue.data[Sample_Red] = samplecolor.r();
				opaqueValue.data[Sample_Green] = samplecolor.g();
				opaqueValue.data[Sample_Blue] = samplecolor.b();
				opaqueValue.data[Sample_ORed] = sampleopacity.r();
				opaqueValue.data[Sample_OGreen] = sampleopacity.g();
				opaqueValue.data[Sample_OBlue] = sampleopacity.b();
				opaqueValue.flags |= SqImageSample::Flag_Valid;

				if ( depthfilter != Filter_Min )
				{
					if ( depthfilter == Filter_MidPoint )
					{
						//Aqsis::log() << debug << "OpaqueDepths: " << opaqueDepths[0] << " - " << opaqueDepths[1] << std::endl;
						// Use midpoint for depth
						if ( sampleData.data.size() > 1 )
							opaqueValue.data[Sample_Depth] = ( ( opaqueDepths[0] + opaqueDepths[1] ) * 0.5f );
						else
							opaqueValue.data[Sample_Depth] = FLT_MAX;
					}
					else if ( depthfilter == Filter_Max)
					{
						opaqueValue.data[Sample_Depth] = maxOpaqueDepth;
					}
					else if ( depthfilter == Filter_Average )
					{
						std::deque<SqImageSample>::iterator sample;
						TqFloat totDepth = 0.0f;
						TqInt totCount = 0;
						for ( sample = sampleData.data.begin(); sample != sampleData.data.end(); sample++ )
						{
							TqFloat* sample_data = sample->data;
							if(sample_data[Sample_ORed] >= zThreshold.r() || sample_data[Sample_OGreen] >= zThreshold.g() || sample_data[Sample_OBlue] >= zThreshold.b())
							{
								totDepth += sample_data[Sample_Depth];
								totCount++;
							}
						}
						totDepth /= totCount;

						opaqueValue.data[Sample_Depth] = totDepth;
					}
					// Default to "min"
				}
				else
					opaqueValue.data[Sample_Depth] = opaqueDepths[0];
			}
		}
		else
		{
			if (opaqueValue.flags & SqImageSample::Flag_Valid)
			{
				samplecount++;
			}
		}
	}
}


//---------------------------------------------------------------------

} // namespace Aqsis
