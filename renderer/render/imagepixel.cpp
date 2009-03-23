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
#include <cfloat> // for FLT_MAX

#ifdef WIN32
#	include <windows.h> // Not needed?
#endif

#include "aqsismath.h"
#include "autobuffer.h"
#include "random.h"


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
		m_refCount(0)
{
	assert(xSamples > 0);
	assert(ySamples > 0);

	TqInt nSamples = numSamples();
	// Allocate sample storage for all the occluding hits.
	m_hitSamples.reserve(nSamples*SqImageSample::sampleSize);
	for(TqInt i = 0; i < nSamples; ++i)
		allocateHitData(m_samples[i].occludingHit);

	// Create the DoF offsets
	initialiseDofOffsets();
}

void CqImagePixel::swap(CqImagePixel& other)
{
	assert(m_XSamples == other.m_XSamples);
	assert(m_YSamples == other.m_YSamples);

	m_hitSamples.swap(other.m_hitSamples);
	m_samples.swap(other.m_samples);
	m_DofOffsetIndices.swap(other.m_DofOffsetIndices);
}

/** \brief Compute subcell coordinates for multijittered sampling.
 *
 * Consider a pixel containing NxM samples points.  We'd like to place N*M
 * sample points inside the pixel so that they're uniformly distributed, but
 * not evenly spaced.  One way to do this is using a "multijitter" pattern
 * which satisfies two types of stratification:
 *   - The pixel is broken up into a NxM grid of subpixels and each of these
 *     rectangles contains exactly one sample point.  This is similar to simple
 *     jittered sampling.
 *   - If you divide the pixel into N*M columns or rows then exactly one
 *     sample is in each column and one in each row.  This is known as the
 *     Latin Hypercube property.
 *
 * Each subpixel is further broken up into MxN subcells - here's the case for
 * 2x3 sampling:
 *
 *  +-----------+-----------+
 *  |   .   .   |   .   . x |
 *  |...........|...........|
 *  |   .   . x |   .   .   |
 *  +-----------+-----------+
 *  |   .   .   |   . x .   |
 *  |...........|...........|
 *  |   . x .   |   .   .   |
 *  +-----------+-----------+
 *  |   .   .   | x .   .   |
 *  |...........|...........|
 *  | x .   .   |   .   .   |
 *  +-----------+-----------+
 *
 * The x's indicate the canonical starting position for sample points inside
 * the subpixels.  The position of each sample point within its subpixel can be
 * specified by integer (x,y) subcell coordinates.  That's what this function
 * computes and places in the "indices" array.
 *
 * The starting positions shown above satisfy the stratification properties,
 * but are too uniformly distributed.  The shuffling procedure swaps the
 * indices randomly while retaining the stratification to result in a
 * well-stratified but randomised set of indices.
 */
void CqImagePixel::multiJitterIndices(TqInt* indices, TqInt numX, TqInt numY)
{
	static CqRandom random(42);

	// Initialise the subcell coordinates to a regular and stratified but
	// non-random initial pattern
	for (TqInt iy = 0; iy < numY; iy++ )
	{
		for (TqInt ix = 0; ix < numX; ix++ )
		{
			TqInt which = 2*(iy*numX + ix);
			indices[which] = iy;
			indices[which+1] = ix;
		}
	}

	// Shuffle y subcell coordinates within each row of subpixels.  This is
	// an in-place "Fisher-Yates shuffle".  Conceptually this is equivilant to:
	// given K objects, take an object randomly and place it on the end of a
	// list, giving K-1 objects remaining.  Repeat until the list contains all
	// K objects.
	for (TqInt iy = 0; iy < numY; iy++ )
	{
		TqInt ix = numX;
		while(ix > 1)
		{
			TqInt ix2 = random.RandomInt(ix);
			--ix;
			std::swap(indices[2*(iy*numX + ix) + 1],
					indices[2*(iy*numX + ix2) + 1]);
		}
	}

	// Shuffle x subcell coordinates within each column of subpixels.
	for (TqInt ix = 0; ix < numX; ix++ )
	{
		TqInt iy = numY;
		while(iy > 1)
		{
			TqInt iy2 = random.RandomInt(iy);
			--iy;
			std::swap(indices[2*(iy*numX + ix)],
					indices[2*(iy2*numX + ix)]);
		}
	}
}

/** \brief Initialise the depth of field offsets
 *
 * We calculate DoF offsets in a multijittered grid inside the unit square and
 * then project them into the unit circle.  This means that the offset
 * positions fall into several DoF sub-bounding boxes as used in the sampling
 * code.  The sub-bounds allow the RenderMicroPoly function to check only a
 * subset of the samples within the any given pixel against a given
 * micropolygon.
 *
 * Note that there is an implicit symmetry to the way we number the bounding
 * boxes here and in the sampling code where the bb's are created (it should be
 * left to right, top to bottom).
 *
 * m_DofOffsetIndices is also filled in with the default non-shuffled order.
 */
void CqImagePixel::initialiseDofOffsets()
{
	TqInt nSamples = numSamples();

	if(nSamples == 1)
	{
		static CqRandom random(42);
		m_samples[0].dofOffset = projectToCircle(
				2*CqVector2D(random.RandomFloat(), random.RandomFloat()) - 1 );
		m_DofOffsetIndices[0] = 0;
	}
	else
	{
		// Buffer to hold the subcell indices.  (The stack-allocated buffer is
		// large enough for 10x10 samples.)
		CqAutoBuffer<TqInt, 200> indices(nSamples);
		multiJitterIndices(&indices[0], m_XSamples, m_YSamples);

		TqFloat subPixelHeight = 1.0f / m_YSamples;
		TqFloat subPixelWidth = 1.0f / m_XSamples;
		TqFloat subcellWidth = 1.0f / nSamples;

		TqInt which = 0;
		for (TqInt iy = 0; iy < m_YSamples; iy++ )
		{
			for (TqInt ix = 0; ix < m_XSamples; ix++ )
			{
				// DoF offsets are created at the centre of the multijittered
				// subcell indices, and shrunk to lie inside the unit circle.
				m_samples[which].dofOffset = projectToCircle(
					-1 + 2*CqVector2D(
						subPixelWidth*ix + subcellWidth*(indices[2*which] + 0.5),
						subPixelHeight*iy + subcellWidth*(indices[2*which+1] + 0.5))
					);
				m_DofOffsetIndices[which] = which;
				which++;
			}
		}
	}
}


