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

#include	"iddmanager.h"
#include	"bitvector.h"
#include	"micropolygon.h"
#include	"renderer.h"
#include	"ri.h"
#include	"surface.h"
#include	"color.h"
#include	"vector2d.h"
#include	"vector2d.h"
#include	"imagepixel.h"
#include	"bucketdata.h"

START_NAMESPACE( Aqsis )

//-----------------------------------------------------------------------
/** Class holding data about a particular bucket.
 */

class CqBucket : public IqBucket
{
	public:
		CqBucket()
		{
			m_bucketData.m_bProcessed = false;
		}

		virtual ~CqBucket();

		// Overridden from IqBucket
		virtual	TqInt	Width() const
		{
			return ( m_bucketData.m_XSize );
		}
		virtual	TqInt	Height() const
		{
			return ( m_bucketData.m_YSize );
		}
		virtual	TqInt	RealWidth() const
		{
			return ( m_bucketData.m_RealWidth );
		}
		virtual	TqInt	RealHeight() const
		{
			return ( m_bucketData.m_RealHeight );
		}
		virtual	TqInt	XOrigin() const
		{
			return ( m_bucketData.m_XOrigin );
		}
		virtual	TqInt	YOrigin() const
		{
			return ( m_bucketData.m_YOrigin );
		}
		static	TqInt	PixelXSamples()
		{
			return m_bucketData.m_PixelXSamples;
		}
		static	TqInt	PixelYSamples()
		{
			return m_bucketData.m_PixelYSamples;
		}
		static	TqFloat	FilterXWidth()
		{
			return m_bucketData.m_FilterXWidth;
		}
		static	TqFloat	FilterYWidth()
		{
			return m_bucketData.m_FilterYWidth;
		}
		static	TqInt	NumTimeRanges()
		{
			return m_bucketData.m_NumTimeRanges;
		}
		static	TqInt	NumDofBounds()
		{
			return m_bucketData.m_NumDofBounds;
		}

		static const CqBound& DofSubBound(TqInt index)
		{
			assert(index < m_bucketData.m_NumDofBounds);
			return m_bucketData.m_DofBounds[index];
		}

		virtual	CqColor Color( TqInt iXPos, TqInt iYPos );
		virtual	CqColor Opacity( TqInt iXPos, TqInt iYPos );
		virtual	TqFloat Coverage( TqInt iXPos, TqInt iYPos );
		virtual	TqFloat Depth( TqInt iXPos, TqInt iYPos );
		virtual	const TqFloat* Data( TqInt iXPos, TqInt iYPos );

		static	void	PrepareBucket( TqInt xorigin, TqInt yorigin, TqInt xsize, TqInt ysize, bool fJitter = true, bool empty = false );
		static	void	CalculateDofBounds();
		static	void	InitialiseFilterValues();
		static	void	ImageElement( TqInt iXPos, TqInt iYPos, CqImagePixel*& pie )
		{
			iXPos -= m_bucketData.m_XOrigin;
			iYPos -= m_bucketData.m_YOrigin;

			// Check within renderable range
			//assert( iXPos < -m_XMax && iXPos < m_XSize + m_XMax &&
			//		iYPos < -m_YMax && iYPos < m_YSize + m_YMax );

			TqInt i = ( ( iYPos + m_bucketData.m_DiscreteShiftY ) * ( m_bucketData.m_RealWidth ) ) + ( iXPos + m_bucketData.m_DiscreteShiftX );
			pie = &m_bucketData.m_aieImage[ i ];
		}
		static CqImagePixel& ImageElement(TqInt index)
		{
			assert(index < m_bucketData.m_aieImage.size());
			return(m_bucketData.m_aieImage[index]);
		}

		static	std::vector<SqSampleData>& SamplePoints()
		{
			return(m_bucketData.m_SamplePoints);
		}

		static TqInt GetNextSamplePointIndex()
		{
			TqInt index = m_bucketData.m_NextSamplePoint;
			m_bucketData.m_NextSamplePoint++;
			return(index);
		}

		void	CombineElements(enum EqFilterDepth eDepthFilter, CqColor zThreshold);
		void	FilterBucket(bool empty, bool fImager);
		void	ExposeBucket();
		void	QuantizeBucket();
		static	void	ShutdownBucket();

		/** Add a GPRim to the stack of deferred GPrims.
		* \param The Gprim to be added.
		 */
		void	AddGPrim( const boost::shared_ptr<CqSurface>& pGPrim )
		{
			m_gPrims.push(pGPrim);
		}

