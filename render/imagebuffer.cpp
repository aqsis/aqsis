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
/** Get the screen position for the current bucket.
 * \return Bucket position as 2d vector (xpos, ypos).
 */

CqVector2D	CqImageBuffer::BucketPosition( ) const
{
    return( BucketPosition( CurrentBucketCol(), CurrentBucketRow() ) );
}


//----------------------------------------------------------------------
/** Get the screen position for the bucket at x,y.
 * \return Bucket position as 2d vector (xpos, ypos).
 */

CqVector2D	CqImageBuffer::BucketPosition( TqInt x, TqInt y ) const
{
    CqVector2D	vecA;
    vecA.x( x );
    vecA.y( y );
    vecA.x( vecA.x() * XBucketSize() );
    vecA.y( vecA.y() * YBucketSize() );

    return ( vecA );
}

//----------------------------------------------------------------------
/** Get the bucket size for the current bucket.
 
    Usually the return value is just (XBucketSize, YBucketSize) except
    for the buckets on the right and bottom side of the image where the
    size can be smaller. The crop window is not taken into account.
 
 * \return Bucket size as 2d vector (xsize, ysize).
 */

CqVector2D CqImageBuffer::BucketSize( ) const
{
    return( BucketSize( CurrentBucketCol(), CurrentBucketRow() ) );
}


//----------------------------------------------------------------------
/** Get the bucket size for the bucket at x,y.
 
    Usually the return value is just (XBucketSize, YBucketSize) except
    for the buckets on the right and bottom side of the image where the
    size can be smaller. The crop window is not taken into account.
 
 * \return Bucket size as 2d vector (xsize, ysize).
 */

CqVector2D CqImageBuffer::BucketSize(TqInt x, TqInt y) const
{
    CqVector2D	vecA = BucketPosition(x,y);
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

    m_Buckets.resize( m_cYBuckets );
    std::vector<std::vector<CqBucket> >::iterator i;
    for( i = m_Buckets.begin(); i!=m_Buckets.end(); i++)
    {
        i->resize( m_cXBuckets );
        std::vector<CqBucket>::iterator b;
        for( b = i->begin(); b!=i->end(); b++)
            b->SetProcessed( TqFalse );
    }

    m_CurrentBucketCol = m_CurrentBucketRow = 0;
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
            CqString objname( "unnamed" );
            const CqString* pattrName = pSurface->pAttributes() ->GetStringAttribute( "identifier", "name" );
            if ( pattrName != 0 ) objname = pattrName[ 0 ];
			std::cerr << warning << "Max eyesplits for object \"" << objname.c_str() << "\" exceeded" << std::endl;
			return( TqTrue );
        }
        return ( TqFalse );
    }

    TqFloat minz = Bound.vecMin().z();
    TqFloat maxz = Bound.vecMax().z();

    // Take into account depth-of-field (in camera space)
    if ( QGetRenderContext() ->UsingDepthOfField() )
    {
        const CqVector2D minZCoc = QGetRenderContext()->GetCircleOfConfusion( minz );
        const CqVector2D maxZCoc = QGetRenderContext()->GetCircleOfConfusion( maxz );
        TqFloat cocX = MAX( minZCoc.x(), maxZCoc.x() );
        TqFloat cocY = MAX( minZCoc.y(), maxZCoc.y() );
        Bound.vecMin().x( Bound.vecMin().x() - cocX );
        Bound.vecMin().y( Bound.vecMin().y() - cocY );
        Bound.vecMax().x( Bound.vecMax().x() + cocX );
        Bound.vecMax().y( Bound.vecMax().y() + cocY );
    }

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
    // Initially, the surface enters this method with zero references, so add one.
    ADDREF( pSurface );

    // Count the number of total gprims
    STATS_INC( GPR_created_total );

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

    // Check if the surface can be culled. (also adjusts for DOF and converts Bound to raster space).
    if ( CullSurface( Bound, pSurface ) )
    {
        pSurface->UnLink();
        RELEASEREF( pSurface );
        STATS_INC( GPR_culled );
        return ;
    }

    // If the primitive has been marked as undiceable by the eyeplane check, then we cannot get a valid
    // bucket index from it as the projection of the bound would cross the camera plane and therefore give a false
    // result, so just put it back in the current bucket for further splitting.
    TqInt XMinb = CurrentBucketCol(), YMinb = CurrentBucketRow();
    if ( !pSurface->IsUndiceable() )
    {
        // Find out which bucket(s) the surface belongs to.
        if ( Bound.vecMin().x() < 0 ) Bound.vecMin().x( 0.0f );
        if ( Bound.vecMin().y() < 0 ) Bound.vecMin().y( 0.0f );

        XMinb = static_cast<TqInt>( Bound.vecMin().x() ) / XBucketSize();
        YMinb = static_cast<TqInt>( Bound.vecMin().y() ) / YBucketSize();

        if ( XMinb >= cXBuckets() || YMinb >= cYBuckets() )
            return;

        XMinb = CLAMP( XMinb, 0, cXBuckets() );
        YMinb = CLAMP( YMinb, 0, cYBuckets() );

        if( Bucket(XMinb, YMinb).IsProcessed() )
        {
            XMinb = CurrentBucketCol();
            YMinb = CurrentBucketRow();
        }
    }
    // Sanity check we are not putting into a bucket that has already been processed.
    assert( !Bucket(XMinb, YMinb).IsProcessed() );
    Bucket(XMinb, YMinb).AddGPrim( pSurface );

    // Release the reference acquired for the surface for this method.
    RELEASEREF( pSurface );
    return ;

}


//----------------------------------------------------------------------
/** Test if this surface can be occlusion culled. If it can then
 * transfer surface to the next bucket it covers, or delete it if it
 * covers no more.
 * \param pSurface A pointer to a CqBasicSurface derived class.
 * \return Boolean indicating that the GPrim has been culled.
*/

TqBool CqImageBuffer::OcclusionCullSurface( CqBasicSurface* pSurface )
{
    CqBound RasterBound( pSurface->GetCachedRasterBound() );

    TqInt nBuckets = m_cXBuckets * m_cYBuckets;

    if ( pSurface->pCSGNode() )
        return ( TqFalse );

    if ( CqOcclusionBox::CanCull( &RasterBound ) )
    {
        // pSurface is behind everying in this bucket but it may be
        // visible in other buckets it overlaps.
        // bucket to the right
        TqInt nextBucket = CurrentBucketCol() + 1;
        CqVector2D pos = BucketPosition( nextBucket, CurrentBucketRow() );
        if ( ( nextBucket < cXBuckets() ) &&
                ( RasterBound.vecMax().x() >= pos.x() ) )
        {
            // Here, pSurface is unlinked from the list in the
            //  current bucket, and then added to the list in
            //  the next bucket.  When pSurface is added to the
            //  next bucket, it gets a new reference (with
            //  ADDREF()), so its original bucket reference
            //  needs to be removed.  However, this original
            //  reference must be removed *after* the new one
            //  has been added to stop the surface from being
            //  deleted. - Jonathan Merritt.
            CqString objname( "unnamed" );
            const CqString* pattrName = pSurface->pAttributes() ->GetStringAttribute( "identifier", "name" );
            if( pattrName )	objname = *pattrName;
            std::cerr << info << "GPrim: \"" << objname << "\" occlusion culled" << std::endl;
            pSurface->UnLink();
            Bucket( nextBucket, CurrentBucketRow() ).AddGPrim( pSurface );
            RELEASEREF( pSurface );
            STATS_INC( GPR_culled );
            return TqTrue;
        }

        // next row
        nextBucket = CurrentBucketRow() + 1;
        // find bucket containing left side of bound
        TqInt nextBucketX = static_cast<TqInt>( RasterBound.vecMin().x() ) / XBucketSize();
        nextBucketX = MAX( nextBucketX, 0 );
        pos = BucketPosition( nextBucketX, nextBucket );

        if ( ( nextBucketX < cXBuckets() ) &&
                ( nextBucket  < cYBuckets() ) &&
                ( RasterBound.vecMax().y() >= pos.y() ) )
        {
            // See above for comments... - Jonathan Merritt.
            CqString objname( "unnamed" );
            const CqString* pattrName = pSurface->pAttributes() ->GetStringAttribute( "identifier", "name" );
            if( pattrName )	objname = *pattrName;
            std::cerr << info << "GPrim: \"" << objname << "\" occlusion culled" << std::endl;
            pSurface->UnLink();
            Bucket( nextBucketX, nextBucket ).AddGPrim( pSurface );
            RELEASEREF( pSurface );
            STATS_INC( GPR_culled );
            return TqTrue;
        }

        // Bound covers no more buckets
        CqString objname( "unnamed" );
        const CqString* pattrName = pSurface->pAttributes() ->GetStringAttribute( "identifier", "name" );
        if( pattrName )	objname = *pattrName;
        std::cerr << info << "GPrim: \"" << objname << "\" occlusion culled" << std::endl;
        pSurface->UnLink();
        RELEASEREF( pSurface );
        STATS_INC( GPR_culled );
        return TqTrue;
    }
    else
    {
        return TqFalse;
    }
}


//----------------------------------------------------------------------
/** Add a new micro polygon to the list of waiting ones.
 * \param pmpgNew Pointer to a CqMicroPolygon derived class.
 */

void CqImageBuffer::AddMPG( CqMicroPolygon* pmpgNew )
{
    // Quick check for outside crop window.
    CqBound	B( pmpgNew->GetTotalBound() );
    ADDREF( pmpgNew );

    if ( B.vecMax().x() < m_CropWindowXMin - m_FilterXWidth / 2 || B.vecMax().y() < m_CropWindowYMin - m_FilterYWidth / 2 ||
            B.vecMin().x() > m_CropWindowXMax + m_FilterXWidth / 2 || B.vecMin().y() > m_CropWindowYMax + m_FilterYWidth / 2 )
    {
        RELEASEREF( pmpgNew );
        return ;
    }

    // Find out the minimum bucket touched by the micropoly bound.
    TqInt iBkt = m_cXBuckets * m_cYBuckets;

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
        RELEASEREF( pmpgNew );
        return ;
    }

    if ( iXBa < 0 ) iXBa = 0;
    if ( iYBa < 0 ) iYBa = 0;

    // If the ideal bucket has already been processed we need to work out why we have got into this
    // situation.
    if ( Bucket(iXBa,iYBa).IsProcessed() )
    {
        PushMPGDown( pmpgNew, iXBa, iYBa );
        PushMPGForward( pmpgNew, iXBa, iYBa );
        RELEASEREF( pmpgNew );
        return ;
    }
    assert( !Bucket(iXBa, iYBa).IsProcessed() );
    Bucket(iXBa, iYBa).AddMPG( pmpgNew );
    ADDREF( pmpgNew );

    RELEASEREF( pmpgNew );
}


