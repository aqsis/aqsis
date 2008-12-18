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

#include	"aqsis.h"

#include	<vector>
#include	<queue>
#include	<deque>

#include	"surface.h"
#include	"color.h"
#include	"imagepixel.h"
#include	"iddmanager.h"

namespace Aqsis {

//-----------------------------------------------------------------------
/** Class holding data about a particular bucket.
 */

class CqBucket
{
	public:
		CqBucket();

		virtual ~CqBucket();

		virtual const CqRegion& SRegion() const;
		virtual const CqRegion& DRegion() const;
		TqUint numSamples() const;
		TqInt	DiscreteShiftX() const
		{
			return m_DiscreteShiftX;
		}
		TqInt	DiscreteShiftY() const
		{
			return m_DiscreteShiftY;
		}
		TqInt	PixelXSamples() const
		{
			return m_PixelXSamples;
		}
		TqInt	PixelYSamples() const
		{
			return m_PixelYSamples;
		}
		TqFloat	FilterXWidth() const
		{
			return m_FilterXWidth;
		}
		TqFloat	FilterYWidth() const
		{
			return m_FilterYWidth;
		}
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
				       bool fJitter = true);

		/** Add a GPRim to the stack of deferred GPrims.
		* \param The Gprim to be added.
		 */
		void	AddGPrim( const boost::shared_ptr<CqSurface>& pGPrim )
		{
			m_gPrims.push(pGPrim);
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

		/** Add an MP to the list of deferred MPs.
		 */
		void	AddMP( boost::shared_ptr<CqMicroPolygon>& pMP );

		std::vector<boost::shared_ptr<CqMicroPolygon> >& micropolygons();

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

		/// Bucket classification information.
		CqRegion	m_SRegion;
		CqRegion	m_DRegion;

		TqInt	m_DiscreteShiftX;
		TqInt	m_DiscreteShiftY;
		TqInt	m_PixelXSamples;
		TqInt	m_PixelYSamples;
		TqFloat	m_FilterXWidth;
		TqFloat	m_FilterYWidth;


		/// Vector of vectors of waiting micropolygons in this bucket
		std::vector<boost::shared_ptr<CqMicroPolygon> > m_micropolygons;

		/// A sorted list of primitives for this bucket
		std::priority_queue<boost::shared_ptr<CqSurface>, std::deque<boost::shared_ptr<CqSurface> >, closest_surface> m_gPrims;
};


//------------------------------------------------------------
// Implementation details
//------------------------------------------------------------


inline TqUint CqBucket::numSamples() const
{
	return m_SRegion.area() * m_PixelXSamples * m_PixelYSamples;
}

inline const CqRegion& CqBucket::SRegion() const
{
	return m_SRegion;
}
inline const CqRegion& CqBucket::DRegion() const
{
	return m_DRegion;
}

inline std::vector<boost::shared_ptr<CqMicroPolygon> >& CqBucket::micropolygons()
{
	return m_micropolygons;
}

} // namespace Aqsis

//}  // End of #ifdef BUCKET_H_INCLUDED
#endif
