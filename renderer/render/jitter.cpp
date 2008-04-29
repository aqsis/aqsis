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
		\brief Implements the CqJitter class responsible for jittering sample positions.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	"aqsis.h"

#include	"jitter.h"

namespace Aqsis {

//----------------------------------------------------------------------
/** Constructor
 */

CqJitter::CqJitter()
{}

//----------------------------------------------------------------------
/** Destructor
 */

CqJitter::~CqJitter()
{}


//----------------------------------------------------------------------
/** Perform the jitter of the samples in the list specified at the start.
 */

void CqJitter::jitterSamples(CqBucket::TqSampleList& samples, TqInt xres, TqInt yres)
{
#if 0
	TqInt numSamples = m_XSamples * m_YSamples;
	TqFloat subcell_width = 1.0f / numSamples;
	TqInt m = m_XSamples;
	TqInt n = m_YSamples;
	TqInt i, j;

	static CqRandom random(  53 );

	// Initialize points to the "canonical" multi-jittered pattern.

	if( m == 1 && n == 1)
	{
		TqFloat ranx = random.RandomFloat( 1.0f );
		TqFloat rany = random.RandomFloat( 1.0f );
		vecSamples[0].x(ranx);
		vecSamples[0].y(rany);
	}
	else
	{
		for ( i = 0; i < n; i++ )
		{
			for ( j = 0; j < m; j++ )
			{
				TqInt which = i * m + j;
				vecSamples[which].x( i );
				vecSamples[which].y( j );
			}
		}

		// Shuffle y coordinates within each row of cells.
		for ( i = 0; i < n; i++ )
		{
			for ( j = 0; j < m; j++ )
			{
				TqFloat t;
				TqInt k;

				k = random.RandomInt( n - 1 - i ) + i;
				TqInt i1 = i * m + j;
				TqInt i2 = k * m + j;
				assert( i1 < static_cast<TqInt>(vecSamples.size()) && i2 < static_cast<TqInt>(vecSamples.size()) );
				t = vecSamples[ i1 ].y();
				vecSamples[ i1 ].y( vecSamples[ i2 ].y() );
				vecSamples[ i2 ].y( t );
			}
		}

		// Shuffle x coordinates within each column of cells.
		for ( i = 0; i < m; i++ )
		{
			for ( j = 0; j < n; j++ )
			{
				TqFloat t;
				TqInt k;

				k = random.RandomInt( n - 1 - j ) + j;
				TqInt i1 = j * m + i;
				TqInt i2 = k * m + i;
				assert( i1 < static_cast<TqInt>(vecSamples.size()) && i2 < static_cast<TqInt>(vecSamples.size()) );
				t = vecSamples[ i1 ].x();
				vecSamples[ i1 ].x( vecSamples[ i2 ].x() );
				vecSamples[ i2 ].x( t );

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
				TqFloat xindex = vecSamples[ which ].x();
				TqFloat yindex = vecSamples[ which ].y();
				vecSamples[ which ].x( xindex * subcell_width + ( subcell_width * 0.5f ) + sx );
				vecSamples[ which ].y( yindex * subcell_width + ( subcell_width * 0.5f ) + sy );
				CqBucket::SamplePoints()[m_SampleIndices[ which ]].m_SubCellIndex = static_cast<TqInt>( ( yindex * m_YSamples ) + xindex );
				which++;
			}
		}
	}

	// Fill in the sample times for motion blur, detail levels for LOD and DoF.

	TqFloat time = 0;
	TqFloat dtime = 1.0f / numSamples;
	// We use the same random offset for each sample within a pixel.
	// This ensures the best possible coverage whilst still avoiding
	// aliasing. (I reckon). should minimise the noise.
	TqFloat randomTime = random.RandomFloat( dtime );

	TqFloat lod = 0;
	TqFloat dlod = dtime;

	for ( i = 0; i < numSamples; i++ )
	{
		// Scale the value of time to the shutter time.
		TqFloat t = time + randomTime;
		t = ( closetime - opentime ) * t + opentime;
		CqBucket::SamplePoints()[m_SampleIndices[ i ]].m_Time = t;
		time += dtime;

		CqBucket::SamplePoints()[m_SampleIndices[ i ]].m_DetailLevel = lod + random.RandomFloat( dlod );
		lod += dlod;
	}

	std::vector<CqVector2D> tmpDofOffsets(numSamples);
	// Store the DoF offsets in the canonical order to ensure that
	// assumptions made about ordering during sampling still hold.
	for( i = 0; i < numSamples; ++i)
	{
		tmpDofOffsets[i] = CqBucket::SamplePoints()[m_SampleIndices[m_DofOffsetIndices[i]]].m_DofOffset;
		m_DofOffsetIndices[i] = i;
	}

	// we now shuffle the dof offsets but remember which one went where.
	for( i = 0; i < numSamples/2; i++)
   	{
      		int k = random.RandomInt(numSamples/2) + numSamples/2;
      		if (k >= numSamples) k = numSamples - 1;
      		int tmp = m_DofOffsetIndices[i];
      		m_DofOffsetIndices[i] = m_DofOffsetIndices[k];
      		m_DofOffsetIndices[k] = tmp;
   	}

	for( i = 0; i < numSamples; ++i)
	{
		CqBucket::SamplePoints()[m_SampleIndices[m_DofOffsetIndices[i]]].m_DofOffset = tmpDofOffsets[i];
		CqBucket::SamplePoints()[m_SampleIndices[m_DofOffsetIndices[i]]].m_DofOffsetIndex = i;
	}
#endif
}


//---------------------------------------------------------------------

} // namespace Aqsis

