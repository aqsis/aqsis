// Aqsis
// Copyright Î÷Î÷ 1997 - 2001, Paul C. Gregory
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
		\brief Declares the CqBucket class responsible for bookeeping the primitives and storing the results.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is imagebuffer.h included already?
#ifndef BUCKET_H_INCLUDED 
//{
#define BUCKET_H_INCLUDED 1

#include	"aqsis.h"

#include	<vector>

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
		CqBucket()
		{}
		CqBucket( const CqBucket& From )
		{
			*this = From;
		}
		virtual ~CqBucket()
		{}

		CqBucket& operator=( const CqBucket& From )
		{
			m_ampgWaiting = From.m_ampgWaiting;
			m_agridWaiting = From.m_agridWaiting;

			return ( *this );
		}

		// Overridden from IqBucket
		virtual	TqInt	XSize() const
		{
			return ( m_XSize );
		}
		virtual	TqInt	YSize() const
		{
			return ( m_YSize );
		}
		virtual	TqInt	XOrigin() const
		{
			return ( m_XOrigin );
		}
		virtual	TqInt	YOrigin() const
		{
			return ( m_YOrigin );
		}
		virtual	TqInt	XFWidth() const
		{
			return ( m_XFWidth );
		}
		virtual	TqInt	YFWidth() const
		{
			return ( m_YFWidth );
		}
		virtual	TqInt	XPixelSamples() const
		{
			return ( m_XPixelSamples );
		}
		virtual	TqInt	YPixelSamples() const
		{
			return ( m_YPixelSamples );
		}

		virtual	CqColor Color( TqInt iXPos, TqInt iYPos );
		virtual	CqColor Opacity( TqInt iXPos, TqInt iYPos );
		virtual	TqFloat Coverage( TqInt iXPos, TqInt iYPos );
		virtual	TqFloat Depth( TqInt iXPos, TqInt iYPos );
		virtual	TqFloat MaxDepth( TqInt iXPos, TqInt iYPos );

		static	void	InitialiseBucket( TqInt xorigin, TqInt yorigin, TqInt xsize, TqInt ysize, TqInt xfwidth, TqInt yfwidth, TqInt xsamples, TqInt ysamples, TqBool fJitter = TqTrue );
		static	void	InitialiseFilterValues();
		static	void	ImageElement( TqInt iXPos, TqInt iYPos, CqImagePixel*& pie )
		{
			iXPos -= m_XOrigin;
			iYPos -= m_YOrigin;

			// Check within renderable range
			//assert( iXPos < -m_XMax && iXPos < m_XSize + m_XMax &&
			//		iYPos < -m_YMax && iYPos < m_YSize + m_YMax );

			TqInt i = ( ( iYPos + m_YMax ) * ( m_XSize + m_XFWidth ) ) + ( iXPos + m_XMax );
			pie = &m_aieImage[ i ];
		}
		static	void	CombineElements();
		void	FilterBucket();
		void	ExposeBucket();
		void	QuantizeBucket();
		static	void	EmptyBucket();
		
		/** Add a GPRim to the list of deferred GPrims.
		 */
		void	AddGPrim( CqBasicSurface* pGPrim );

		/** Add an MPG to the list of deferred MPGs.
		 */
		void	AddMPG( CqMicroPolygon* pmpgNew )
		{
#ifdef _DEBUG
			std::vector<CqMicroPolygon*>::iterator end = m_ampgWaiting.end();
			for (std::vector<CqMicroPolygon*>::iterator i = m_ampgWaiting.begin(); i != end; i++)
				if ((*i) == pmpgNew) 
					assert( TqFalse );
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
					assert( TqFalse );
#endif
			m_agridWaiting.push_back( pgridNew );
		}
		/** Get a pointer to the top GPrim in the list of deferred GPrims.
		 */
		CqBasicSurface* pTopSurface()
		{
			return ( m_aGPrims.pFirst() );
		}
		/** Get a reference to the vetor of deferred MPGs.
		 */
		std::vector<CqMicroPolygon*>& aMPGs()
		{
			return ( m_ampgWaiting );
		}
		/** Get a reference to the vetor of deferred grids.
		 */
		std::vector<CqMicroPolyGridBase*>& aGrids()
		{
			return ( m_agridWaiting );
		}

	private:
		static	TqInt	m_XSize;
		static	TqInt	m_YSize;
		static	TqInt	m_XFWidth;
		static	TqInt	m_YFWidth;
		static	TqInt	m_XMax;
		static	TqInt	m_YMax;
		static	TqInt	m_XPixelSamples;
		static	TqInt	m_YPixelSamples;
		static	TqInt	m_XOrigin;
		static	TqInt	m_YOrigin;
		static	std::vector<CqImagePixel>	m_aieImage;
		static	std::vector<TqFloat>	m_aFilterValues;				///< Vector of filter weights precalculated.

		std::vector<CqMicroPolygon*> m_ampgWaiting;			///< Vector of vectors of waiting micropolygons in this bucket
		std::vector<CqMicroPolyGridBase*> m_agridWaiting;		///< Vector of vectors of waiting micropolygrids in this bucket
		CqList<CqBasicSurface>	m_aGPrims;						///< Vector of lists of split surfaces for this bucket.
}
;

END_NAMESPACE( Aqsis )

//}  // End of #ifdef BUCKET_H_INCLUDED
#endif
