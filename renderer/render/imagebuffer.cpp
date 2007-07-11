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

#include	"MultiTimer.h"

#include	"aqsis.h"

#ifdef WIN32
#include    <windows.h>
#endif
#include	<math.h>

#include	"stats.h"
#include	"options.h"
#include	"renderer.h"
#include	"surface.h"
#include	"imagebuffer.h"
#include	"bucketprocessor.h"


START_NAMESPACE( Aqsis )

static TqInt bucketmodulo = -1;
static TqInt bucketdirection = -1;

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
	if ( vecA.x() > m_XBucketSize )
		vecA.x( m_XBucketSize );
	vecA.y( m_iYRes - vecA.y() );
	if ( vecA.y() > m_YBucketSize )
		vecA.y( m_YBucketSize );

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
	const TqInt* poptBucketSize = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "limits", "bucketsize" );
	if ( poptBucketSize != 0 )
	{
		m_XBucketSize = poptBucketSize[ 0 ];
		m_YBucketSize = poptBucketSize[ 1 ];
	}
	/* add artificially a border around based on the current filterwidth so the diced primitives
	    * may fit better within a bucket */
	m_FilterXWidth = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "FilterWidth" ) [ 0 ];
	m_FilterYWidth = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "FilterWidth" ) [ 1 ];

	m_iXRes = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "System", "Resolution" ) [ 0 ];
	m_iYRes = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "System", "Resolution" ) [ 1 ];
	m_CropWindowXMin = static_cast<TqInt>( CLAMP( CEIL( m_iXRes * QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "CropWindow" ) [ 0 ] ), 0, m_iXRes ) );
	m_CropWindowXMax = static_cast<TqInt>( CLAMP( CEIL( m_iXRes * QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "CropWindow" ) [ 1 ] ), 0, m_iXRes ) );
	m_CropWindowYMin = static_cast<TqInt>( CLAMP( CEIL( m_iYRes * QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "CropWindow" ) [ 2 ] ), 0, m_iYRes ) );
	m_CropWindowYMax = static_cast<TqInt>( CLAMP( CEIL( m_iYRes * QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "CropWindow" ) [ 3 ] ), 0, m_iYRes ) );
	m_cXBuckets = ( ( m_iXRes + ( m_XBucketSize-1 ) ) / m_XBucketSize );
	m_cYBuckets = ( ( m_iYRes + ( m_YBucketSize-1 ) ) / m_YBucketSize );
	m_PixelXSamples = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "System", "PixelSamples" ) [ 0 ];
	m_PixelYSamples = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "System", "PixelSamples" ) [ 1 ];

	m_ClippingNear = static_cast<TqFloat>( QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Clipping" ) [ 0 ] );
	m_ClippingFar = static_cast<TqFloat>( QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Clipping" ) [ 1 ] );
	m_DisplayMode = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "System", "DisplayMode" ) [ 0 ];

	m_MaxEyeSplits = 10;
	const TqInt* poptEyeSplits = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "limits", "eyesplits" );
	if ( poptEyeSplits != 0 )
		m_MaxEyeSplits = poptEyeSplits[ 0 ];

	CqBucket::SetImageBuffer(this);
	m_Buckets.resize( m_cYBuckets );
	std::vector<std::vector<CqBucket> >::iterator i;
	for( i = m_Buckets.begin(); i!=m_Buckets.end(); i++)
	{
		i->resize( m_cXBuckets );
		std::vector<CqBucket>::iterator b;
		for( b = i->begin(); b!=i->end(); b++)
			b->SetProcessed( false );
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
 * \param pSurface Pointer to the CqSurface derived class being processed.
 * \return Boolean indicating that the GPrim can be culled.
 
  \bug If the gprim spans the eye plane the bound is not transformed into raster
   space (how could it anyway), but PostSurface() relies on this behaviour and
   inserts EVERY gprim into buckets (using a bound that is still in camera space).
 */

bool CqImageBuffer::CullSurface( CqBound& Bound, const boost::shared_ptr<CqSurface>& pSurface )
{
	// If the primitive is completely outside of the hither-yon z range, cull it.
	if ( Bound.vecMin().z() >= QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Clipping" ) [ 1 ] ||
	        Bound.vecMax().z() <= QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Clipping" ) [ 0 ] )
		return ( true );

	if(QGetRenderContext()->clippingVolume().whereIs(Bound) == CqBound::Side_Outside)
	{
		return(true);
	}

	// If the primitive spans the epsilon plane and the hither plane and can be split,
	if ( Bound.vecMin().z() <= 0.0f && Bound.vecMax().z() > FLT_EPSILON )
	{
		// Mark the primitive as not dicable.
		pSurface->ForceUndiceable();

		if ( pSurface->EyeSplitCount() > m_MaxEyeSplits )
		{
			CqString objname( "unnamed" );
			const CqString* pattrName = pSurface->pAttributes() ->GetStringAttribute( "identifier", "name" );
			if ( pattrName != 0 )
				objname = pattrName[ 0 ];
			Aqsis::log() << warning << "Max eyesplits for object \"" << objname.c_str() << "\" exceeded" << std::endl;
			return( true );
		}
		return ( false );
	}

	TqFloat minz = Bound.vecMin().z();
	TqFloat maxz = Bound.vecMax().z();


	// Convert the bounds to raster space.
	Bound.Transform( QGetRenderContext() ->matSpaceToSpace( "camera", "raster", NULL, NULL, QGetRenderContext()->Time() ) );

	// Take into account depth-of-field
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

	// And expand to account for filter size.
	Bound.vecMin().x( Bound.vecMin().x() - m_FilterXWidth / 2.0f );
	Bound.vecMin().y( Bound.vecMin().y() - m_FilterYWidth / 2.0f );
	Bound.vecMax().x( Bound.vecMax().x() + m_FilterXWidth / 2.0f );
	Bound.vecMax().y( Bound.vecMax().y() + m_FilterYWidth / 2.0f );

	// If the bounds are completely outside the viewing frustum, cull the primitive.
	if ( Bound.vecMin().x() > CropWindowXMax() ||
	        Bound.vecMin().y() > CropWindowYMax() ||
	        Bound.vecMax().x() < CropWindowXMin() ||
	        Bound.vecMax().y() < CropWindowYMin() )
		return ( true );

	// Restore Z-Values to camera space.
	Bound.vecMin().z( minz );
	Bound.vecMax().z( maxz );

	// Cache the Bound.
	pSurface->CacheRasterBound( Bound );
	return ( false );
}


//----------------------------------------------------------------------
/** Add a new surface to the front of the list of waiting ones.
 * \param pSurface A pointer to a CqSurface derived class, surface should at this point be in camera space.
 */

void CqImageBuffer::PostSurface( const boost::shared_ptr<CqSurface>& pSurface )
{
	// Count the number of total gprims
	STATS_INC( GPR_created_total );


	// Bound the primitive in its current space (camera) space taking into account any motion specification.
	CqBound Bound( pSurface->Bound() );

	// Take into account the displacement bound extension.
	TqFloat db = 0.0f;
	CqString strCoordinateSystem( "object" );
	const TqFloat* pattrDispclacementBound = pSurface->pAttributes() ->GetFloatAttribute( "displacementbound", "sphere" );
	const CqString* pattrCoordinateSystem = pSurface->pAttributes() ->GetStringAttribute( "displacementbound", "coordinatesystem" );
	if ( pattrDispclacementBound != 0 )
		db = pattrDispclacementBound[ 0 ];
	if ( pattrCoordinateSystem != 0 )
		strCoordinateSystem = pattrCoordinateSystem[ 0 ];

	if ( db != 0.0f )
	{
		CqVector3D	vecDB( db, 0, 0 );
		const IqTransform* transShaderToWorld = NULL;
		// Default "shader" space to the displacement shader, unless there isn't one, in which
		// case use the surface shader.
		if ( pSurface->pAttributes() ->pshadDisplacement(QGetRenderContextI()->Time()) )
			transShaderToWorld = pSurface->pAttributes() ->pshadDisplacement(QGetRenderContextI()->Time()) ->getTransform();
		else if ( pSurface->pAttributes() ->pshadSurface(QGetRenderContextI()->Time()) )
			transShaderToWorld = pSurface->pAttributes() ->pshadSurface(QGetRenderContextI()->Time()) ->getTransform();
		vecDB = QGetRenderContext() ->matVSpaceToSpace( strCoordinateSystem.c_str(), "camera", transShaderToWorld, pSurface->pTransform().get(), QGetRenderContextI()->Time() ) * vecDB;
		db = vecDB.Magnitude();

		Bound.vecMax() += db;
		Bound.vecMin() -= db;
	}

	// Check if the surface can be culled. (also adjusts for DOF and converts Bound to raster space).
	if ( CullSurface( Bound, pSurface ) )
	{
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
		if ( Bound.vecMin().x() < 0 )
			Bound.vecMin().x( 0.0f );
		if ( Bound.vecMin().y() < 0 )
			Bound.vecMin().y( 0.0f );

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
}

//----------------------------------------------------------------------
/** Test if this surface can be occlusion culled. If it can then
 * transfer surface to the next bucket it covers, or delete it if it
 * covers no more.
 * \param pSurface A pointer to a CqSurface derived class.
 * \return Boolean indicating that the GPrim has been culled.
*/

bool CqImageBuffer::OcclusionCullSurface( const CqBucketProcessor& bucketProcessor, const boost::shared_ptr<CqSurface>& pSurface )
{
	const CqBound RasterBound( pSurface->GetCachedRasterBound() );

	if ( bucketProcessor.canCull( &RasterBound ) )
	{
		// pSurface is behind everying in this bucket but it may be
		// visible in other buckets it overlaps.
		// bucket to the right
		TqInt nextBucket = CurrentBucketCol() + 1;
		CqVector2D pos = BucketPosition( nextBucket, CurrentBucketRow() );
		if ( ( nextBucket < cXBuckets() ) &&
		        ( RasterBound.vecMax().x() >= pos.x() ) )
		{
			Bucket( nextBucket, CurrentBucketRow() ).AddGPrim( pSurface );
			return true;
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
			Bucket( nextBucketX, nextBucket ).AddGPrim( pSurface );
			return true;
		}

		// Bound covers no more buckets therefore we can delete the surface completely.
		CqString objname( "unnamed" );
		const CqString* pattrName = pSurface->pAttributes() ->GetStringAttribute( "identifier", "name" );
		if( pattrName )
			objname = *pattrName;
		Aqsis::log() << info << "GPrim: \"" << objname << "\" occlusion culled" << std::endl;
		STATS_INC( GPR_culled );
		return true;
	}
	else
	{
		return false;
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

	if ( B.vecMax().x() < m_CropWindowXMin - m_FilterXWidth / 2.0f || B.vecMax().y() < m_CropWindowYMin - m_FilterYWidth / 2.0f ||
	        B.vecMin().x() > m_CropWindowXMax + m_FilterXWidth / 2.0f || B.vecMin().y() > m_CropWindowYMax + m_FilterYWidth / 2.0f )
	{
		return ;
	}

	////////// Dump the micro polygon into a dump file //////////
#if ENABLE_MPDUMP
	if(m_mpdump.IsOpen())
		m_mpdump.dump(*pmpgNew);
#endif
	/////////////////////////////////////////////////////////////


	// Find out the minimum bucket touched by the micropoly bound.

	B.vecMin().x( B.vecMin().x() - m_FilterXWidth / 2.0f );
	B.vecMin().y( B.vecMin().y() - m_FilterYWidth / 2.0f );
	B.vecMax().x( B.vecMax().x() + m_FilterXWidth / 2.0f );
	B.vecMax().y( B.vecMax().y() + m_FilterYWidth / 2.0f );

	TqInt iXBa = static_cast<TqInt>( B.vecMin().x() / m_XBucketSize );
	TqInt iYBa = static_cast<TqInt>( B.vecMin().y() / m_YBucketSize );
	TqInt iXBb = static_cast<TqInt>( B.vecMax().x() / m_XBucketSize );
	TqInt iYBb = static_cast<TqInt>( B.vecMax().y() / m_YBucketSize );

	if ( ( iXBb < 0 ) || ( iYBb < 0 ) ||
	        ( iXBa >= m_cXBuckets ) || ( iYBa >= m_cYBuckets ) )
	{
		return ;
	}

	// Use sane values -- otherwise sometimes crashes, probably
	// due to precision problems
	if ( iXBa < 0 )	iXBa = 0;
	if ( iYBa < 0 )	iYBa = 0;
	if ( iXBb >= m_cXBuckets ) iXBb = m_cXBuckets - 1;
	if ( iYBb >= m_cYBuckets ) iYBb = m_cYBuckets - 1;

	// Add the MP to all the Buckets that it touches
	for ( TqInt i = iXBa; i <= iXBb; i++ )
	{
		for ( TqInt j = iYBa; j <= iYBb; j++ )
		{
			CqBucket* bucket = &Bucket( i, j );
			if ( bucket->IsProcessed() )
			{
				Aqsis::log() << warning << "Bucket already processed but a new MP touches it" << std::endl;
			}
			else
			{
				ADDREF( pmpgNew );
				bucket->AddMPG( pmpgNew );
			}
		}
	}
}

//----------------------------------------------------------------------
/** Render the given Surface
 
    This method loops through all the gprims stored in the specified bucket
    and checks if the gprim can be diced and turned into a grid of micro
    polygons or if it is still too large and has to be split (this check
    is done in CqSurface::Diceable()).
 
    The dicing is done by the gprim in CqSurface::Dice(). After that
    the entire grid is shaded by calling CqMicroPolyGridBase::Shade().
    The shaded grid is then stored in the current bucket and will eventually
    be further processed by RenderGrids().
 
    If the gprim could not yet be diced, it is split into a number of
    smaller gprims (CqSurface::Split()) which are again assigned to
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

void CqImageBuffer::RenderSurface( boost::shared_ptr<CqSurface>& pSurface )
{
	// If the epsilon check has deemed this surface to be undiceable, don't bother asking.
	bool fDiceable = false;
	{
		TIME_SCOPE("Diceable check");
		fDiceable = pSurface->Diceable();
	}

	// Dice & shade the surface if it's small enough...
	if ( fDiceable )
	{
		CqMicroPolyGridBase* pGrid = 0;
		{
			TIME_SCOPE("Dicing");
			pGrid = pSurface->Dice();
		}

		if ( NULL != pGrid )
		{
			ADDREF( pGrid );
			// Only shade in all cases since the Displacement could be called in the shadow map creation too.
			pGrid->Shade();
			pGrid->TransferOutputVariables();

			if ( pGrid->vfCulled() == false )
			{
				// Split any grids in this bucket waiting to be processed.
				std::vector<CqMicroPolygon*> newMPs;
				pGrid->Split( newMPs );
				for ( std::vector<CqMicroPolygon*>::iterator it = newMPs.begin();
				      it != newMPs.end();
				      it++ )
				{
					AddMPG( *it );
				}
			}
			RELEASEREF( pGrid );
		}
	}
	// The surface is not small enough, so split it...
	else if ( !pSurface->fDiscard() )
	{
		// Decrease the total gprim count since this gprim is replaced by other gprims
		STATS_DEC( GPR_created_total );

		// Split it
		{
			TIME_SCOPE("Splits");
			std::vector<boost::shared_ptr<CqSurface> > aSplits;
			TqInt cSplits = pSurface->Split( aSplits );
			for ( TqInt i = 0; i < cSplits; i++ )
				PostSurface( aSplits[ i ] );

			/// \debug:
			/*
			  if(pSurface->IsUndiceable())
			  {
			  CqBound Bound( pSurface->Bound() );
			  std::cout << pSurface << " - " << Bound.vecMin().z() << " --> " << Bound.vecMax().z() << std::endl;
			  for ( i = 0; i < cSplits; i++ )
			  {
			  CqBound SBound( aSplits[i]->Bound() );
			  std::cout << "\t" << aSplits[i] << " - " << SBound.vecMin().z() << " --> " << SBound.vecMax().z() << std::endl;
			  }
			  }
			*/			
		}
	}
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
	////////// Create a new dump file  //////////
#if ENABLE_MPDUMP
	const TqInt* poptDump = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "mpdump", "enabled" );
	if(poptDump && (*poptDump != 0))
	{
		m_mpdump.open();
		m_mpdump.dumpImageInfo();
	}
#endif
	/////////////////////////////////////////////

	STATS_SETF( MPG_min_area, FLT_MAX );
	STATS_SETF( MPG_max_area, FLT_MIN );

	if ( bucketmodulo == -1 )
	{
		// Small change which allows full control of virtual memory on NT swapping
		bucketmodulo = m_cXBuckets;
		TqInt *poptModulo = ( TqInt * ) QGetRenderContext() ->poptCurrent()->GetIntegerOption( "limits", "bucketmodulo" );
		if ( poptModulo != 0 )
		{
			bucketmodulo = poptModulo[ 0 ];
		}
		if ( bucketmodulo <= 0 )
			bucketmodulo = m_cXBuckets;
	}

	bool fImager = false;
	const CqString* systemOptions;
	if( ( systemOptions = QGetRenderContext() ->poptCurrent()->GetStringOption( "System", "Imager" ) ) != 0 )
		if( systemOptions[ 0 ].compare("null") != 0 )
			fImager = true;

	const CqString* pstrDepthFilter = QGetRenderContext() ->poptCurrent()->GetStringOption( "Hider", "depthfilter" );
	const CqColor* pzThreshold = QGetRenderContext() ->poptCurrent()->GetColorOption( "limits", "zthreshold" );
	CqColor zThreshold(1.0f, 1.0f, 1.0f);	// Default threshold of 1,1,1 means that any objects that are partially transparent won't appear in shadow maps.
	if(NULL != pzThreshold)
		zThreshold = pzThreshold[0];
	enum EqFilterDepth depthfilter = Filter_Min;
	if ( NULL != pstrDepthFilter )
	{
		Aqsis::log() << debug << "Depth filter = " << pstrDepthFilter[0].c_str() << std::endl;
		if( !pstrDepthFilter[ 0 ].compare( "min" ) )
			depthfilter = Filter_Min;
		else if ( !pstrDepthFilter[ 0 ].compare( "midpoint" ) )
			depthfilter = Filter_MidPoint;
		else if ( !pstrDepthFilter[ 0 ].compare( "max" ) )
			depthfilter = Filter_Max;
		else if ( !pstrDepthFilter[ 0 ].compare( "average" ) )
			depthfilter = Filter_Average;
		else
			Aqsis::log() << warning << "Invalid depthfilter \"" << pstrDepthFilter[ 0 ].c_str() << "\", depthfilter set to \"min\"" << std::endl;
	}

	// Render the surface at the front of the list.
	m_fDone = false;

	CqVector2D bHalf = CqVector2D( FLOOR(m_FilterXWidth / 2.0f), FLOOR(m_FilterYWidth / 2.0f) );

	RtProgressFunc pProgressHandler = NULL;
	pProgressHandler = QGetRenderContext()->pProgressHandler();

	const CqString* pstrBucketOrder = QGetRenderContext() ->poptCurrent()->GetStringOption( "render", "bucketorder" );
	enum EqBucketOrder order = Bucket_Horizontal;
	if ( NULL != pstrBucketOrder )
	{
	if( !pstrBucketOrder[ 0 ].compare( "vertical" ) )
		order = Bucket_Vertical;
	else if ( !pstrBucketOrder[ 0 ].compare( "horizontal" ) )
		order = Bucket_Horizontal;
	else {
		Aqsis::log() << warning << "Not supported \"" << pstrBucketOrder[ 0 ] << "\" " << std::endl;
	}
#ifdef NOTREADY
	else if ( !pstrBucketOrder[ 0 ].compare( "zigzag" ) )
		order = Bucket_ZigZag;
	else if ( !pstrBucketOrder[ 0 ].compare( "circle" ) )
		order = Bucket_Circle;
	else if ( !pstrBucketOrder[ 0 ].compare( "random" ) )
		order = Bucket_Random;
#endif
	}

	// A counter for the number of processed buckets (used for progress reporting)
	TqInt iBucket = 0;

	CqBucketProcessor bucketProcessor;

	// Iterate over all buckets...
	do
	{
		bucketProcessor.setBucket(&CurrentBucket());

		bool bIsEmpty = CurrentBucket().IsEmpty();
		if (fImager)
			bIsEmpty = false;

		// Prepare the bucket.
		CqVector2D bPos = BucketPosition();
		CqVector2D bSize = BucketSize();

		{
			TIME_SCOPE("Prepare bucket")
			CurrentBucket().PrepareBucket( static_cast<TqInt>( bPos.x() ), static_cast<TqInt>( bPos.y() ), static_cast<TqInt>( bSize.x() ), static_cast<TqInt>( bSize.y() ), true, bIsEmpty );
			CurrentBucket().InitialiseFilterValues();
		}

		if ( !bIsEmpty )
		{
			TIME_SCOPE("Occlusion culling");
			bucketProcessor.prepareOcclusionData();
		}

		////////// Dump the pixel sample positions into a dump file //////////
#if ENABLE_MPDUMP
		if(m_mpdump.IsOpen())
			m_mpdump.dumpPixelSamples();
#endif
		/////////////////////////////////////////////////////////////////////////

		// Set up some bounds for the bucket.
		CqVector2D vecMin = bPos;
		CqVector2D vecMax = bPos + bSize;
		vecMin -= bHalf;
		vecMax += bHalf;

		long xmin = static_cast<long>( vecMin.x() );
		long ymin = static_cast<long>( vecMin.y() );
		long xmax = static_cast<long>( vecMax.x() );
		long ymax = static_cast<long>( vecMax.y() );

		if ( xmin < CropWindowXMin() - m_FilterXWidth / 2 )
			xmin = static_cast<long>(CropWindowXMin() - m_FilterXWidth / 2.0f);
		if ( ymin < CropWindowYMin() - m_FilterYWidth / 2 )
			ymin = static_cast<long>(CropWindowYMin() - m_FilterYWidth / 2.0f);
		if ( xmax > CropWindowXMax() + m_FilterXWidth / 2 )
			xmax = static_cast<long>(CropWindowXMax() + m_FilterXWidth / 2.0f);
		if ( ymax > CropWindowYMax() + m_FilterYWidth / 2 )
			ymax = static_cast<long>(CropWindowYMax() + m_FilterYWidth / 2.0f);


		if ( pProgressHandler )
		{
			// Inform the status class how far we have got, and update UI.
			float Complete = ( float ) ( cXBuckets() * cYBuckets() );
			Complete = 100.0f * iBucket / Complete;
			QGetRenderContext() ->Stats().SetComplete( Complete );
			( *pProgressHandler ) ( Complete, QGetRenderContext() ->CurrentFrame() );
		}

		// Render any waiting subsurfaces.
		while( !CurrentBucket().IsEmpty() && !m_fQuit )
		{
			boost::shared_ptr<CqSurface> pSurface = CurrentBucket().pTopSurface();
			if(pSurface)
			{
				// Cull surface if it's hidden
				if ( !( DisplayMode() & ModeZ ) && !pSurface->pCSGNode() )
				{
					TIME_SCOPE("Occlusion culling");
					if ( !bIsEmpty &&
					     pSurface->fCachedBound() &&
					     OcclusionCullSurface( bucketProcessor, pSurface ) )
					{
						// Advance to next surface
						CurrentBucket().popSurface();
						pSurface = CurrentBucket().pTopSurface();
						continue;
					}
				}

				RenderSurface( pSurface );
				
				// Advance to next surface
				CurrentBucket().popSurface();
				pSurface = CurrentBucket().pTopSurface();
			}

			// Render any waiting micro polygons.
			{
				TIME_SCOPE("Render MPs");
				bucketProcessor.process( xmin, xmax, ymin, ymax, ClippingFar(), ClippingNear() );
			}
		}

		if ( m_fQuit )
		{
			m_fDone = true;
			return ;
		}

		// Now combine the colors at each pixel sample for any micropolygons rendered to that pixel.
		if (!bIsEmpty)
		{
			TIME_SCOPE("Combine");
			CurrentBucket().CombineElements(depthfilter, zThreshold);
		}

		TIMER_START("Filter");
		CurrentBucket().FilterBucket(bIsEmpty, fImager);
		if (!bIsEmpty)
		{
			CurrentBucket().ExposeBucket();
			// \note: Used to quantize here too, but not any more, as it is handled by
			//	  ddmanager in a specific way for each display.
		}
		TIMER_STOP("Filter");

		BucketComplete();
		{
			TIME_SCOPE("Display bucket");
			QGetRenderContext() ->pDDmanager() ->DisplayBucket( &CurrentBucket() );
		}

#ifdef WIN32
		if ( !( iBucket % bucketmodulo ) )
			SetProcessWorkingSetSize( GetCurrentProcess(), 0xffffffff, 0xffffffff );
#endif

		bucketProcessor.finishProcessing();
		bucketProcessor.reset();

		// Increase the bucket counter...
		iBucket += 1;
	} while( NextBucket(order) );

	ImageComplete();
	CqBucket::ShutdownBucket();

	// Pass >100 through to progress to allow it to indicate completion.

	if ( pProgressHandler )
	{
		( *pProgressHandler ) ( 100.0f, QGetRenderContext() ->CurrentFrame() );
	}
	m_fDone = true;
}


