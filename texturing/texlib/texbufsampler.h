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

/**
 * \file
 *
 * \brief A class for performing filtering on simple "flat" texture buffers.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 */

#ifndef TEXBUFSAMPLER_H_INCLUDED
#define TEXBUFSAMPLER_H_INCLUDED

#include "aqsis.h"

#include <boost/scoped_array.hpp>

#include "filtersupport.h"
#include "wrapmode.h"

namespace Aqsis {

//------------------------------------------------------------------------------
/** \class SupportIteratorConcept
 * \brief Iterator concept for the points in a filter support.
 *
 * In order to filter over some support region of a raster texture, we need to
 * generate points inside that region at which the texture will be sampled and
 * accumlated.  SupportIteratorConcept is an abstraction of the process of
 * generating these points for a given support.
 *
 * The simplest support iterator simply generates a sequence of which covers
 * *all* the points inside the support.  However, we sometimes can't afford to
 * use so many points (eg, for shadow maps).  In the case that we intend to
 * skip some points for efficiency, we can implement a stochastic support
 * iterator which takes some well-stratified subset of these points.
 *
 * Any realization of SupportIteratorConcept is required to have the following
 * methods:
 *
 * \code
 *
 * // Go to next point in the support.
 * SupportIteratorConcept& operator++()
 *
 * // Reset the iterator to the beginning of the supplied support.
 * void reset(const SqFilterSupport& support);
 *
 * // Determine if the current point is still validly in the support.
 * bool inSupport() const
 * 
 * // Get position of current point
 * TqInt x() const
 * TqInt y() const
 *
 * \endcode
 *
 */

//------------------------------------------------------------------------------
/** \brief A deterministic support iterator.
 *
 * CqSimpleSuppIter implements the SupportIteratorConcept in the
 * simplest possible way: by iterating over the entire support region.  The
 * iteration order is chosen to be cache-efficient for accessing row major
 * arrays: the x-coordinate varies fastest.
 *
 * In other words, it implements the following loop:
 *
 * \code
 *
 * for(TqInt y = support.sy.start; y < support.sy.end; ++y)
 * {
 *     for(TqInt x = support.sx.start; x < support.sx.end; ++x)
 *         // do something.
 * }
 *
 * \endcode
 */
class CqSimpleSuppIter
{
	public:
		/** \brief Construct a support iterator at the beginning of the support
		 *
		 * \param support - region to iterate over.
		 */
		CqSimpleSuppIter();

		/// Reset the iterator to the beginning of the supplied support.
		void reset(const SqFilterSupport& support);

		/// Move to the next support position
		CqSimpleSuppIter& operator++();

		/// Determine whether the current point is still in the support.
		bool inSupport() const;

		/// Return the current x-position.
		TqInt x() const;
		/// Return the current y-position.
		TqInt y() const;

	private:
		SqFilterSupport m_support;
		TqInt m_x;
		TqInt m_y;
};


//------------------------------------------------------------------------------
/** \brief A class to sample data held in a simple buffers such as CqTextureBuffer
 *
 * This class supplies functions for performing filtering of data held in a
 * simple texture buffer like CqTextureBuffer.
 *
 * Type params:
 *
 * ArrayT - this is an array/buffer type which must have methods width(),
 * height(), numChannels(), and 2D indexing via operator().  The vector type
 * which results from ArrayT::operator() must return float samples when indexed
 * with operator[].
 *
 * SupportIterT - an type implementing the SupportIteratorConcept which knows
 * how to generate points within a given filter support.
 */
template<typename ArrayT, typename SupportIterT = CqSimpleSuppIter>
class AQSISTEX_SHARE CqTexBufSampler
{
	public:
		/** \brief Construct a texture buffer sampler.
		 *
		 * \param buf - reference to the buffer to be sampled.
		 * \param supportIter - iterator which will provide the points to
		 *                      sample at during integration over a filter
		 *                      support.
		 */
		CqTexBufSampler(const ArrayT& buf,
				const SupportIterT& supportIter = SupportIterT());

		/// Get the underlying texture buffer.
		const ArrayT& buffer() const;

		/** \brief Filter the texture by accumulating pixels a given filter support.
		 *
		 * \param sampleAccum - accumulator for sample data.  Must implement
		 *                      SampleAccumulatorConcept
		 * \param supp - iterator over support region
		 * \param xWrapMode - wrap behaviour at the x-boundaries of the buffer
		 * \param yWrapMode - wrap behaviour at the y-boundaries of the buffer
		 */
		template<typename SampleAccumT>
		void applyFilter(SampleAccumT& sampleAccum, const SqFilterSupport& support,
				EqWrapMode xWrapMode, EqWrapMode yWrapMode);

	private:
		/** \brief Apply a filter on the internal part of the buffer
		 *
		 * \param sampleAccum - accumulator for samples in the support
		 * \param supp - iterator over support region; assumed to supply points
		 *               which are fully contained within the image!
		 */
		template<typename SampleAccumT>
		void applyFilterInternal(SampleAccumT& sampleAccum, SupportIterT& supp);
		/** \brief Apply a filter to the buffer near the edge
		 *
		 * \param sampleAccum - accumulator for samples in the support
		 * \param supp - iterator over support region.  may provide points
		 *               (partially) outside the texture boundaries.
		 * \param xWrapMode
		 * \param yWrapMode - wrap modes for texture coordinates off the image
		 *                    edge.
		 */
		template<typename SampleAccumT>
		void applyFilterBoundary(SampleAccumT& sampleAccum, SupportIterT& supp,
				EqWrapMode xWrapMode, EqWrapMode yWrapMode);

		/// Underlying array to sample
		const ArrayT& m_pixelBuf;
		/// Iterator moving over the filter support.
		SupportIterT m_supportIter;
};



//==============================================================================
// Implementation details
//==============================================================================

// CqSimpleSuppIter
inline CqSimpleSuppIter::CqSimpleSuppIter()
	: m_support(),
	m_x(0),
	m_y(0)
{ }

inline void CqSimpleSuppIter::reset(const SqFilterSupport& support)
{
	m_support = support;
	m_x = support.sx.start;
	m_y = support.sy.start;
}

inline CqSimpleSuppIter& CqSimpleSuppIter::operator++()
{
	++m_x;
	if(m_x >= m_support.sx.end)
	{
		m_x = m_support.sx.start;
		++m_y;
	}
	return *this;
}

inline bool CqSimpleSuppIter::inSupport() const
{
	return m_y < m_support.sy.end;
}

inline TqInt CqSimpleSuppIter::x() const
{
	return m_x;
}

inline TqInt CqSimpleSuppIter::y() const
{
	return m_y;
}

//------------------------------------------------------------------------------
// CqTexBufSampler implementation

template<typename ArrayT, typename SupportIterT>
CqTexBufSampler<ArrayT, SupportIterT>::CqTexBufSampler(const ArrayT& buf,
		const SupportIterT& supportIter)
	: m_pixelBuf(buf),
	m_supportIter(supportIter)
{ }

template<typename ArrayT, typename SupportIterT>
inline const ArrayT& CqTexBufSampler<ArrayT, SupportIterT>:: buffer() const
{
	return m_pixelBuf;
}

template<typename ArrayT, typename SupportIterT>
template<typename SampleAccumT>
void CqTexBufSampler<ArrayT, SupportIterT>::applyFilter(SampleAccumT& sampleAccum,
		const SqFilterSupport& support, EqWrapMode xWrapMode, EqWrapMode yWrapMode)
{
	sampleAccum.setSampleVectorLength(m_pixelBuf.numChannels());
	if( support.inRange(0, m_pixelBuf.width(), 0, m_pixelBuf.height()) )
	{
		// The bounds for the filter support are all inside the texture; do
		// simple and efficient filtering.
		m_supportIter.reset(support);
		applyFilterInternal(sampleAccum, m_supportIter);
	}
	else
	{
		SqFilterSupport modifiedSupport(support);
		// If we get here, the filter support falls at least partially outside
		// the texture range.
		if(xWrapMode == WrapMode_Trunc)
		{
			modifiedSupport.sx.truncate(0, m_pixelBuf.width());
			if(modifiedSupport.sx.isEmpty())
				return;
		}
		if(yWrapMode == WrapMode_Trunc)
		{
			modifiedSupport.sy.truncate(0, m_pixelBuf.height());
			if(modifiedSupport.sy.isEmpty())
				return;
		}
		// Apply a filter using careful boundary checking.
		m_supportIter.reset(modifiedSupport);
		applyFilterBoundary(sampleAccum, m_supportIter, xWrapMode, yWrapMode);
	}
}

template<typename ArrayT, typename SupportIterT>
template<typename SampleAccumT>
void CqTexBufSampler<ArrayT, SupportIterT>::applyFilterInternal(
		SampleAccumT& sampleAccum, SupportIterT& supp)
{
	while(supp.inSupport())
	{
		sampleAccum.accumulate(supp.x(), supp.y(), m_pixelBuf(supp.x(), supp.y()));
		++supp;
	}
}


namespace detail {

inline TqInt wrapCoord(TqInt x, TqInt width, EqWrapMode wrapMode)
{
	switch(wrapMode)
	{
		case WrapMode_Black:
			if(x < 0 || x >= width)
				return -1;
			break;
		case WrapMode_Periodic:
			// the factor of 100 is a bit of a hack to try to make sure the
			// returned x is always positive.  As long as we restrict to filter
			// widths less than 100, this should do the trick...
			return (x + 100*width) % width;
		case WrapMode_Clamp:
			return clamp(x, 0, width-1);
		default:
			break;
	}
	return x;
}

} // namespace detail


template<typename ArrayT, typename SupportIterT>
template<typename SampleAccumT>
void CqTexBufSampler<ArrayT, SupportIterT>::applyFilterBoundary(
		SampleAccumT& sampleAccum, SupportIterT& supp,
		EqWrapMode xWrapMode, EqWrapMode yWrapMode)
{
	boost::scoped_array<float> blackBuf;
	bool haveBlackWrapMode = (xWrapMode == WrapMode_Black | yWrapMode == WrapMode_Black);
	if(haveBlackWrapMode)
	{
		blackBuf.reset(new float[m_pixelBuf.numChannels()]);
		for(int i = 0; i < m_pixelBuf.numChannels(); ++i)
			blackBuf[i] = 0;
	}
	while(supp.inSupport())
	{
		TqInt yWrap = detail::wrapCoord(supp.y(), m_pixelBuf.height(), yWrapMode);
		TqInt xWrap = detail::wrapCoord(supp.x(), m_pixelBuf.width(), xWrapMode);
		if(haveBlackWrapMode && (yWrap == -1 || xWrap == -1))
			sampleAccum.accumulate(supp.x(), supp.y(), blackBuf);
		else
			sampleAccum.accumulate(supp.x(), supp.y(), m_pixelBuf(xWrap, yWrap));
		++supp;
	}
}


//------------------------------------------------------------------------------
}

#endif // TEXBUFSAMPLER_H_INCLUDED
