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
#include    "imagepixel.h"

START_NAMESPACE( Aqsis )

//-----------------------------------------------------------------------
/** Class holding data about a particular bucket.
 */

class CqBucket : public IqBucket
{
	public:
		CqBucket() : m_bProcessed( false )
		{}
		CqBucket( const CqBucket& From )
		{
			*this = From;
		}
		virtual ~CqBucket();

		CqBucket& operator=( const CqBucket& From )
		{
			m_ampgWaiting = From.m_ampgWaiting;
			m_agridWaiting = From.m_agridWaiting;
			m_bProcessed = From.m_bProcessed;

			return ( *this );
		}

		// Overridden from IqBucket
		virtual	TqInt	Width() const
		{
			return ( m_XSize );
		}
		virtual	TqInt	Height() const
		{
			return ( m_YSize );
		}
		virtual	TqInt	RealWidth() const
		{
			return ( m_RealWidth );
		}
		virtual	TqInt	RealHeight() const
		{
			return ( m_RealHeight );
		}
		virtual	TqInt	XOrigin() const
		{
			return ( m_XOrigin );
		}
		virtual	TqInt	YOrigin() const
		{
			return ( m_YOrigin );
		}
		static	TqInt	PixelXSamples()
		{
			return m_PixelXSamples;
		}
		static	TqInt	PixelYSamples()
		{
			return m_PixelYSamples;
		}
		static	TqFloat	FilterXWidth()
		{
			return m_FilterXWidth;
		}
		static	TqFloat	FilterYWidth()
		{
			return m_FilterYWidth;
		}
		static	TqInt	NumTimeRanges()
		{
			return m_NumTimeRanges;
		}
		static	TqInt	NumDofBounds()
		{
			return m_NumDofBounds;
		}

		static const CqBound& DofSubBound(TqInt index)
		{
			assert(index < m_NumDofBounds);
			return m_DofBounds[index];
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
			iXPos -= m_XOrigin;
			iYPos -= m_YOrigin;

			// Check within renderable range
			//assert( iXPos < -m_XMax && iXPos < m_XSize + m_XMax &&
			//		iYPos < -m_YMax && iYPos < m_YSize + m_YMax );

			TqInt i = ( ( iYPos + m_DiscreteShiftY ) * ( m_RealWidth ) ) + ( iXPos + m_DiscreteShiftX );
			pie = &m_aieImage[ i ];
		}
		static CqImagePixel& ImageElement(TqInt index)
		{
			assert(index < m_aieImage.size());
			return(m_aieImage[index]);
		}

		static	std::vector<SqSampleData>& SamplePoints()
		{
			return(m_SamplePoints);
		}

		static TqInt GetNextSamplePointIndex()
		{
			TqInt index = m_NextSamplePoint;
			m_NextSamplePoint++;
			return(index);
		}

		static	void	CombineElements(enum EqFilterDepth eDepthFilter, CqColor zThreshold);
		void	FilterBucket(bool empty);
		void	ExposeBucket();
		void	QuantizeBucket();
		static	void	ShutdownBucket();

		/** Add a GPRim to the stack of deferred GPrims.
		* \param The Gprim to be added.
		 */
		void	AddGPrim( const boost::shared_ptr<CqSurface>& pGPrim )
		{
			m_aGPrims.push(pGPrim);
		}

		/** Add an MPG to the list of deferred MPGs.
		 */
		void	AddMPG( CqMicroPolygon* pmpgNew )
		{
#ifdef _DEBUG
			std::vector<CqMicroPolygon*>::iterator end = m_ampgWaiting.end();
			for (std::vector<CqMicroPolygon*>::iterator i = m_ampgWaiting.begin(); i != end; i++)
				if ((*i) == pmpgNew)
					assert( false );
#endif

			m_ampgWaiting.push_back( pmpgNew );
		}
		/** Add a Micropoly grid to the list of deferred grids.
		 */
		void	AddGrid( CqMicroPolyGridBase* pgridNew )
		{
#ifdef _DEBUG
			std::vector<CqMicroPolyGridBase*>::iterator end = m_agridWaiting.end();
			for (std::vector<CqMicroPolyGridBase*>::iterator i = m_agridWaiting.begin(); i != end; i++)
				if ((*i) == pgridNew)
					assert( false );
#endif

			m_agridWaiting.push_back( pgridNew );
		}
		/** Get a pointer to the top GPrim in the stack of deferred GPrims.
		 */
		boost::shared_ptr<CqSurface> pTopSurface()
		{
			if (!m_aGPrims.empty())
			{
				return m_aGPrims.top();
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
			m_aGPrims.pop();
		}
		/** Get a count of deferred GPrims.
		 */
		TqInt cGPrims()
		{
			return ( m_aGPrims.size() );
		}
		/** Get a reference to the vector of deferred MPGs.
		 */
		std::vector<CqMicroPolygon*>& aMPGs()
		{
			return ( m_ampgWaiting );
		}
		/** Get a reference to the vector of deferred grids.
		 */
		std::vector<CqMicroPolyGridBase*>& aGrids()
		{
			return ( m_agridWaiting );
		}
		/** Get the flag that indicates if the bucket has been processed yet.
		 */
		bool IsProcessed() const
		{
			return( m_bProcessed );
		}

		/** Mark this bucket as processed
		 */
		void SetProcessed( bool bProc =  true)
		{
			m_bProcessed = bProc;
		}
		/** Set the pointer to the image buffer
		 */
		static void SetImageBuffer( CqImageBuffer* pBuffer )
		{
			m_ImageBuffer = pBuffer;
		}


	private:
		static	TqInt	m_XOrigin;		///< Origin in discrete coordinates of this bucket.
		static	TqInt	m_YOrigin;		///< Origin in discrete coordinates of this bucket.
		static	TqInt	m_XSize;		///< Size of the rendered area of this bucket in discrete coordinates.
		static	TqInt	m_YSize;		///< Size of the rendered area of this bucket in discrete coordinates.
		static	TqInt	m_RealWidth;	///< Actual size of the data for this bucket including filter overlap.
		static	TqInt	m_RealHeight;	///< Actual size of the data for this bucket including filter overlap.
		static	TqInt	m_DiscreteShiftX;	///<
		static	TqInt	m_DiscreteShiftY;
		static	TqInt	m_PixelXSamples;
		static	TqInt	m_PixelYSamples;
		static	TqFloat	m_FilterXWidth;
		static	TqFloat	m_FilterYWidth;
		static	TqInt	m_NumTimeRanges;
		static	TqInt	m_NumDofBounds;
		static	std::vector<CqBound>		m_DofBounds;
		static	std::vector<CqImagePixel>	m_aieImage;
		static	std::vector<SqSampleData>	m_SamplePoints;
		static	TqInt	m_NextSamplePoint;
		static	std::vector<std::vector<CqVector2D> >	m_aSamplePositions;///< Vector of vectors of jittered sample positions precalculated.
		static	std::vector<TqFloat>	m_aFilterValues;				///< Vector of filter weights precalculated.
		static	std::vector<TqFloat>	m_aDatas;
		static	std::vector<TqFloat>	m_aCoverages;
		static	CqImageBuffer*	m_ImageBuffer;	///< Pointer to the image buffer this bucket belongs to.

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

		std::vector<CqMicroPolygon*> m_ampgWaiting;			///< Vector of vectors of waiting micropolygons in this bucket
		std::vector<CqMicroPolyGridBase*> m_agridWaiting;		///< Vector of vectors of waiting micropolygrids in this bucket

		/// A sorted list of primitives for this bucket
		std::priority_queue<boost::shared_ptr<CqSurface>, std::deque<boost::shared_ptr<CqSurface> >, closest_surface> m_aGPrims;
		bool	m_bProcessed;	///< Flag indicating if this bucket has been processed yet.
}
;

END_NAMESPACE( Aqsis )

//}  // End of #ifdef BUCKET_H_INCLUDED
#endif
