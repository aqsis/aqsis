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

#include "imagepixel.h"

#include <algorithm>

#ifdef WIN32
#	include <windows.h> // Not needed?
#endif

#include <aqsis/math/math.h>
#include <aqsis/math/random.h>
#include "renderer.h"


namespace Aqsis {

TqInt SqImageSample::sampleSize(9);

//----------------------------------------------------------------------
/** Constructor
 */

CqImagePixel::CqImagePixel(TqInt xSamples, TqInt ySamples)
		: m_XSamples(xSamples),
		m_YSamples(ySamples),
		m_samples(new SqSampleData[xSamples*ySamples]),
		m_hitSamples(),
		m_DofOffsetIndices(new TqInt[xSamples*ySamples]),
		m_refCount(0),
		m_hasValidSamples(false)
{
	assert(xSamples > 0);
	assert(ySamples > 0);

	TqInt nSamples = numSamples();
	// Allocate sample storage for all the occluding hits.
	TqInt sampSize = SqImageSample::sampleSize;
	m_hitSamples.resize(nSamples*sampSize);
	for(TqInt i = 0; i < nSamples; ++i)
		m_samples[i].occludingHit.index = i*sampSize;
}

void CqImagePixel::swap(CqImagePixel& other)
{
	assert(m_XSamples == other.m_XSamples);
	assert(m_YSamples == other.m_YSamples);

	m_hitSamples.swap(other.m_hitSamples);
	m_samples.swap(other.m_samples);
	m_DofOffsetIndices.swap(other.m_DofOffsetIndices);
	m_hasValidSamples = other.m_hasValidSamples;
}

void CqImagePixel::setupGridPattern(CqVector2D& offset, TqFloat opentime,
		TqFloat closetime)
{
	TqInt nSamples = numSamples();
	// Initialize positions to a grid.
	TqFloat xScale = 1.0/m_XSamples;
	TqFloat yScale = 1.0/m_YSamples;
	for(TqInt j = 0; j < m_YSamples; j++)
	{
		for(TqInt i = 0; i < m_XSamples; i++)
		{
			m_samples[j*m_XSamples + i].position =
				offset + CqVector2D(xScale*(i+0.5), yScale*(j+0.5));
		}
	}

	// Fill in motion blur and LoD with the same regular grid
	TqFloat dt = 1/nSamples;
	TqFloat time = dt*0.5;
	for(TqInt i = 0; i < nSamples; ++i)
	{
		m_samples[i].time = time;
		m_samples[i].detailLevel = time;
		time += dt;
	}
}

void CqImagePixel::clear()
{
	TqInt nSamples = numSamples();
	TqInt sampSize = SqImageSample::sampleSize;
	m_hitSamples.resize(nSamples*sampSize);
	m_hasValidSamples = false;
	for(TqInt i = 0; i < nSamples; ++i)
	{
		if(!m_samples[i].data.empty())
			m_samples[i].data.clear();
		m_samples[i].occludingHit.flags = 0;
		// Reallocate the occluding samples, as their storage indices may have
		// changed during the Combine() stage.
		m_samples[i].occludingHit.index = i*sampSize;
		// Reset the occluding depth to the maximum.
		m_samples[i].occlZ = FLT_MAX;
	}
}


//----------------------------------------------------------------------
/** Get the color at the specified sample point by blending the colors that appear at that point.
 */
// Ascending depth sorting functor
class CqAscendingDepthSort
{
	private:
		const CqImagePixel& m_pixel;
	public:
		CqAscendingDepthSort(const CqImagePixel& pixel)
			: m_pixel(pixel)
		{ }
		bool operator()(const SqImageSample& splStart, const SqImageSample& splEnd) const
		{
			return m_pixel.sampleHitData(splStart)[Sample_Depth]
				< m_pixel.sampleHitData(splEnd)[Sample_Depth];
		}
};

void CqImagePixel::Combine( enum EqDepthFilter depthfilter, CqColor zThreshold )
{
	TqUint samplecount = 0;
	TqInt sampleIndex = 0;
	TqInt nSamples = numSamples();
	for(TqInt sampIdx = 0; sampIdx < nSamples; ++sampIdx)
	{
		SqSampleData& sampleData = m_samples[sampIdx];

		SqImageSample& occlHit = sampleData.occludingHit;
		sampleIndex++;

		if(!sampleData.data.empty())
		{
			if (occlHit.flags & SqImageSample::Flag_Valid)
			{
				//	insert occlHit into samples if it holds valid data.
				sampleData.data.push_back(occlHit);
			}
			// Sort the samples by depth.
			std::sort(sampleData.data.begin(), sampleData.data.end(), CqAscendingDepthSort(*this));

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
					for ( std::vector<SqImageSample>::iterator isample = sampleData.data.begin();
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

			CqColor samplecolor;
			CqColor sampleopacity;
			bool samplehit = false;
			TqFloat opaqueDepths[2] = { sampleData.occlZ, FLT_MAX };
			TqFloat maxOpaqueDepth = FLT_MAX;

			for ( std::vector<SqImageSample>::reverse_iterator sample = sampleData.data.rbegin();
			        sample != sampleData.data.rend();
			        sample++ )
			{
				TqFloat* sample_data = sampleHitData(*sample);
				if ( sample->flags & SqImageSample::Flag_Matte )
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
				else
				{
					samplecolor = ( samplecolor *
					                ( gColWhite - CqColor(clamp(sample_data[Sample_ORed], 0.0f, 1.0f), clamp(sample_data[Sample_OGreen], 0.0f, 1.0f), clamp(sample_data[Sample_OBlue], 0.0f, 1.0f)) ) ) +
					              CqColor(sample_data[Sample_Red], sample_data[Sample_Green], sample_data[Sample_Blue]);
					sampleopacity = ( ( gColWhite - sampleopacity ) *
					                  CqColor(sample_data[Sample_ORed], sample_data[Sample_OGreen], sample_data[Sample_OBlue]) ) +
					                sampleopacity;
				}

				// Now determine if the sample opacity meets the limit for
				// depth mapping.  If so, store the depth in the appropriate
				// nearest opaque sample slot.  The test is, if all channels of
				// the opacity color are greater or equal to the threshold.
				if(   sample_data[Sample_ORed]   >= zThreshold.r()
				   && sample_data[Sample_OGreen] >= zThreshold.g()
				   && sample_data[Sample_OBlue]  >= zThreshold.b())
				{
					// Make sure we store the nearest and second nearest depth values.
					opaqueDepths[1] = opaqueDepths[0];
					opaqueDepths[0] = sample_data[Sample_Depth];
					// Store the max opaque depth too, if not already stored.
					if(!(maxOpaqueDepth < FLT_MAX))
						maxOpaqueDepth = sample_data[Sample_Depth];
				}
				samplehit = true;
			}

			if ( samplehit )
			{
				samplecount++;
			}

			// Write the collapsed color values back into the occluding entry.
			if ( !sampleData.data.empty() )
			{
				// Make sure the extra sample data from the top entry is copied
				// to the occluding sample, which is then sent to the display.
				occlHit = *sampleData.data.begin();
				TqFloat* occlData = sampleHitData(occlHit);
				// Set the color and opacity.
				occlData[Sample_Red] = samplecolor.r();
				occlData[Sample_Green] = samplecolor.g();
				occlData[Sample_Blue] = samplecolor.b();
				occlData[Sample_ORed] = sampleopacity.r();
				occlData[Sample_OGreen] = sampleopacity.g();
				occlData[Sample_OBlue] = sampleopacity.b();
				occlHit.flags |= SqImageSample::Flag_Valid;

				TqFloat& occlDepth = occlData[Sample_Depth];
				if ( depthfilter != Filter_Min )
				{
					if ( depthfilter == Filter_MidPoint )
					{
						// Use midpoint for depth
						if ( sampleData.data.size() > 1 )
							occlDepth = ( ( opaqueDepths[0] + opaqueDepths[1] ) * 0.5f );
						else
							occlDepth = FLT_MAX;
					}
					else if ( depthfilter == Filter_Max)
					{
						occlDepth = maxOpaqueDepth;
					}
					else if ( depthfilter == Filter_Average )
					{
						std::vector<SqImageSample>::iterator sample;
						TqFloat totDepth = 0.0f;
						TqInt totCount = 0;
						for ( sample = sampleData.data.begin(); sample != sampleData.data.end(); sample++ )
						{
							TqFloat* sample_data = sampleHitData(*sample);
							if(sample_data[Sample_ORed] >= zThreshold.r() || sample_data[Sample_OGreen] >= zThreshold.g() || sample_data[Sample_OBlue] >= zThreshold.b())
							{
								totDepth += sample_data[Sample_Depth];
								totCount++;
							}
						}
						totDepth /= totCount;

						occlDepth = totDepth;
					}
					// Default to "min"
				}
				else
					occlDepth = opaqueDepths[0];
			}
		}
		else
		{
			if (occlHit.flags & SqImageSample::Flag_Valid)
			{
				TqFloat* occlData = sampleHitData(occlHit);
				if(occlHit.flags & SqImageSample::Flag_Matte)
				{
					// Opaque matte objects are fully transparent black; need
					// to set the hit data to reflect this.
					occlData[Sample_Red] = 0;
					occlData[Sample_Green] = 0;
					occlData[Sample_Blue] = 0;
					occlData[Sample_ORed] = 0;
					occlData[Sample_OGreen] = 0;
					occlData[Sample_OBlue] = 0;
				}
				if(depthfilter == Filter_MidPoint)
				{
					// For midpoint depth filters, average the occluding depth
					// and the depth of the opaque sample.  The occluding depth
					// represents one surface *behind* the opaque depth in this
					// case.
					occlData[Sample_Depth] = 0.5*(occlData[Sample_Depth]
					                              + sampleData.occlZ);
				}
				samplecount++;
			}
		}
	}
}

void CqImagePixel::setSamples(IqSampler* sampler, CqVector2D& offset)
{
	TqInt nSamps = numSamples();

	const TqInt* shuffledIndices = sampler->getShuffledIndices();
	for(TqInt i = 0; i < nSamps; ++i)
		m_DofOffsetIndices[i] = shuffledIndices[i];

	// Get random distributions from the sample generator, and save them into
	// the pixel sample data structures.
	const CqVector2D* positions = sampler->get2DSamples();
	const CqVector2D* dofOffsets = sampler->get2DSamples();
	const TqFloat* times = sampler->get1DSamples();
	const TqFloat* lods = sampler->get1DSamples();

	TqFloat opentime = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Shutter" ) [ 0 ];
	TqFloat closetime = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Shutter" ) [ 1 ];

	for(TqInt i = 0; i < nSamps; ++i)
	{
		m_samples[i].position = offset + positions[i];
		m_samples[i].time = ( closetime - opentime ) * times[i] + opentime;
		m_samples[i].detailLevel = lods[i];
		m_samples[m_DofOffsetIndices[i]].dofOffset = projectToCircle( -1 + 2 * (dofOffsets[i]) );
	}
}



//---------------------------------------------------------------------

} // namespace Aqsis
