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

#include	"aqsis.h"

#include	<strstream>
#ifdef WIN32
#include    <windows.h>
#endif
#include	<math.h>

#include	"stats.h"
#include	"options.h"
#include	"renderer.h"
#include	"surface.h"
#include	"micropolygon.h"
#include	"imagebuffer.h"
#include	"occlusion.h"


START_NAMESPACE( Aqsis )

static TqInt bucketmodulo = -1;

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
	/* add artificially a border around based on the current filterwidth so the diced primitives 
     * may fit better within a bucket */
	m_FilterXWidth = static_cast<TqInt>( QGetRenderContext() ->optCurrent().GetFloatOption( "System", "FilterWidth" ) [ 0 ] );
	m_FilterYWidth = static_cast<TqInt>( QGetRenderContext() ->optCurrent().GetFloatOption( "System", "FilterWidth" ) [ 1 ] );

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
	
//    m_XBucketSize += (2 * m_FilterXWidth);
//    m_YBucketSize += (2 * m_FilterYWidth);
	
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

	assert( nBucket >= iCurrentBucket() ); 
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
 * \param pmpgNew Pointer to a CqMicroPolygon derived class.
 */

void CqImageBuffer::AddMPG( CqMicroPolygon* pmpgNew )
{
	// Quick check for outside crop window.
	CqBound	B( pmpgNew->GetTotalBound() );
	pmpgNew->AddRef();

	if ( B.vecMax().x() < m_CropWindowXMin - m_FilterXWidth / 2 || B.vecMax().y() < m_CropWindowYMin - m_FilterYWidth / 2 ||
		 B.vecMin().x() > m_CropWindowXMax + m_FilterXWidth / 2 || B.vecMin().y() > m_CropWindowYMax + m_FilterYWidth / 2 )
	{
		pmpgNew->Release();
		return ;
	}

	// Find out the minimum bucket touched by the micropoly bound.
	TqInt iBkt = m_cXBuckets*m_cYBuckets;

	B.vecMin().x( B.vecMin().x() - m_FilterXWidth / 2 );
	B.vecMin().y( B.vecMin().y() - m_FilterYWidth / 2 );
	B.vecMax().x( B.vecMax().x() + m_FilterXWidth / 2 );
	B.vecMax().y( B.vecMax().y() + m_FilterYWidth / 2 );

	TqInt iXBa = static_cast<TqInt>( B.vecMin().x() / ( m_XBucketSize ) );
	TqInt iYBa = static_cast<TqInt>( B.vecMin().y() / ( m_YBucketSize ) );
	TqInt iXBb = static_cast<TqInt>( B.vecMax().x() / ( m_XBucketSize ) );
	TqInt iYBb = static_cast<TqInt>( B.vecMax().y() / ( m_YBucketSize ) );

	if ( ( iXBb < 0 ) || ( iYBb < 0 ) ||
		 ( iXBa >= m_cXBuckets ) || ( iYBa >= m_cYBuckets ) )
	{
		pmpgNew->Release();
		return;
	}

	if ( iXBa < 0 ) iXBa = 0;
	if ( iYBa < 0 ) iYBa = 0;
	iBkt = ( iYBa * m_cXBuckets ) + iXBa;

	if ( ( iBkt >= iCurrentBucket() ) && ( iBkt < ( m_cXBuckets*m_cYBuckets ) ) )
	{
		m_aBuckets[ iBkt ].AddMPG( pmpgNew );
		pmpgNew->AddRef();
	}
	pmpgNew->Release();
}


//----------------------------------------------------------------------
/** Add a new micro polygon to the list of waiting ones.
 * \param pmpg Pointer to a CqMicroPolygon derived class.
 */

TqBool CqImageBuffer::PushMPGForward( CqMicroPolygon* pmpg )
{
	TqInt CurrBucketIndex = iCurrentBucket();

	// Check if there is anywhere to push forward to.
	TqInt BucketCol = CurrBucketIndex % m_cXBuckets;
	if( BucketCol == ( m_cXBuckets - 1 ) )
		return( TqFalse );

	TqInt BucketRow = CurrBucketIndex / m_cXBuckets;
	TqInt NextBucketForward = ( BucketRow * m_cXBuckets ) + BucketCol + 1;

	// Find out if any of the subbounds touch this bucket.
	CqVector2D BucketMin = Position( NextBucketForward );
	CqVector2D BucketMax = BucketMin + Size( NextBucketForward );
	CqVector2D FilterWidth( m_FilterXWidth * 0.5f, m_FilterYWidth * 0.5f );
	BucketMin -= FilterWidth;
	BucketMax += FilterWidth;

	TqInt NumBounds = pmpg->cSubBounds();
	TqInt BoundIndex;
	for(BoundIndex = 0; BoundIndex<NumBounds; BoundIndex++)
	{
		TqFloat btime;
		CqBound	B( pmpg->SubBound( BoundIndex, btime) );

		const CqVector3D& vMin = B.vecMin();
		const CqVector3D& vMax = B.vecMax();
		if( ( vMin.x() > BucketMax.x() ) ||
			( vMin.y() > BucketMax.y() ) ||
			( vMax.x() < BucketMin.x() ) ||
			( vMax.y() < BucketMin.y() ) )
			continue;
		else
		{
			pmpg->AddRef();
			m_aBuckets[ NextBucketForward ].AddMPG( pmpg );
			pmpg->MarkPushedForward();
			return( TqTrue );
		}
	}
	return( TqFalse );
}


//----------------------------------------------------------------------
/** Add a new micro polygon to the list of waiting ones.
 * \param pmpg Pointer to a CqMicroPolygon derived class.
 * \param CurrBucketIndex Index of the bucket from which we are pushing down.
 */

TqBool CqImageBuffer::PushMPGDown( CqMicroPolygon* pmpg, TqInt CurrBucketIndex )
{
	if( pmpg->IsPushedForward() )	return( TqFalse );

	// Check if there is anywhere to push down to.
	TqInt BucketRow = CurrBucketIndex / m_cXBuckets;
	if( BucketRow == ( m_cYBuckets - 1 ) )
		return( TqFalse );

	TqInt BucketCol = CurrBucketIndex % m_cXBuckets;
	TqInt NextBucketDown = ( ( BucketRow + 1 ) * m_cXBuckets ) + BucketCol;

	// Find out if any of the subbounds touch this bucket.
	CqVector2D BucketMin = Position( NextBucketDown );
	CqVector2D BucketMax = BucketMin + Size( NextBucketDown );
	CqVector2D FilterWidth( m_FilterXWidth * 0.5f, m_FilterYWidth * 0.5f );
	BucketMin -= FilterWidth;
	BucketMax += FilterWidth;

	TqInt NumBounds = pmpg->cSubBounds();
	TqInt BoundIndex;
	for(BoundIndex = 0; BoundIndex<NumBounds; BoundIndex++)
	{
		TqFloat btime;
		CqBound	B( pmpg->SubBound( BoundIndex, btime) );

		const CqVector3D& vMin = B.vecMin();
		const CqVector3D& vMax = B.vecMax();
		if( ( vMin.x() > BucketMax.x() ) ||
			( vMin.y() > BucketMax.y() ) ||
			( vMax.x() < BucketMin.x() ) ||
			( vMax.y() < BucketMin.y() ) )
			continue;
		{
			pmpg->AddRef();
			m_aBuckets[ NextBucketDown ].AddMPG( pmpg );
			// See if it needs to be pushed further down (extreme Motion Blur)
			if( PushMPGDown( pmpg, NextBucketDown ) )
				QGetRenderContext()->Stats().IncMPGsPushedFarDown();
			return( TqTrue );
		}
	}
	return( TqFalse );
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
		std::vector<CqMicroPolyGridBase*>::iterator lastgrid = m_aBuckets[ iBucket ].aGrids().end();

		for ( std::vector<CqMicroPolyGridBase*>::iterator igrid = m_aBuckets[ iBucket ].aGrids().begin(); igrid != lastgrid; igrid++ )
		{
			( *igrid ) ->Split( this, iBucket, xmin, xmax, ymin, ymax );

			// Render any waiting MPGs
			std::vector<CqMicroPolygon*>::iterator lastmpg = m_aBuckets[ iBucket ].aMPGs().end();
			for ( std::vector<CqMicroPolygon*>::iterator impg = m_aBuckets[ iBucket ].aMPGs().begin(); impg != lastmpg; impg++ )
			{
				RenderMicroPoly( *impg, iBucket, xmin, xmax, ymin, ymax );
				if( PushMPGDown( ( *impg ), iBucket ) )	QGetRenderContext()->Stats().IncMPGsPushedDown();
				if( PushMPGForward( ( *impg ) ) )		QGetRenderContext()->Stats().IncMPGsPushedForward();
				( *impg ) ->Release();
			}
			m_aBuckets[ iBucket ].aMPGs().clear();
		}
	}
	m_aBuckets[ iBucket ].aGrids().clear();

	if ( m_aBuckets[ iBucket ].aMPGs().empty() ) return ;

	// Render any waiting MPGs
	std::vector<CqMicroPolygon*>::iterator lastmpg = m_aBuckets[ iBucket ].aMPGs().end();
	for ( std::vector<CqMicroPolygon*>::iterator impg = m_aBuckets[ iBucket ].aMPGs().begin(); impg != lastmpg; impg++ )
	{
		RenderMicroPoly( *impg, iBucket, xmin, xmax, ymin, ymax );
		if( PushMPGDown( ( *impg ), iBucket ) )		QGetRenderContext()->Stats().IncMPGsPushedDown();
		if( PushMPGForward( ( *impg ) ) )			QGetRenderContext()->Stats().IncMPGsPushedForward();
		( *impg ) ->Release();
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

inline void CqImageBuffer::RenderMicroPoly( CqMicroPolygon* pMPG, TqInt iBucket, long xmin, long xmax, long ymin, long ymax )
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
		CqBound Bound = pMPG->SubBound( bound_num, time0 );
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

		CqImagePixel* pie, *pie2;

		TqInt iXSamples = PixelXSamples();
		TqInt iYSamples = PixelYSamples();

		TqInt im = ( bminx < sX ) ? 0 : FLOOR( ( bminx - sX ) * iXSamples );
		TqInt in = ( bminy < sY ) ? 0 : FLOOR( ( bminy - sY ) * iYSamples );
		TqInt em = ( bmaxx > eX ) ? iXSamples : CEIL( ( bmaxx - ( eX - 1 ) ) * iXSamples );
		TqInt en = ( bmaxy > eY ) ? iYSamples : CEIL( ( bmaxy - ( eY - 1 ) ) * iYSamples );

		register long iY = sY;

		CqColor colMPGColor( 1.0f,1.0f,1.0f );
		CqColor colMPGOpacity( 1.0f,1.0f,1.0f );
		// Must check if opacity is needed, as if not, the variable will have been deleted from the grid.
		if( QGetRenderContext() ->pDDmanager()->fDisplayNeeds( "Ci" ) )
			colMPGColor = pMPG->colColor();
		// Must check if opacity is needed, as if not, the variable will have been deleted from the grid.
		if( QGetRenderContext() ->pDDmanager()->fDisplayNeeds( "Oi" ) )
			colMPGOpacity = pMPG->colOpacity();

		TqInt nextx = Bucket.XSize() + Bucket.XFWidth();
		Bucket.ImageElement( sX, sY, pie );

		while ( iY < eY )
		{
			register long iX = sX;

			pie2 = pie;
			pie += nextx;

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
						const CqVector2D& vecP = pie2->SamplePoint( m, n );
						theStats.IncSamples();

						TqFloat t = pie2->SampleTime( m, n );
						// First, check if the subsample point lies within the micropoly bound
						if ( t >= time0 && t <= time1 && Bound.Contains2D( vecP ) )
						{
							theStats.IncSampleBoundHits();

							// Check to see if the sample is within the sample's level of detail
							if ( UsingLevelOfDetail )
							{
								TqFloat LevelOfDetail = pie2->SampleLevelOfDetail( m, n );
								if ( LodBounds[ 0 ] > LevelOfDetail || LevelOfDetail >= LodBounds[ 1 ] )
								{
									continue;
								}
							}

							// Now check if the subsample hits the micropoly
							if ( pMPG->Sample( vecP, ImageVal.m_Depth, t ) )
							{
								theStats.IncSampleHits();
								pMPG->MarkHit();
								// Sort the color/opacity into the visible point list
								std::vector<SqImageSample>& aValues = pie2->Values( m, n );
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
									CqOcclusionBox::MarkForUpdate( pie2->OcclusionBoxId() );
									pie2->MarkForZUpdate();
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
				pie2++;
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
				if (!bIsEmpty && pSurface->fCachedBound())
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
					Bucket.AddGrid( pGrid );
					// Render any waiting micro polygon grids.
					QGetRenderContext() ->Stats().RenderMPGsTimer().Start();
					RenderMPGs( iBucket, xmin, xmax, ymin, ymax );
					QGetRenderContext() ->Stats().RenderMPGsTimer().Stop();
				}
				else
					delete( pGrid );
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
		//if ( !( iBucket % bucketmodulo ) )
			SetProcessWorkingSetSize( GetCurrentProcess(), 0xffffffff, 0xffffffff );
#endif
		TqInt iB2, NumGrids = 0, NumPolys = 0;
		for ( iB2 = 0; iB2 < nBuckets; iB2++ )
		{
			NumGrids += m_aBuckets[ iB2 ].aGrids().size();
			NumPolys += m_aBuckets[ iB2 ].aMPGs().size();
		}
	}

	ImageComplete();

	CqBucket::EmptyBucket();

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


