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
		\brief Declares the CqImageBuffer class responsible for rendering the primitives and storing the results.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is imagebuffer.h included already?
#ifndef IMAGEBUFFER_H_INCLUDED
//{
#define IMAGEBUFFER_H_INCLUDED 1

#include	<aqsis/aqsis.h>

#include	<vector>

#include	"surface.h"
#include	<aqsis/math/vector2d.h>
#include   	"bucket.h"
#include	"mpdump.h"
#include	"optioncache.h"

namespace Aqsis {


class CqMicroPolygon;


// Enumeration of the type of rendering order of the buckets (experimental)
enum EqBucketOrder {
        Bucket_Horizontal = 0,
        Bucket_Vertical,
        Bucket_ZigZag,
        Bucket_Circle,
        Bucket_Random
};


//-----------------------------------------------------------------------
/**
  The main image and related data, also responsible for processing the rendering loop.
 
  Before the image can be rendered the image buffer has to be initialised by calling the
  SetImage() method. The parameters for the creation of the buffer are read from the
  current options (this includes things like the image resolution, bucket size,
  number of pixel samples, etc.).
  
  After the buffer is initialized the surfaces (gprims) that are to be rendered 
  can be added to the buffer. This is done by calling PostSurface() for each gprim.
  (note: before calling this method the gprim has to be transformed into camera space!)
  All the gprims that can be culled at this point (i.e. CullSurface() returns true) 
  won't be stored inside the buffer. If a gprim can't be culled it is assigned to
  the first bucket that touches its bound.
 
  Once all the gprims are posted to the buffer the image can be rendered by calling
  RenderImage(). Now all buckets will be processed one after another. 
 
  \see CqBucket, CqSurface, CqRenderer
 */

class CqImageBuffer
{
	public:
		CqImageBuffer() :
				m_fQuit( false ),
				m_cXBuckets( 0 ),
				m_cYBuckets( 0 ),
				m_CurrentBucketCol( 0 ),
				m_CurrentBucketRow( 0 )
		{}
		~CqImageBuffer();

		void AddMPG( boost::shared_ptr<CqMicroPolygon>& pmpgNew );
		void PostSurface( const boost::shared_ptr<CqSurface>& pSurface );
		/** \brief Repost a previously posted surface into the next unfinished bucket.
		 *
		 * The surface is reposted it to the next unfinished bucket within the
		 * surface's bound, skipping the old bucket.  Using this function is
		 * relevant when the surface is culled in the current bucket for
		 * reasons such as occlusion culling.
		 *
		 * \param oldBucket - previous bucket holding the surface.
		 * \param surface - surface to repost
		 */
		void RepostSurface( const CqBucket& oldBucket,
		                    const boost::shared_ptr<CqSurface>& surface );
		void RenderImage();

		void SetImage();
		void Quit();
		void Release();

		enum EqNeighbourLocation
		{
			left = 0,
			right = 1,
			above = 2,
			below = 3,
		};
		/** Get an array of neighbour buckets.
		 *  
		 *  \param bucket - The bucket whose neighours are to be found.
		 *  \param neighbours - A reference to the array to be filled.
		 */
		void	axialNeighbours(CqBucket const& bucket, std::vector<CqBucket*>& neighbours);

	private:
		/// Get a pointer to the bucket at position x,y in the grid.
		CqBucket& Bucket( TqInt x, TqInt y)
		{
			return m_Buckets[y][x];
		}

		bool	m_fQuit;			///< Set by system if a quit has been requested.

		/** m_bucketRegion defines the set of non-cropped buckets.  The set of
		 * valid buckets is from m_bucketRegion.xMin() to m_bucketRegion.xMax()-1
		 * inclusive, and similarly in the y-direction.
		 */
		CqRegion m_bucketRegion;
		SqOptionCache m_optCache;   ///< Cache of commonly used RiOptions
		TqInt	m_cXBuckets;		///< Integer horizontal bucket count.
		TqInt	m_cYBuckets;		///< Integer vertical bucket count.

		std::vector<std::vector<CqBucket> >	m_Buckets; ///< Array of bucket storage classes (row/col)
		TqInt	m_CurrentBucketCol;	///< Column index of the bucket currently being processed.
		TqInt	m_CurrentBucketRow;	///< Row index of the bucket currently being processed.

#if ENABLE_MPDUMP
		CqMPDump	m_mpdump;
#endif

		bool	CullSurface( CqBound& Bound, const boost::shared_ptr<CqSurface>& pSurface );
		void	DeleteImage();

		/** Move to the next bucket to process.
		 */
		bool NextBucket(EqBucketOrder order);

		/** Get a pointer to the current bucket
		 */
		CqBucket& CurrentBucket()
		{
			return( m_Buckets[m_CurrentBucketRow][m_CurrentBucketCol] );
		}
};

//-----------------------------------------------------------------------
// Implementation
//

//----------------------------------------------------------------------

inline void CqImageBuffer::axialNeighbours(CqBucket const& bucket, std::vector<CqBucket*>& neighbours)
{
	TqInt bx = bucket.getCol();
	TqInt by = bucket.getRow();
	neighbours.clear();
	neighbours.resize(4, 0);
		
	// Populate the array slots depending on if there are usable neighbours in that direction.
	if(bx > 0)
		neighbours[left] = &Bucket(bx-1, by);
	if(bx < m_cXBuckets - 1)
		neighbours[right] = &Bucket(bx+1, by);
	if(by > 0)
		neighbours[above] = &Bucket(bx, by-1);
	if(by < m_cYBuckets - 1)
		neighbours[below] = &Bucket(bx, by+1);
}

//-----------------------------------------------------------------------

} // namespace Aqsis

//}  // End of #ifdef IMAGEBUFFER_H_INCLUDED
#endif