//----------------------------------------------------------------------
void CqImagePixel::setupJitterPattern(CqVector2D& offset, TqFloat opentime,
		TqFloat closetime)
{
	TqInt nSamples = numSamples();
	static CqRandom random(  53 );

	// Initialize points to the "canonical" multi-jittered pattern.

	if( m_XSamples == 1 && m_YSamples == 1)
	{
		m_samples[0].position = offset
			+ CqVector2D(random.RandomFloat(), random.RandomFloat());
	}
	else
	{
		// Buffer to hold the subcell indices.  (The stack-allocated buffer is
		// large enough for 10x10 samples.)
		CqAutoBuffer<TqInt, 200> indices(nSamples);

		multiJitterIndices(&indices[0], m_XSamples, m_YSamples);

		TqFloat subPixelHeight = 1.0f / m_YSamples;
		TqFloat subPixelWidth = 1.0f / m_XSamples;

		// Use the shuffled subcell coordinates to compute the posititions of
		// the samples.
		TqFloat subcellWidth = 1.0f / nSamples;
		TqInt which = 0;
		for (TqInt iy = 0; iy < m_YSamples; iy++ )
		{
			for (TqInt ix = 0; ix < m_XSamples; ix++ )
			{
				TqInt xindex = indices[2*which];
				TqInt yindex = indices[2*which+1];
				// Sample positions are placed in a randomly jittered position
				// within their subcell.  This avoids any remaining aliasing
				// which would result if we placed the sample positions at the
				// centre of the subcell.
				m_samples[which].position = offset + CqVector2D(
					(xindex+random.RandomFloat())*subcellWidth + ix*subPixelWidth,
					(yindex+random.RandomFloat())*subcellWidth + iy*subPixelHeight);
				// Initialise the subcell index (used to lookup filter values
				// during filtering)
				m_samples[which].subCellIndex = yindex*m_XSamples + xindex;
				++which;
			}
		}
	}

	// Fill in the sample times for motion blur, detail levels for LOD and DoF.

	TqFloat time = 0;
	TqFloat dtime = 1.0f / nSamples;
	// We use the same random offset for each sample within a pixel.
	// This ensures the best possible coverage whilst still avoiding
	// aliasing. (I reckon). should minimise the noise.
	//
	// TODO: In fact, this can be improved using a randomized low discrepency
	// sequence (suitably shuffled or randomized between pixels)
	TqFloat randomTime = random.RandomFloat( dtime );

	TqFloat lod = 0;
	TqFloat dlod = dtime;

	for (TqInt i = 0; i < nSamples; i++ )
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
	for(TqInt i = 0; i < nSamples; ++i)
	{
		tmpDofOffsets[i] = m_samples[m_DofOffsetIndices[i]].dofOffset;
		m_DofOffsetIndices[i] = i;
	}

	// we now shuffle the dof offsets but remember which one went where.
	for(TqInt i = 0; i < nSamples/2; i++)
   	{
		int k = random.RandomInt(nSamples/2) + nSamples/2;
		if (k >= nSamples) k = nSamples - 1;
		std::swap(m_DofOffsetIndices[i], m_DofOffsetIndices[k]);
   	}

	for(TqInt i = 0; i < nSamples; ++i)
		m_samples[m_DofOffsetIndices[i]].dofOffset = tmpDofOffsets[i];
}


void CqImagePixel::setupGridPattern(CqVector2D& offset, TqFloat opentime,
		TqFloat closetime)
{
	TqInt nSamples = numSamples();
	// Initialize positions to a grid.
	TqInt subCellIndex = nSamples/2;
	TqFloat xScale = 1.0/m_XSamples;
	TqFloat yScale = 1.0/m_YSamples;
	for(TqInt j = 0; j < m_YSamples; j++)
	{
		for(TqInt i = 0; i < m_XSamples; i++)
		{
			m_samples[j*m_XSamples + i].position =
				offset + CqVector2D(xScale*(i+0.5), yScale*(j+0.5));
			m_samples[j*m_XSamples + i].subCellIndex = subCellIndex;
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
	m_hitSamples.clear();
	for ( TqInt i = nSamples - 1; i >= 0; i-- )
	{
		if(!m_samples[i].data.empty())
			m_samples[i].data.clear();
		m_samples[i].occludingHit.flags = 0;
		// Reallocate the occluding samples, as their storage indices may have
		// changed during the Combine() stage.
		m_samples[i].occludingHit.index = -1;
		allocateHitData(m_samples[i].occludingHit);
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

void CqImagePixel::Combine( enum EqFilterDepth depthfilter, CqColor zThreshold )
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
			TqFloat opaqueDepths[2] = { FLT_MAX, FLT_MAX };
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

				// Now determine if the sample opacity meets the limit for depth mapping.
				// If so, store the depth in the appropriate nearest opaque sample slot.
				// The test is, if any channel of the opacity color is greater or equal to the threshold.
				if(sample_data[Sample_ORed] >= zThreshold.r() || sample_data[Sample_OGreen] >= zThreshold.g() || sample_data[Sample_OBlue] >= zThreshold.b())
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
				if(occlHit.flags & SqImageSample::Flag_Matte)
				{
					TqFloat* occlData = sampleHitData(occlHit);
					occlData[Sample_Red] = 0;
					occlData[Sample_Green] = 0;
					occlData[Sample_Blue] = 0;
					occlData[Sample_ORed] = 0;
					occlData[Sample_OGreen] = 0;
					occlData[Sample_OBlue] = 0;
				}
				samplecount++;
			}
		}
	}
}


//---------------------------------------------------------------------

} // namespace Aqsis
