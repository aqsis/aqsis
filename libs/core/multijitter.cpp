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
		\brief Implements the CqMultiJittered class responsible for providing stratified sample positions.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include "multijitter.h"

#include <algorithm>

#include <aqsis/math/math.h>
#include <aqsis/util/autobuffer.h>
#include <aqsis/math/random.h>

namespace Aqsis {


//----------------------------------------------------------------------

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
 * O---------------------------> +x
 * :
 * :  +-----------+-----------+
 * :  | x .   .   |   .   .   |
 * :  |...........|...........|
 * :  |   .   .   | x .   .   |
 * :  +-----------+-----------+
 * :  |   . x .   |   .   .   |
 * :  |...........|...........|
 * :  |   .   .   |   . x .   |
 * :  +-----------+-----------+
 * :  |   .   . x |   .   .   |
 * :  |...........|...........|
 * :  |   .   .   |   .   . x |
 * :  +-----------+-----------+
 * V
 * +y
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
void CqMultiJitteredSampler::multiJitterIndices(TqInt* indices, TqInt numX, TqInt numY)
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

void CqMultiJitteredSampler::setupJitterPattern(TqInt offset)
{
	TqInt nSamples = numSamples();
	static CqRandom random(  53 );

	// Initialize points to the "canonical" multi-jittered pattern.

	if( m_pixelXSamples == 1 && m_pixelYSamples == 1)
	{
		m_2dSamples[offset] = CqVector2D(random.RandomFloat(), random.RandomFloat());
		m_1dSamples[offset] = random.RandomFloat();
	}
	else
	{
		// Buffer to hold the subcell indices.  (The stack-allocated buffer is
		// large enough for 10x10 samples.)
		CqAutoBuffer<TqInt, 200> indices(nSamples * 2);

		multiJitterIndices(&indices[0], m_pixelXSamples, m_pixelYSamples);

		// Use the shuffled subcell coordinates to compute the posititions of
		// the samples.
		TqFloat subPixelHeight = 1.0f / m_pixelYSamples;
		TqFloat subPixelWidth = 1.0f / m_pixelXSamples;
		TqFloat subcellWidth = 1.0f / nSamples;
		TqInt which = 0;
		for (TqInt iy = 0; iy < m_pixelYSamples; iy++ )
		{
			for (TqInt ix = 0; ix < m_pixelXSamples; ix++ )
			{
				TqInt xindex = indices[2*which];
				TqInt yindex = indices[2*which+1];
				// Sample positions are placed in a randomly jittered position
				// within their subcell.  This avoids any remaining aliasing
				// which would result if we placed the sample positions at the
				// centre of the subcell.
				m_2dSamples[offset+which] = CqVector2D(
					(xindex+random.RandomFloat())*subcellWidth + ix*subPixelWidth,
					(yindex+random.RandomFloat())*subcellWidth + iy*subPixelHeight);
				++which;
			}
		}
	}

	TqFloat sample1d = 0;
	TqFloat delta1d = 1.0f / nSamples;
	// We use the same random offset for each sample within a pixel.
	// This ensures the best possible coverage whilst still avoiding
	// aliasing. (I reckon). should minimise the noise.
	//
	// TODO: In fact, this can be improved using a randomized low discrepency
	// sequence (suitably shuffled or randomized between pixels)
	TqFloat random1d = random.RandomFloat( delta1d );

	for (TqInt i = 0; i < nSamples; i++ )
	{
		// Scale the value of time to the shutter time.
		TqFloat t = sample1d + random1d;
		m_1dSamples[offset+i] = t;
		sample1d += delta1d;
	}

	// Setup a randomly shuffled array of indices, between 0 and nSamples.
	for(TqInt i = 0; i < nSamples; ++i)
		m_shuffledIndices[offset+i] = i;
	TqInt j = nSamples;
	while(j > 1)
	{
		TqInt j2 = random.RandomInt(j);
		--j;
		std::swap(m_shuffledIndices[offset+j], m_shuffledIndices[offset+j2]);
	}
}


const CqVector2D* CqMultiJitteredSampler::get2DSamples()		
{
	TqInt jitterIndex = m_random.RandomInt(m_cacheSize);
	return &m_2dSamples[this->numSamples()*jitterIndex];
}


const TqFloat* CqMultiJitteredSampler::get1DSamples()		
{
	TqInt jitterIndex = m_random.RandomInt(m_cacheSize);
	return &m_1dSamples[this->numSamples()*jitterIndex];
}

const TqInt* CqMultiJitteredSampler::getShuffledIndices()
{
	TqInt jitterIndex = m_random.RandomInt(m_cacheSize);
	return &m_shuffledIndices[this->numSamples()*jitterIndex];
}

//---------------------------------------------------------------------

} // namespace Aqsis
