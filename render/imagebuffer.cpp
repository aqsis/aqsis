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
		\author Andy Gill (buzz@ucky.com)
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
#include	"shadervm.h"
#include	"micropolygon.h"
#include	"imagebuffer.h"
#include	"imagers.h"

#include	<map>

START_NAMESPACE( Aqsis )


//----------------------------------------------------------------------
/** Constructor
 */

CqImagePixel::CqImagePixel() :
		m_XSamples( 0 ),
		m_YSamples( 0 ),
		//				m_aValues(0),
		//				m_avecSamples(0),
		m_colColor( 0, 0, 0 )
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

	if ( XSamples > 0 && YSamples > 0 )
	{
		m_aValues.resize( m_XSamples * m_YSamples );
		m_avecSamples.resize( m_XSamples * m_YSamples );
		m_aSubCellIndex.resize( m_XSamples * m_YSamples );
		m_aTimes.resize( m_XSamples * m_YSamples );
	}
}


//----------------------------------------------------------------------
/** Fill in the sample array usig the multijitter function from GG IV.
 * \param vecPixel Cq2DVector pixel coordinate of this image element, used to make sure sample points are absolute, not relative.
 * \param fJitter Flag indicating whether to apply jittering to the sample points or not.
 */

void CqImagePixel::InitialiseSamples( CqVector2D& vecPixel, TqBool fJitter )
{
	TqFloat subcell_width = 1.0f / ( m_XSamples * m_YSamples );
	CqRandom random;
	TqInt m = m_XSamples;
	TqInt n = m_YSamples;

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
	}
	else
	{
		// Initialize points to the "canonical" multi-jittered pattern.
		TqInt i, j;
		for ( i = 0; i < n; i++ )
		{
			for ( j = 0; j < m; j++ )
			{
				m_avecSamples[ i * m + j ].x( i );
				m_avecSamples[ i * m + j ].y( j );
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
				TqFloat xindex = m_avecSamples[ i * m + j ].x();
				TqFloat yindex = m_avecSamples[ i * m + j ].y();
				m_avecSamples[ i * m + j ].x( xindex * subcell_width + ( subcell_width * 0.5f ) + sx );
				m_avecSamples[ i * m + j ].y( yindex * subcell_width + ( subcell_width * 0.5f ) + sy );
				m_avecSamples[ i * m + j ] += vecPixel;
				m_aSubCellIndex[ i * m + j ] = static_cast<TqInt>( ( yindex * m_YSamples ) + xindex );
			}
		}
	}

	// Fill in the sample times for motion blur.
	TqFloat time = 0;
	TqFloat dtime = 1.0f / ( ( m_XSamples * m_YSamples ) );
	TqInt i;
	for ( i = 0; i < m_XSamples*m_YSamples; i++ )
	{
		m_aTimes[ i ] = time + random.RandomFloat( dtime );
		time += dtime;
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

	TqUint samplecount = 0;
	TqUint numsamples = XSamples() * YSamples();
	for ( std::vector<std::vector<SqImageSample> >::iterator samples = m_aValues.begin(); samples != m_aValues.end(); samples++ )
	{
		// Find out if any of the samples are in a CSG tree.
		while ( 1 )
		{
			TqBool bProcessed = TqFalse;
			for ( std::vector<SqImageSample>::iterator isample = samples->begin(); isample != samples->end(); isample++ )
			{
				if ( NULL != isample->m_pCSGNode )
				{
					isample->m_pCSGNode->ProcessTree( *samples );
					bProcessed = TqTrue;
					break;
				}
			}
			if ( !bProcessed )
				break;
		}

		CqColor samplecolor = gColBlack;
		CqColor sampleopacity = gColBlack;
		TqBool samplehit = TqFalse;
		for ( std::vector<SqImageSample>::reverse_iterator sample = samples->rbegin(); sample != samples->rend(); sample++ )
		{
			samplecolor = ( samplecolor * ( gColWhite - sample->m_colOpacity ) ) + sample->m_colColor;
			sampleopacity = ( ( gColWhite - sampleopacity ) * sample->m_colOpacity ) + sampleopacity;
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
		}
	}

	if ( samplecount )
		m_Coverage /= numsamples;
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
TqFloat CqBucket::m_MaxDepth;
TqInt	CqBucket::m_MaxDepthCount;
std::vector<CqImagePixel>	CqBucket::m_aieImage;
std::vector<TqFloat> CqBucket::m_aFilterValues;

//----------------------------------------------------------------------
/** Get a reference to pixel data.
 * \param iXPos Integer pixel coordinate.
 * \param iYPos Integer pixel coordinate.
 * \param iBucket Integer bucket index.
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
		std::cerr << "CqBucket::ImageElement() outside bucket boundary!" << std::endl;
		return ( TqFalse );
	}
}


//----------------------------------------------------------------------
/** Clear the image data storage area.
 */

void CqBucket::Clear()
{
	// Call the clear function on each element in the bucket.
	for ( std::vector<CqImagePixel>::iterator iElement = m_aieImage.begin(); iElement != m_aieImage.end(); iElement++ )
		iElement->Clear();
}


//----------------------------------------------------------------------
/** Initialise the static image storage area.
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

	// Reset max depth value to infinity
	m_MaxDepth = FLT_MAX;
	m_MaxDepthCount = ( m_YSize + m_YFWidth ) * ( m_XSize + m_XFWidth ) * xsamples * ysamples;

	// Allocate the image element storage for a single bucket
	m_aieImage.resize( ( xsize + xfwidth ) * ( ysize + yfwidth ) );

	// Initialise the samples for this bucket.
	TqInt i;
	for ( i = 0; i < ( m_YSize + m_YFWidth ); i++ )
	{
		TqInt j;
		for ( j = 0; j < ( m_XSize + m_XFWidth ); j++ )
		{
			CqVector2D bPos2( m_XOrigin, m_YOrigin );
			bPos2 += CqVector2D( ( j - m_XFWidth / 2 ), ( i - m_YFWidth / 2 ) );
			m_aieImage[ ( i * ( m_XSize + m_XFWidth ) ) + j ].AllocateSamples( xsamples, ysamples );
			m_aieImage[ ( i * ( m_XSize + m_XFWidth ) ) + j ].InitialiseSamples( bPos2, fJitter );
		}
	}
}


//----------------------------------------------------------------------
/** Initialise the static filter values.
 */

void CqBucket::InitialiseFilterValues()
{
	// Allocate and fill in the filter values array for each pixel.
	RtFilterFunc pFilter;
	pFilter = QGetRenderContext() ->optCurrent().funcFilter();

	TqFloat xmax = m_XMax;
	TqFloat ymax = m_YMax;
	TqFloat xfwo2 = m_XFWidth * 0.5f;
	TqFloat yfwo2 = m_YFWidth * 0.5f;
	TqFloat xfw = m_XFWidth;
	TqFloat yfw = m_YFWidth;
	TqInt numsubpixels = ( m_XPixelSamples * m_YPixelSamples );
	TqInt numsubcells = numsubpixels;
	TqFloat subcellwidth = 1.0f / numsubcells;
	TqFloat subcellcentre = subcellwidth * 0.5f;
	TqInt numperpixel = numsubpixels * numsubcells;
	TqInt numvalues = static_cast<TqInt>( ( ( xfw + 1 ) * ( yfw + 1 ) ) * ( numperpixel ) );

	m_aFilterValues.resize( numvalues );

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
					TqInt sindex = index + ( ( ( sy * m_XPixelSamples ) + sx ) * numsubcells );
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
	for ( std::vector<CqImagePixel>::iterator i = m_aieImage.begin(); i != m_aieImage.end(); i++ )
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
		return ( CqColor( 0.0f, 0.0f, 0.0f ) );
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
		return ( CqColor( 0.0f, 0.0f, 0.0f ) );
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
/** Calculates the current biggest Z-Value and updates MaxDepth and
 * MaxDepthCount accordingly.
 */

void CqBucket::UpdateMaxDepth()
{
	TqFloat currentMax = -FLT_MAX;
	TqInt	count = 0;

	// Go through each pixel in the bucket
	TqInt i, j;
	for ( i = 0; i < ( m_YSize + m_YFWidth ); i++ )
	{
		for ( j = 0; j < ( m_XSize + m_XFWidth ); j++ )
		{
			CqImagePixel* pie = &m_aieImage[ ( i * ( m_XSize + m_XFWidth ) ) + j ];
			// Now go through each subpixel sample
			TqInt sx, sy;
			for ( sy = 0; sy < m_YPixelSamples; sy++ )
			{
				for ( sx = 0; sx < m_XPixelSamples; sx++ )
				{
					std::vector<SqImageSample>& aValues = pie->Values( sx, sy );

					if ( aValues.size() > 0 )
					{
						std::vector<SqImageSample>::iterator sc = aValues.begin();
						// find first opaque sample
						while ( sc != aValues.end() && ( ( sc->m_colOpacity != gColWhite ) || ( sc->m_pCSGNode != NULL ) ) )
							sc++;
						if ( sc != aValues.end() )
						{
							if ( sc->m_Depth > currentMax )
							{
								currentMax = sc->m_Depth;
								count = 1;
							}
							else if ( sc->m_Depth == currentMax )
								count++;
						}
						else
						{
							if ( currentMax == FLT_MAX )
								count++;
							else
							{
								currentMax = FLT_MAX;
								count = 1;
							}
						}
					}
					else
					{
						if ( currentMax == FLT_MAX )
							count++;
						else
						{
							currentMax = FLT_MAX;
							count = 1;
						}
					}
				}
			}
		}
	}

	m_MaxDepth = currentMax;
	m_MaxDepthCount = count;
}

//----------------------------------------------------------------------
/** Filter the samples in this bucket according to type and filter widths.
 */

void CqBucket::FilterBucket()
{
	CqImagePixel * pie;

	QGetRenderContext()->Stats().MakeFilterBucket().Start();
	CqColor* pCols = new CqColor[ XSize() * YSize() ];
	CqColor* pOpacs = new CqColor[ XSize() * YSize() ];
	TqFloat* pDepths = new TqFloat[ XSize() * YSize() ];
	TqFloat* pCoverages=new TqFloat[XSize()*YSize()];
	
	TqInt xmax = static_cast<TqInt>( CEIL( ( XFWidth() - 1 ) * 0.5f ) );
	TqInt ymax = static_cast<TqInt>( CEIL( ( YFWidth() - 1 ) * 0.5f ) );
	TqFloat xfwo2 = XFWidth() * 0.5f;
	TqFloat yfwo2 = YFWidth() * 0.5f;
	TqInt numsubpixels = ( m_XPixelSamples * m_YPixelSamples );
	TqInt numsubcells = numsubpixels;
	TqInt numperpixel = numsubpixels * numsubcells;
	TqInt	xlen = XSize() + XFWidth();

    TqInt SampleCount=0;
    CqColor imager;

	TqInt x, y;
	TqInt i = 0;
	TqFloat total = numsubpixels;

	for ( y = YOrigin(); y < YOrigin() + YSize(); y++ )
	{
		TqFloat ycent = y + 0.5f;
		for ( x = XOrigin(); x < XOrigin() + XSize(); x++ )
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
							TqInt sindex = index + ( ( ( sy * m_XPixelSamples ) + sx ) * numsubcells );
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
				fy++;
			}
			pCols[ i ] = c / gTot;
			pOpacs[ i ] = o / gTot;

			if (SampleCount> m_YPixelSamples * m_XPixelSamples)
	            pCoverages[i] = 1.0;
			else
				pCoverages[i] = (TqFloat) SampleCount/ (TqFloat)( m_YPixelSamples * m_XPixelSamples);

			if( NULL != QGetRenderContext()->optCurrent().pshadImager() && NULL != QGetRenderContext()->optCurrent().pshadImager()->pShader())
			{
				// Init & Execute the imager shader
				QGetRenderContext()->optCurrent().InitialiseColorImager(1, 1, 
																x, y, 
																&pCols[i], &pOpacs[i], 
																&pDepths[i], &pCoverages[i]);

				if (QGetRenderContext()->optCurrent().strImager() != "null")
				{
					imager = QGetRenderContext()->optCurrent().GetColorImager( x , y ); 
					// Normal case will be to poke the alpha from the image shader and 
					// multiply imager color with it... but after investigation alpha is always 
					// == 1 after a call to imager shader in 3delight and BMRT.
					// Therefore I did not ask for alpha value and set directly the pCols[i]
					// with imager value. see imagers.cpp 
					pCols[i] = imager;
					imager =  QGetRenderContext()->optCurrent().GetOpacityImager( x , y);
					pOpacs[i] = imager;
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
	for ( y = 0; y < YSize(); y++ )
	{
		CqImagePixel* pie2 = pie;
		for ( x = 0; x < XSize(); x++ )
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
	delete[](pCoverages);
	QGetRenderContext()->Stats().MakeFilterBucket().Stop();
}


//----------------------------------------------------------------------
/** Expose the samples in this bucket according to specified gain and gamma settings.
 */

void CqBucket::ExposeBucket()
{
	if ( QGetRenderContext() ->optCurrent().fExposureGain() == 1.0 &&
	        QGetRenderContext() ->optCurrent().fExposureGamma() == 1.0 )
		return ;
	else
	{
		CqImagePixel* pie;
		ImageElement( XOrigin(), YOrigin(), pie );
		TqInt x, y;
		TqFloat exposegain = QGetRenderContext() ->optCurrent().fExposureGain();
		TqFloat exposegamma = QGetRenderContext() ->optCurrent().fExposureGamma();
		TqFloat oneovergamma = 1.0f / exposegamma;
		for ( y = 0; y < YSize(); y++ )
		{
			CqImagePixel* pie2 = pie;
			for ( x = 0; x < XSize(); x++ )
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
			pie += XSize() + XFWidth();
		}
	}
}


//----------------------------------------------------------------------
/** Quantize the samples in this bucket according to type.
 */

void CqBucket::QuantizeBucket()
{
	CqRandom random;

	if ( QGetRenderContext() ->optCurrent().iDisplayMode() & ModeRGB )
	{
		double ditheramplitude = QGetRenderContext() ->optCurrent().fColorQuantizeDitherAmplitude();
		TqInt one = QGetRenderContext() ->optCurrent().iColorQuantizeOne();
		TqInt min = QGetRenderContext() ->optCurrent().iColorQuantizeMin();
		TqInt max = QGetRenderContext() ->optCurrent().iColorQuantizeMax();

		CqImagePixel* pie;
		ImageElement( XOrigin(), YOrigin(), pie );
		TqInt x, y;
		for ( y = 0; y < YSize(); y++ )
		{
			CqImagePixel* pie2 = pie;
			for ( x = 0; x < XSize(); x++ )
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
			pie += XSize() + XFWidth();
		}
	}
	else
	{
		double ditheramplitude = QGetRenderContext() ->optCurrent().fDepthQuantizeDitherAmplitude();
		if ( ditheramplitude == 0 ) return ;
		TqInt one = QGetRenderContext() ->optCurrent().iDepthQuantizeOne();
		TqInt min = QGetRenderContext() ->optCurrent().iDepthQuantizeMin();
		TqInt max = QGetRenderContext() ->optCurrent().iDepthQuantizeMax();

		CqImagePixel* pie;
		ImageElement( XOrigin(), YOrigin(), pie );
		TqInt x, y;
		for ( y = 0; y < YSize(); y++ )
		{
			CqImagePixel* pie2 = pie;
			for ( x = 0; x < XSize(); x++ )
			{
				double d;
				if ( modf( one * pie2->Depth() + ditheramplitude * random.RandomFloat(), &d ) > 0.5 ) d += 1;
				d = CLAMP( d, min, max );
				pie2->SetDepth( d );
				pie2++;
			}
			pie += XSize() + XFWidth();
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
	m_aGPrims.LinkLast( pGPrim );
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

	m_iXRes = QGetRenderContext() ->optCurrent().iXResolution();
	m_iYRes = QGetRenderContext() ->optCurrent().iYResolution();
	m_CropWindowXMin = static_cast<TqInt>( CLAMP( CEIL( m_iXRes * QGetRenderContext() ->optCurrent().fCropWindowXMin() ), 0, m_iXRes ) );
	m_CropWindowXMax = static_cast<TqInt>( CLAMP( CEIL( m_iXRes * QGetRenderContext() ->optCurrent().fCropWindowXMax() ), 0, m_iXRes ) );
	m_CropWindowYMin = static_cast<TqInt>( CLAMP( CEIL( m_iYRes * QGetRenderContext() ->optCurrent().fCropWindowYMin() ), 0, m_iYRes ) );
	m_CropWindowYMax = static_cast<TqInt>( CLAMP( CEIL( m_iYRes * QGetRenderContext() ->optCurrent().fCropWindowYMax() ), 0, m_iYRes ) );
	m_cXBuckets = ( m_iXRes / m_XBucketSize ) + 1;
	m_cYBuckets = ( m_iYRes / m_YBucketSize ) + 1;
	m_PixelXSamples = QGetRenderContext() ->optCurrent().PixelXSamples();
	m_PixelYSamples = QGetRenderContext() ->optCurrent().PixelYSamples();
	m_FilterXWidth = static_cast<TqInt>( QGetRenderContext() ->optCurrent().fFilterXWidth() );
	m_FilterYWidth = static_cast<TqInt>( QGetRenderContext() ->optCurrent().fFilterYWidth() );
	m_DisplayMode = QGetRenderContext() ->optCurrent().iDisplayMode();

	m_aBuckets.resize( m_cXBuckets * m_cYBuckets );


	TqBool	fJitter = (
	                     ( ( QGetRenderContext() ->optCurrent().iDisplayMode() & ModeRGB ) != 0 ) &&
	                     ( QGetRenderContext() ->optCurrent().strHider() != "painter" ) );

	CqBucket::InitialiseBucket( 0, 0, m_XBucketSize, m_YBucketSize, m_FilterXWidth, m_FilterYWidth, m_PixelXSamples, m_PixelXSamples, fJitter );
	CqBucket::InitialiseFilterValues();
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
	if ( Bound.vecMin().z() >= QGetRenderContext() ->optCurrent().fClippingPlaneFar() ||
	        Bound.vecMax().z() <= QGetRenderContext() ->optCurrent().fClippingPlaneNear() )
		return ( TqTrue );

	// If the primitive spans the epsilon plane and the hither plane and can be split,
	if ( Bound.vecMin().z() <= FLT_EPSILON && Bound.vecMax().z() >= FLT_EPSILON )
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
	TqFloat db = 0;
	CqString strCoordinateSystem( "object" );
	const TqFloat* pattrDispclacementBound = pSurface->pAttributes() ->GetFloatAttribute( "displacementbound", "sphere" );
	const CqString* pattrCoordinateSystem = pSurface->pAttributes() ->GetStringAttribute( "displacementbound", "coordinatesystem" );
	if ( pattrDispclacementBound != 0 ) db = pattrDispclacementBound[ 0 ];
	if ( pattrCoordinateSystem != 0 ) strCoordinateSystem = pattrCoordinateSystem[ 0 ];

	CqVector3D	vecDB( db, 0, 0 );
	vecDB = QGetRenderContext() ->matVSpaceToSpace( strCoordinateSystem.c_str(), "camera", pSurface->pAttributes() ->pshadSurface() ->matCurrent(), pSurface->pTransform() ->matObjectToWorld() ) * vecDB;
	db = vecDB.Magnitude();

	Bound.vecMax() += db;
	Bound.vecMin() -= db;

	// Check if the surface can be culled. (also converts Bound to raster space).
	if ( CullSurface( Bound, pSurface ) )
	{
		pSurface->UnLink();
		pSurface->Release();
		QGetRenderContext() ->Stats().IncCulledGPrims();
		return ;
	}

	// Find out which bucket(s) the surface belongs to.
	TqInt XMinb, YMinb;
	if ( Bound.vecMin().x() < 0 ) Bound.vecMin().x( 0.0f );
	if ( Bound.vecMin().y() < 0 ) Bound.vecMin().y( 0.0f );
	TqInt iBucket = Bucket( static_cast<TqInt>( Bound.vecMin().x() ), static_cast<TqInt>( Bound.vecMin().y() ), XMinb, YMinb );

	if ( XMinb >= m_cXBuckets || YMinb >= m_cYBuckets ) return ;

	if ( XMinb < 0 || YMinb < 0 )
	{
		if ( XMinb < 0 ) XMinb = 0;
		if ( YMinb < 0 ) YMinb = 0;
		iBucket = ( YMinb * m_cXBuckets ) + XMinb;
	}

	m_aBuckets[ iBucket ].AddGPrim( pSurface );

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

	// Update bucket max depth values
	CqBucket currentBucket = m_aBuckets[ iBucket ];
	if ( currentBucket.MaxDepthCount() <= 0 )
		currentBucket.UpdateMaxDepth();

	if ( pSurface->pCSGNode() != NULL )
		return ( TqFalse );

	if ( RasterBound.vecMin().z() > currentBucket.MaxDepth() )
	{
		// pSurface is behind everying in this bucket but it may be
		// visible in other buckets it overlaps.
		// bucket to the right
		TqInt nextBucket = iBucket + 1;
		CqVector2D pos = Position( nextBucket );
		if ( ( nextBucket < m_cXBuckets * m_cYBuckets ) &&
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

		if ( ( nextBucket < m_cXBuckets * m_cYBuckets ) &&
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
		for ( std::vector<CqMicroPolyGridBase*>::iterator i = m_aBuckets[ iBucket ].aGrids().begin(); i != m_aBuckets[ iBucket ].aGrids().end(); i++ )
			( *i ) ->Split( this, iBucket, xmin, xmax, ymin, ymax );
	}
	m_aBuckets[ iBucket ].aGrids().clear();

	// Render any waiting MPGs
	static	CqVector2D	vecP;

	if ( m_aBuckets[ iBucket ].aMPGs().empty() ) return ;

	for ( std::vector<CqMicroPolygonBase*>::iterator i = m_aBuckets[ iBucket ].aMPGs().begin(); i != m_aBuckets[ iBucket ].aMPGs().end(); i++ )
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

	for ( TqInt bound_num = 0; bound_num < pMPG->cSubBounds(); bound_num++ )
	{
		TqFloat time0;
		CqBound& Bound = pMPG->SubBound( bound_num, time0 );
		TqFloat time1 = 1.0f;
		if ( bound_num != pMPG->cSubBounds() - 1 )
			pMPG->SubBound( bound_num + 1, time1 );

		TqFloat bminx = Bound.vecMin().x();
		TqFloat bmaxx = Bound.vecMax().x();
		TqFloat bminy = Bound.vecMin().y();
		TqFloat bmaxy = Bound.vecMax().y();

		if ( bmaxx < xmin || bmaxy < ymin || bminx > xmax || bminy > ymax )
		{
			if ( bound_num == pMPG->cSubBounds() - 1 )
			{
				// last bound so we can delete the mpg
				QGetRenderContext() ->Stats().IncCulledMPGs();
				return ;
			}
			else
				continue;
		}

		// If the micropolygon is outside the hither-yon range, cull it.
		if ( Bound.vecMin().z() > QGetRenderContext() ->optCurrent().fClippingPlaneFar() ||
		        Bound.vecMax().z() < QGetRenderContext() ->optCurrent().fClippingPlaneNear() )
		{
			if ( bound_num == pMPG->cSubBounds() - 1 )
			{
				// last bound so we can delete the mpg
				QGetRenderContext() ->Stats().IncCulledMPGs();
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

		TqInt iXSamples = QGetRenderContext() ->optCurrent().PixelXSamples();
		TqInt iYSamples = QGetRenderContext() ->optCurrent().PixelYSamples();

		TqInt im = ( bminx < sX ) ? 0 : FLOOR( ( bminx - sX ) * iXSamples );
		TqInt in = ( bminy < sY ) ? 0 : FLOOR( ( bminy - sY ) * iYSamples );
		TqInt em = ( bmaxx > eX ) ? iXSamples : CEIL( ( bmaxx - ( eX - 1 ) ) * iXSamples );
		TqInt en = ( bmaxy > eY ) ? iYSamples : CEIL( ( bmaxy - ( eY - 1 ) ) * iYSamples );

		register long iY = sY;

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
						QGetRenderContext() ->Stats().IncSamples();

						TqFloat t = pie->SampleTime( m, n );
						// First, check if the subsample point lies within the micropoly bound
						if ( t >= time0 && t <= time1 && Bound.Contains2D( vecP ) )
						{
							QGetRenderContext() ->Stats().IncSampleBoundHits();

							// Now check if the subsample hits the micropoly
							if ( pMPG->Sample( vecP, t, ImageVal.m_Depth ) )
							{
								QGetRenderContext() ->Stats().IncSampleHits();
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
										p[ i ].m_colColor = ( p[ i ].m_colColor + pMPG->colColor() ) * 0.5f;
										p[ i ].m_colOpacity = ( p[ i ].m_colOpacity + pMPG->colOpacity() ) * 0.5f;
										continue;
									}
								}

								// Update max depth values
								if ( pMPG->colOpacity() == gColWhite )
								{
									if ( c == 0 )
									{
										Bucket.DecMaxDepthCount();
									}
									else
									{
										int j = 0;
										// Find first opaque sample
										while ( j < c && aValues[ j ].m_colOpacity != gColWhite ) j++;
										if ( j < c && aValues[ j ].m_Depth == Bucket.MaxDepth() && aValues[ j ].m_colOpacity == gColWhite )
										{
											if ( aValues[ j ].m_Depth > ImageVal.m_Depth )
											{
												Bucket.DecMaxDepthCount();
											}
										}
									}
								}

								ImageVal.m_colColor = pMPG->colColor();
								ImageVal.m_colOpacity = pMPG->colOpacity();
								ImageVal.m_pCSGNode = pMPG->pGrid() ->pCSGNode();
								if ( NULL != ImageVal.m_pCSGNode ) ImageVal.m_pCSGNode->AddRef();

								// Truncate sample list if opaque.
								if ( ( pMPG->colOpacity() == gColWhite ) && ( pMPG->pGrid() ->pCSGNode() == NULL ) )
								{
									aValues.erase( aValues.begin() + i, aValues.end() );
									aValues.push_back( ImageVal );
								}
								else
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

		// Dice & shade the surface if it's small enough...
		QGetRenderContext() ->Stats().DiceableTimer().Start();
		TqBool fDiceable = pSurface->Diceable();
		QGetRenderContext() ->Stats().DiceableTimer().Stop();
		if ( fDiceable )
		{
			//Cull surface if it's hidden
			QGetRenderContext() ->Stats().OcclusionCullTimer().Start();
			TqBool fCull = OcclusionCullSurface( iBucket, pSurface );
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


			CqMicroPolyGridBase* pGrid;
			QGetRenderContext() ->Stats().DicingTimer().Start();
			pGrid = pSurface->Dice();
			QGetRenderContext() ->Stats().DicingTimer().Stop();
			if ( NULL != pGrid )
			{
				// Only shade in all cases since the Displacement could be called in the shadow map creation too.
				pGrid->Shade();

				pGrid->Project();
				Bucket.AddGrid( pGrid );
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
	}

	// Now combine the colors at each pixel sample for any micropolygons rendered to that pixel.
	if ( m_fQuit ) return ;

	CqBucket::CombineElements();

	Bucket.FilterBucket();
	Bucket.ExposeBucket();
	Bucket.QuantizeBucket();

	BucketComplete( iBucket );
	QGetRenderContext() ->pDDmanager() ->DisplayBucket( &m_aBuckets[ iBucket ] );
}


//----------------------------------------------------------------------
/** Render any waiting Surfaces
 
    Starting from the upper left corner of the image every bucket is
    processed by computing its extent and calling RenderSurfaces().
    After the image is complete ImageComplete() is called.
 */

void CqImageBuffer::RenderImage()
{
	// Render the surface at the front of the list.
	m_fDone = TqFalse;

	TqInt iBucket;
	for ( iBucket = 0; iBucket < m_cXBuckets*m_cYBuckets; iBucket++ )
	{
		SetiCurrentBucket( iBucket );
		// Prepare the bucket.
		CqBucket::Clear();
		CqVector2D bPos = Position( iBucket );
		CqVector2D bSize = Size( iBucket );
		CqBucket::InitialiseBucket( static_cast<TqInt>( bPos.x() ), static_cast<TqInt>( bPos.y() ), static_cast<TqInt>( bSize.x() ), static_cast<TqInt>( bSize.y() ), m_FilterXWidth, m_FilterYWidth, m_PixelXSamples, m_PixelYSamples );

		// Set up some bounds for the bucket.
		CqVector2D vecMin = bPos;
		CqVector2D vecMax = bPos + bSize;
		vecMin -= CqVector2D( m_FilterXWidth / 2, m_FilterYWidth / 2 );
		vecMax += CqVector2D( m_FilterXWidth / 2, m_FilterYWidth / 2 );

		long xmin = static_cast<long>( vecMin.x() );
		long ymin = static_cast<long>( vecMin.y() );
		long xmax = static_cast<long>( vecMax.x() );
		long ymax = static_cast<long>( vecMax.y() );

		if ( xmin < CropWindowXMin() - m_FilterXWidth / 2 ) xmin = CropWindowXMin() - m_FilterXWidth / 2;
		if ( ymin < CropWindowYMin() - m_FilterYWidth / 2 ) ymin = CropWindowYMin() - m_FilterYWidth / 2;
		if ( xmax > CropWindowXMax() + m_FilterXWidth / 2 ) xmax = CropWindowXMax() + m_FilterXWidth / 2;
		if ( ymax > CropWindowYMax() + m_FilterYWidth / 2 ) ymax = CropWindowYMax() + m_FilterYWidth / 2;

		// Inform the status class how far we have got, and update UI.
		float Complete = ( m_cXBuckets * m_cYBuckets );
		Complete /= iBucket;
		Complete = 100.0f / Complete;
		QGetRenderContext() ->Stats().SetComplete( Complete );

		RtProgressFunc pProgressHandler;
		if ( ( pProgressHandler = QGetRenderContext() ->optCurrent().pProgressHandler() ) != 0 )
		{
			( *pProgressHandler ) ( Complete );
		}

		RenderSurfaces( iBucket, xmin, xmax, ymin, ymax );
		if ( m_fQuit )
		{
			m_fDone = TqTrue;
			return ;
		}
#ifdef WIN32
		if ( !( iBucket % m_cXBuckets ) )
			SetProcessWorkingSetSize( GetCurrentProcess(), 0xffffffff, 0xffffffff );
#endif

	}

	ImageComplete();
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


