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

#include "wrapmode.h"
#include "filtersupport.h"

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief A class to sample data held in a simple buffers such as CqTextureBuffer
 *
 * This class supplies functions for performing filtering of data held in a
 * simple texture buffer like CqTextureBuffer.
 *
 * The array/buffer type (ArrayT) must have methods width(), height(),
 * numChannels(), and indexing via operator() which returns a vector producing
 * float samples.
 */
template<typename ArrayT>
class AQSISTEX_SHARE CqTexBufSampler
{
	public:
		/** \brief Construct a texture tile
		 *
		 * \param buf - reference to the buffer to be sampled.
		 */
		CqTexBufSampler(const ArrayT& buf);

		/// Get the underlying texture buffer.
		const ArrayT& buffer() const;

		/** \brief Filter the texture by accumulating pixels a given filter support.
		 *
		 * \param sampleAccum - accumulator for sample data.  Must implement
		 *                      SampleAccumulatorConcept
		 * \param support - support region to iterate over
		 * \param xWrapMode - wrap behaviour at the x-boundaries of the buffer
		 * \param yWrapMode - wrap behaviour at the y-boundaries of the buffer
		 */
		template<typename SampleAccumT>
		void applyFilter(SampleAccumT& sampleAccum,
				const SqFilterSupport& support,
				EqWrapMode xWrapMode, EqWrapMode yWrapMode) const;

	private:
		/** \brief Apply a filter on the internal part of the buffer
		 *
		 * \param sampleAccum - accumulator for samples in the support
		 * \param support - support for the filter; assumed to be fully
		 *                  contained withing the image!
		 */
		template<typename SampleAccumT>
		void applyFilterInternal(SampleAccumT& sampleAccum,
				const SqFilterSupport& support) const;
		/** \brief Apply a filter to the buffer near the edge
		 *
		 * \param sampleAccum - accumulator for samples in the support
		 * \param support - support for the filter; may lie (partially)
		 *                  outside the texture boundaries.
		 * \param xWrapMode
		 * \param yWrapMode - wrap modes for texture coordinates off the image
		 *                    edge.
		 */
		template<typename SampleAccumT>
		void applyFilterBoundary(SampleAccumT& sampleAccum,
				const SqFilterSupport& support,
				EqWrapMode xWrapMode, EqWrapMode yWrapMode) const;

		const ArrayT& m_pixelBuf;
};



//==============================================================================
// Implementation details
//==============================================================================
template<typename ArrayT>
CqTexBufSampler<ArrayT>::CqTexBufSampler(const ArrayT& buf)
	: m_pixelBuf(buf)
{ }

template<typename ArrayT>
inline const ArrayT& CqTexBufSampler<ArrayT>:: buffer() const
{
	return m_pixelBuf;
}

template<typename ArrayT>
template<typename SampleAccumT>
void CqTexBufSampler<ArrayT>::applyFilter(SampleAccumT& sampleAccum,
		const SqFilterSupport& support,
		EqWrapMode xWrapMode, EqWrapMode yWrapMode) const
{
	sampleAccum.setSampleVectorLength(m_pixelBuf.numChannels());
	if( support.inRange(0, m_pixelBuf.width(), 0, m_pixelBuf.height()) )
	{
		// The bounds for the filter support are all inside the texture; do
		// simple and efficient filtering.
		applyFilterInternal(sampleAccum, support);
	}
	else
	{
		SqFilterSupport modifiedSupport(support);
		// If we get here, the filter support falls at least partially outside
		// the texture range.
		if(xWrapMode == WrapMode_Black)
		{
			modifiedSupport.sx.truncate(0, m_pixelBuf.width());
			if(modifiedSupport.sx.isEmpty())
				return;
		}
		if(yWrapMode == WrapMode_Black)
		{
			modifiedSupport.sy.truncate(0, m_pixelBuf.height());
			if(modifiedSupport.sy.isEmpty())
				return;
		}
		// Apply a filter using careful boundary checking.
		applyFilterBoundary(sampleAccum, modifiedSupport, xWrapMode, yWrapMode);
	}
}

template<typename ArrayT>
template<typename SampleAccumT>
void CqTexBufSampler<ArrayT>::applyFilterInternal(SampleAccumT& sampleAccum,
		const SqFilterSupport& support) const
{
	for(TqInt y = support.sy.start; y < support.sy.end; ++y)
	{
		for(TqInt x = support.sx.start; x < support.sx.end; ++x)
			sampleAccum.accumulate(x, y, m_pixelBuf(x,y));
	}
}


namespace detail {

inline TqInt wrapCoord(TqInt x, TqInt width, EqWrapMode wrapMode)
{
	switch(wrapMode)
	{
		case WrapMode_Black:
			break;
		case WrapMode_Periodic:
			// the factor of 100 is a bit of a hack to try to make sure the
			// returned x is always positive.  As long as we restrict to filter
			// widths less than 100, this should do the trick...
			return (x + 100*width) % width;
		case WrapMode_Clamp:
			return clamp(x, 0, width-1);
	}
	return x;
}

} // namespace detail


template<typename ArrayT>
template<typename SampleAccumT>
void CqTexBufSampler<ArrayT>::applyFilterBoundary(SampleAccumT& sampleAccum,
		const SqFilterSupport& support,
		EqWrapMode xWrapMode, EqWrapMode yWrapMode) const
{
	for(TqInt y = support.sy.start; y < support.sy.end; ++y)
	{
		TqInt yWrap = detail::wrapCoord(y, m_pixelBuf.height(), yWrapMode);
		for(TqInt x = support.sx.start; x < support.sx.end; ++x)
		{
			TqInt xWrap = detail::wrapCoord(x, m_pixelBuf.width(), xWrapMode);
			sampleAccum.accumulate(x, y, m_pixelBuf(xWrap, yWrap) );
		}
	}
}


//------------------------------------------------------------------------------
}

#endif // TEXBUFSAMPLER_H_INCLUDED
