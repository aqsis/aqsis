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
		\author Andy Gill (billybobjimboy@users.sf.net)
*/

#include	<strstream>
#ifdef WIN32
#include    <windows.h>
#endif
#include	<math.h>

#include	"aqsis.h"
#include	"stats.h"
#include	"options.h"
#include	"renderer.h"
#include	"surface.h"
#include	"micropolygon.h"
#include	"imagebuffer.h"
#include	"imagers.h"
#include	"occlusion.h"

#include	<map>

START_NAMESPACE( Aqsis )

static TqInt bucketmodulo = -1;

//----------------------------------------------------------------------
/** Constructor
 */

CqImagePixel::CqImagePixel() :
		m_XSamples( 0 ),
		m_YSamples( 0 ),
		//				m_aValues(0),
		//				m_avecSamples(0),
		m_colColor( 0, 0, 0 ),
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

CqImagePixel::CqImagePixel( const CqImagePixel& ieFrom ) : m_aValues( 0 ), m_avecSamples( 0 )
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
	m_XSamples = XSamples;
	m_YSamples = YSamples;
	TqInt numSamples = m_XSamples * m_YSamples;

	if ( XSamples > 0 && YSamples > 0 )
	{
		m_aValues.resize( numSamples );
		m_avecSamples.resize( numSamples);
		m_aSubCellIndex.resize( numSamples);
		m_aTimes.resize( numSamples );
		// XXX TODO: Compute this lazily
		m_aDetailLevels.resize( numSamples );
	}
}

void CqImagePixel::FreeSamples()
{
		m_aValues.clear();
		m_avecSamples.clear();
		m_aSubCellIndex.clear();
		m_aTimes.clear();
		m_aDetailLevels.clear();

}
//----------------------------------------------------------------------
/** Fill in the sample array usig the multijitter function from GG IV.
 * \param vecPixel Cq2DVector pixel coordinate of this image element, used to make sure sample points are absolute, not relative.
 * \param fJitter Flag indicating whether to apply jittering to the sample points or not.
 */

void CqImagePixel::InitialiseSamples( CqVector2D& vecPixel, TqBool fJitter )
{
	TqFloat subcell_width = 1.0f / ( m_XSamples * m_YSamples );
	TqInt m = m_XSamples;
	TqInt n = m_YSamples;
	TqInt i, j;	

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
				m_avecSamples[ ( y * m_XSamples ) + x ] = CqVector2D( XInc + ( XInc * x ), YSam ) + vecPixel;
		}
		
	
		// Fill in the sample times for motion blur, LOD and SubCellIndex entries
		
		TqFloat time = 0;
		TqInt nSamples = m_XSamples*m_YSamples;
		TqFloat dtime = 1.0f / nSamples;

		for ( i = 0; i < nSamples; i++ )
		{
			m_aSubCellIndex[ i ] = 0;
			m_aDetailLevels[ i ] = m_aTimes[ i ] = time;
			time += dtime;
			
		}


	}
	else
	{
    	// Initiliaze the random with a value based on the X,Y coordinate
		CqRandom random(  vecPixel.Magnitude()  );

		// Initialize points to the "canonical" multi-jittered pattern.

		for ( i = 0; i < n; i++ )
		{
			for ( j = 0; j < m; j++ )
			{
				TqInt which = i * m + j;
				m_avecSamples[which].x( i );
				m_avecSamples[which].y( j );
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
				t = m_avecSamples[ i * m + j ].y();
				m_avecSamples[ i * m + j ].y( m_avecSamples[ i * m + k ].y() );
				m_avecSamples[ i * m + k ].y( t );
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
				t = m_avecSamples[ j * m + i ].x();
				m_avecSamples[ j * m + i ].x( m_avecSamples[ k * m + i ].x() );
				m_avecSamples[ k * m + i ].x( t );

			}
		}


		TqFloat subpixelheight = 1.0f / m_YSamples;
		TqFloat subpixelwidth = 1.0f / m_XSamples;

		// finally add in the pixel offset
		for ( i = 0; i < n; i++ )
		{
			TqFloat sy = i * subpixelheight;
			for ( j = 0; j < m; j++ )
			{
				TqFloat sx = j * subpixelwidth;
				TqInt which = i * m + j;
				TqFloat xindex = m_avecSamples[ which ].x();
				TqFloat yindex = m_avecSamples[ which ].y();
				m_avecSamples[ which ].x( xindex * subcell_width + ( subcell_width * 0.5f ) + sx );
				m_avecSamples[ which ].y( yindex * subcell_width + ( subcell_width * 0.5f ) + sy );
				m_avecSamples[ which ] += vecPixel;
				m_aSubCellIndex[ which ] = static_cast<TqInt>( ( yindex * m_YSamples ) + xindex );
			}
		}


		// Fill in the sample times for motion blur, detail levels for LOD.
		
		TqFloat time = 0;
		TqInt nSamples = m_XSamples*m_YSamples;
		TqFloat dtime = 1.0f / nSamples;
		TqFloat lod = 0;
		TqFloat dlod = dtime;

		for ( i = 0; i < nSamples; i++ )
		{
			m_aTimes[ i ] = time + random.RandomFloat( dtime );
			time += dtime;
			m_aDetailLevels[ i ] = lod + random.RandomFloat( dlod );
			lod += dlod;

		}


	}
}


//----------------------------------------------------------------------
/** Clear the relevant data from the image element preparing it for the next usage.
 */

void CqImagePixel::Clear()
{
	TqInt i;
	for ( i = ( m_XSamples * m_YSamples ) - 1; i >= 0; i-- )
		m_aValues[ i ].resize( 0 );
}


//----------------------------------------------------------------------
/** Get the color at the specified sample point by blending the colors that appear at that point.
 */

void CqImagePixel::Combine()
{
	m_colColor = gColBlack;
	m_Depth = 0;
	m_Coverage = 0;
	TqInt depthfilter = 0;

	const CqString* pstrDepthFilter = QGetRenderContext() ->optCurrent().GetStringOption( "Hider", "depthfilter" );

	if ( NULL != pstrDepthFilter )
	{
		if ( !pstrDepthFilter[ 0 ].compare( "midpoint" ) )
			depthfilter = 1;
		else if ( !pstrDepthFilter[ 0 ].compare( "max" ) )
			depthfilter = 2;
		else if ( !pstrDepthFilter[ 0 ].compare( "average" ) )
			depthfilter = 3;
	}
	

	TqUint samplecount = 0;
	TqUint numsamples = XSamples() * YSamples();
	std::vector<std::vector<SqImageSample> >::iterator end = m_aValues.end();
	for ( std::vector<std::vector<SqImageSample> >::iterator samples = m_aValues.begin(); samples != end; samples++ )
	{
		// Find out if any of the samples are in a CSG tree.
		TqBool bProcessed;
        TqBool CqCSGRequired = CqCSGTreeNode::IsRequired();
        if (CqCSGRequired)
		do 
		{
			bProcessed = TqFalse;
            //Warning ProcessTree add or remove elements in samples list
            //We could not optimized the for loop here at all.
			for ( std::vector<SqImageSample>::iterator isample = samples->begin(); isample != samples->end(); isample++ )
			{
				if ( NULL != isample->m_pCSGNode )
				{
					isample->m_pCSGNode->ProcessTree( *samples );
					bProcessed = TqTrue;
					break;
				}
			}
		} while ( bProcessed );

		CqColor samplecolor = gColBlack;
		CqColor sampleopacity = gColBlack;
		TqBool samplehit = TqFalse;

		for ( std::vector<SqImageSample>::reverse_iterator sample = samples->rbegin(); sample != samples->rend(); sample++ )
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
					    LERP( sample->m_colOpacity.fRed(), samplecolor.fRed(), 0 ),
					    LERP( sample->m_colOpacity.fGreen(), samplecolor.fGreen(), 0 ),
					    LERP( sample->m_colOpacity.fBlue(), samplecolor.fBlue(), 0 )
					);
					sampleopacity.SetColorRGB(
					    LERP( sample->m_colOpacity.fRed(), sampleopacity.fRed(), 0 ),
					    LERP( sample->m_colOpacity.fGreen(), sampleopacity.fGreen(), 0 ),
					    LERP( sample->m_colOpacity.fBlue(), sampleopacity.fBlue(), 0 )
					);
				}
			}
			else
			{
				samplecolor = ( samplecolor * ( gColWhite - sample->m_colOpacity ) ) + sample->m_colColor;
				sampleopacity = ( ( gColWhite - sampleopacity ) * sample->m_colOpacity ) + sampleopacity;
			}
			samplehit = TqTrue;
		}

		if ( samplehit )
		{
			m_Coverage += 1;
			samplecount++;
		}

		// Write the collapsed color values back into the top entry.
		if ( samples->size() > 0 )
		{
			samples->begin() ->m_colColor = samplecolor;
			samples->begin() ->m_colOpacity = sampleopacity;
		
			if ( depthfilter != 0)
			{
				if ( depthfilter == 1 )
				{
					// Use midpoint for depth
					if ( samples->size() > 1 )
						( *samples ) [ 0 ].m_Depth = ( ( *samples ) [ 0 ].m_Depth + ( *samples ) [ 1 ].m_Depth ) * 0.5f;
					else
						( *samples ) [ 0 ].m_Depth = FLT_MAX;
				}
				else if ( depthfilter == 2)
				{
					( *samples ) [ 0 ].m_Depth = samples->back().m_Depth;
				}
				else if ( depthfilter == 3 )
				{
					std::vector<SqImageSample>::iterator sample;
					TqFloat totDepth = 0.0f;
					for ( sample = samples->begin(); sample != samples->end(); sample++ )
						totDepth += sample->m_Depth;
					totDepth /= samples->size();

					( *samples ) [ 0 ].m_Depth = totDepth;
				}
				// Default to "min"
			}
		}
	}

	if ( samplecount )
		m_Coverage /= numsamples;
}

