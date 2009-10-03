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

#include	"imagebuffer.h"

#ifdef WIN32
#include    <windows.h>
#endif
#include	<math.h>

#include	<aqsis/math/math.h>
#include	"stats.h"
#include	"options.h"
#include	"renderer.h"
#include	"surface.h"
#include	"micropolygon.h"
#include	"bucketprocessor.h"
#include	"threadscheduler.h"
#include	"multijitter.h"
#include	"grid.h"


namespace Aqsis {

static TqInt bucketmodulo = -1;
//static TqInt bucketdirection = -1;

#define MULTIPROCESSING_NBUCKETS 1
/** Implementing a work unit for a boost::thread (the operator()()
 * method), that is, a piece of code that will run in parallel.
 */
class CqThreadProcessor
{
public:
	CqThreadProcessor(CqBucketProcessor* bucketProcessor) :
		m_bucketProcessor(bucketProcessor) { }
	void operator()()
	{
		// printf("%p > start\n", m_bucketProcessor);
		m_bucketProcessor->process();
		// printf("%p < stop\n", m_bucketProcessor);
	}

private:
	CqBucketProcessor* m_bucketProcessor;
};


/* mafm: implementation of a processor (work unit for a boost::thread)
 * where it takes buckets ready to process and does so continuously.
 * One simple addition would be to wait for new available buckets
 * which are provided dynamically.  This is not applicable at the
 * moment though, it would need RenderSurfaces() to process all of the
 * surfaces of the buckets before assigning them as ready; but at the
 * moment RenderSurfaces() can't run before assigning the bucket to
 * the bucket processor, and it doesn't seem possible to break this
 * interdependency.

boost::mutex displayMutex;

class CqThreadProcessor2
{
public:
	CqThreadProcessor2(const CqImageBuffer* imageBuffer,
			   std::vector<CqBucket*>& bucketList,
			   bool fImager,
			   const CqColor& zThreshold,
			   EqDepthFilter depthfilter,
			   const CqVector2D& bHalf) :
		m_imageBuffer(imageBuffer),
		m_bucketList(bucketList),
		m_fImager(fImager),
		m_zThreshold(zThreshold),
		m_depthfilter(depthfilter),
		m_bHalf(bHalf) { }

	void operator()()
	{
		while ( !m_bucketList.empty() )
		{
			CqBucket* bucket = m_bucketList.front();
			m_bucketList.erase( m_bucketList.begin() );

			m_bucketProcessor.setBucket( bucket );
			if ( m_fImager )
				m_bucketProcessor.setInitiallyEmpty(false);
			
			// Set up some bounds for the bucket.
			const CqVector2D bPos = m_imageBuffer->BucketPosition( bucket->getCol(),
									       bucket->getRow() );
			const CqVector2D bSize = m_imageBuffer->BucketSize( bucket->getCol(),
									    bucket->getRow() );
			const CqVector2D vecMin = bPos - m_bHalf;
			const CqVector2D vecMax = bPos + bSize + m_bHalf;

			TqInt xmin = static_cast<TqInt>( vecMin.x() );
			TqInt ymin = static_cast<TqInt>( vecMin.y() );
			TqInt xmax = static_cast<TqInt>( vecMax.x() );
			TqInt ymax = static_cast<TqInt>( vecMax.y() );
			if ( xmin < m_imageBuffer->CropWindowXMin() - m_imageBuffer->FilterXWidth() / 2 )
				xmin = static_cast<TqInt>(m_imageBuffer->CropWindowXMin() - m_imageBuffer->FilterXWidth() / 2.0f);
			if ( ymin < m_imageBuffer->CropWindowYMin() - m_imageBuffer->FilterYWidth() / 2 )
				ymin = static_cast<TqInt>(m_imageBuffer->CropWindowYMin() - m_imageBuffer->FilterYWidth() / 2.0f);
			if ( xmax > m_imageBuffer->CropWindowXMax() + m_imageBuffer->FilterXWidth() / 2 )
				xmax = static_cast<TqInt>(m_imageBuffer->CropWindowXMax() + m_imageBuffer->FilterXWidth() / 2.0f);
			if ( ymax > m_imageBuffer->CropWindowYMax() + m_imageBuffer->FilterYWidth() / 2 )
				ymax = static_cast<TqInt>(m_imageBuffer->CropWindowYMax() + m_imageBuffer->FilterYWidth() / 2.0f);

			m_bucketProcessor.preProcess( bPos, bSize,
						      m_imageBuffer->PixelXSamples(), m_imageBuffer->PixelYSamples(), m_imageBuffer->FilterXWidth(), m_imageBuffer->FilterYWidth(),
						      xmin, xmax, ymin, ymax,
						      m_imageBuffer->ClippingNear(), m_imageBuffer->ClippingFar() );
			m_bucketProcessor.process();
			m_bucketProcessor.postProcess( m_fImager, m_depthfilter, m_zThreshold );
			{
				TIME_SCOPE("Display bucket");
				boost::mutex::scoped_lock lock(displayMutex);
				QGetRenderContext() ->pDDmanager() ->DisplayBucket( bucket );
			}
			m_bucketProcessor.reset();
		}
	}

private:
	CqBucketProcessor m_bucketProcessor;
	const CqImageBuffer* m_imageBuffer;
	std::vector<CqBucket*> m_bucketList;
	bool m_fImager;
	CqColor m_zThreshold;
	EqDepthFilter m_depthfilter;
	CqVector2D m_bHalf;
};

*/


//----------------------------------------------------------------------
/** Destructor
 */

CqImageBuffer::~CqImageBuffer()
{
	DeleteImage();
}



//----------------------------------------------------------------------
/** Construct the image buffer to an initial state using the current options.
 */

void	CqImageBuffer::SetImage()
{
	DeleteImage();

	const CqRenderer& ctx = *QGetRenderContext();
	const IqOptions& opts = *ctx.poptCurrent();
	m_optCache.cacheOptions(opts);

	TqInt xRes = opts.GetIntegerOption("System", "Resolution")[0];
	TqInt yRes = opts.GetIntegerOption("System", "Resolution")[1];
	m_cXBuckets = (xRes-1)/m_optCache.xBucketSize + 1;
	m_cYBuckets = (yRes-1)/m_optCache.yBucketSize + 1;

	// The bucket region specifies the rectangle of buckets which actually need
	// to be rendered.  m_bucketRegion.xMax() is actually 1 _greater_ than the
	// maximum bucket coordinate.
	m_bucketRegion = CqRegion(
			ctx.cropWindowXMin()/m_optCache.xBucketSize,
			ctx.cropWindowYMin()/m_optCache.yBucketSize,
			(ctx.cropWindowXMax()-1)/m_optCache.xBucketSize + 1,
			(ctx.cropWindowYMax()-1)/m_optCache.yBucketSize + 1);

	TqInt row = 0;
	TqInt colPos = 0, rowPos = 0;
	m_Buckets.resize( m_cYBuckets );
	std::vector<std::vector<CqBucket> >::iterator i;
	for( i = m_Buckets.begin(); i!=m_Buckets.end(); i++)
	{
		TqInt rowSize = yRes - rowPos;
		if(rowSize > m_optCache.yBucketSize)
			rowSize = m_optCache.yBucketSize;
		i->resize( m_cXBuckets );
		std::vector<CqBucket>::iterator b;
		TqInt column = 0;
		colPos = 0;
		for ( b = i->begin(); b!=i->end(); b++ )
		{
			b->SetProcessed( false );
			b->setCol( column );
			b->setRow( row );
			TqInt colSize = xRes - colPos;
			if(colSize > m_optCache.xBucketSize)
				colSize = m_optCache.xBucketSize;
			b->setPosition( colPos, rowPos );
			b->setSize( colSize, rowSize );
			column++;
			colPos += m_optCache.xBucketSize;
		}
		row++;
		rowPos += m_optCache.yBucketSize;
	}

	m_CurrentBucketCol = m_bucketRegion.xMin();
	m_CurrentBucketRow = m_bucketRegion.yMin();
}


//----------------------------------------------------------------------
/** Delete the allocated memory for the image buffer.
 */

void	CqImageBuffer::DeleteImage()
{
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
	if ( Bound.vecMin().z() >= m_optCache.clipFar ||
	     Bound.vecMax().z() <= m_optCache.clipNear )
		return true;

	// This needs to be re-enabled when the RiClippingPlane code is wired up.
#if 0
	if(QGetRenderContext()->clippingVolume().whereIs(Bound) == CqBound::Side_Outside)
	{
		return(true);
	}
#endif

	// If the primitive spans the epsilon plane and the hither plane and can be split,
	if ( Bound.vecMin().z() <= FLT_EPSILON )
	{
		// Mark the primitive as not dicable.
		pSurface->ForceUndiceable();

		CqString objname( "unnamed" );
		const CqString* pattrName = pSurface->pAttributes() ->GetStringAttribute( "identifier", "name" );
		if ( pattrName != 0 )
			objname = pattrName[ 0 ];
		Aqsis::log() << info << "Object \"" << objname.c_str() << "\" spans the epsilon plane" << std::endl;

		if ( pSurface->SplitCount() > m_optCache.maxEyeSplits )
		{
			Aqsis::log() << warning << "Max eyesplits for object \"" << objname.c_str() << "\" exceeded" << std::endl;
			return( true );
		}
		return ( false );
	}

	TqFloat minz = Bound.vecMin().z();
	TqFloat maxz = Bound.vecMax().z();


	// Convert the bounds to raster space.
	CqMatrix mat;
	QGetRenderContext() ->matSpaceToSpace( "camera", "raster", NULL, NULL, QGetRenderContext()->Time(), mat );
	Bound.Transform( mat );

	// Take into account depth-of-field
	if ( QGetRenderContext() ->UsingDepthOfField() )
	{
		const CqVector2D minZCoc = QGetRenderContext()->GetCircleOfConfusion( minz );
		const CqVector2D maxZCoc = QGetRenderContext()->GetCircleOfConfusion( maxz );
		TqFloat cocX = max( minZCoc.x(), maxZCoc.x() );
		TqFloat cocY = max( minZCoc.y(), maxZCoc.y() );
		Bound.vecMin().x( Bound.vecMin().x() - cocX );
		Bound.vecMin().y( Bound.vecMin().y() - cocY );
		Bound.vecMax().x( Bound.vecMax().x() + cocX );
		Bound.vecMax().y( Bound.vecMax().y() + cocY );
	}

	// And expand to account for filter size.
	Bound.vecMin().x( Bound.vecMin().x() - m_optCache.xFiltSize / 2.0f );
	Bound.vecMin().y( Bound.vecMin().y() - m_optCache.yFiltSize / 2.0f );
	Bound.vecMax().x( Bound.vecMax().x() + m_optCache.xFiltSize / 2.0f );
	Bound.vecMax().y( Bound.vecMax().y() + m_optCache.yFiltSize / 2.0f );

	// If the bounds are completely outside the viewing frustum, cull the primitive.
	if( Bound.vecMin().x() > QGetRenderContext()->cropWindowXMax() ||
		Bound.vecMin().y() > QGetRenderContext()->cropWindowYMax() ||
		Bound.vecMax().x() < QGetRenderContext()->cropWindowXMin() ||
		Bound.vecMax().y() < QGetRenderContext()->cropWindowYMin() )
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
	AQSIS_TIME_SCOPE(Post_surface);
	// Count the number of total gprims
	STATS_INC( GPR_created_total );

	// Bound the primitive in its current space (camera) space taking into account any motion specification.
	CqBound Bound;
	pSurface->Bound(&Bound);

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
		CqMatrix mat;
		QGetRenderContext() ->matVSpaceToSpace( strCoordinateSystem.c_str(), "camera", transShaderToWorld, pSurface->pTransform().get(), QGetRenderContextI()->Time(), mat );
		vecDB = mat * vecDB;
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
	TqInt XMinb = 0;
	TqInt YMinb = 0;
	TqInt XMaxb = 0;
	TqInt YMaxb = 0;
	if (! pSurface->IsUndiceable() )
	{
		XMinb = static_cast<TqInt>( Bound.vecMin().x() ) / m_optCache.xBucketSize;
		YMinb = static_cast<TqInt>( Bound.vecMin().y() ) / m_optCache.yBucketSize;
		XMaxb = static_cast<TqInt>( Bound.vecMax().x() ) / m_optCache.xBucketSize;
		YMaxb = static_cast<TqInt>( Bound.vecMax().y() ) / m_optCache.yBucketSize;
	}
	XMinb = clamp( XMinb, m_bucketRegion.xMin(), m_bucketRegion.xMax()-1 );
	YMinb = clamp( YMinb, m_bucketRegion.yMin(), m_bucketRegion.yMax()-1 );
	XMaxb = clamp( XMaxb, m_bucketRegion.xMin(), m_bucketRegion.xMax()-1 );
	YMaxb = clamp( YMaxb, m_bucketRegion.yMin(), m_bucketRegion.yMax()-1 );

	// Sanity check we are not putting into a bucket that has already been processed.
	CqBucket* bucket = &Bucket( XMinb, YMinb );
	if ( bucket->IsProcessed() )
	{
		// Scan over the buckets that the bound touches, looking for the first one that isn't processed.
		TqInt yb = YMinb;
		TqInt xb = XMinb + 1;
		bool done = false;
		while(!done && yb <= YMaxb)
		{
			while(!done && xb <= XMaxb)
			{
				CqBucket& availBucket = Bucket(xb, yb);
				if(!availBucket.IsProcessed())
				{
					availBucket.AddGPrim(pSurface);
					done = true;
				}
				++xb;
			}
			xb = XMinb;
			++yb;
		}
	}
	else
	{
		bucket->AddGPrim( pSurface );
	}
}


void CqImageBuffer::RepostSurface(const CqBucket& oldBucket,
                                  const boost::shared_ptr<CqSurface>& surface)
{
	const CqBound rasterBound = surface->GetCachedRasterBound();

	bool wasPosted = false;
	// Surface is behind everying in this bucket but it may be visible in other
	// buckets it overlaps.
	//
	// First look in bucket to the right
	TqInt nextBucketX = oldBucket.getCol() + 1;
	TqInt nextBucketY = oldBucket.getRow();
	TqInt xpos = oldBucket.getXPosition() + oldBucket.getXSize();
	if ( nextBucketX < m_bucketRegion.xMax() && rasterBound.vecMax().x() >= xpos )
	{
		Bucket( nextBucketX, nextBucketY ).AddGPrim( surface );
		wasPosted = true;
	}
	else
	{
		// next row
		++nextBucketY;
		// find bucket containing left side of bound
		nextBucketX = max<TqInt>(m_bucketRegion.xMin(),
				lfloor(rasterBound.vecMin().x())/m_optCache.xBucketSize);
		TqInt ypos = oldBucket.getYPosition() + oldBucket.getYSize();

		if ( ( nextBucketX < m_bucketRegion.xMax() ) &&
			( nextBucketY  < m_bucketRegion.yMax() ) &&
			( rasterBound.vecMax().y() >= ypos ) )
		{
			Bucket( nextBucketX, nextBucketY ).AddGPrim( surface );
			wasPosted = true;
		}
	}

#ifdef DEBUG
	// Print info about reposting.  This is protected by DEBUG so that scenes
	// with huge amounts of occlusion won't silently be causing *heaps* of
	// logging traffic.
	CqString objName("unnamed");
	if(const CqString* name = surface->pAttributes()
			->GetStringAttribute("identifier", "name"))
		objName = name[0];
	if(wasPosted)
	{
		Aqsis::log() << info << "GPrim: \"" << objName
			<< "\" occluded in bucket: "
			<< oldBucket.getCol() << ", " << oldBucket.getRow()
			<< " shifted into bucket: "
			<< nextBucketX << ", " << nextBucketY << "\n";
	}
	else
	{
		Aqsis::log() << info
			<< "GPrim: \"" << objName << "\" occlusion culled" << std::endl;
	}
#endif
}

//----------------------------------------------------------------------
/** Add a new micro polygon to the list of waiting ones.
 * \param pmpgNew Pointer to a CqMicroPolygon derived class.
 */

void CqImageBuffer::AddMPG( boost::shared_ptr<CqMicroPolygon>& pmpgNew )
{
	CqRenderer* renderContext = QGetRenderContext();
	CqBound B = pmpgNew->GetBound();

	// Expand the micropolygon bound for DoF if necessary.
	if(renderContext->UsingDepthOfField())
	{
		// Get the maximum CoC multiplier for the micropolygon depth.
		const CqVector2D maxCoC = max(
			renderContext->GetCircleOfConfusion(B.vecMin().z()),
			renderContext->GetCircleOfConfusion(B.vecMax().z())
		);
		// Expand the bound by the CoC radius
		B.vecMin() -= vectorCast<CqVector3D>(maxCoC);
		B.vecMax() += vectorCast<CqVector3D>(maxCoC);
	}

	// Discard when outside the crop window.
	if ( B.vecMax().x() < renderContext->cropWindowXMin() - m_optCache.xFiltSize / 2.0f ||
	     B.vecMax().y() < renderContext->cropWindowYMin() - m_optCache.yFiltSize / 2.0f ||
	     B.vecMin().x() > renderContext->cropWindowXMax() + m_optCache.xFiltSize / 2.0f ||
	     B.vecMin().y() > renderContext->cropWindowYMax() + m_optCache.yFiltSize / 2.0f )
	{
		return;
	}

	////////// Dump the micro polygon into a dump file //////////
#if ENABLE_MPDUMP
	if(m_mpdump.IsOpen())
		m_mpdump.dump(*pmpgNew);
#endif
	/////////////////////////////////////////////////////////////


	// Find out the minimum bucket touched by the micropoly bound.

	B.vecMin().x( B.vecMin().x() - (lfloor(m_optCache.xFiltSize / 2.0f)) );
	B.vecMin().y( B.vecMin().y() - (lfloor(m_optCache.yFiltSize / 2.0f)) );
	B.vecMax().x( B.vecMax().x() + (lfloor(m_optCache.xFiltSize / 2.0f)) );
	B.vecMax().y( B.vecMax().y() + (lfloor(m_optCache.yFiltSize / 2.0f)) );

	TqInt iXBa = static_cast<TqInt>( B.vecMin().x() / m_optCache.xBucketSize );
	TqInt iYBa = static_cast<TqInt>( B.vecMin().y() / m_optCache.yBucketSize );
	TqInt iXBb = static_cast<TqInt>( B.vecMax().x() / m_optCache.xBucketSize );
	TqInt iYBb = static_cast<TqInt>( B.vecMax().y() / m_optCache.yBucketSize );

	if ( ( iXBb < m_bucketRegion.xMin() ) || ( iYBb < m_bucketRegion.yMin() ) ||
	        ( iXBa >= m_bucketRegion.xMax() ) || ( iYBa >= m_bucketRegion.yMax() ) )
	{
		return ;
	}

	// Use sane values -- otherwise sometimes crashes, probably
	// due to precision problems
	if ( iXBa < m_bucketRegion.xMin() )  iXBa = m_bucketRegion.xMin();
	if ( iYBa < m_bucketRegion.yMin() )  iYBa = m_bucketRegion.yMin();
	if ( iXBb >= m_bucketRegion.xMax() )  iXBb = m_bucketRegion.xMax() - 1;
	if ( iYBb >= m_bucketRegion.yMax() )  iYBb = m_bucketRegion.yMax() - 1;

	// Add the MP to all the Buckets that it touches
	for ( TqInt i = iXBa; i <= iXBb; i++ )
	{
		for ( TqInt j = iYBa; j <= iYBb; j++ )
		{
			CqBucket* bucket = &Bucket( i, j );
			// Only add the MPG if the bucket isn't processed.
			// \note It is possible for this to happen validly, if a primitive is occlusion culled in a 
			// previous bucket, and not in a subsequent one. When it gets processed in the later bucket
			// the MPGs can leak into the previous one, shouldn't be a problem, as the occlusion culling 
			// means the MPGs shouldn't be rendered in that bucket anyway.
			if ( !bucket->IsProcessed() )
			{
				bucket->AddMP( pmpgNew );
			}
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

	// Render the surface at the front of the list.
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

	std::vector<boost::shared_ptr<CqBucketProcessor> > bucketProcessors;
	TqInt numConcurrentBuckets = 1;

/* mafm: Sample code to use with the CqThreadProcessor2 alternative
 * approach.

	std::vector<std::vector<CqBucket*> > bucketLists;
	bucketLists.resize(MULTIPROCESSING_NBUCKETS);
	int index = 0;
	do
	{
		bucketLists[index % MULTIPROCESSING_NBUCKETS].push_back( &(CurrentBucket()) );
		++index;
	} while ( NextBucket(order) );
	boost::thread_group threadGroup;
	for (int i = 0; i < MULTIPROCESSING_NBUCKETS; ++i)
	{
		threadGroup.create_thread( CqThreadProcessor2(this, bucketLists[i], fImager, zThreshold, depthfilter, bHalf) );
	}
	threadGroup.join_all();
	return;
*/

#ifdef		ENABLE_THREADING
	numConcurrentBuckets = MULTIPROCESSING_NBUCKETS;
#endif

	for(int i = 0; i < numConcurrentBuckets; ++i)
	{
		bucketProcessors.push_back(boost::shared_ptr<CqBucketProcessor>(
					new CqBucketProcessor(*this, m_optCache)));
	}
	CqMultiJitteredSampler jitteredSampler(m_optCache.xSamps, m_optCache.ySamps);
	CqGridSampler gridSampler(m_optCache.xSamps, m_optCache.ySamps);

	// Determine whether the user has asked for sample jittering
	IqSampler* sampler = &jitteredSampler;
	if(const TqInt* jitter = QGetRenderContext()->poptCurrent()->
			GetIntegerOption("Hider", "jitter"))
	{
		if(jitter[0] == 0)
			sampler = &gridSampler;
	}

	// Iterate over all buckets...
	bool pendingBuckets = true;
	while ( pendingBuckets && !m_fQuit )
	{
		CqThreadScheduler threadScheduler(numConcurrentBuckets);
		std::vector<CqThreadProcessor> threadProcessors;

		for (int i = 0; pendingBuckets && i < numConcurrentBuckets; ++i)
		{
			bucketProcessors[i]->setBucket(&CurrentBucket());

			// Prepare the bucket processor
			bucketProcessors[i]->preProcess(sampler);

#if ENABLE_MPDUMP
			// Dump the pixel sample positions into a dump file
			if(m_mpdump.IsOpen())
				m_mpdump.dumpPixelSamples(*bucketProcessors[i]);
#endif

			// Kick off a thread to process this bucket.
			threadProcessors.push_back( CqThreadProcessor( bucketProcessors[i].get() ) );
			threadScheduler.addWorkUnit( threadProcessors.back() );

			// Advance to next bucket, quit if nothing left
			iBucket += 1;
			pendingBuckets = NextBucket(order);
		}

		// Wait for all current buckets to complete before allocating more to the available threads.
		threadScheduler.joinAll();
		threadProcessors.clear();

		for (int i = 0; !m_fQuit && i < numConcurrentBuckets; ++i)
		{
			bucketProcessors[i]->postProcess();
			{
				AQSIS_TIME_SCOPE(Display_bucket);
				const CqBucket* bucket = bucketProcessors[i]->getBucket();
				if (bucket)
				{
					QGetRenderContext() ->pDDmanager() ->DisplayBucket( bucketProcessors[i]->DisplayRegion(), &(bucketProcessors[i]->getChannelBuffer()) );
				}
			}
			bucketProcessors[i]->reset();

			if ( pProgressHandler )
			{
				// Inform the status class how far we have got, and update UI.
				float Complete = (100.0f * iBucket) / static_cast<float> ( m_bucketRegion.area() );
				QGetRenderContext() ->Stats().SetComplete( Complete );
				( *pProgressHandler ) ( Complete, QGetRenderContext() ->CurrentFrame() );
			}

#ifdef WIN32
			if ( !( iBucket % bucketmodulo ) )
				SetProcessWorkingSetSize( GetCurrentProcess(), 0xffffffff, 0xffffffff );
#endif
		}
	}

	// Pass >100 through to progress to allow it to indicate completion.
	if ( pProgressHandler )
	{
		( *pProgressHandler ) ( 100.0f, QGetRenderContext() ->CurrentFrame() );
	}
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
	// only deal with horizontal bucket orders for now.
	m_CurrentBucketCol++;

	if( m_CurrentBucketCol >= m_bucketRegion.xMax() )
	{
		m_CurrentBucketCol = m_bucketRegion.xMin();
		m_CurrentBucketRow++;
		if( m_CurrentBucketRow >= m_bucketRegion.yMax() )
			return false;
	}

	// General bucket orders are not ready for prime time.
	// WARNING: The code below needs to be adjusted to deal with m_bucketRegion
#if 0
	TqInt cnt = 0;
	TqInt total = m_bucketRegion.area();

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
				m_CurrentBucketCol = clamp(m_CurrentBucketCol, 0, m_cXBuckets - 1);
				m_CurrentBucketRow = clamp(m_CurrentBucketRow, 0, m_cYBuckets - 1);
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

				m_CurrentBucketCol = clamp(m_CurrentBucketCol, 0, m_cXBuckets - 1);
				m_CurrentBucketRow = clamp(m_CurrentBucketRow, 0, m_cYBuckets - 1);
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

			if( m_CurrentBucketCol >= m_bucketRegion.xMax() )
			{
				m_CurrentBucketCol = m_bucketRegion.xMin();
				m_CurrentBucketRow++;
				if( m_CurrentBucketRow >= m_bucketRegion.yMax() )
					return( false );
			}
		}
		break;
	}
#endif
	return( true );
}

//---------------------------------------------------------------------

} // namespace Aqsis