//----------------------------------------------------------------------
/** Add a new micro polygon to the list of waiting ones.
 * \param pmpg Pointer to a CqMicroPolygon derived class.
 */

TqBool CqImageBuffer::PushMPGForward( CqMicroPolygon* pmpg, TqInt Col, TqInt Row )
{
    // Should always mark as pushed forward even if not. As this is an idicator
    // that the attempt has been made, used by the PushDown function. If this wasn't set
    // then the mpg would be pushed down again when the next row is hit.
    pmpg->MarkPushedForward();

    // Check if there is anywhere to push forward to.
    if ( Col == ( cXBuckets() - 1 ) )
        return ( TqFalse );

    TqInt NextBucketForward = Col + 1;

    // If the next bucket forward has already been processed, try the one following that.
    if( Bucket( NextBucketForward, Row ).IsProcessed() )
        return( PushMPGForward( pmpg, NextBucketForward, Row ) );

    // Find out if any of the subbounds touch this bucket.
    CqVector2D BucketMin = BucketPosition( NextBucketForward, Row );
    CqVector2D BucketMax = BucketMin + BucketSize( NextBucketForward, Row );
    CqVector2D FilterWidth( m_FilterXWidth * 0.5f, m_FilterYWidth * 0.5f );
    BucketMin -= FilterWidth;
    BucketMax += FilterWidth;

    CqBound	B( pmpg->GetTotalBound() );

    const CqVector3D& vMin = B.vecMin();
    const CqVector3D& vMax = B.vecMax();
    if ( ( vMin.x() > BucketMax.x() ) ||
            ( vMin.y() > BucketMax.y() ) ||
            ( vMax.x() < BucketMin.x() ) ||
            ( vMax.y() < BucketMin.y() ) )
    {
        return ( TqFalse );
    }
    else
    {
        ADDREF( pmpg );
        Bucket( NextBucketForward, Row ).AddMPG( pmpg );
        return ( TqTrue );
    }
    return ( TqFalse );
}


//----------------------------------------------------------------------
/** Add a new micro polygon to the list of waiting ones.
 * \param pmpg Pointer to a CqMicroPolygon derived class.
 * \param CurrBucketIndex Index of the bucket from which we are pushing down.
 */

TqBool CqImageBuffer::PushMPGDown( CqMicroPolygon* pmpg, TqInt Col, TqInt Row )
{
    if ( pmpg->IsPushedForward() ) return ( TqFalse );

    // Check if there is anywhere to push down to.
    if ( Row == ( m_cYBuckets - 1 ) )
        return ( TqFalse );

    TqInt NextBucketDown = Row + 1;

    // If the next bucket down has already been processed,
    // try pushing forward from there.
    if( Bucket( Col, NextBucketDown ).IsProcessed() )
    {
        if( PushMPGForward( pmpg, Col, NextBucketDown ) )
            return( TqTrue );
        else
            // If that fails, push down again.
            return( PushMPGDown( pmpg, Col, NextBucketDown ) );
    }

    // Find out if any of the subbounds touch this bucket.
    CqVector2D BucketMin = BucketPosition( Col, NextBucketDown );
    CqVector2D BucketMax = BucketMin + BucketSize( Col, NextBucketDown );
    CqVector2D FilterWidth( m_FilterXWidth * 0.5f, m_FilterYWidth * 0.5f );
    BucketMin -= FilterWidth;
    BucketMax += FilterWidth;

    CqBound	B( pmpg->GetTotalBound( ) );

    const CqVector3D& vMin = B.vecMin();
    const CqVector3D& vMax = B.vecMax();
    if ( ( vMin.x() > BucketMax.x() ) ||
            ( vMin.y() > BucketMax.y() ) ||
            ( vMax.x() < BucketMin.x() ) ||
            ( vMax.y() < BucketMin.y() ) )
    {
        return ( TqFalse );
    }
    else
    {
        ADDREF( pmpg );
        Bucket(Col, NextBucketDown ).AddMPG( pmpg );
        // See if it needs to be pushed further down (extreme Motion Blur)
        if ( PushMPGDown( pmpg, Col, NextBucketDown ) )
            STATS_INC( MPG_pushed_far_down );
        return ( TqTrue );
    }
    return ( TqFalse );
}



//----------------------------------------------------------------------
/** Render any waiting MPGs.
 
    All micro polygon grids in the specified bucket are bust into
    individual micro polygons which are assigned to their appropriate
    bucket. Then RenderMicroPoly() is called for each micro polygon in
    the current bucket.
 
 * \param xmin Integer minimum extend of the image part being rendered, takes into account buckets and clipping.
 * \param xmax Integer maximum extend of the image part being rendered, takes into account buckets and clipping.
 * \param ymin Integer minimum extend of the image part being rendered, takes into account buckets and clipping.
 * \param ymax Integer maximum extend of the image part being rendered, takes into account buckets and clipping.
 */

void CqImageBuffer::RenderMPGs( long xmin, long xmax, long ymin, long ymax )
{
    // First of all split any grids in this bucket waiting to be processed.
    if ( !CurrentBucket().aGrids().empty() )
    {
        std::vector<CqMicroPolyGridBase*>::iterator lastgrid = CurrentBucket().aGrids().end();

        for ( std::vector<CqMicroPolyGridBase*>::iterator igrid = CurrentBucket().aGrids().begin(); igrid != lastgrid; igrid++ )
        {
            ( *igrid ) ->Split( this, xmin, xmax, ymin, ymax );

            // Render any waiting MPGs
            std::vector<CqMicroPolygon*>::iterator lastmpg = CurrentBucket().aMPGs().end();
            for ( std::vector<CqMicroPolygon*>::iterator impg = CurrentBucket().aMPGs().begin(); impg != lastmpg; impg++ )
            {
                RenderMicroPoly( *impg, xmin, xmax, ymin, ymax );
                if ( PushMPGDown( ( *impg ), CurrentBucketCol(), CurrentBucketRow() ) )
                    STATS_INC( MPG_pushed_down );
                if ( PushMPGForward( ( *impg ), CurrentBucketCol(), CurrentBucketRow() ) )
                    STATS_INC( MPG_pushed_forward );
                RELEASEREF( ( *impg ) );
            }
            CurrentBucket().aMPGs().clear();
        }
    }
    CurrentBucket().aGrids().clear();

    if ( CurrentBucket().aMPGs().empty() ) return ;

    // Render any waiting MPGs
    std::vector<CqMicroPolygon*>::iterator lastmpg = CurrentBucket().aMPGs().end();
    for ( std::vector<CqMicroPolygon*>::iterator impg = CurrentBucket().aMPGs().begin(); impg != lastmpg; impg++ )
    {
        RenderMicroPoly( *impg, xmin, xmax, ymin, ymax );
        if ( PushMPGDown( ( *impg ), CurrentBucketCol(), CurrentBucketRow() ) )
            STATS_INC( MPG_pushed_down );
        if ( PushMPGForward( ( *impg ), CurrentBucketCol(), CurrentBucketRow() ) )
            STATS_INC( MPG_pushed_forward );
        RELEASEREF( ( *impg ) );
    }
    CurrentBucket().aMPGs().clear();
}


//----------------------------------------------------------------------
/** Render a particular micropolygon.
 
 * \param pMPG Pointer to the micropolygon to process.
 * \param xmin Integer minimum extend of the image part being rendered, takes into account buckets and clipping.
 * \param xmax Integer maximum extend of the image part being rendered, takes into account buckets and clipping.
 * \param ymin Integer minimum extend of the image part being rendered, takes into account buckets and clipping.
 * \param ymax Integer maximum extend of the image part being rendered, takes into account buckets and clipping.
 
   \see CqBucket, CqImagePixel
 */

inline void CqImageBuffer::RenderMicroPoly( CqMicroPolygon* pMPG, long xmin, long xmax, long ymin, long ymax )
{
    CqBucket & Bucket = CurrentBucket();
    CqStats& theStats = QGetRenderContext() ->Stats();

    const TqFloat* LodBounds = pMPG->pGrid() ->pAttributes() ->GetFloatAttribute( "System", "LevelOfDetailBounds" );
    TqBool UsingLevelOfDetail = LodBounds[ 0 ] >= 0.0f;

    TqBool UsingDepthOfField = QGetRenderContext() ->UsingDepthOfField();

    TqInt bound_max = pMPG->cSubBounds();
    TqInt bound_max_1 = bound_max - 1;
    TqInt sample_hits = 0;
    TqInt shd_rate = pMPG->pGrid() ->pAttributes() ->GetFloatAttribute( "System", "ShadingRate" ) [ 0 ];

    // fill in sample info for this mpg so we don't have to keep fetching it for each sample.
    m_CurrentMpgSampleInfo.m_IsMatte = pMPG->pGrid() ->pAttributes() ->GetIntegerAttribute( "System", "Matte" ) [ 0 ] == 1;
    // Must check if colour is needed, as if not, the variable will have been deleted from the grid.
    if ( QGetRenderContext() ->pDDmanager() ->fDisplayNeeds( "Ci" ) )
    {
        m_CurrentMpgSampleInfo.m_Colour = pMPG->colColor();
    }
    else
    {
        m_CurrentMpgSampleInfo.m_Colour = gColWhite;
    }

    // Must check if opacity is needed, as if not, the variable will have been deleted from the grid.
    if ( QGetRenderContext() ->pDDmanager() ->fDisplayNeeds( "Oi" ) )
    {
        m_CurrentMpgSampleInfo.m_Opacity = pMPG->colOpacity();
        m_CurrentMpgSampleInfo.m_Occludes = pMPG->colOpacity() >= gColWhite;
    }
    else
    {
        m_CurrentMpgSampleInfo.m_Opacity = gColWhite;
        m_CurrentMpgSampleInfo.m_Occludes = TqTrue;
    }

    for ( TqInt bound_num = 0; bound_num < bound_max ; bound_num++ )
    {
        TqFloat time0;
        CqBound Bound = pMPG->SubBound( bound_num, time0 );
        TqFloat time1 = QGetRenderContext() ->optCurrent().GetFloatOptionWrite( "System", "Shutter" ) [ 1 ];
        if ( bound_num != bound_max_1 )
            pMPG->SubBound( bound_num + 1, time1 );

        TqFloat bminx = Bound.vecMin().x();
        TqFloat bmaxx = Bound.vecMax().x();
        TqFloat bminy = Bound.vecMin().y();
        TqFloat bmaxy = Bound.vecMax().y();

        // these values are the bound of the mpg not including dof extension.
        // the values above (bminx etc) *do* include dof.
        // if dof is turned off the values will be the same.
        TqFloat mpgbminx;
        TqFloat mpgbmaxx;
        TqFloat mpgbminy;
        TqFloat mpgbmaxy;
        if ( UsingDepthOfField )
        {
            const CqVector2D minZCoc = QGetRenderContext()->GetCircleOfConfusion( Bound.vecMin().z() );
            const CqVector2D maxZCoc = QGetRenderContext()->GetCircleOfConfusion( Bound.vecMax().z() );
            TqFloat cocX = MAX( minZCoc.x(), maxZCoc.x() );
            TqFloat cocY = MAX( minZCoc.y(), maxZCoc.y() );

            // reduce the mpg bound so it doesn't include the coc.
            mpgbminx = bminx + cocX;
            mpgbmaxx = bmaxx - cocX;
            mpgbminy = bminy + cocY;
            mpgbmaxy = bmaxy - cocY;
        }
        else
        {
            mpgbminx = bminx;
            mpgbmaxx = bmaxx;
            mpgbminy = bminy;
            mpgbmaxy = bmaxy;
        }

        if ( bmaxx < xmin || bmaxy < ymin || bminx > xmax || bminy > ymax )
        {
            if ( bound_num == bound_max_1 )
            {
                // last bound so we can delete the mpg
                STATS_INC( MPG_culled );
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
                STATS_INC( MPG_culled );
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
        if ( eX > xmax ) eX = xmax;
        if ( eY > ymax ) eY = ymax;

        TqInt sX = FLOOR( bminx );
        TqInt sY = FLOOR( bminy );
        if ( sY < ymin ) sY = ymin;
        if ( sX < xmin ) sX = xmin;

        // these values give the pixel bound of the mpg *not* including dof.
        TqInt mpg_eX = CEIL( mpgbmaxx );
        TqInt mpg_eY = CEIL( mpgbmaxy );
        if ( mpg_eX > xmax ) mpg_eX = xmax;
        if ( mpg_eY > ymax ) mpg_eY = ymax;

        TqInt mpg_sX = FLOOR( mpgbminx );
        TqInt mpg_sY = FLOOR( mpgbminy );
        if ( mpg_sY < ymin ) mpg_sY = ymin;
        if ( mpg_sX < xmin ) mpg_sX = xmin;

        CqImagePixel* pie, *pie2;

        TqInt iXSamples = PixelXSamples();
        TqInt iYSamples = PixelYSamples();

        TqInt im = ( bminx < sX ) ? 0 : FLOOR( ( bminx - sX ) * iXSamples );
        TqInt in = ( bminy < sY ) ? 0 : FLOOR( ( bminy - sY ) * iYSamples );
        TqInt em = ( bmaxx > eX ) ? iXSamples : CEIL( ( bmaxx - ( eX - 1 ) ) * iXSamples );
        TqInt en = ( bmaxy > eY ) ? iYSamples : CEIL( ( bmaxy - ( eY - 1 ) ) * iYSamples );

        register long iY = sY;

        CqVector2D coc;
        if( UsingDepthOfField )
        {
            TqFloat ad; // Average depth

            ad = pMPG->PointA().z() + pMPG->PointB().z() + pMPG->PointC().z() + pMPG->PointD().z();
            ad /= 4;
            coc = QGetRenderContext()->GetCircleOfConfusion( ad );
        }

        TqInt nextx = Bucket.Width() + Bucket.FilterXWidth();
        Bucket.ImageElement( sX, sY, pie );

        // quadX and quadY hold which quadrant we are currently sampling in
        // reletive to the mpg. they are used for dof sampling optimisations.
        // a value of -1 means we are in a pixel wholly left (or above for quadY) of the mpg.
        // a value of +1 means we are wholly to the right of the mpg.
        // a value of 0 means the current pixel intersects the mpg bound.
        // if we are wholly left of the mpg, and the dof sample offset has a -ve x value
        // then we needn't bother checking any further. ditto for y.
        TqInt quadX;
        TqInt quadY;

        while ( iY < eY )
        {
            register long iX = sX;

            pie2 = pie;
            pie += nextx;

            if(iY < mpg_sY)
                quadY = -1;
            else if(iY > mpg_eY)
                quadY = 1;
            else
                quadY = 0;

            while ( iX < eX )
            {
                if(iX < mpg_sX)
                    quadX = -1;
                else if(iX > mpg_eX)
                    quadX = 1;
                else
                    quadX = 0;

                // Now sample the micropolygon at several subsample positions
                // within the pixel. The subsample indices range from (start_m, n)
                // to (end_m-1, end_n-1).
                register int m, n;
                n = ( iY == sY ) ? in : 0;
                int end_n = ( iY == ( eY - 1 ) ) ? en : iYSamples;
                int start_m = ( iX == sX ) ? im : 0;
                int end_m = ( iX == ( eX - 1 ) ) ? em : iXSamples;
                int index_start = n*iXSamples + start_m;
                for ( ; n < end_n; n++ )
                {
                    int index = index_start;
                    for ( m = start_m; m < end_m; m++ )
                    {
                        const SqSampleData& sampleData = pie2->SampleData( index );
                        const CqVector2D& vecP = sampleData.m_Position;

						STATS_INC( SPL_count );

                        index++;

                        TqFloat t;
                        t = sampleData.m_Time;
                        if( t < time0 || t > time1)
                            continue;

                        TqFloat vecX;
                        TqFloat vecY;
                        if ( UsingDepthOfField )
                        {
                            // check if the offset is pointing away from the mpg.
                            if(	(sampleData.m_DofOffsetXQuad == quadX) ||
                                    (sampleData.m_DofOffsetYQuad == quadY) )
                            {
                                continue;
                            }
                            else
                            {
                                STATS_INC( SPL_bound_hits );
                                // check if the displaced sample will fall outside the mpg
                                // if outside in x dimension, don't bother transforming y
                                vecX = vecP.x() + coc.x()*sampleData.m_DofOffset.x();
                                if(mpgbminx > vecX || mpgbmaxx < vecX)
                                {
                                    continue;
                                }
                                else
                                {
                                    vecY = vecP.y() + coc.y()*sampleData.m_DofOffset.y();
                                    if(mpgbminy > vecY || mpgbmaxy < vecY)
                                        continue;
                                }
                            }
                        }

                        // Check to see if the sample is within the sample's level of detail
                        if ( UsingLevelOfDetail)
                        {
                            TqFloat LevelOfDetail = sampleData.m_DetailLevel;
                            if ( LodBounds[ 0 ] > LevelOfDetail || LevelOfDetail >= LodBounds[ 1 ] )
                            {
                                continue;
                            }
                        }

                        if ( !UsingDepthOfField ) // already checked this if we're using dof.
                        {
                            if(!Bound.Contains2D( vecP ))
                                continue;
                        }

                        STATS_INC( SPL_bound_hits );


                        // Now check if the subsample hits the micropoly
                        TqBool SampleHit;
                        TqFloat D;

                        if( UsingDepthOfField )
                        {
                            CqVector2D vecPDof(vecX, vecY);
                            SampleHit = pMPG->Sample( vecPDof, D, t );
                        }
                        else
                        {
                            SampleHit = pMPG->Sample( vecP, D, t );
                        }

                        if ( SampleHit )
                        {
                            STATS_INC( SPL_hits );
                            pMPG->MarkHit();

                            sample_hits++;

                            StoreSample( pMPG, pie2, m, n, D );
                        }
                    }
                    index_start += iXSamples;
                }
                iX++;
                pie2++;

                // Now compute the % of samples that hit...
                TqInt scount = iXSamples * iYSamples;
                TqFloat max_hits = scount * shd_rate;
                TqInt hit_rate = ( sample_hits / max_hits ) / 0.125;
                STATS_INC( MPG_sample_coverage0_125 + CLAMP( hit_rate - 1 , 0, 7 ) );
            }
            iY++;
        }
    }
}



inline void CqImageBuffer::StoreSample( CqMicroPolygon* pMPG, CqImagePixel* pie2, TqInt m, TqInt n, TqFloat D )
{
    static SqImageSample ImageVal( QGetRenderContext() ->GetOutputDataTotalSize() );
    ImageVal.SetDepth( D );

    std::valarray<TqFloat>& val = ImageVal.m_Data;
    val[ 0 ] = m_CurrentMpgSampleInfo.m_Colour.fRed();
    val[ 1 ] = m_CurrentMpgSampleInfo.m_Colour.fGreen();
    val[ 2 ] = m_CurrentMpgSampleInfo.m_Colour.fBlue();
    val[ 3 ] = m_CurrentMpgSampleInfo.m_Opacity.fRed();
    val[ 4 ] = m_CurrentMpgSampleInfo.m_Opacity.fGreen();
    val[ 5 ] = m_CurrentMpgSampleInfo.m_Opacity.fBlue();
    val[ 6 ] = D;

    TqBool Occludes = m_CurrentMpgSampleInfo.m_Occludes;

    // Now store any other data types that have been registered.
    std::map<std::string, CqRenderer::SqOutputDataEntry>& DataMap = QGetRenderContext() ->GetMapOfOutputDataEntries();
    std::map<std::string, CqRenderer::SqOutputDataEntry>::iterator entry;
    for ( entry = DataMap.begin(); entry != DataMap.end(); entry++ )
    {
        IqShaderData* pData;
        if ( ( pData = pMPG->pGrid() ->FindStandardVar( entry->first.c_str() ) ) != NULL )
        {
            switch ( pData->Type() )
            {
            case type_float:
            case type_integer:
                {
                    TqFloat f;
                    pData->GetFloat( f, pMPG->GetIndex() );
                    val[ entry->second.m_Offset ] = f;
                    break;
                }
            case type_point:
            case type_normal:
            case type_vector:
            case type_hpoint:
                {
                    CqVector3D v;
                    pData->GetPoint( v, pMPG->GetIndex() );
                    val[ entry->second.m_Offset ] = v.x();
                    val[ entry->second.m_Offset + 1 ] = v.y();
                    val[ entry->second.m_Offset + 2 ] = v.z();
                    break;
                }
            case type_color:
                {
                    CqColor c;
                    pData->GetColor( c, pMPG->GetIndex() );
                    val[ entry->second.m_Offset ] = c.fRed();
                    val[ entry->second.m_Offset + 1 ] = c.fGreen();
                    val[ entry->second.m_Offset + 2 ] = c.fBlue();
                    break;
                }
            case type_matrix:
                {
                    CqMatrix m;
                    pData->GetMatrix( m, pMPG->GetIndex() );
                    TqFloat* pElements = m.pElements();
                    val[ entry->second.m_Offset ] = pElements[ 0 ];
                    val[ entry->second.m_Offset + 1 ] = pElements[ 1 ];
                    val[ entry->second.m_Offset + 2 ] = pElements[ 2 ];
                    val[ entry->second.m_Offset + 3 ] = pElements[ 3 ];
                    val[ entry->second.m_Offset + 4 ] = pElements[ 4 ];
                    val[ entry->second.m_Offset + 5 ] = pElements[ 5 ];
                    val[ entry->second.m_Offset + 6 ] = pElements[ 6 ];
                    val[ entry->second.m_Offset + 7 ] = pElements[ 7 ];
                    val[ entry->second.m_Offset + 8 ] = pElements[ 8 ];
                    val[ entry->second.m_Offset + 9 ] = pElements[ 9 ];
                    val[ entry->second.m_Offset + 10 ] = pElements[ 10 ];
                    val[ entry->second.m_Offset + 11 ] = pElements[ 11 ];
                    val[ entry->second.m_Offset + 12 ] = pElements[ 12 ];
                    val[ entry->second.m_Offset + 13 ] = pElements[ 13 ];
                    val[ entry->second.m_Offset + 14 ] = pElements[ 14 ];
                    val[ entry->second.m_Offset + 15 ] = pElements[ 15 ];
                    break;
                }
            default:
                // left blank to avoid compiler warnings about unhandled
                //  types
                break;
            }
        }
    }

    // Sort the color/opacity into the visible point list
    std::vector<SqImageSample>& aValues = pie2->Values( m, n );
    int i = 0;
    int c = aValues.size();
    if ( c > 0 && aValues[ 0 ].Depth() < ImageVal.Depth() )
    {
        SqImageSample * p = &aValues[ 0 ];
        while ( i < c && p[ i ].Depth() < ImageVal.Depth() ) i++;
        // If it is exactly the same, chances are we've hit a MPG grid line.
        if ( i < c && p[ i ].Depth() == ImageVal.Depth() )
        {
            p[ i ].m_Data = ( p[ i ].m_Data + val ) * 0.5f;
            return ;
        }
    }

    // Update max depth values
    if ( !( DisplayMode() & ModeZ ) && Occludes )
    {
        CqOcclusionBox::MarkForUpdate( pie2->OcclusionBoxId() );
        pie2->MarkForZUpdate();
    }


    ImageVal.m_pCSGNode = pMPG->pGrid() ->pCSGNode();

    ImageVal.m_flags = 0;
    if ( Occludes )
    {
        ImageVal.m_flags |= SqImageSample::Flag_Occludes;
    }
    if( m_CurrentMpgSampleInfo.m_IsMatte )
    {
        ImageVal.m_flags |= SqImageSample::Flag_Matte;
    }

    aValues.insert( aValues.begin() + i, ImageVal );
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
 
 * \param xmin Integer minimum extend of the image part being rendered, takes into account buckets and clipping.
 * \param xmax Integer maximum extend of the image part being rendered, takes into account buckets and clipping.
 * \param ymin Integer minimum extend of the image part being rendered, takes into account buckets and clipping.
 * \param ymax Integer maximum extend of the image part being rendered, takes into account buckets and clipping.
 */

void CqImageBuffer::RenderSurfaces( long xmin, long xmax, long ymin, long ymax )
{
    int counter = 0;
    int MaxEyeSplits = 10;
    TqBool bIsEmpty = IsCurrentBucketEmpty();

    // Render any waiting micro polygon grids.
    QGetRenderContext() ->Stats().RenderMPGsTimer().Start();
    RenderMPGs( xmin, xmax, ymin, ymax );
    QGetRenderContext() ->Stats().RenderMPGsTimer().Stop();

    CqBucket& Bucket = CurrentBucket();

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
                if ( !bIsEmpty && pSurface->fCachedBound() )
                    fCull = OcclusionCullSurface( pSurface );
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
                        //CqAttributeError( ErrorID_OcclusionMaxEyeSplits, Severity_Normal, "Geometry gets culled too many times", pSurface->pAttributes(), TqTrue );
                        CqString objname( "unnamed" );
                        const CqString* pattrName = pSurface ->pAttributes() ->GetStringAttribute( "identifier", "name" );
                        if ( pattrName != 0 ) objname = pattrName[ 0 ];
                        std::cerr << warning << "Primitive \"" << objname.c_str() << "\" gets culled too many times" << std::endl;

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
                ADDREF( pGrid );
                // Only shade in all cases since the Displacement could be called in the shadow map creation too.
				pGrid->Shade();
                pGrid->TransferOutputVariables();

                if ( pGrid->vfCulled() == TqFalse )
                {
                    // Only project micropolygon not culled
                    Bucket.AddGrid( pGrid );
                    // Render any waiting micro polygon grids.
                    QGetRenderContext() ->Stats().RenderMPGsTimer().Start();
                    RenderMPGs( xmin, xmax, ymin, ymax );
                    QGetRenderContext() ->Stats().RenderMPGsTimer().Stop();
                }
                RELEASEREF( pGrid );
            }
        }
        // The surface is not small enough, so split it...
        else if ( !pSurface->fDiscard() )
        {
            std::vector<CqBasicSurface*> aSplits;
            // Decrease the total gprim count since this gprim is replaced by other gprims
            STATS_DEC( GPR_created_total );
            // Split it
            QGetRenderContext() ->Stats().SplitsTimer().Start();
            TqInt cSplits = pSurface->Split( aSplits );
            TqInt i;
            for ( i = 0; i < cSplits; i++ )
                PostSurface( aSplits[ i ] );

            QGetRenderContext() ->Stats().SplitsTimer().Stop();
        }

        pSurface->RenderComplete();
        pSurface = Bucket.pTopSurface();
        // Render any waiting micro polygon grids.
        QGetRenderContext() ->Stats().RenderMPGsTimer().Start();
        RenderMPGs( xmin, xmax, ymin, ymax );
        QGetRenderContext() ->Stats().RenderMPGsTimer().Stop();

        QGetRenderContext() ->Stats().OcclusionCullTimer().Start();
        // Update our occlusion hierarchy after each grid that gets drawn.
        if ( !bIsEmpty )
            CqOcclusionBox::Update();
        QGetRenderContext() ->Stats().OcclusionCullTimer().Stop();
    }

    // Now combine the colors at each pixel sample for any micropolygons rendered to that pixel.
    if ( m_fQuit ) return ;

    if(!bIsEmpty)
    {
        QGetRenderContext() ->Stats().MakeCombine().Start();
        CqBucket::CombineElements();
        QGetRenderContext() ->Stats().MakeCombine().Stop();
    }

    QGetRenderContext() ->Stats().MakeFilterBucket().Start();

    TqBool fImager = ( QGetRenderContext() ->optCurrent().GetStringOption( "System", "Imager" ) [ 0 ] != "null" );

    if (fImager)
        bIsEmpty = TqFalse;

    Bucket.FilterBucket(bIsEmpty);
    if(!bIsEmpty)
    {
        Bucket.ExposeBucket();
    }
    Bucket.QuantizeBucket();

    QGetRenderContext() ->Stats().MakeFilterBucket().Stop();

    BucketComplete();
    QGetRenderContext() ->Stats().MakeDisplayBucket().Start();
    QGetRenderContext() ->pDDmanager() ->DisplayBucket( &CurrentBucket() );
    QGetRenderContext() ->Stats().MakeDisplayBucket().Stop();
}

//----------------------------------------------------------------------
/** Return if a certain bucket is completely empty.
 
    True or False.
 
     It is empty only when this bucket doesn't contain any surface, 
	 micropolygon or grids.
 */
TqBool CqImageBuffer::IsCurrentBucketEmpty()
{
    TqBool retval = TqFalse;

    CqBucket& Bucket = CurrentBucket();

    if ( ( !Bucket.pTopSurface() ) &&
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
        }
        if ( bucketmodulo <= 0 ) bucketmodulo = m_cXBuckets;
    }



    // Render the surface at the front of the list.
    m_fDone = TqFalse;

    CqVector2D bHalf = CqVector2D( m_FilterXWidth / 2, m_FilterYWidth / 2 );

    // Setup the hierarchy of boxes for occlusion culling
    CqOcclusionBox::CreateHierarchy( m_XBucketSize, m_YBucketSize, m_FilterXWidth, m_FilterYWidth );

    RtProgressFunc pProgressHandler = NULL;
    pProgressHandler = QGetRenderContext()->pProgressHandler();

    do
    {
        TqBool bIsEmpty = IsCurrentBucketEmpty();
        QGetRenderContext() ->Stats().Others().Start();
        // Prepare the bucket.
        CqVector2D bPos = BucketPosition();
        CqVector2D bSize = BucketSize();
        // TODO: fix non jittered bucket initialisation.
        // Warning Jitter must be True is all cases; the InitialiseBucket when it is not in jittering mode
        // doesn't initialise correctly so later we have problem in the FilterBucket()
        CqBucket::InitialiseBucket( static_cast<TqInt>( bPos.x() ), static_cast<TqInt>( bPos.y() ), static_cast<TqInt>( bSize.x() ), static_cast<TqInt>( bSize.y() ), m_FilterXWidth, m_FilterYWidth, m_PixelXSamples, m_PixelYSamples, true );
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

        if ( !bIsEmpty )
        {
            QGetRenderContext() ->Stats().OcclusionCullTimer().Start();
            CqOcclusionBox::SetupHierarchy( &CurrentBucket(), xmin, ymin, xmax, ymax );
            QGetRenderContext() ->Stats().OcclusionCullTimer().Stop();
        }



        if ( pProgressHandler )
        {
            // Inform the status class how far we have got, and update UI.
            float Complete = ( float ) ( cXBuckets() * cYBuckets() );
            TqInt iBucket = ( CurrentBucketRow() * cXBuckets() ) + CurrentBucketCol();
            Complete /= iBucket;
            Complete = 100.0f / Complete;
            QGetRenderContext() ->Stats().SetComplete( Complete );
            ( *pProgressHandler ) ( Complete, QGetRenderContext() ->CurrentFrame() );
        }


        RenderSurfaces( xmin, xmax, ymin, ymax );
        if ( m_fQuit )
        {
            m_fDone = TqTrue;
            return ;
        }
#ifdef WIN32
        //		if ( !( iBucket % bucketmodulo ) )
        //			SetProcessWorkingSetSize( GetCurrentProcess(), 0xffffffff, 0xffffffff );
#endif
#if defined(REQUIRED)
        TqInt iB2, NumGrids = 0, NumPolys = 0;
        for ( iB2 = 0; iB2 < nBuckets; iB2++ )
        {
            NumGrids += m_aBuckets[ iB2 ].aGrids().size();
            NumPolys += m_aBuckets[ iB2 ].aMPGs().size();
        }
#endif
        CurrentBucket().SetProcessed();
    } while( NextBucket() );

    ImageComplete();

    CqBucket::ShutdownBucket();

    CqOcclusionBox::DeleteHierarchy();
    // Pass >100 through to progress to allow it to indicate completion.

    if ( pProgressHandler )
    {
        ( *pProgressHandler ) ( 100.0f, QGetRenderContext() ->CurrentFrame() );
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