//----------------------------------------------------------------------
/** ReCalculate the min and max z values for this pixel
 */

void CqImagePixel::UpdateZValues()
{
	float currentMax = 0.0f;
	float currentMin = FLT_MAX;
	TqInt sx, sy;
	for ( sy = 0; sy < m_YSamples; sy++ )
	{
		for ( sx = 0; sx < m_XSamples; sx++ )
		{
			std::vector<SqImageSample>& aValues = m_aValues[ sy * m_XSamples + sx ];

			if ( aValues.size() > 0 )
			{
				std::vector<SqImageSample>::iterator sc = aValues.begin();
				// find first opaque sample
				while ( sc != aValues.end() && ( ! ( sc->m_flags & SqImageSample::Flag_Occludes ) || ( sc->m_pCSGNode != NULL ) ) )
					sc++;
				if ( sc != aValues.end() )
				{
					if ( sc->m_Depth > currentMax )
					{
						currentMax = sc->m_Depth;
					}
					if ( sc->m_Depth < currentMin )
					{
						currentMin = sc->m_Depth;
					}
				}
				else
				{
					currentMax = FLT_MAX;
				}
			}
			else
			{
				currentMax = FLT_MAX;
			}
		}
	}

	m_MaxDepth = currentMax;
	m_MinDepth = currentMin;
}


//----------------------------------------------------------------------
/** Static data on CqBucket
 */

TqInt	CqBucket::m_XSize;
TqInt	CqBucket::m_YSize;
TqInt	CqBucket::m_XFWidth;
TqInt	CqBucket::m_YFWidth;
TqInt	CqBucket::m_XMax;
TqInt	CqBucket::m_YMax;
TqInt	CqBucket::m_XOrigin;
TqInt	CqBucket::m_YOrigin;
TqInt	CqBucket::m_XPixelSamples;
TqInt	CqBucket::m_YPixelSamples;
//TqFloat CqBucket::m_MaxDepth;
//TqInt	CqBucket::m_MaxDepthCount;
std::vector<CqImagePixel>	CqBucket::m_aieImage;
std::vector<TqFloat> CqBucket::m_aFilterValues;

//----------------------------------------------------------------------
/** Get a reference to pixel data.
 * \param iXPos Integer pixel coordinate.
 * \param iYPos Integer pixel coordinate.
 * \param pie Pointer to CqImagePixel to fill in.
 * \return Boolean indicating success, could fail if the specified pixel is not within the specified bucket.
 */

TqBool CqBucket::ImageElement( TqInt iXPos, TqInt iYPos, CqImagePixel*& pie )
{
	iXPos -= m_XOrigin;
	iYPos -= m_YOrigin;

	TqInt fxo2 = m_XMax;
	TqInt fyo2 = m_YMax;

	// Check within renderable range
	if ( iXPos >= -fxo2 && iXPos <= m_XSize + fxo2 &&
	        iYPos >= -fyo2 && iYPos <= m_YSize + fyo2 )
	{
		TqInt i = ( ( iYPos + fyo2 ) * ( m_XSize + m_XFWidth ) ) + ( iXPos + fxo2 );
		pie = &m_aieImage[ i ];
		return ( TqTrue );
	}
	else
	{
		std::cerr << "CqBucket::ImageElement() outside bucket boundary!\n";
		return ( TqFalse );
	}
}


//----------------------------------------------------------------------
/** Initialise the static image storage area.
 *  Clear,Allocate, Init. the m_aieImage samples
 */

void CqBucket::InitialiseBucket( TqInt xorigin, TqInt yorigin, TqInt xsize, TqInt ysize, TqInt xfwidth, TqInt yfwidth, TqInt xsamples, TqInt ysamples, TqBool fJitter )
{
	m_XOrigin = xorigin;
	m_YOrigin = yorigin;
	m_XSize = xsize;
	m_YSize = ysize;
	m_XFWidth = xfwidth;
	m_YFWidth = yfwidth;
	m_XMax = static_cast<TqInt>( CEIL( ( xfwidth - 1 ) * 0.5f ) );
	m_YMax = static_cast<TqInt>( CEIL( ( xfwidth - 1 ) * 0.5f ) );
	m_XPixelSamples = xsamples;
	m_YPixelSamples = ysamples;

	TqInt ywidth1, xwidth1;
	ywidth1 = m_YSize + m_YFWidth;
	xwidth1 = m_XSize + m_XFWidth;

	// Allocate the image element storage for a single bucket if it different 
	// but from bucket to bucket it will be the same size all the times.
	if (m_aieImage.size() != (xwidth1 * ywidth1) )
		m_aieImage.resize( xwidth1 * ywidth1);

	// Initialise the samples for this bucket.
	TqInt i;

	for ( i = 0; i < ywidth1; i++ )
	{
		TqInt j;
		for ( j = 0; j < xwidth1; j++ )
		{
			CqVector2D bPos2( m_XOrigin, m_YOrigin );
			bPos2 += CqVector2D( ( j - m_XFWidth / 2 ), ( i - m_YFWidth / 2 ) );
			TqInt which = ( i * ( xwidth1 ) ) + j ;

		    m_aieImage[which].Clear();
			//m_aieImage[which].FreeSamples();
			m_aieImage[which].AllocateSamples( xsamples, ysamples );
			m_aieImage[which].InitialiseSamples( bPos2, fJitter );
		}
	}
}


//----------------------------------------------------------------------
/** Initialise the static filter values.
 */

void CqBucket::InitialiseFilterValues()
{
	// Allocate and fill in the filter values array for each pixel.
	TqInt numsubpixels = ( m_XPixelSamples * m_YPixelSamples );
	TqInt numperpixel = numsubpixels * numsubpixels;

	TqInt numvalues = static_cast<TqInt>( ( ( m_XFWidth + 1 ) * ( m_YFWidth + 1 ) ) * ( numperpixel ) );

	if (m_aFilterValues.size() != numvalues)
	{
		m_aFilterValues.resize( numvalues );
	} else return;
	RtFilterFunc pFilter;
	pFilter = QGetRenderContext() ->optCurrent().funcFilter();

	TqFloat xmax = m_XMax;
	TqFloat ymax = m_YMax;
	TqFloat xfwo2 = m_XFWidth * 0.5f;
	TqFloat yfwo2 = m_YFWidth * 0.5f;
	TqFloat xfw = m_XFWidth;
	TqFloat yfw = m_YFWidth;
	
	TqFloat subcellwidth = 1.0f / numsubpixels;
	TqFloat subcellcentre = subcellwidth * 0.5f;
	
	// Go over every pixel touched by the filter
	TqInt px, py;
	for ( py = static_cast<TqInt>( -ymax ); py <= static_cast<TqInt>( ymax ); py++ )
	{
		for( px = static_cast<TqInt>( -xmax ); px <= static_cast<TqInt>( xmax ); px++ )
		{
			// Get the index of the pixel in the array.
			TqInt index = static_cast<TqInt>( ( ( ( py + ymax ) * xfw ) + ( px + xmax ) ) * numperpixel );
			TqFloat pfx = px - 0.5f;
			TqFloat pfy = py - 0.5f;
			// Go over every subpixel in the pixel.
			TqInt sx, sy;
			for ( sy = 0; sy < m_YPixelSamples; sy++ )
			{
				for ( sx = 0; sx < m_XPixelSamples; sx++ )
				{
					// Get the index of the subpixel in the array
					TqInt sindex = index + ( ( ( sy * m_XPixelSamples ) + sx ) * numsubpixels );
					TqFloat sfx = static_cast<TqFloat>( sx ) / m_XPixelSamples;
					TqFloat sfy = static_cast<TqFloat>( sy ) / m_YPixelSamples;
					// Go over each subcell in the subpixel
					TqInt cx, cy;
					for ( cy = 0; cy < m_XPixelSamples; cy++ )
					{
						for ( cx = 0; cx < m_YPixelSamples; cx++ )
						{
							// Get the index of the subpixel in the array
							TqInt cindex = sindex + ( ( cy * m_YPixelSamples ) + cx );
							TqFloat fx = ( cx * subcellwidth ) + sfx + pfx + subcellcentre;
							TqFloat fy = ( cy * subcellwidth ) + sfy + pfy + subcellcentre;
							TqFloat w = 0.0f;
							if ( fx >= -xfwo2 && fy >= -yfwo2 && fx <= xfwo2 && fy <= yfwo2 )
								w = ( *pFilter ) ( fx, fy, m_XFWidth, m_YFWidth );
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

void CqBucket::CombineElements()
{
	std::vector<CqImagePixel>::iterator end = m_aieImage.end();
	for ( std::vector<CqImagePixel>::iterator i = m_aieImage.begin(); i != end ; i++ )
		i->Combine();
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
	if ( ImageElement( iXPos, iYPos, pie ) )
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
	if ( ImageElement( iXPos, iYPos, pie ) )
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
	if ( ImageElement( iXPos, iYPos, pie ) )
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
	if ( ImageElement( iXPos, iYPos, pie ) )
		return ( pie->Depth() );
	else
		return ( FLT_MAX );
}


//----------------------------------------------------------------------
/** Get the maximum sample depth for the specified screen position.
 * If position is outside bucket, returns FLT_MAX.
 * \param iXPos Screen position of sample.
 * \param iYPos Screen position of sample.
 */

TqFloat CqBucket::MaxDepth( TqInt iXPos, TqInt iYPos )
{
	CqImagePixel * pie;
	if ( ImageElement( iXPos, iYPos, pie ) )
		return ( pie->MaxDepth() );
	else
		return ( FLT_MAX );
}


//----------------------------------------------------------------------
/** Filter the samples in this bucket according to type and filter widths.
 */

void CqBucket::FilterBucket()
{
	CqImagePixel * pie;


	CqColor* pCols = new CqColor[ XSize() * YSize() ];
	CqColor* pOpacs = new CqColor[ XSize() * YSize() ];
	TqFloat* pDepths = new TqFloat[ XSize() * YSize() ];
	TqFloat* pCoverages = new TqFloat[ XSize() * YSize() ];

	TqInt xmax = static_cast<TqInt>( CEIL( ( XFWidth() - 1 ) * 0.5f ) );
	TqInt ymax = static_cast<TqInt>( CEIL( ( YFWidth() - 1 ) * 0.5f ) );
	TqFloat xfwo2 = XFWidth() * 0.5f;
	TqFloat yfwo2 = YFWidth() * 0.5f;
	TqInt numsubpixels = ( m_XPixelSamples * m_YPixelSamples );

	TqInt numperpixel = numsubpixels * numsubpixels;
	TqInt	xlen = XSize() + XFWidth();

	TqInt SampleCount = 0;
	CqColor imager;

	TqInt x, y;
	TqInt i = 0;
	TqFloat total = numsubpixels;

	TqBool fImager = ( QGetRenderContext() ->optCurrent().GetStringOption( "System", "Imager" ) [ 0 ] != "null" );

	TqFloat endy = YOrigin() + YSize();
	TqFloat endx = XOrigin() + XSize();
	for ( y = YOrigin(); y < endy ; y++ )
	{
		TqFloat ycent = y + 0.5f;
		for ( x = XOrigin(); x < endx ; x++ )
		{
			TqFloat xcent = x + 0.5f;
			CqColor c = gColBlack;
			CqColor o = gColBlack;
			TqFloat d = 0;
			TqFloat gTot = 0.0;
			SampleCount = 0;

			TqInt fx, fy;
			// Get the element at the upper left corner of the filter area.
			ImageElement( x - xmax, y - ymax, pie );
			for ( fy = -ymax; fy <= ymax; fy++ )
			{
				CqImagePixel* pie2 = pie;
				for ( fx = -xmax; fx <= xmax; fx++ )
				{
					TqInt index = ( ( ( fy + ymax ) * XFWidth() ) + ( fx + xmax ) ) * numperpixel;
					// Now go over each subsample within the pixel
					TqInt sx, sy;
					for ( sy = 0; sy < m_YPixelSamples; sy++ )
					{
						for ( sx = 0; sx < m_XPixelSamples; sx++ )
						{
							TqInt sindex = index + ( ( ( sy * m_XPixelSamples ) + sx ) * numsubpixels );
							CqVector2D vecS = pie2->SamplePoint( sx, sy );
							vecS -= CqVector2D( xcent, ycent );
							if ( vecS.x() >= -xfwo2 && vecS.y() >= -yfwo2 && vecS.x() <= xfwo2 && vecS.y() <= yfwo2 )
							{
								TqInt cindex = sindex + pie2->SubCellIndex( sx, sy );
								TqFloat g = m_aFilterValues[ cindex ];
								gTot += g;
								if ( pie2->Values( sx, sy ).size() > 0 )
								{
									c += pie2->Values( sx, sy ) [ 0 ].m_colColor * g;
									o += pie2->Values( sx, sy ) [ 0 ].m_colOpacity * g;
									d += pie2->Values( sx, sy ) [ 0 ].m_Depth;
									SampleCount++;
								}
							}
						}
					}
					pie2++;
				}
				pie += xlen;
			}
			pCols[ i ] = c / gTot;
			pOpacs[ i ] = o / gTot;

			if ( SampleCount > numsubpixels)
				pCoverages[ i ] = 1.0;
			else
				pCoverages[ i ] = ( TqFloat ) SampleCount / ( TqFloat ) (numsubpixels );

			if ( NULL != QGetRenderContext() ->optCurrent().pshadImager() && NULL != QGetRenderContext() ->optCurrent().pshadImager() ->pShader() )
			{
				// Init & Execute the imager shader
				QGetRenderContext() ->optCurrent().InitialiseColorImager( 1, 1,
				        x, y,
				        &pCols[ i ], &pOpacs[ i ],
				        &pDepths[ i ], &pCoverages[ i ] );

				if ( fImager )
				{
					imager = QGetRenderContext() ->optCurrent().GetColorImager( x , y );
					// Normal case will be to poke the alpha from the image shader and
					// multiply imager color with it... but after investigation alpha is always
					// == 1 after a call to imager shader in 3delight and BMRT.
					// Therefore I did not ask for alpha value and set directly the pCols[i]
					// with imager value. see imagers.cpp
					pCols[ i ] = imager;
					imager = QGetRenderContext() ->optCurrent().GetOpacityImager( x , y );
					pOpacs[ i ] = imager;
				}
			}

			if ( SampleCount > 0 )
				pDepths[ i++ ] = d / SampleCount;
			else
				pDepths[ i++ ] = FLT_MAX;
		}
	}

	i = 0;
	ImageElement( XOrigin(), YOrigin(), pie );
	endy = YSize();
	endx = XSize();
	
	for ( y = 0; y < endy; y++ )
	{
		CqImagePixel* pie2 = pie;
		for ( x = 0; x < endx; x++ )
		{
			pie2->Color() = pCols[ i ];
			pie2->Opacity() = pOpacs[ i ];
			pie2->SetDepth( pDepths[ i++ ] );
			pie2++;
		}
		pie += xlen;
	}
	delete[] ( pCols );
	delete[] ( pOpacs );
	delete[] ( pDepths );
	delete[] ( pCoverages );

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
		endy = YSize();
		endx = XSize();
		nextx = XSize() + XFWidth();
	
		for ( y = 0; y < endy; y++ )
		{
			CqImagePixel* pie2 = pie;
			for ( x = 0; x < endx; x++ )
			{
				// color=(color*gain)^1/gamma
				if ( exposegain != 1.0 )
					pie2->Color() *= exposegain;

				if ( exposegamma != 1.0 )
				{
					pie2->Color().SetfRed ( pow( pie2->Color().fRed (), oneovergamma ) );
					pie2->Color().SetfGreen( pow( pie2->Color().fGreen(), oneovergamma ) );
					pie2->Color().SetfBlue ( pow( pie2->Color().fBlue (), oneovergamma ) );
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
        CqVector2D area(XOrigin(), YOrigin()); 
	CqRandom random( area.Magnitude() );
	TqFloat endx, endy;
	TqInt   nextx;
	endy = YSize();
	endx = XSize();
	nextx = XSize() + XFWidth();


	if ( QGetRenderContext() ->optCurrent().GetIntegerOption( "System", "DisplayMode" ) [ 0 ] & ModeRGB )
	{
		double ditheramplitude = QGetRenderContext() ->optCurrent().GetFloatOption( "System", "ColorQuantizeDitherAmplitude" ) [ 0 ];
		TqInt one = QGetRenderContext() ->optCurrent().GetIntegerOption( "System", "ColorQuantizeOne" ) [ 0 ];
		TqInt min = QGetRenderContext() ->optCurrent().GetIntegerOption( "System", "ColorQuantizeMin" ) [ 0 ];
		TqInt max = QGetRenderContext() ->optCurrent().GetIntegerOption( "System", "ColorQuantizeMax" ) [ 0 ];

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
				double r, g, b;
				double _or, _og, _ob;
				double s = random.RandomFloat();
				if ( modf( one * pie2->Color().fRed () + ditheramplitude * s, &r ) > 0.5 ) r += 1;
				if ( modf( one * pie2->Color().fGreen() + ditheramplitude * s, &g ) > 0.5 ) g += 1;
				if ( modf( one * pie2->Color().fBlue () + ditheramplitude * s, &b ) > 0.5 ) b += 1;
				if ( modf( one * pie2->Opacity().fRed () + ditheramplitude * s, &_or ) > 0.5 ) _or += 1;
				if ( modf( one * pie2->Opacity().fGreen() + ditheramplitude * s, &_og ) > 0.5 ) _og += 1;
				if ( modf( one * pie2->Opacity().fBlue () + ditheramplitude * s, &_ob ) > 0.5 ) _ob += 1;
				r = CLAMP( r, min, max );
				g = CLAMP( g, min, max );
				b = CLAMP( b, min, max );
				_or = CLAMP( _or, min, max );
				_og = CLAMP( _og, min, max );
				_ob = CLAMP( _ob, min, max );
				pie2->Color().SetfRed ( r );
				pie2->Color().SetfGreen( g );
				pie2->Color().SetfBlue ( b );
				pie2->Opacity().SetfRed ( _or );
				pie2->Opacity().SetfGreen( _og );
				pie2->Opacity().SetfBlue ( _ob );

				pie2++;
			}
			pie += nextx;
		}
	}
	else
	{
		double ditheramplitude = QGetRenderContext() ->optCurrent().GetFloatOption( "System", "DepthQuantizeDitherAmplitude" ) [ 0 ];
		if ( ditheramplitude == 0 ) return ;
		TqInt one = QGetRenderContext() ->optCurrent().GetIntegerOption( "System", "DepthQuantizeOne" ) [ 0 ];
		TqInt min = QGetRenderContext() ->optCurrent().GetIntegerOption( "System", "DepthQuantizeMin" ) [ 0 ];
		TqInt max = QGetRenderContext() ->optCurrent().GetIntegerOption( "System", "DepthQuantizeMax" ) [ 0 ];

		CqImagePixel* pie;
		ImageElement( XOrigin(), YOrigin(), pie );
		TqInt x, y;
		for ( y = 0; y < endy; y++ )
		{
			CqImagePixel* pie2 = pie;
			for ( x = 0; x < endx; x++ )
			{
				double d;
				if ( modf( one * pie2->Depth() + ditheramplitude * random.RandomFloat(), &d ) > 0.5 ) d += 1;
				d = CLAMP( d, min, max );
				pie2->SetDepth( d );
				pie2++;
			}
			pie += nextx;
		}
	}
}

//----------------------------------------------------------------------
/** Place GPrim into bucket in order of depth
 * \param The Gprim to be added.
 */
void CqBucket::AddGPrim( CqBasicSurface* pGPrim )
{
	if ( pGPrim->fCachedBound() )
	{
		CqBasicSurface * surf = m_aGPrims.pFirst();
		while ( surf != 0 )
		{
			if (surf == pGPrim)
			{
				return;
			}

			if ( surf->fCachedBound() )
			{
				if ( surf->GetCachedRasterBound().vecMin().z() > pGPrim->GetCachedRasterBound().vecMin().z() )
				{
					pGPrim->LinkBefore( surf );
					return ;
				}
			}
			surf = surf->pNext();
		}
	}
	m_aGPrims.LinkFirst( pGPrim );
}



//----------------------------------------------------------------------
/** Destructor
 */

CqImageBuffer::~CqImageBuffer()
{
	DeleteImage();
}


//----------------------------------------------------------------------
/** Get the bucket index for the specified image coordinates.
 * \param X Integer pixel coordinate in X.
 * \param Y Integer pixel coordinate in Y.
 * \param Xb Storage for integer bucket index in X.
 * \param Yb Storage for integer bucket index in Y.
 * \return Integer bucket index.
 */

TqInt CqImageBuffer::Bucket( TqInt X, TqInt Y, TqInt& Xb, TqInt& Yb ) const
{
	Xb = static_cast<TqInt>( ( X / m_XBucketSize ) );
	Yb = static_cast<TqInt>( ( Y / m_YBucketSize ) );

	return ( ( Yb * m_cXBuckets ) + Xb );
}


//----------------------------------------------------------------------
/** Get the bucket index for the specified image coordinates.
 * \param X Integer pixel coordinate in X.
 * \param Y Integer pixel coordinate in Y.
 * \return Integer bucket index.
 */

TqInt CqImageBuffer::Bucket( TqInt X, TqInt Y ) const
{
	static TqInt Xb, Yb;
	return ( Bucket( X, Y, Xb, Yb ) );
}


//----------------------------------------------------------------------
/** Get the screen position for the specified bucket index.
 * \param iBucket Integer bucket index (0-based).
 * \return Bucket position as 2d vector (xpos, ypos).
 */

CqVector2D	CqImageBuffer::Position( TqInt iBucket ) const
{
	CqVector2D	vecA;
	vecA.y( iBucket / m_cXBuckets );
	vecA.x( iBucket % m_cXBuckets );
	vecA.x( vecA.x() * XBucketSize() );
	vecA.y( vecA.y() * YBucketSize() );

	return ( vecA );
}

//----------------------------------------------------------------------
/** Get the bucket size for the specified index.
 
    Usually the return value is just (XBucketSize, YBucketSize) except
    for the buckets on the right and bottom side of the image where the
    size can be smaller. The crop window is not taken into account.
 
 * \param iBucket Integer bucket index.
 * \return Bucket size as 2d vector (xsize, ysize).
 */

CqVector2D CqImageBuffer::Size( TqInt iBucket ) const
{
	CqVector2D	vecA = Position( iBucket );
	vecA.x( m_iXRes - vecA.x() );
	if ( vecA.x() > m_XBucketSize ) vecA.x( m_XBucketSize );
	vecA.y( m_iYRes - vecA.y() );
	if ( vecA.y() > m_YBucketSize ) vecA.y( m_YBucketSize );

	return ( vecA );
}


//----------------------------------------------------------------------
/** Construct the image buffer to an initial state using the current options.
 */

void	CqImageBuffer::SetImage()
{
	DeleteImage();

	m_XBucketSize = 16;
	m_YBucketSize = 16;
	const TqInt* poptBucketSize = QGetRenderContext() ->optCurrent().GetIntegerOption( "limits", "bucketsize" );
	if ( poptBucketSize != 0 )
	{
		m_XBucketSize = poptBucketSize[ 0 ];
		m_YBucketSize = poptBucketSize[ 1 ];
	}

	m_iXRes = QGetRenderContext() ->optCurrent().GetIntegerOption( "System", "Resolution" ) [ 0 ];
	m_iYRes = QGetRenderContext() ->optCurrent().GetIntegerOption( "System", "Resolution" ) [ 1 ];
	m_CropWindowXMin = static_cast<TqInt>( CLAMP( CEIL( m_iXRes * QGetRenderContext() ->optCurrent().GetFloatOption( "System", "CropWindow" ) [ 0 ] ), 0, m_iXRes ) );
	m_CropWindowXMax = static_cast<TqInt>( CLAMP( CEIL( m_iXRes * QGetRenderContext() ->optCurrent().GetFloatOption( "System", "CropWindow" ) [ 1 ] ), 0, m_iXRes ) );
	m_CropWindowYMin = static_cast<TqInt>( CLAMP( CEIL( m_iYRes * QGetRenderContext() ->optCurrent().GetFloatOption( "System", "CropWindow" ) [ 2 ] ), 0, m_iYRes ) );
	m_CropWindowYMax = static_cast<TqInt>( CLAMP( CEIL( m_iYRes * QGetRenderContext() ->optCurrent().GetFloatOption( "System", "CropWindow" ) [ 3 ] ), 0, m_iYRes ) );
	m_cXBuckets = ( m_iXRes / m_XBucketSize ) + 1;
	m_cYBuckets = ( m_iYRes / m_YBucketSize ) + 1;
	m_PixelXSamples = QGetRenderContext() ->optCurrent().GetIntegerOption( "System", "PixelSamples" ) [ 0 ];
	m_PixelYSamples = QGetRenderContext() ->optCurrent().GetIntegerOption( "System", "PixelSamples" ) [ 1 ];
	m_FilterXWidth = static_cast<TqInt>( QGetRenderContext() ->optCurrent().GetFloatOption( "System", "FilterWidth" ) [ 0 ] );
	m_FilterYWidth = static_cast<TqInt>( QGetRenderContext() ->optCurrent().GetFloatOption( "System", "FilterWidth" ) [ 1 ] );
	m_ClippingNear = static_cast<TqFloat>( QGetRenderContext() ->optCurrent().GetFloatOption( "System", "Clipping" ) [ 0 ] );
	m_ClippingFar = static_cast<TqFloat>( QGetRenderContext() ->optCurrent().GetFloatOption( "System", "Clipping" ) [ 1 ] );
	m_DisplayMode = QGetRenderContext() ->optCurrent().GetIntegerOption( "System", "DisplayMode" ) [ 0 ];

	m_aBuckets.resize( m_cXBuckets * m_cYBuckets );

	m_iCurrentBucket = 0;
}


//----------------------------------------------------------------------
/** Delete the allocated memory for the image buffer.
 */

void	CqImageBuffer::DeleteImage()
{
	m_iXRes = 0;
	m_iYRes = 0;
}


//----------------------------------------------------------------------
/** This is called by the renderer to inform an image buffer it is no longer needed.
 */

void	CqImageBuffer::Release()
{
	delete( this );
}


//----------------------------------------------------------------------
/** Check if a surface can be culled and transform bound.
 
    This method checks if the surface lies outside the viewing volume
    and returns true if it does.
    Additionally it checks if the surface spans the eye plane and marks it
    as undiceable if it does (it might even be marked as discarded).
    It also grows the bound by half the filter width and transforms it
    into raster space.
 
 * \param Bound CqBound containing the geometric bound in camera space.
 * \param pSurface Pointer to the CqBasicSurface derived class being processed.
 * \return Boolean indicating that the GPrim can be culled.
 
  \bug If the gprim spans the eye plane the bound is not transformed into raster
   space (how could it anyway), but PostSurface() relies on this behaviour and
   inserts EVERY gprim into buckets (using a bound that is still in camera space).
 */

TqBool CqImageBuffer::CullSurface( CqBound& Bound, CqBasicSurface* pSurface )
{
	// If the primitive is completely outside of the hither-yon z range, cull it.
	if ( Bound.vecMin().z() >= QGetRenderContext() ->optCurrent().GetFloatOption( "System", "Clipping" ) [ 1 ] ||
	        Bound.vecMax().z() <= QGetRenderContext() ->optCurrent().GetFloatOption( "System", "Clipping" ) [ 0 ] )
		return ( TqTrue );

	// If the primitive spans the epsilon plane and the hither plane and can be split,
	if ( Bound.vecMin().z() <= 0.0f && Bound.vecMax().z() > FLT_EPSILON )
	{
		// Mark the primitive as not dicable.
		pSurface->ForceUndiceable();
		TqInt MaxEyeSplits = 10;
		const TqInt* poptEyeSplits = QGetRenderContext() ->optCurrent().GetIntegerOption( "limits", "eyesplits" );
		if ( poptEyeSplits != 0 )
			MaxEyeSplits = poptEyeSplits[ 0 ];

		if ( pSurface->EyeSplitCount() > MaxEyeSplits )
		{
			CqAttributeError( ErrorID_MaxEyeSplits, Severity_Normal, "Max eyesplits exceeded", pSurface->pAttributes(), TqTrue );
			pSurface->Discard();
		}
		return ( TqFalse );
	}

	TqFloat minz = Bound.vecMin().z();
	TqFloat maxz = Bound.vecMax().z();

	// Convert the bounds to raster space.
	Bound.Transform( QGetRenderContext() ->matSpaceToSpace( "camera", "raster" ) );
	Bound.vecMin().x( Bound.vecMin().x() - m_FilterXWidth / 2 );
	Bound.vecMin().y( Bound.vecMin().y() - m_FilterYWidth / 2 );
	Bound.vecMax().x( Bound.vecMax().x() + m_FilterXWidth / 2 );
	Bound.vecMax().y( Bound.vecMax().y() + m_FilterYWidth / 2 );

	// If the bounds are completely outside the viewing frustum, cull the primitive.
	if ( Bound.vecMin().x() > CropWindowXMax() + m_FilterXWidth / 2 ||
	        Bound.vecMin().y() > CropWindowYMax() + m_FilterYWidth / 2 ||
	        Bound.vecMax().x() < CropWindowXMin() - m_FilterXWidth / 2 ||
	        Bound.vecMax().y() < CropWindowYMin() - m_FilterYWidth / 2 )
		return ( TqTrue );

	// Restore Z-Values to camera space.
	Bound.vecMin().z( minz );
	Bound.vecMax().z( maxz );

	// Cache the Bound.
	pSurface->CacheRasterBound( Bound );
	return ( TqFalse );
}


//----------------------------------------------------------------------
/** Add a new surface to the front of the list of waiting ones.
 * \param pSurface A pointer to a CqBasicSurface derived class, surface should at this point be in camera space.
 */

void CqImageBuffer::PostSurface( CqBasicSurface* pSurface )
{
	// Count the number of total gprims
	QGetRenderContext() ->Stats().IncTotalGPrims();

	// Bound the primitive in its current space (camera) space taking into account any motion specification.
	CqBound Bound( pSurface->Bound() );

	// Take into account the displacement bound extension.
	TqFloat db = 0.0f;
	CqString strCoordinateSystem( "object" );
	const TqFloat* pattrDispclacementBound = pSurface->pAttributes() ->GetFloatAttribute( "displacementbound", "sphere" );
	const CqString* pattrCoordinateSystem = pSurface->pAttributes() ->GetStringAttribute( "displacementbound", "coordinatesystem" );
	if ( pattrDispclacementBound != 0 ) db = pattrDispclacementBound[ 0 ];
	if ( pattrCoordinateSystem != 0 ) strCoordinateSystem = pattrCoordinateSystem[ 0 ];

	//if ( db != 0.0f )
	{
		CqVector3D	vecDB( db, 0, 0 );
		CqMatrix matShaderToWorld;
		if ( NULL != pSurface->pAttributes() ->pshadSurface() )
			matShaderToWorld = pSurface->pAttributes() ->pshadSurface() ->matCurrent();
		vecDB = QGetRenderContext() ->matVSpaceToSpace( strCoordinateSystem.c_str(), "camera", matShaderToWorld, pSurface->pTransform() ->matObjectToWorld() ) * vecDB;
		db = vecDB.Magnitude();

		Bound.vecMax() += db;
		Bound.vecMin() -= db;
	}

	// Check if the surface can be culled. (also converts Bound to raster space).
	if ( CullSurface( Bound, pSurface ) )
	{
		pSurface->UnLink();
		pSurface->Release();
		QGetRenderContext() ->Stats().IncCulledGPrims();
		return ;
	}

	// If the primitive has been marked as undiceable by the eyeplane check, then we cannot get a valid
	// bucket index from it as the projection of the bound would cross the camera plane and therefore give a false
	// result, so just put it back in the current bucket for further splitting.
	TqInt iBucket = iCurrentBucket();
	TqInt nBucket = iBucket;
	if ( !pSurface->IsUndiceable() )
	{
		// Find out which bucket(s) the surface belongs to.
		TqInt XMinb, YMinb;
		if ( Bound.vecMin().x() < 0 ) Bound.vecMin().x( 0.0f );
		if ( Bound.vecMin().y() < 0 ) Bound.vecMin().y( 0.0f );
		nBucket = Bucket( static_cast<TqInt>( Bound.vecMin().x() ), static_cast<TqInt>( Bound.vecMin().y() ), XMinb, YMinb );

		if ( XMinb >= m_cXBuckets || YMinb >= m_cYBuckets ) return ;

		if ( XMinb < 0 || YMinb < 0 )
		{
			if ( XMinb < 0 ) XMinb = 0;
			if ( YMinb < 0 ) YMinb = 0;
			nBucket = ( YMinb * m_cXBuckets ) + XMinb;
		}
	}

	m_aBuckets[ nBucket ].AddGPrim( pSurface );

}


//----------------------------------------------------------------------
/** Test if this surface can be occlusion culled. If it can then
 * transfer surface to the next bucket it covers, or delete it if it
 * covers no more.
 * \param iBucket Integer index of bucket being processed.
 * \param pSurface A pointer to a CqBasicSurface derived class.
 * \return Boolean indicating that the GPrim has been culled.
*/

TqBool CqImageBuffer::OcclusionCullSurface( TqInt iBucket, CqBasicSurface* pSurface )
{
	CqBound RasterBound( pSurface->GetCachedRasterBound() );

	TqInt nBuckets = m_cXBuckets * m_cYBuckets;

	if ( pSurface->pCSGNode() != NULL )
		return ( TqFalse );

	if ( CqOcclusionBox::CanCull( &RasterBound ) )
	{
		// pSurface is behind everying in this bucket but it may be
		// visible in other buckets it overlaps.
		// bucket to the right
		TqInt nextBucket = iBucket + 1;
		CqVector2D pos = Position( nextBucket );
		if ( ( nextBucket < nBuckets  ) &&
		        ( RasterBound.vecMax().x() >= pos.x() ) )
		{
			pSurface->UnLink();
			m_aBuckets[ nextBucket ].AddGPrim( pSurface );
			return TqTrue;
		}

		// bucket below
		nextBucket = iBucket + m_cXBuckets;
		pos = Position( nextBucket );
		// find bucket containing left side of bound
		pos.x( RasterBound.vecMin().x() );
		nextBucket = Bucket( static_cast<TqInt>( pos.x() ), static_cast<TqInt>( pos.y() ) );

		if ( ( nextBucket <nBuckets ) &&
		        ( RasterBound.vecMax().y() >= pos.y() ) )
		{
			pSurface->UnLink();
			m_aBuckets[ nextBucket ].AddGPrim( pSurface );
			return TqTrue;
		}

		// Bound covers no more buckets
		pSurface->UnLink();
		pSurface->Release();
		QGetRenderContext() ->Stats().IncCulledGPrims();
		return TqTrue;
	}
	else
		return TqFalse;
}


//----------------------------------------------------------------------
/** Add a new micro polygon to the list of waiting ones.
 * \param pmpgNew Pointer to a CqMicroPolygonBase derived class.
 */

void CqImageBuffer::AddMPG( CqMicroPolygonBase* pmpgNew )
{
	// Find out which bucket(s) the mpg belongs to.
	CqBound	B( pmpgNew->GetTotalBound() );
	pmpgNew->AddRef();

	if ( B.vecMax().x() < m_CropWindowXMin - m_FilterXWidth / 2 || B.vecMax().y() < m_CropWindowYMin - m_FilterYWidth / 2 ||
	        B.vecMin().x() > m_CropWindowXMax + m_FilterXWidth / 2 || B.vecMin().y() > m_CropWindowYMax + m_FilterYWidth / 2 )
	{
		pmpgNew->Release();
		return ;
	}

	B.vecMin().x( B.vecMin().x() - m_FilterXWidth / 2 );
	B.vecMin().y( B.vecMin().y() - m_FilterYWidth / 2 );
	B.vecMax().x( B.vecMax().x() + m_FilterXWidth / 2 );
	B.vecMax().y( B.vecMax().y() + m_FilterYWidth / 2 );

	TqInt iXBa = static_cast<TqInt>( B.vecMin().x() / ( m_XBucketSize ) );
	TqInt iXBb = static_cast<TqInt>( B.vecMax().x() / ( m_XBucketSize ) );
	TqInt iYBa = static_cast<TqInt>( B.vecMin().y() / ( m_YBucketSize ) );
	TqInt iYBb = static_cast<TqInt>( B.vecMax().y() / ( m_YBucketSize ) );
	// Now duplicate and link into any buckets it crosses.
	TqInt iXB = iXBa, iYB = iYBa;

	if ( ( iXBb < 0 ) || ( iYBb < 0 ) )
	{
		pmpgNew->Release();
		return ;
	}
	if ( ( iXBa >= m_cXBuckets ) || ( iYBa >= m_cYBuckets ) )
	{
		pmpgNew->Release();
		return ;
	}

	if ( iXBa < 0 ) iXB = iXBa = 0;
	if ( iYBa < 0 ) iYB = iYBa = 0;

	do
	{
		if ( iYB >= 0 && iYB < m_cYBuckets )
		{
			iXB = iXBa;
			do
			{
				if ( iXB >= 0 && iXB < m_cXBuckets )
				{
					TqInt iBkt = ( iYB * m_cXBuckets ) + iXB;
					if ( iBkt >= iCurrentBucket() )
					{
						m_aBuckets[ iBkt ].AddMPG( pmpgNew );
						pmpgNew->AddRef();
					}
				}
				iXB += 1;
			}
			while ( iXB <= iXBb );
		}
		iYB += 1;
	}
	while ( iYB <= iYBb );
	pmpgNew->Release();
}


//----------------------------------------------------------------------
/** Render any waiting MPGs.
 
    All micro polygon grids in the specified bucket are bust into
    individual micro polygons which are assigned to their appropriate
    bucket. Then RenderMicroPoly() is called for each micro polygon in
    the current bucket.
 
 * \param iBucket Integer index of bucket being processed.
 * \param xmin Integer minimum extend of the image part being rendered, takes into account buckets and clipping.
 * \param xmax Integer maximum extend of the image part being rendered, takes into account buckets and clipping.
 * \param ymin Integer minimum extend of the image part being rendered, takes into account buckets and clipping.
 * \param ymax Integer maximum extend of the image part being rendered, takes into account buckets and clipping.
 */

void CqImageBuffer::RenderMPGs( TqInt iBucket, long xmin, long xmax, long ymin, long ymax )
{
	// First of all split any grids in this bucket waiting to be processed.
	if ( !m_aBuckets[ iBucket ].aGrids().empty() )
	{
		std::vector<CqMicroPolyGridBase*>::iterator j = m_aBuckets[ iBucket ].aGrids().end();

		for ( std::vector<CqMicroPolyGridBase*>::iterator i = m_aBuckets[ iBucket ].aGrids().begin(); i != j; i++ )
			( *i ) ->Split( this, iBucket, xmin, xmax, ymin, ymax );
	}
	m_aBuckets[ iBucket ].aGrids().clear();

	if ( m_aBuckets[ iBucket ].aMPGs().empty() ) return ;

	// Render any waiting MPGs
	std::vector<CqMicroPolygonBase*>::iterator j = m_aBuckets[ iBucket ].aMPGs().end();
	for ( std::vector<CqMicroPolygonBase*>::iterator i = m_aBuckets[ iBucket ].aMPGs().begin(); i != j; i++ )
	{
		RenderMicroPoly( *i, iBucket, xmin, xmax, ymin, ymax );
		( *i ) ->Release();
	}

	m_aBuckets[ iBucket ].aMPGs().clear();
}


//----------------------------------------------------------------------
/** Render a particular micropolygon.
 
 * \param pMPG Pointer to the micropolygon to process.
 * \param iBucket Integer index of bucket being processed.
 * \param xmin Integer minimum extend of the image part being rendered, takes into account buckets and clipping.
 * \param xmax Integer maximum extend of the image part being rendered, takes into account buckets and clipping.
 * \param ymin Integer minimum extend of the image part being rendered, takes into account buckets and clipping.
 * \param ymax Integer maximum extend of the image part being rendered, takes into account buckets and clipping.
 
   \see CqBucket, CqImagePixel
 */

inline void CqImageBuffer::RenderMicroPoly( CqMicroPolygonBase* pMPG, TqInt iBucket, long xmin, long xmax, long ymin, long ymax )
{
	static	SqImageSample	ImageVal;

	CqBucket& Bucket = m_aBuckets[ iBucket ];
	CqStats& theStats = QGetRenderContext() ->Stats();

	const TqFloat* LodBounds = pMPG->pGrid() ->pAttributes() ->GetFloatAttribute( "System", "LevelOfDetailBounds" );
	TqBool UsingLevelOfDetail = LodBounds[ 0 ] >= 0.0f;

	TqBool IsMatte = ( TqBool ) ( pMPG->pGrid() ->pAttributes() ->GetIntegerAttribute( "System", "Matte" ) [ 0 ] == 1 );

	TqInt bound_max = pMPG->cSubBounds();
	TqInt bound_max_1 = bound_max - 1;
	for ( TqInt bound_num = 0; bound_num < bound_max ; bound_num++ )
	{
		TqFloat time0;
		CqBound& Bound = pMPG->SubBound( bound_num, time0 );
		TqFloat time1 = 1.0f;
		if ( bound_num != bound_max_1 )
			pMPG->SubBound( bound_num + 1, time1 );

		TqFloat bminx = Bound.vecMin().x();
		TqFloat bmaxx = Bound.vecMax().x();
		TqFloat bminy = Bound.vecMin().y();
		TqFloat bmaxy = Bound.vecMax().y();

		if ( bmaxx < xmin || bmaxy < ymin || bminx > xmax || bminy > ymax )
		{
			if ( bound_num == bound_max_1)
			{
				// last bound so we can delete the mpg
				theStats.IncCulledMPGs();
				return ;
			}
			else
				continue;
		}

		// If the micropolygon is outside the hither-yon range, cull it.
		if ( Bound.vecMin().z() > ClippingFar() ||
		        Bound.vecMax().z() < ClippingNear() )
		{
			if ( bound_num == bound_max_1 )
			{
				// last bound so we can delete the mpg
				theStats.IncCulledMPGs();
				return ;
			}
			else
				continue;
		}

		// Now go across all pixels touched by the micropolygon bound.
		// The first pixel position is at (sX, sY), the last one
		// at (eX, eY).
		TqInt eX = CEIL( bmaxx );
		TqInt eY = CEIL( bmaxy );
		if ( eX >= xmax ) eX = xmax;
		if ( eY >= ymax ) eY = ymax;

		TqInt sX = FLOOR( bminx );
		TqInt sY = FLOOR( bminy );
		if ( sY < ymin ) sY = ymin;
		if ( sX < xmin ) sX = xmin;

		CqImagePixel* pie;

		TqInt iXSamples = PixelXSamples();
		TqInt iYSamples = PixelYSamples();

		TqInt im = ( bminx < sX ) ? 0 : FLOOR( ( bminx - sX ) * iXSamples );
		TqInt in = ( bminy < sY ) ? 0 : FLOOR( ( bminy - sY ) * iYSamples );
		TqInt em = ( bmaxx > eX ) ? iXSamples : CEIL( ( bmaxx - ( eX - 1 ) ) * iXSamples );
		TqInt en = ( bmaxy > eY ) ? iYSamples : CEIL( ( bmaxy - ( eY - 1 ) ) * iYSamples );

		register long iY = sY;

		CqColor colMPGColor = pMPG->colColor();
		CqColor colMPGOpacity = pMPG->colOpacity();

		while ( iY < eY )
		{
			register long iX = sX;

			Bucket.ImageElement( iX, iY, pie );

			while ( iX < eX )
			{
				// Now sample the micropolygon at several subsample positions
				// within the pixel. The subsample indices range from (start_m, n)
				// to (end_m-1, end_n-1).
				register int m, n;
				n = ( iY == sY ) ? in : 0;
				int end_n = ( iY == ( eY - 1 ) ) ? en : iYSamples;
				int start_m = ( iX == sX ) ? im : 0;
				int end_m = ( iX == ( eX - 1 ) ) ? em : iXSamples;
				TqBool brkVert = TqFalse;
				for ( ; n < end_n && !brkVert; n++ )
				{
					TqBool brkHoriz = TqFalse;
					for ( m = start_m; m < end_m && !brkHoriz; m++ )
					{
						CqVector2D vecP( pie->SamplePoint( m, n ) );
						theStats.IncSamples();

						TqFloat t = pie->SampleTime( m, n );
						// First, check if the subsample point lies within the micropoly bound
						if ( t >= time0 && t <= time1 && Bound.Contains2D( vecP ) )
						{
							theStats.IncSampleBoundHits();

							// Check to see if the sample is within the sample's level of detail
							if ( UsingLevelOfDetail )
							{
								TqFloat LevelOfDetail = pie->SampleLevelOfDetail( m, n );
								if ( LodBounds[ 0 ] > LevelOfDetail || LevelOfDetail >= LodBounds[ 1 ] )
								{
									continue;
								}
							}

							// Now check if the subsample hits the micropoly
							if ( pMPG->Sample( vecP, t, ImageVal.m_Depth ) )
							{
								theStats.IncSampleHits();
								pMPG->BeenHit();
								// Sort the color/opacity into the visible point list
								std::vector<SqImageSample>& aValues = pie->Values( m, n );
								int i = 0;
								int c = aValues.size();
								if ( c > 0 && aValues[ 0 ].m_Depth < ImageVal.m_Depth )
								{
									SqImageSample * p = &aValues[ 0 ];
									while ( i < c && p[ i ].m_Depth < ImageVal.m_Depth ) i++;
									// If it is exactly the same, chances are we've hit a MPG grid line.
									if ( i < c && p[ i ].m_Depth == ImageVal.m_Depth )
									{
										p[ i ].m_colColor = ( p[ i ].m_colColor + colMPGColor ) * 0.5f;
										p[ i ].m_colOpacity = ( p[ i ].m_colOpacity + colMPGOpacity ) * 0.5f;
										continue;
									}
								}

								TqBool Occludes = colMPGOpacity >= gColWhite;

								// Update max depth values
								if ( !( DisplayMode() & ModeZ ) && Occludes )
								{
									CqOcclusionBox::MarkForUpdate( pie->OcclusionBoxId() );
									pie->MarkForZUpdate();
								}


								// Now that we have updated the samples, set the
								ImageVal.m_colColor = colMPGColor;
								ImageVal.m_colOpacity = colMPGOpacity;
								ImageVal.m_pCSGNode = pMPG->pGrid() ->pCSGNode();
								if ( NULL != ImageVal.m_pCSGNode ) ImageVal.m_pCSGNode->AddRef();

								ImageVal.m_flags = 0;
								if ( Occludes )
								{
									ImageVal.m_flags |= SqImageSample::Flag_Occludes;
								}
								if ( IsMatte )
								{
									ImageVal.m_flags |= SqImageSample::Flag_Matte;
								}

								aValues.insert( aValues.begin() + i, ImageVal );
							}
						}
						else
						{
							if ( vecP.y() > bmaxy )
								brkVert = TqTrue;
							if ( vecP.x() > bmaxx )
								brkHoriz = TqTrue;
						}
					}
				}
				iX++;
				pie++;
			}
			iY++;
		}
	}
}

//----------------------------------------------------------------------
/** Render any waiting Surfaces
 
    This method loops through all the gprims stored in the specified bucket
    and checks if the gprim can be diced and turned into a grid of micro
    polygons or if it is still too large and has to be split (this check 
    is done in CqBasicSurface::Diceable()).
 
    The dicing is done by the gprim in CqBasicSurface::Dice(). After that
    the entire grid is shaded by calling CqMicroPolyGridBase::Shade().
    The shaded grid is then stored in the current bucket and will eventually 
    be further processed by RenderMPGs().
 
    If the gprim could not yet be diced, it is split into a number of
    smaller gprims (CqBasicSurface::Split()) which are again assigned to
    buckets (this doesn't necessarily have to be the current one again)
    by calling PostSurface() (just as if it were regular gprims).
 
    Finally, when all the gprims are diced and the resulting micro polygons
    are rendered, the individual subpixel samples are combined into one
    pixel color and opacity which is then exposed and quantized.
    After that the method BucketComplete() and IqDDManager::DisplayBucket()
    is called which can be used to display the bucket inside a window or
    save it to disk.
 
 * \param iBucket Integer index of bucket being processed (0-based).
 * \param xmin Integer minimum extend of the image part being rendered, takes into account buckets and clipping.
 * \param xmax Integer maximum extend of the image part being rendered, takes into account buckets and clipping.
 * \param ymin Integer minimum extend of the image part being rendered, takes into account buckets and clipping.
 * \param ymax Integer maximum extend of the image part being rendered, takes into account buckets and clipping.
 */

void CqImageBuffer::RenderSurfaces( TqInt iBucket, long xmin, long xmax, long ymin, long ymax )
{
	int counter = 0;
	int MaxEyeSplits = 10;
	TqBool bIsEmpty = IsEmpty(iBucket);
	const TqInt* poptEyeSplits = QGetRenderContext() ->optCurrent().GetIntegerOption( "limits", "eyesplits" );
	if ( poptEyeSplits != 0 )
		MaxEyeSplits = poptEyeSplits[ 0 ];

	// Render any waiting micro polygon grids.
	QGetRenderContext() ->Stats().RenderMPGsTimer().Start();
	RenderMPGs( iBucket, xmin, xmax, ymin, ymax );
	QGetRenderContext() ->Stats().RenderMPGsTimer().Stop();

	CqBucket& Bucket = m_aBuckets[ iBucket ];
	
	// Render any waiting subsurfaces.
	CqBasicSurface* pSurface = Bucket.pTopSurface();
	while ( pSurface != 0 )
	{
		if ( m_fQuit ) return ;

		// If the epsilon check has deemed this surface to be undiceable, don't bother asking.
		TqBool fDiceable = TqFalse;
		// Dice & shade the surface if it's small enough...
		QGetRenderContext() ->Stats().DiceableTimer().Start();
		fDiceable = pSurface->Diceable();
		QGetRenderContext() ->Stats().DiceableTimer().Stop();

		if ( fDiceable )
		{
			//Cull surface if it's hidden
			if ( !( DisplayMode() & ModeZ ) )
			{
				QGetRenderContext() ->Stats().OcclusionCullTimer().Start();
				TqBool fCull = TqFalse;
				if (!bIsEmpty)
					fCull = OcclusionCullSurface( iBucket, pSurface );
				QGetRenderContext() ->Stats().OcclusionCullTimer().Stop();
				if ( fCull )
				{
					if ( pSurface == Bucket.pTopSurface() )
						counter ++;
					else
						counter = 0;
					pSurface = Bucket.pTopSurface();
					if ( counter > MaxEyeSplits )
					{
						/* the same primitive was processed by the occlusion a MaxEyeSplits times !!
						 * A bug occurred with the renderer probably.
						 * We need a way out of this forloop. So I unlink the primitive
						 * and try with the next one
						 * 
						 */
						CqAttributeError( ErrorID_OcclusionMaxEyeSplits, Severity_Normal, "Geometry gets culled too many times", pSurface->pAttributes(), TqTrue );
						counter = 0;
						pSurface->UnLink();
						pSurface = Bucket.pTopSurface();
					}
					continue;
				}
			}


			CqMicroPolyGridBase* pGrid;
			QGetRenderContext() ->Stats().DicingTimer().Start();
			pGrid = pSurface->Dice();
			QGetRenderContext() ->Stats().DicingTimer().Stop();
			if ( NULL != pGrid )
			{
				// Only shade in all cases since the Displacement could be called in the shadow map creation too.
				pGrid->Shade();

				if ( pGrid->vfCulled() == TqFalse )
				{
					// Only project micropolygon not culled
					pGrid->Project();
					Bucket.AddGrid( pGrid );
				}
			}
		}
		// The surface is not small enough, so split it...
		else if ( !pSurface->fDiscard() )
		{
			std::vector<CqBasicSurface*> aSplits;
			// Decrease the total gprim count since this gprim is replaced by other gprims
			QGetRenderContext() ->Stats().DecTotalGPrims();
			// Split it
			QGetRenderContext() ->Stats().SplitsTimer().Start();
			TqInt cSplits = pSurface->Split( aSplits );
			TqInt i;
			for ( i = 0; i < cSplits; i++ )
				PostSurface( aSplits[ i ] );
			QGetRenderContext() ->Stats().SplitsTimer().Stop();
		}

		pSurface->UnLink();
		pSurface->Release();
		pSurface = Bucket.pTopSurface();
		// Render any waiting micro polygon grids.
		QGetRenderContext() ->Stats().RenderMPGsTimer().Start();
		RenderMPGs( iBucket, xmin, xmax, ymin, ymax );
		QGetRenderContext() ->Stats().RenderMPGsTimer().Stop();

		QGetRenderContext() ->Stats().OcclusionCullTimer().Start();
		// Update our occlusion hierarchy after each grid that gets drawn.
		if (!bIsEmpty)
			CqOcclusionBox::Update();
		QGetRenderContext() ->Stats().OcclusionCullTimer().Stop();
	}

	// Now combine the colors at each pixel sample for any micropolygons rendered to that pixel.
	if ( m_fQuit ) return ;

	QGetRenderContext() ->Stats().MakeCombine().Start();
	CqBucket::CombineElements();
	QGetRenderContext() ->Stats().MakeCombine().Stop();

	QGetRenderContext() ->Stats().MakeFilterBucket().Start();

	Bucket.FilterBucket();
	Bucket.ExposeBucket();
	Bucket.QuantizeBucket();

	QGetRenderContext() ->Stats().MakeFilterBucket().Stop();

	BucketComplete( iBucket );
	QGetRenderContext() ->Stats().MakeDisplayBucket().Start();
	QGetRenderContext() ->pDDmanager() ->DisplayBucket( &m_aBuckets[ iBucket ] );
	QGetRenderContext() ->Stats().MakeDisplayBucket().Stop();
}

//----------------------------------------------------------------------
/** Return if a certain bucket is completely empty.
 
    True or False.

     It is empty only when this bucket doesn't contain any surface, 
	 micropolygon or grids.
 */
TqBool CqImageBuffer::IsEmpty(TqInt which)
{
	TqBool retval = TqFalse;

	CqBucket& Bucket = m_aBuckets[ which ];

	if ((!Bucket.pTopSurface())	&& 
		Bucket.aGrids().empty() && 
		Bucket.aMPGs().empty() ) 
		retval = TqTrue;

	return retval;
}
//----------------------------------------------------------------------
/** Render any waiting Surfaces
 
    Starting from the upper left corner of the image every bucket is
    processed by computing its extent and calling RenderSurfaces().
    After the image is complete ImageComplete() is called.

    It will be nice to be able to remove Occlusion at demands.
	However I did not see a case when Occlusion took longer without occlusion.
 */

void CqImageBuffer::RenderImage()
{
	if ( bucketmodulo == -1 )
	{
		// Small change which allows full control of virtual memory on NT swapping
		bucketmodulo = m_cXBuckets;
		TqInt *poptModulo = ( TqInt * ) QGetRenderContext() ->optCurrent().GetIntegerOption( "limits", "bucketmodulo" );
		if ( poptModulo != 0 )
		{
			bucketmodulo = poptModulo[ 0 ];
			std::cout << "Number of Buckets per line " << m_cXBuckets << "\n";
		}
		if ( bucketmodulo <= 0 ) bucketmodulo = m_cXBuckets;
	}



	// Render the surface at the front of the list.
	m_fDone = TqFalse;

	CqVector2D bHalf = CqVector2D( m_FilterXWidth / 2, m_FilterYWidth / 2 );
	
	// Setup the hierarchy of boxes for occlusion culling
	CqOcclusionBox::CreateHierarchy( m_XBucketSize, m_YBucketSize, m_FilterXWidth, m_FilterYWidth );

	TqInt iBucket;
	TqInt nBuckets = m_cXBuckets*m_cYBuckets;
	RtProgressFunc pProgressHandler = NULL;
	pProgressHandler = QGetRenderContext() ->optCurrent().pProgressHandler();

	
	for ( iBucket = 0; iBucket < nBuckets; iBucket++ )
	{
		TqBool bIsEmpty = IsEmpty(iBucket);
	    QGetRenderContext() ->Stats().Others().Start();
		SetiCurrentBucket( iBucket );
		// Prepare the bucket.
		CqVector2D bPos = Position( iBucket );
		CqVector2D bSize = Size( iBucket );
		// Warning Jitter must be True is all cases; the InitialiseBucket when it is not in jittering mode
		// doesn't initialise correctly so later we have problem in the FilterBucket()
		// It is crucial !IsEmpty set the jitter here when something is present in this bucket.
		CqBucket::InitialiseBucket( static_cast<TqInt>( bPos.x() ), static_cast<TqInt>( bPos.y() ), static_cast<TqInt>( bSize.x() ), static_cast<TqInt>( bSize.y() ), m_FilterXWidth, m_FilterYWidth, m_PixelXSamples, m_PixelYSamples, !bIsEmpty);
		CqBucket::InitialiseFilterValues();

		// Set up some bounds for the bucket.
		CqVector2D vecMin = bPos;
		CqVector2D vecMax = bPos + bSize;
		vecMin -= bHalf;
		vecMax += bHalf;

		long xmin = static_cast<long>( vecMin.x() );
		long ymin = static_cast<long>( vecMin.y() );
		long xmax = static_cast<long>( vecMax.x() );
		long ymax = static_cast<long>( vecMax.y() );

		if ( xmin < CropWindowXMin() - m_FilterXWidth / 2 ) xmin = CropWindowXMin() - m_FilterXWidth / 2;
		if ( ymin < CropWindowYMin() - m_FilterYWidth / 2 ) ymin = CropWindowYMin() - m_FilterYWidth / 2;
		if ( xmax > CropWindowXMax() + m_FilterXWidth / 2 ) xmax = CropWindowXMax() + m_FilterXWidth / 2;
		if ( ymax > CropWindowYMax() + m_FilterYWidth / 2 ) ymax = CropWindowYMax() + m_FilterYWidth / 2;

		QGetRenderContext() ->Stats().Others().Stop();

		QGetRenderContext() ->Stats().OcclusionCullTimer().Start();
		if (!bIsEmpty)
			CqOcclusionBox::SetupHierarchy( &m_aBuckets[ iBucket ], xmin, ymin, xmax, ymax );

		QGetRenderContext() ->Stats().OcclusionCullTimer().Stop();
	
		

		if (pProgressHandler)
		{
			// Inform the status class how far we have got, and update UI.
			float Complete = (float) nBuckets;
			Complete /= iBucket;
			Complete = 100.0f / Complete;
			QGetRenderContext() ->Stats().SetComplete( Complete );
			( *pProgressHandler ) ( Complete );
		}
	    

		RenderSurfaces( iBucket, xmin, xmax, ymin, ymax );
		if ( m_fQuit )
		{
			m_fDone = TqTrue;
			return ;
		}
#ifdef WIN32
		if ( !( iBucket % bucketmodulo ) )
			SetProcessWorkingSetSize( GetCurrentProcess(), 0xffffffff, 0xffffffff );
#endif

	}

	ImageComplete();

	CqOcclusionBox::DeleteHierarchy();
	// Pass >100 through to progress to allow it to indicate completion.

	if ( pProgressHandler )
	{
		( *pProgressHandler ) ( 100.0f );
	}
	m_fDone = TqTrue;
}


//----------------------------------------------------------------------
/** Stop rendering.
 */

void CqImageBuffer::Quit()
{
	m_fQuit = TqTrue;
}


//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )


