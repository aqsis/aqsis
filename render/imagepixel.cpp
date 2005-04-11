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

#include	"aqsis.h"

#ifdef WIN32
#include    <windows.h>
#endif
#include	<math.h>

#include	"options.h"
#include	"renderer.h"
#include    "random.h"
#include	"imagepixel.h"
#include	"logging.h"


START_NAMESPACE( Aqsis )

//----------------------------------------------------------------------
/** Constructor
 */

CqImagePixel::CqImagePixel() :
        m_XSamples( 0 ),
        m_YSamples( 0 ),
        m_MaxDepth( FLT_MAX ),
        m_MinDepth( FLT_MAX ),
        m_OcclusionBoxId( -1 ),
        m_NeedsZUpdate( TqFalse )
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

void CqImagePixel::AllocateSamples( TqInt XSamples, TqInt YSamples )
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
			SqImageSample def( QGetRenderContext() ->GetOutputDataTotalSize() );
            m_Samples.resize( numSamples );
			m_DofOffsetIndices.resize( numSamples );
        }
    }
}

//----------------------------------------------------------------------
/** Fill in the sample array usig the multijitter function from GG IV.
 * \param vecPixel Cq2DVector pixel coordinate of this image element, used to make sure sample points are absolute, not relative.
 * \param fJitter Flag indicating whether to apply jittering to the sample points or not.
 */

void CqImagePixel::InitialiseSamples( std::vector<CqVector2D>& vecSamples, TqBool fJitter )
{
    TqFloat opentime = QGetRenderContext() ->optCurrent().GetFloatOption( "System", "Shutter" ) [ 0 ];
    TqFloat closetime = QGetRenderContext() ->optCurrent().GetFloatOption( "System", "Shutter" ) [ 1 ];

	TqInt numSamples = m_XSamples * m_YSamples;
    TqFloat subcell_width = 1.0f / numSamples;
    TqInt m = m_XSamples;
    TqInt n = m_YSamples;
    TqInt i, j;

    vecSamples.resize(numSamples);
    if ( !fJitter )
    {
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


        // Fill in the sample times for motion blur, LOD and SubCellIndex entries

        TqFloat time = 0;
        TqInt nSamples = m_XSamples*m_YSamples;
        TqFloat dtime = 1.0f / nSamples;

        for ( i = 0; i < nSamples; i++ )
        {
            m_Samples[ i ].m_SubCellIndex = 0;
            m_Samples[ i ].m_DetailLevel = m_Samples[ i ].m_Time = time;
            time += dtime;
        }


    }
    else
    {
    	static CqRandom random(  53 );

		// Initialize points to the "canonical" multi-jittered pattern.

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
                m_Samples[ which ].m_SubCellIndex = static_cast<TqInt>( ( yindex * m_YSamples ) + xindex );
                which++;
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
			m_Samples[ i ].m_Time = t;
			time += dtime;

			m_Samples[ i ].m_DetailLevel = lod + random.RandomFloat( dlod );
			lod += dlod;
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
		which = 0;
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
		std::random_shuffle(m_DofOffsetIndices.begin(), m_DofOffsetIndices.end());
		for( i = 0; i < numSamples; ++i)
		{
			m_Samples[m_DofOffsetIndices[i]].m_DofOffset = tmpDofOffsets[i];
		}
	}
	m_Data.m_Data.resize( QGetRenderContext()->GetOutputDataTotalSize() );
}


//----------------------------------------------------------------------
/** Shuffle the sample data to avoid repeating patterns in the sampling.
 */

void CqImagePixel::ShuffleSamples( )
{
	TqInt numSamples = m_XSamples * m_YSamples;

	// Shuffle the DoF offset indices.
	TqInt which = 0;
	std::vector<CqVector2D> tmpDofOffsets(numSamples);

	// Store the DoF offsets in the canonical order to ensure that
	// assumptions made about ordering during sampling still hold.
	TqInt i;
	for( i = 0; i < numSamples; ++i)
	{
		tmpDofOffsets[i] = m_Samples[m_DofOffsetIndices[i]].m_DofOffset;
		m_DofOffsetIndices[i] = i;
	}

	// we now shuffle the dof offsets but remember which one went where.
	std::random_shuffle(m_DofOffsetIndices.begin(), m_DofOffsetIndices.end());
	for( i = 0; i < numSamples; ++i)
	{
		m_Samples[m_DofOffsetIndices[i]].m_DofOffset = tmpDofOffsets[i];
	}
}

//----------------------------------------------------------------------
/** Clear the relevant data from the image element preparing it for the next usage.
 */

void CqImagePixel::Clear()
{
    TqInt i;
    for ( i = ( m_XSamples * m_YSamples ) - 1; i >= 0; i-- )
	{
		if(!m_Samples[i].m_Data.empty())
			m_Samples[ i ].m_Data.clear( );
		m_Samples[ i ].m_OpaqueSample.m_flags=0;
	}

    m_MaxDepth = FLT_MAX;
    m_MinDepth =  FLT_MAX;
    m_OcclusionBoxId = -1;
    m_NeedsZUpdate = TqFalse;
}


//----------------------------------------------------------------------
/** Get the color at the specified sample point by blending the colors that appear at that point.
 */

void CqImagePixel::Combine()
{
    TqInt depthfilter = 0;

    const CqString* pstrDepthFilter = QGetRenderContext() ->optCurrent().GetStringOption( "Hider", "depthfilter" );
    const CqColor* pzThreshold = QGetRenderContext() ->optCurrent().GetColorOption( "limits", "zthreshold" );
	CqColor zThreshold(1.0f, 1.0f, 1.0f);	// Default threshold of 1,1,1 means that any objects that are partially transparent won't appear in shadow maps.
	if(NULL != pzThreshold)
		zThreshold = pzThreshold[0];

    if ( NULL != pstrDepthFilter )
    {
        if( !pstrDepthFilter[ 0 ].compare( "min" ) )
            depthfilter = 0;
        else if ( !pstrDepthFilter[ 0 ].compare( "midpoint" ) )
            depthfilter = 1;
        else if ( !pstrDepthFilter[ 0 ].compare( "max" ) )
            depthfilter = 2;
        else if ( !pstrDepthFilter[ 0 ].compare( "average" ) )
            depthfilter = 3;
        else
            std::cerr << warning << "Invalid depthfilter \"" << pstrDepthFilter[ 0 ].c_str() << "\", depthfilter set to \"min\"" << std::endl;
    }

    TqUint samplecount = 0;
    TqUint numsamples = XSamples() * YSamples();
	TqInt sampleIndex = 0;
	std::vector<SqSampleData>::iterator end = m_Samples.end();
	for ( std::vector<SqSampleData>::iterator samples = m_Samples.begin(); samples != end; ++samples )
	{
		SqImageSample& opaqueValue = samples->m_OpaqueSample;
		sampleIndex++;

		if(!samples->m_Data.empty())
		{
			if(opaqueValue.m_flags & SqImageSample::Flag_Valid)
			{
				//	insert opaqueValue into samples in the right place.
				std::vector<SqImageSample>::iterator isi = samples->m_Data.begin();
				std::vector<SqImageSample>::iterator isend = samples->m_Data.end();
				while( isi != isend )
				{
					if((*isi).Depth() >= opaqueValue.Depth())
						break;

					++isi;
				}
				samples->m_Data.insert(isi, opaqueValue);
			}

			// Find out if any of the samples are in a CSG tree.
			TqBool bProcessed;
			TqBool CqCSGRequired = CqCSGTreeNode::IsRequired();
			if (CqCSGRequired)
				do
				{
					bProcessed = TqFalse;
					//Warning ProcessTree add or remove elements in samples list
					//We could not optimized the for loop here at all.
					for ( std::vector<SqImageSample>::iterator isample = samples->m_Data.begin(); isample != samples->m_Data.end(); ++isample )
					{
						if ( isample->m_pCSGNode )
						{
							isample->m_pCSGNode->ProcessTree( samples->m_Data );
							bProcessed = TqTrue;
							break;
						}
					}
				} while ( bProcessed );

			CqColor samplecolor = gColBlack;
			CqColor sampleopacity = gColBlack;
			TqBool samplehit = TqFalse;
			TqFloat opaqueDepths[2] = { FLT_MAX, FLT_MAX };
			TqFloat maxOpaqueDepth = FLT_MAX;

			for ( std::vector<SqImageSample>::reverse_iterator sample = samples->m_Data.rbegin(); sample != samples->m_Data.rend(); sample++ )
			{
				if ( sample->m_flags & SqImageSample::Flag_Matte )
				{
					if ( sample->m_flags & SqImageSample::Flag_Occludes )
					{
						// Optimise common case
						samplecolor = gColBlack;
						sampleopacity = gColBlack;
					}
					else
					{
						samplecolor.SetColorRGB(
							LERP( sample->Os().fRed(), samplecolor.fRed(), 0 ),
							LERP( sample->Os().fGreen(), samplecolor.fGreen(), 0 ),
							LERP( sample->Os().fBlue(), samplecolor.fBlue(), 0 )
						);
						sampleopacity.SetColorRGB(
							LERP( sample->Os().fRed(), sampleopacity.fRed(), 0 ),
							LERP( sample->Os().fGreen(), sampleopacity.fGreen(), 0 ),
							LERP( sample->Os().fBlue(), sampleopacity.fBlue(), 0 )
						);
					}
				}
				else
				{
					samplecolor = ( samplecolor * ( gColWhite - sample->Os() ) ) + sample->Cs();
					sampleopacity = ( ( gColWhite - sampleopacity ) * sample->Os() ) + sampleopacity;
				}

				// Now determine if the sample opacity meets the limit for depth mapping.
				// If so, store the depth in the appropriate nearest opaque sample slot.
				// The test is, if any channel of the opacity color is greater or equal to the threshold.
				if(sample->Os().fRed() >= zThreshold.fRed() || sample->Os().fGreen() >= zThreshold.fGreen() || sample->Os().fBlue() >= zThreshold.fBlue())
				{
					// Make sure we store the nearest and second nearest depth values.
					opaqueDepths[1] = opaqueDepths[0];
					opaqueDepths[0] = sample->Depth();
					// Store the max opaque depth too, if not already stored.
					if(!(maxOpaqueDepth < FLT_MAX))
						maxOpaqueDepth = sample->Depth();
				}
				samplehit = TqTrue;
			}

			if ( samplehit )
			{
				samplecount++;
			}

			// Write the collapsed color values back into the opaque entry.
			if ( !samples->m_Data.empty() )
			{
				// Set the color and opacity.
				opaqueValue.SetCs( samplecolor );
				opaqueValue.SetOs( sampleopacity );
				opaqueValue.m_flags |= SqImageSample::Flag_Valid;

				if ( depthfilter != 0)
				{
					if ( depthfilter == 1 )
					{
						//std::cerr << debug << "OpaqueDepths: " << opaqueDepths[0] << " - " << opaqueDepths[1] << std::endl;
						// Use midpoint for depth
						if ( samples->m_Data.size() > 1 )
							opaqueValue.SetDepth( ( opaqueDepths[0] + opaqueDepths[1] ) * 0.5f );
						else
							opaqueValue.SetDepth( FLT_MAX );
					}
					else if ( depthfilter == 2)
					{
						opaqueValue.SetDepth( maxOpaqueDepth );
					}
					else if ( depthfilter == 3 )
					{
						std::vector<SqImageSample>::iterator sample;
						TqFloat totDepth = 0.0f;
						TqInt totCount = 0;
						for ( sample = samples->m_Data.begin(); sample != samples->m_Data.end(); sample++ )
							if(sample->Os().fRed() >= zThreshold.fRed() || sample->Os().fGreen() >= zThreshold.fGreen() || sample->Os().fBlue() >= zThreshold.fBlue())
							{
								totDepth += sample->Depth();
								totCount++;
							}
						totDepth /= totCount;

						opaqueValue.SetDepth( totDepth );
					}
					// Default to "min"
				}
				else
					opaqueValue.SetDepth( opaqueDepths[0] );
			}
		}
		else
		{
			if(opaqueValue.m_flags & SqImageSample::Flag_Valid)
			{
				samplecount++;
			}
		}
	}
}

//----------------------------------------------------------------------
/** ReCalculate the min and max z values for this pixel
 */

void CqImagePixel::UpdateZValues()
{
    float currentMax = 0.0f;
    float currentMin = FLT_MAX;
	TqInt sampleIndex = 0;
    TqInt sx, sy;
    for ( sy = 0; sy < m_YSamples; sy++ )
    {
        for ( sx = 0; sx < m_XSamples; sx++ )
        {
			//SqImageSample& opaqueSample = m_OpaqueValues[ sampleIndex ];
			SqImageSample& opaqueSample = m_Samples[ sampleIndex ].m_OpaqueSample;
			if(opaqueSample.m_flags & SqImageSample::Flag_Valid)
            {
				if ( opaqueSample.Depth() > currentMax )
				{
					currentMax = opaqueSample.Depth();
				}
				if ( opaqueSample.Depth() < currentMin )
				{
					currentMin = opaqueSample.Depth();
				}
            }
            else
            {
                currentMax = FLT_MAX;
            }

			sampleIndex++;
        }
    }

    m_MaxDepth = currentMax;
    m_MinDepth = currentMin;
}

//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )
