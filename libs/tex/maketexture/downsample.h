// Aqsis
// Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the software's owners nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// (This is the New BSD license)

/** \file
 *
 * \brief Classes and functions for creating mipmaps
 *
 * \author Chris Foster  [chris42f _at_ gmail.com]
 */

#ifndef DOWNSAMPLE_H_INCLUDED
#define DOWNSAMPLE_H_INCLUDED

#include <aqsis/aqsis.h>

#include <boost/shared_ptr.hpp>

#include <aqsis/math/math.h>
#include "cachedfilter.h"
#include <aqsis/tex/filtering/sampleaccum.h>
#include <aqsis/tex/filtering/filtertexture.h>
#include <aqsis/tex/buffers/texturebuffer.h>
#include <aqsis/tex/filtering/wrapmode.h>

namespace Aqsis
{

//------------------------------------------------------------------------------
/** \brief Input iterator for creating a sequence of downsampled images for
 * mipmapping.
 *
 * To create each downsampled image level in the mipmap, downsample() is called
 * on the previous image level to reduce the size by a factor of 2.  The buffer
 * is set to null when the size of the previous buffer reaches 1x1.
 */
template<typename ArrayT>
class CqDownsampleIterator
{
	public:
		/** \brief Construct the null downsampler iterator
		 *
		 * The null downsampler iterator is to be used as the "end" iterator in
		 * the sequence generated from any input image, after the last image of
		 * dimensions 1x1.
		 */
		CqDownsampleIterator();
		/** \brief Construct a downsampler iterator
		 *
		 * \param buf - source buffer to downsample
		 * \param filterInfo - information about which filter type and size to use
		 * \param wrapModes - specifies how the texture will be wrapped at the edges.
		 */
		CqDownsampleIterator(boost::shared_ptr<ArrayT> buf,
				const SqFilterInfo& filterInfo, const SqWrapModes& wrapModes);
		/** \brief Advance to the next image in the sequence
		 *
		 * The current image is downsampled to obtain the next one.
		 */
		CqDownsampleIterator& operator++();
		/// Return the current buffer.
		const boost::shared_ptr<ArrayT>& operator*();
		/** \brief Test if this iterator is equal to another.
		 *
		 * Useful for testing the ending condition.  A default-constructed
		 * CqDownsampleIterator acts as the "end" iterator in a range.
		 */
		bool operator!=(const CqDownsampleIterator<ArrayT>& rhs);
	private:
		boost::shared_ptr<ArrayT> m_buf;
		SqFilterInfo m_filterInfo;
		SqWrapModes m_wrapModes;
};

/** \brief Downsample an image to the next smaller mipmap size.
 *
 * The size of the new image is ceil(width/2) x ceil(height/2).  This is one
 * possible choice, the other being to take the floor.  Neither of these is
 * obviously preferable; the essential problem is that non-power of two
 * textures aren't perfect for mipmapping, though they do a fairly good job.
 * In particular, it's not easy to efficiently and accurately use non power of
 * two textures with periodic wrapping.
 *
 * \param srcBuf - input texture buffer.
 * \param filterInfo - information about which filter type and size to use
 * \param wrapModes - specifies how the texture will be wrapped at the edges.
 */
template<typename ArrayT>
boost::shared_ptr<ArrayT> downsample(const ArrayT& srcBuf,
		const SqFilterInfo& filterInfo, const SqWrapModes& wrapModes);



//==============================================================================
// Implementation details
//==============================================================================
// CqDownsampleIterator implementation
template<typename ArrayT>
CqDownsampleIterator<ArrayT>::CqDownsampleIterator()
	: m_buf(),
	m_filterInfo(),
	m_wrapModes()
{ }

template<typename ArrayT>
CqDownsampleIterator<ArrayT>::CqDownsampleIterator(boost::shared_ptr<ArrayT> buf,
		const SqFilterInfo& filterInfo, const SqWrapModes& wrapModes)
	: m_buf(buf),
	m_filterInfo(filterInfo),
	m_wrapModes(wrapModes)
{ }

template<typename ArrayT>
CqDownsampleIterator<ArrayT>& CqDownsampleIterator<ArrayT>::operator++()
{
	if(!m_buf)
		return *this;
	if(m_buf->width() > 1 || m_buf->height() > 1)
		m_buf = downsample(*m_buf, m_filterInfo, m_wrapModes);
	else
		m_buf.reset();
	return *this;
}

template<typename ArrayT>
const boost::shared_ptr<ArrayT>& CqDownsampleIterator<ArrayT>::operator*()
{
	return m_buf;
}

template<typename ArrayT>
bool CqDownsampleIterator<ArrayT>::operator!=(const CqDownsampleIterator<ArrayT>& rhs)
{
	return m_buf != rhs.m_buf;
}


//------------------------------------------------------------------------------
// free functions implementation

namespace detail {

/** \brief Downsample a buffer for mipmapping via a nonseperable convolution.
 *
 * Nonseperable convolution is the most general way of forming a weighted
 * average during filtering.  A seperable convolution should be faster, but is
 * more complicated to implement.
 *
 * \param srcBuf - input texture buffer.
 * \param mipmapRatio - scale factor for the new file (0.5 for normal mipmapping)
 * \param filterWeights - precomputed kernel of filter weights
 * \param wrapModes - specify how the texture will be wrapped at the edges.
 */
template<typename ArrayT>
boost::shared_ptr<ArrayT> downsampleNonseperable(
		const ArrayT& srcBuf, TqInt mipmapRatio,
		CqCachedFilter& filterWeights, const SqWrapModes& wrapModes)
{
	TqInt newWidth = lceil(TqFloat(srcBuf.width())/mipmapRatio);
	TqInt newHeight = lceil(TqFloat(srcBuf.height())/mipmapRatio);
	TqInt numChannels = srcBuf.numChannels();
	boost::shared_ptr<ArrayT> destBuf(new ArrayT(newWidth, newHeight, numChannels));
	TqInt filterOffsetX = (filterWeights.width()-1) / 2;
	TqInt filterOffsetY = (filterWeights.height()-1) / 2;
	std::vector<TqFloat> accumBuf(numChannels);
	// Loop over pixels in the output image.
	for(TqInt y = 0; y < newHeight; ++y)
	{
		for(TqInt x = 0; x < newWidth; ++x)
		{
			// Filter the source buffer to get the channels for a single pixel
			// in the destination buffer.
			filterWeights.setSupportTopLeft(2*x-filterOffsetX, 2*y-filterOffsetY);
			CqSampleAccum<CqCachedFilter> accumulator(filterWeights, 0, numChannels, &accumBuf[0]);
			filterTexture(accumulator, srcBuf, filterWeights.support(),
					SqWrapModes(wrapModes.sWrap, wrapModes.tWrap));
			destBuf->setPixel(x, y, &accumBuf[0]);
		}
	}
	return destBuf;
}

} // namespace detail


template<typename ArrayT>
boost::shared_ptr<ArrayT> downsample(const ArrayT& srcBuf,
		const SqFilterInfo& filterInfo, const SqWrapModes& wrapModes)
{
	// Amount to scale the image by.  Fixed at a factor of 2 for now.
	TqInt mipmapRatio = 2;
	TqFloat scale = 1.0f/mipmapRatio;

	/// \todo Optimize for seperable filters.  This should also give some idea
	/// about how to optimize the analogous filtering operations in the core renderer.
	//
	// if(FilterFunctorT::isSeperable()) ...

	// General case: Non-seperable filter.

	CqCachedFilter weights(filterInfo, srcBuf.width() % 2 != 0,
			srcBuf.height() % 2 != 0, scale);
	return detail::downsampleNonseperable(srcBuf, mipmapRatio, weights, wrapModes);
}

} // namespace Aqsis

#endif // DOWNSAMPLE_H_INCLUDED
