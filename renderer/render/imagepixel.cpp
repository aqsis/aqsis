// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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
#include	"bucket.h"

#include	<algorithm>

#ifdef WIN32
#include	<windows.h>
#endif
#include	<math.h>



START_NAMESPACE( Aqsis )

CqSampleDataPool	SqImageSample::m_theSamplePool;

//----------------------------------------------------------------------
/** Constructor
 */

CqImagePixel::CqImagePixel() :
		m_XSamples( 0 ),
		m_YSamples( 0 )
{}


//----------------------------------------------------------------------
/** Destructor
 */

CqImagePixel::~CqImagePixel()
{}


//----------------------------------------------------------------------
/** Copy constructor
 */

CqImagePixel::CqImagePixel( const CqImagePixel& ieFrom )
{
	*this = ieFrom;
}


//----------------------------------------------------------------------
/** Allocate the subpixel samples array.
 * \param XSamples Integer samples count in X.
 * \param YSamples Integer samples count in Y.
 */

void CqImagePixel::AllocateSamples( CqBucket* bucket, TqInt XSamples, TqInt YSamples )
{
	if( m_XSamples != XSamples || m_YSamples != YSamples )
	{
		m_XSamples = XSamples;
		m_YSamples = YSamples;
		TqInt numSamples = m_XSamples * m_YSamples;

		if ( XSamples > 0 && YSamples > 0 )
		{
			// Initialise the OpaqueSampleEntries to the correct depth for the data we are
			// rendering, including any AOV data.
			m_SampleIndices.resize( numSamples );
			for(TqInt i=0; i<numSamples; i++)
				m_SampleIndices[i] = bucket->GetNextSamplePointIndex();
			m_DofOffsetIndices.resize( numSamples );
		}
	}
}

//----------------------------------------------------------------------
/** Fill in the sample array usig the multijitter function from GG IV.
 * \param vecPixel Cq2DVector pixel coordinate of this image element, used to make sure sample points are absolute, not relative.
 * \param fJitter Flag indicating whether to apply jittering to the sample points or not.
 */

void CqImagePixel::InitialiseSamples( std::vector<SqSampleData>& samplePoints, std::vector<CqVector2D>& vecSamples )
{
	TqInt numSamples = m_XSamples * m_YSamples;
	TqInt i, j;

	vecSamples.resize(numSamples);
	// Initialise the samples to the centre points.
	TqFloat XInc = ( 1.0f / m_XSamples ) / 2.0f;
	TqFloat YInc = ( 1.0f / m_YSamples ) / 2.0f;
	TqInt y;
	for ( y = 0; y < m_YSamples; y++ )
	{
		TqFloat YSam = YInc + ( YInc * y );
		TqInt x;
		for ( x = 0; x < m_XSamples; x++ )
			vecSamples[ ( y * m_XSamples ) + x ] = CqVector2D( XInc + ( XInc * x ), YSam );
	}


	static CqRandom random(  53 );
	// Fill in the sample times for motion blur, LOD and SubCellIndex entries

	TqFloat time = 0;
	TqInt nSamples = m_XSamples*m_YSamples;
	TqFloat dtime = 1.0f / nSamples;

	for ( i = 0; i < nSamples; i++ )
	{
		samplePoints[m_SampleIndices[ i ]].m_SubCellIndex = 0;
		samplePoints[m_SampleIndices[ i ]].m_DetailLevel = samplePoints[m_SampleIndices[ i ]].m_Time = time;
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
	std::vector<CqVector2D> tmpDofOffsets(numSamples);
	for ( i = 0; i < m_YSamples; ++i )
	{
		for ( j = 0; j < m_XSamples; ++j )
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
		samplePoints[m_SampleIndices[m_DofOffsetIndices[i]]].m_DofOffset = tmpDofOffsets[i];
		samplePoints[m_SampleIndices[m_DofOffsetIndices[i]]].m_DofOffsetIndex = i;
	}
}


//----------------------------------------------------------------------
/** Shuffle the sample data to avoid repeating patterns in the sampling.
 */

void CqImagePixel::JitterSamples( std::vector<SqSampleData>& samplePoints, std::vector<CqVector2D>& vecSamples, TqFloat opentime, TqFloat closetime )
{
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
				assert( i1 < vecSamples.size() && i2 < vecSamples.size() );
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
				assert( i1 < vecSamples.size() && i2 < vecSamples.size() );
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
				samplePoints[m_SampleIndices[ which ]].m_SubCellIndex = static_cast<TqInt>( ( yindex * m_YSamples ) + xindex );
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
		samplePoints[m_SampleIndices[ i ]].m_Time = t;
		time += dtime;

		samplePoints[m_SampleIndices[ i ]].m_DetailLevel = lod + random.RandomFloat( dlod );
		lod += dlod;
	}

	std::vector<CqVector2D> tmpDofOffsets(numSamples);
	// Store the DoF offsets in the canonical order to ensure that
	// assumptions made about ordering during sampling still hold.
	for( i = 0; i < numSamples; ++i)
	{
		tmpDofOffsets[i] = samplePoints[m_SampleIndices[m_DofOffsetIndices[i]]].m_DofOffset;
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
		samplePoints[m_SampleIndices[m_DofOffsetIndices[i]]].m_DofOffset = tmpDofOffsets[i];
		samplePoints[m_SampleIndices[m_DofOffsetIndices[i]]].m_DofOffsetIndex = i;
	}
}

//----------------------------------------------------------------------
/** Clear the relevant data from the image element preparing it for the next usage.
 */

void CqImagePixel::Clear( std::vector<SqSampleData>& samplePoints )
{
	for ( TqInt i = ( m_XSamples * m_YSamples ) - 1; i >= 0; i-- )
	{
		if(!samplePoints[m_SampleIndices[i]].m_Data.empty())
			samplePoints[m_SampleIndices[ i ]].m_Data.clear( );
		samplePoints[m_SampleIndices[ i ]].m_OpaqueSample.resetFlags();
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
          return splStart.Data()[Sample_Depth] < splEnd.Data()[Sample_Depth];
     }
};

void CqImagePixel::Combine( std::vector<SqSampleData>& samplePoints, enum EqFilterDepth depthfilter, CqColor zThreshold )
{
	TqUint samplecount = 0;
	TqInt sampleIndex = 0;
	std::vector<TqInt>::iterator end = m_SampleIndices.end();
	for ( std::vector<TqInt>::iterator sample_index = m_SampleIndices.begin(); sample_index != end; ++sample_index )
	{
		SqSampleData* samples = &samplePoints[*sample_index];

		SqImageSample& opaqueValue = samples->m_OpaqueSample;
		sampleIndex++;

		if(!samples->m_Data.empty())
		{
			// Sort the samples by depth.
			std::sort(samples->m_Data.begin(), samples->m_Data.end(), SqAscendingDepthSort());
			if (opaqueValue.isValid())
			{
				//	insert opaqueValue into samples in the right place.
				std::deque<SqImageSample>::iterator isi = samples->m_Data.begin();
				std::deque<SqImageSample>::iterator isend = samples->m_Data.end();
				while( isi != isend )
				{
					if((*isi).Data()[Sample_Depth] >= opaqueValue.Data()[Sample_Depth])
						break;

					++isi;
				}
				samples->m_Data.insert(isi, opaqueValue);
			}

			// Find out if any of the samples are in a CSG tree.
			bool bProcessed;
			bool CqCSGRequired = CqCSGTreeNode::IsRequired();
			if (CqCSGRequired)
				do
				{
					bProcessed = false;
					//Warning ProcessTree add or remove elements in samples list
					//We could not optimized the for loop here at all.
					for ( std::deque<SqImageSample>::iterator isample = samples->
					        m_Data.begin();
					        isample != samples->m_Data.end();
					        ++isample )
					{
						if ( isample->m_pCSGNode )
						{
							isample->m_pCSGNode->ProcessTree( samples->m_Data );
							bProcessed = true;
							break;
						}
					}
				}
				while ( bProcessed );

			CqColor samplecolor = gColBlack;
			CqColor sampleopacity = gColBlack;
			bool samplehit = false;
			TqFloat opaqueDepths[2] = { FLT_MAX, FLT_MAX };
			TqFloat maxOpaqueDepth = FLT_MAX;

			for ( std::deque<SqImageSample>::reverse_iterator sample = samples->
			        m_Data.rbegin();
			        sample != samples->m_Data.rend();
			        sample++ )
			{
				TqFloat* sample_data = sample->Data();
				if ( sample->isMatte() )
				{
					if ( sample->isOccludes() )
					{
						// Optimise common case
						samplecolor = gColBlack;
						sampleopacity = gColBlack;
					}
					else
					{
						samplecolor.SetColorRGB(
						    LERP( sample_data[Sample_ORed], samplecolor.fRed(), 0 ),
						    LERP( sample_data[Sample_OGreen], samplecolor.fGreen(), 0 ),
						    LERP( sample_data[Sample_OBlue], samplecolor.fBlue(), 0 )
						);
						sampleopacity.SetColorRGB(
						    LERP( sample_data[Sample_Red], sampleopacity.fRed(), 0 ),
						    LERP( sample_data[Sample_Green], sampleopacity.fGreen(), 0 ),
						    LERP( sample_data[Sample_Blue], sampleopacity.fBlue(), 0 )
						);
					}
				}
				else
				{
					samplecolor = ( samplecolor *
					                ( gColWhite - CqColor(CLAMP(sample_data[Sample_ORed], 0.0f, 1.0f), CLAMP(sample_data[Sample_OGreen], 0.0f, 1.0f), CLAMP(sample_data[Sample_OBlue], 0.0f, 1.0f)) ) ) +
					              CqColor(sample_data[Sample_Red], sample_data[Sample_Green], sample_data[Sample_Blue]);
					sampleopacity = ( ( gColWhite - sampleopacity ) *
					                  CqColor(sample_data[Sample_ORed], sample_data[Sample_OGreen], sample_data[Sample_OBlue]) ) +
					                sampleopacity;
				}

				// Now determine if the sample opacity meets the limit for depth mapping.
				// If so, store the depth in the appropriate nearest opaque sample slot.
				// The test is, if any channel of the opacity color is greater or equal to the threshold.
				if(sample_data[Sample_ORed] >= zThreshold.fRed() || sample_data[Sample_OGreen] >= zThreshold.fGreen() || sample_data[Sample_OBlue] >= zThreshold.fBlue())
				{
					// Make sure we store the nearest and second nearest depth values.
					opaqueDepths[1] = opaqueDepths[0];
					opaqueDepths[0] = sample->Data()[Sample_Depth];
					// Store the max opaque depth too, if not already stored.
					if(!(maxOpaqueDepth < FLT_MAX))
						maxOpaqueDepth = sample->Data()[Sample_Depth];
				}
				samplehit = true;
			}

			if ( samplehit )
			{
				samplecount++;
			}

			// Write the collapsed color values back into the opaque entry.
			if ( !samples->m_Data.empty() )
			{
				// Make sure the extra sample data from the top entry is copied
				// to the opaque sample, which is then sent to the display.
				opaqueValue = *samples->m_Data.begin();
				// Set the color and opacity.
				opaqueValue.Data()[Sample_Red] = samplecolor.fRed();
				opaqueValue.Data()[Sample_Green] = samplecolor.fGreen();
				opaqueValue.Data()[Sample_Blue] = samplecolor.fBlue();
				opaqueValue.Data()[Sample_ORed] = sampleopacity.fRed();
				opaqueValue.Data()[Sample_OGreen] = sampleopacity.fGreen();
				opaqueValue.Data()[Sample_OBlue] = sampleopacity.fBlue();
				opaqueValue.setValid();

				if ( depthfilter != Filter_Min )
				{
					if ( depthfilter == Filter_MidPoint )
					{
						//Aqsis::log() << debug << "OpaqueDepths: " << opaqueDepths[0] << " - " << opaqueDepths[1] << std::endl;
						// Use midpoint for depth
						if ( samples->m_Data.size() > 1 )
							opaqueValue.Data()[Sample_Depth] = ( ( opaqueDepths[0] + opaqueDepths[1] ) * 0.5f );
						else
							opaqueValue.Data()[Sample_Depth] = FLT_MAX;
					}
					else if ( depthfilter == Filter_Max)
					{
						opaqueValue.Data()[Sample_Depth] = maxOpaqueDepth;
					}
					else if ( depthfilter == Filter_Average )
					{
						std::deque<SqImageSample>::iterator sample;
						TqFloat totDepth = 0.0f;
						TqInt totCount = 0;
						for ( sample = samples->m_Data.begin(); sample != samples->m_Data.end(); sample++ )
						{
							TqFloat* sample_data = sample->Data();
							if(sample_data[Sample_ORed] >= zThreshold.fRed() || sample_data[Sample_OGreen] >= zThreshold.fGreen() || sample_data[Sample_OBlue] >= zThreshold.fBlue())
							{
								totDepth += sample_data[Sample_Depth];
								totCount++;
							}
						}
						totDepth /= totCount;

						opaqueValue.Data()[Sample_Depth] = totDepth;
					}
					// Default to "min"
				}
				else
					opaqueValue.Data()[Sample_Depth] = opaqueDepths[0];
			}
		}
		else
		{
			if (opaqueValue.isValid())
			{
				samplecount++;
			}
		}
	}
}

void CqImagePixel::OffsetSamples( std::vector<SqSampleData>& samplePoints, CqVector2D& vecPixel, std::vector<CqVector2D>& vecSamples )
{
	// add in the pixel offset
	const TqInt numSamples = m_XSamples * m_YSamples;
	for ( TqInt i = 0; i < numSamples; i++ )
	{
		samplePoints[ m_SampleIndices[i] ].m_Position = vecSamples[ i ];
		samplePoints[ m_SampleIndices[i] ].m_Position += vecPixel;
	}
}

//std::deque<SqImageSample>& CqImagePixel::Values( TqInt index )
//{
//    assert( index < m_XSamples*m_YSamples );
//	return ( CqBucket::SamplePoints()[m_SampleIndices[ index ]].m_Data );
//}

SqImageSample& CqImagePixel::OpaqueValues( std::vector<SqSampleData>& samplePoints, TqInt index )
{
	assert( index < m_XSamples*m_YSamples );
	return ( samplePoints[m_SampleIndices[ index ]].m_OpaqueSample );
}


const SqSampleData& CqImagePixel::SampleData( std::vector<SqSampleData>& samplePoints, TqInt index ) const
{
	assert( index < m_XSamples*m_YSamples );
	return ( samplePoints[m_SampleIndices[index]] );
}

SqSampleData& CqImagePixel::SampleData( std::vector<SqSampleData>& samplePoints, TqInt index )
{
	assert( index < m_XSamples*m_YSamples );
	return ( samplePoints[m_SampleIndices[index]] );
}


//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )
