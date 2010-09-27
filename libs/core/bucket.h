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
		\brief Declares the CqBucket class responsible for bookeeping the primitives and storing the results.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is imagebuffer.h included already?
#ifndef BUCKET_H_INCLUDED
//{
#define BUCKET_H_INCLUDED 1

#include	<aqsis/aqsis.h>

#include	<algorithm>
#include	<vector>
#include	<deque>
#include	<boost/shared_ptr.hpp>
#include	<boost/array.hpp>

#include	"surface.h"
#include	<aqsis/math/color.h>
#include	"imagepixel.h"
#include	"iddmanager.h"
#include	<aqsis/math/region.h>

namespace Aqsis {

struct SqBucketCacheSegment
{
	enum	EqBucketCacheSide
	{
		left = 0,
		right = 1,
		top = 2,
		bottom = 3,

		top_left = 4,
		bottom_left = 5,
		top_right = 6,
		bottom_right = 7,

		last
	};
	EqBucketCacheSide side;
	TqInt			  size;
	std::vector<CqImagePixelPtr>	cache;
};


//-----------------------------------------------------------------------
/** Class holding data about a particular bucket.
 */

class CqBucket
{
	public:
		typedef boost::array<boost::shared_ptr<SqBucketCacheSegment>, SqBucketCacheSegment::last > TqCache;
		
		CqBucket();

		/** Add a GPRim to the stack of deferred GPrims.
		* \param The Gprim to be added.
		 */
		void	AddGPrim( const boost::shared_ptr<CqSurface>& pGPrim )
		{
			m_gPrims.push_back(pGPrim);
			std::push_heap(m_gPrims.begin(), m_gPrims.end(),
						   closest_surface());
		}

		/** Get a pointer to the top GPrim in the stack of deferred GPrims.
		 */
		boost::shared_ptr<CqSurface> pTopSurface()
		{
			if (!m_gPrims.empty())
			{
				return m_gPrims.front();
			}
			else
			{
				return boost::shared_ptr<CqSurface>();
			}
		}
		/** Pop the top GPrim in the stack of deferred GPrims.
		 */
		void popSurface()
		{
			if (!m_gPrims.empty())
			{
				std::pop_heap(m_gPrims.begin(), m_gPrims.end(),
							  closest_surface());
				m_gPrims.pop_back();
			}
		}
		/** Get a count of deferred GPrims.
		 */
		TqInt cGPrims() const
		{
			return ( m_gPrims.size() );
		}
		bool hasPendingSurfaces() const;
		/** Get the flag that indicates if the bucket has been processed yet.
		 */
		bool IsProcessed() const
		{
			return( m_bProcessed );
		}
		/** Mark this bucket as processed
		 */
		void SetProcessed( bool bProc =  true);

		/** Get the column of the bucket in the image */
		TqInt getCol() const;
		/** Set the column of the bucket in the image */
		void setCol(TqInt value);
		/** Get the row of the bucket in the image */
		TqInt getRow() const;
		/** Set the row of the bucket in the image */
		void setRow(TqInt value);
		/** Get the x position of the bucket in raster space */
		TqInt getXPosition() const;
		/** Get the y position of the bucket in raster space */
		TqInt getYPosition() const;
		/** Set the position of the bucket in raster space */
		void setPosition(TqInt xpos, TqInt ypos);
		/** Get the x size of the bucket in raster space */
		TqInt getXSize() const;
		/** Get the y size of the bucket in raster space */
		TqInt getYSize() const;
		/** Set the size of the bucket in raster space */
		void setSize(TqInt xsize, TqInt ysize);

		/** Add an MP to the list of deferred MPs.
		 */
		void	AddMP( boost::shared_ptr<CqMicroPolygon>& pMP );

		std::vector<boost::shared_ptr<CqMicroPolygon> >& micropolygons();

		const TqCache& cacheSegments() const;
		void setCacheSegment(SqBucketCacheSegment::EqBucketCacheSide side, boost::shared_ptr<SqBucketCacheSegment>& seg);
		void clearCache();

	private:
		/// This is a compare functor for sorting surfaces in order of depth.
		struct closest_surface
		{
			bool operator()(const boost::shared_ptr<CqSurface>& s1, const boost::shared_ptr<CqSurface>& s2) const
			{
				if ( s1->fCachedBound() && s2->fCachedBound() )
				{
					return ( s1->GetCachedRasterBound().vecMin().z() > s2->GetCachedRasterBound().vecMin().z() );
				}

				// don't have bounds for the surface(s). I suspect we should assert here.
				return true;
			}
		};

		/// Flag indicating if this bucket has been processed yet.
		bool	m_bProcessed;

		/// Bucket column in the image
		TqInt m_col;
		/// Bucket row in the image
		TqInt m_row;
		/// Bucket x position in raster space.
		TqInt m_xPosition;
		/// Bucket y position in raster space.
		TqInt m_yPosition;
		/// Bucket x size in raster space.
		TqInt m_xSize;
		/// Bucket y size in raster space.
		TqInt m_ySize;

		/// Vector of vectors of waiting micropolygons in this bucket
		typedef std::vector<boost::shared_ptr<CqMicroPolygon> > TqPolyStorage;
		TqPolyStorage m_micropolygons;

		/// A sorted list of primitives for this bucket
		///
		/// Beware - we use a plain std::vector here and use the heap
		/// operations directly rather than using std::priority_queue.  This
		/// is so that we can make sure the underlying container storage is
		/// completely deallocated when the bucket is done.
		typedef std::vector<boost::shared_ptr<CqSurface> > TqSurfaceQueue;
		TqSurfaceQueue m_gPrims;

		TqCache m_cacheSegments;
};


//------------------------------------------------------------
// Implementation details
//------------------------------------------------------------

inline std::vector<boost::shared_ptr<CqMicroPolygon> >& CqBucket::micropolygons()
{
	return m_micropolygons;
}

inline void CqBucket::setCacheSegment(SqBucketCacheSegment::EqBucketCacheSide side, boost::shared_ptr<SqBucketCacheSegment>& seg)
{
	// Check there isn't already a cache segment for the position.
	assert(!m_cacheSegments[side]);
	m_cacheSegments[side] = seg;
}

inline const CqBucket::TqCache& CqBucket::cacheSegments() const
{
	return m_cacheSegments;
}

inline TqInt CqBucket::getXPosition() const
{
	return m_xPosition;
}

inline TqInt CqBucket::getYPosition() const
{
	return m_yPosition;
}

inline void CqBucket::setPosition(TqInt xpos, TqInt ypos)
{
	m_xPosition = xpos;
	m_yPosition = ypos;
}

inline TqInt CqBucket::getXSize() const
{
	return m_xSize;
}

inline TqInt CqBucket::getYSize() const
{
	return m_ySize;
}

inline void CqBucket::setSize(TqInt xsize, TqInt ysize)
{
	m_xSize = xsize;
	m_ySize = ysize;
}

inline void CqBucket::clearCache()
{

	for(TqInt cseg = 0; cseg < SqBucketCacheSegment::last; ++cseg)
		m_cacheSegments[cseg].reset();
}


} // namespace Aqsis

//}  // End of #ifdef BUCKET_H_INCLUDED
#endif