		/** Add an MPG to the list of deferred MPGs.
		 */
		void	AddMPG( CqMicroPolygon* pmpgNew )
		{
#ifdef _DEBUG
			std::vector<CqMicroPolygon*>::iterator end = m_micropolygons.end();
			for (std::vector<CqMicroPolygon*>::iterator i = m_micropolygons.begin(); i != end; i++)
				if ((*i) == pmpgNew)
					assert( false );
#endif

			m_micropolygons.push_back( pmpgNew );
		}
		/** Add a Micropoly grid to the list of deferred grids.
		 */
		void	AddGrid( CqMicroPolyGridBase* pgridNew )
		{
#ifdef _DEBUG
			std::vector<CqMicroPolyGridBase*>::iterator end = m_grids.end();
			for (std::vector<CqMicroPolyGridBase*>::iterator i = m_grids.begin(); i != end; i++)
				if ((*i) == pgridNew)
					assert( false );
#endif

			m_grids.push_back( pgridNew );
		}
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
		TqInt cGPrims()
		{
			return ( m_gPrims.size() );
		}
		/** Get a reference to the vector of deferred MPGs.
		 */
		std::vector<CqMicroPolygon*>& aMPGs()
		{
			return ( m_micropolygons );
		}
		/** Get a reference to the vector of deferred grids.
		 */
		std::vector<CqMicroPolyGridBase*>& aGrids()
		{
			return ( m_grids );
		}
		/** Get the flag that indicates whether the bucket is
		 * empty.  It is empty only when this bucket doesn't
		 * contain any surface, micropolygon or grids.
		 */
		bool IsEmpty();
		/** Get the flag that indicates if the bucket has been processed yet.
		 */
		bool IsProcessed() const
		{
			return( m_bucketData.m_bProcessed );
		}

		/** Mark this bucket as processed
		 */
		void SetProcessed( bool bProc =  true)
		{
			m_bucketData.m_bProcessed = bProc;
		}
		/** Set the pointer to the image buffer
		 */
		static void SetImageBuffer( CqImageBuffer* pBuffer )
		{
			m_ImageBuffer = pBuffer;
		}

		/** Render a particular micropolygon.
		 *    
		 * \param pMPG Pointer to the micropolygon to process.
		 *
		 * \param xmin Integer minimum extend of the image
		 * part being rendered, takes into account buckets and
		 * clipping.
		 *
		 * \param xmax Integer maximum extend of the image
		 * part being rendered, takes into account buckets and
		 * clipping.
		 *
		 * \param ymin Integer minimum extend of the image
		 * part being rendered, takes into account buckets and
		 * clipping.
		 *
		 * \param ymax Integer maximum extend of the image
		 * part being rendered, takes into account buckets and
		 * clipping.
		 *
		 * \see CqBucket, CqImagePixel
		 */
		void	RenderMicroPoly( CqMicroPolygon* pMPG, long xmin, long xmax, long ymin, long ymax );
		/** This function assumes that either dof or mb or
		 * both are being used. */
		void	RenderMPG_MBOrDof( CqMicroPolygon* pMPG, long xmin, long xmax, long ymin, long ymax, bool IsMoving, bool UsingDof );
		/** This function assumes that neither dof or mb are
		 * being used. It is much simpler than the general
		 * case dealt with above. */
		void	RenderMPG_Static( CqMicroPolygon* pMPG, long xmin, long xmax, long ymin, long ymax );


	private:
		/// Pointer to the image buffer this bucket belongs to.
		static	CqImageBuffer*	m_ImageBuffer;

		/// Dynamic bucket data
		static CqBucketData m_bucketData;

		// this is a compare functor for sorting surfaces in order of depth.
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

		std::vector<CqMicroPolygon*> m_micropolygons;			///< Vector of vectors of waiting micropolygons in this bucket
		std::vector<CqMicroPolyGridBase*> m_grids;		///< Vector of vectors of waiting micropolygrids in this bucket

		/// A sorted list of primitives for this bucket
		std::priority_queue<boost::shared_ptr<CqSurface>, std::deque<boost::shared_ptr<CqSurface> >, closest_surface> m_gPrims;
}
;

END_NAMESPACE( Aqsis )

//}  // End of #ifdef BUCKET_H_INCLUDED
#endif