//----------------------------------------------------------------------
/** Stop rendering.
 */

void CqImageBuffer::Quit()
{
	m_fQuit = true;
}

//----------------------------------------------------------------------
/** Move to the next bucket to process.

  Computes the next bucket based on the "render" "bucketorder" given.

  \return True if there is still an unprocessed bucket left, otherwise False.
 */
bool CqImageBuffer::NextBucket(EqBucketOrder order)
{

	TqInt cnt = 0;
	TqInt total = ((m_cXBuckets - 1) * (m_cYBuckets - 1));


	for (TqInt i =0; i < m_cYBuckets - 1 ; i++)
		for (TqInt j = 0; j < m_cXBuckets - 1; j++)
			if (Bucket(j, i).IsProcessed() )
				cnt ++;
         
	if ((order != Bucket_Vertical) &&
	        (order != Bucket_Horizontal) &&
	        (cnt == total))
		return false;

	switch (order)
	{

		case Bucket_Random :
		{
			CqRandom rg;
			do
			{
				m_CurrentBucketCol = (TqInt) rg.RandomFloat(m_cXBuckets);
				m_CurrentBucketRow = (TqInt) rg.RandomFloat(m_cYBuckets);
				m_CurrentBucketCol = CLAMP(m_CurrentBucketCol, 0, m_cXBuckets - 1);
				m_CurrentBucketRow = CLAMP(m_CurrentBucketRow, 0, m_cYBuckets - 1);
			}
			while ( Bucket(m_CurrentBucketCol, m_CurrentBucketRow).IsProcessed() );

		}
		break;

		case Bucket_Circle :
		{
			static TqInt radius = 0;
			static float theta = 0.0f;

			if ((m_CurrentBucketCol == m_CurrentBucketRow) &&
				(m_CurrentBucketRow == 0))
			{
				radius = 0;
				theta = 0.0f;
			}
			TqFloat r = (TqFloat) sqrt(static_cast<double>((m_cXBuckets * m_cXBuckets) + (m_cYBuckets * m_cYBuckets))) + 2;
			TqInt midx = m_cXBuckets/2;
			TqInt midy = m_cYBuckets/2;


			TqFloat delta = 0.01f;

			do
			{
				if (radius > r)
					break;
				m_CurrentBucketCol = midx + (TqInt) (radius * cos(theta));
				m_CurrentBucketRow = midy + (TqInt) (radius * sin(theta));

				theta += delta;
				if (theta > 6.28f)
				{
					theta = 0.0f;
					radius ++;
				}
				if (radius > r)
					break;

				m_CurrentBucketCol = CLAMP(m_CurrentBucketCol, 0, m_cXBuckets - 1);
				m_CurrentBucketRow = CLAMP(m_CurrentBucketRow, 0, m_cYBuckets - 1);
			}
			while (Bucket(m_CurrentBucketCol, m_CurrentBucketRow).IsProcessed());

			if (radius > r)
			{
				// Maybe a bucket was not done
				for (TqInt i =0; i < m_cYBuckets - 1 ; i++)
					for (TqInt j = 0; j < m_cXBuckets - 1; j++)
						if (!Bucket(j, i).IsProcessed() )
						{
							m_CurrentBucketCol = j;
							m_CurrentBucketRow = i;

						}
			}


		}
		break;

		case Bucket_ZigZag :
		{

			if (bucketdirection == 1)
				m_CurrentBucketCol++;
			else
				m_CurrentBucketCol--;

			if((bucketdirection == 1) && ( m_CurrentBucketCol >= m_cXBuckets ))
			{

				m_CurrentBucketCol = m_cXBuckets-1;
				m_CurrentBucketRow++;
				bucketdirection = -1;
				if( m_CurrentBucketRow >= m_cYBuckets )
					return( false );

			}
			else if((bucketdirection == -1) && ( m_CurrentBucketCol < 0 ))
			{

				m_CurrentBucketCol = 0;
				m_CurrentBucketRow++;
				bucketdirection = 1;

				if( m_CurrentBucketRow >= m_cYBuckets )
					return( false );

			}
		}
		break;
		case Bucket_Vertical :
		{

			m_CurrentBucketRow++;

			if( m_CurrentBucketRow >= m_cYBuckets )
			{

				m_CurrentBucketRow = 0;
				m_CurrentBucketCol++;

				if( m_CurrentBucketCol >= m_cXBuckets )
					return( false );

			}
		}
		break;
		default:
		case Bucket_Horizontal :
		{

			m_CurrentBucketCol++;

			if( m_CurrentBucketCol >= m_cXBuckets )
			{

				m_CurrentBucketCol = 0;
				m_CurrentBucketRow++;
				if( m_CurrentBucketRow >= m_cYBuckets )
					return( false );

			}
		}
		break;
	}
	return( true );
}

//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )


