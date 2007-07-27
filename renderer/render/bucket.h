// Aqsis
// Copyright Î÷Î÷ 1997 - 2001, Paul C. Gregory
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

#include	"aqsis.h"

#include	<vector>
#include	<queue>
#include	<deque>

#include	"surface.h"
#include	"color.h"
#include	"imagepixel.h"
#include	"iddmanager.h"
#include	"bucketdata.h"


START_NAMESPACE( Aqsis )

//-----------------------------------------------------------------------
/** Class holding data about a particular bucket.
 */

class CqBucket : public IqBucket
{
	public:
		CqBucket();

		virtual ~CqBucket();

		// Overridden from IqBucket
		virtual	TqInt	Width() const
		{
			return ( m_bucketData->m_XSize );
		}
		virtual	TqInt	Height() const
		{
			return ( m_bucketData->m_YSize );
		}
		virtual	TqInt	RealWidth() const
		{
			return ( m_bucketData->m_RealWidth );
		}
		virtual	TqInt	RealHeight() const
		{
			return ( m_bucketData->m_RealHeight );
		}
		virtual	TqInt	XOrigin() const
		{
			return ( m_bucketData->m_XOrigin );
		}
		virtual	TqInt	YOrigin() const
		{
			return ( m_bucketData->m_YOrigin );
		}
		TqInt	PixelXSamples() const
		{
			return m_bucketData->m_PixelXSamples;
		}
		TqInt	PixelYSamples() const
		{
			return m_bucketData->m_PixelYSamples;
		}
		TqFloat	FilterXWidth() const
		{
			return m_bucketData->m_FilterXWidth;
		}
		TqFloat	FilterYWidth() const
		{
			return m_bucketData->m_FilterYWidth;
		}

		virtual	CqColor Color( TqInt iXPos, TqInt iYPos );
		virtual	CqColor Opacity( TqInt iXPos, TqInt iYPos );
		virtual	TqFloat Coverage( TqInt iXPos, TqInt iYPos );
		virtual	TqFloat Depth( TqInt iXPos, TqInt iYPos );
		virtual	const TqFloat* Data( TqInt iXPos, TqInt iYPos );

		/** Set up the necessary data for the bucket to render
		 *
		 * \param viewRangeXMin Integer minimum extend of the
		 * image part being rendered, takes into account
		 * buckets and clipping.
		 *
		 * \param viewRangeXMax Integer maximum extend of the
		 * image part being rendered, takes into account
		 * buckets and clipping.
		 *
		 * \param viewRangeYMin Integer minimum extend of the
		 * image part being rendered, takes into account
		 * buckets and clipping.
		 *
		 * \param viewRangeYMax Integer maximum extend of the
		 * image part being rendered, takes into account
		 * buckets and clipping.
		 */
		void	PrepareBucket( const CqVector2D& pos, const CqVector2D& size,
				       TqInt pixelXSamples, TqInt pixelYSamples, TqFloat filterXWidth, TqFloat filterYWidth,
				       TqInt viewRangeXMin, TqInt viewRangeXMax, TqInt viewRangeYMin, TqInt viewRangeYMax,
				       TqFloat clippingNear, TqFloat clippingFar,
				       bool fJitter = true, bool empty = false );

		CqImagePixel& ImageElement(TqInt index) const;

		std::vector<SqSampleData>& SamplePoints() const
		{
			return(m_bucketData->m_SamplePoints);
		}

		TqInt GetNextSamplePointIndex()
		{
			TqInt index = m_bucketData->m_NextSamplePoint;
			m_bucketData->m_NextSamplePoint++;
			return(index);
		}

		void	CombineElements(enum EqFilterDepth eDepthFilter, CqColor zThreshold);
		void	FilterBucket(bool empty, bool fImager);
		void	ExposeBucket();
		void	QuantizeBucket();

		/** Add a GPRim to the stack of deferred GPrims.
		* \param The Gprim to be added.
		 */
		void	AddGPrim( const boost::shared_ptr<CqSurface>& pGPrim )
		{
			m_gPrims.push(pGPrim);
		}

		/** Add an MP to the list of deferred MPs.
		 */
		void	AddMP( boost::shared_ptr<CqMicroPolygon>& pMP );

		/** Get a pointer to the top GPrim in the stack of deferred GPrims.
		 */
		boost::shared_ptr<CqSurface> pTopSurface()
		{
			if (!m_gPrims.empty())
			{
				return m_gPrims.top();
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
			m_gPrims.pop();
		}
		/** Get a count of deferred GPrims.
		 */
		TqInt cGPrims() const
		{
			return ( m_gPrims.size() );
		}
		/** Get the flag that indicates whether the bucket is
		 * empty.  It is empty only when this bucket doesn't
		 * contain any surface, micropolygon or grids.
		 */
		bool IsEmpty();
		/** Get the flag that indicates whether the bucket has
		 * pending surfaces.
		 */
		bool hasPendingSurfaces() const;
		/** Get the flag that indicates whether the bucket has
		 * pending MPs to render.
		 */
		bool hasPendingMPs() const;
		/** Get the flag that indicates if the bucket has been processed yet.
		 */
		bool IsProcessed() const
		{
			return( m_bProcessed );
		}
		/** Mark this bucket as processed
		 */
		void SetProcessed( bool bProc =  true);
		/** Set the pointer to the bucket data
		 */
		void SetBucketData( CqBucketData* bucketData )
		{
			m_bucketData = bucketData;
		}

		/** Render any waiting MPs.
		 */
		void RenderWaitingMPs();

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

		/// Dynamic bucket data
		CqBucketData* m_bucketData;

		/// Vector of vectors of waiting micropolygons in this bucket
		std::vector<boost::shared_ptr<CqMicroPolygon> > m_micropolygons;

		/// A sorted list of primitives for this bucket
		std::priority_queue<boost::shared_ptr<CqSurface>, std::deque<boost::shared_ptr<CqSurface> >, closest_surface> m_gPrims;

		void	ImageElement( TqInt iXPos, TqInt iYPos, CqImagePixel*& pie );

		void	InitialiseFilterValues();

		void	CalculateDofBounds();

		const CqBound& DofSubBound(TqInt index) const
		{
			assert(index < m_bucketData->m_NumDofBounds);
			return m_bucketData->m_DofBounds[index];
		}

		/** Render a particular micropolygon.
		 *
		 * \param pMPG Pointer to the micropolygon to process.
		 * \see CqBucket, CqImagePixel
		 */
		void	RenderMicroPoly( CqMicroPolygon* pMP );

		/** This function assumes that either dof or mb or
		 * both are being used. */
		void	RenderMP_MBOrDof( CqMicroPolygon* pMP, bool IsMoving, bool UsingDof );
		/** This function assumes that neither dof or mb are
		 * being used. It is much simpler than the general
		 * case dealt with above. */
		void	RenderMP_Static( CqMicroPolygon* pMP );
};

END_NAMESPACE( Aqsis )

//}  // End of #ifdef BUCKET_H_INCLUDED
#endif
