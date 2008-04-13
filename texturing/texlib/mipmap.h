// Aqsis
// Copyright (C) 1997 - 2007, Paul C. Gregory
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
 *
 * \brief Classes and functions for creating mipmaps
 *
 * \author Chris Foster  [chris42f _at_ gmail.com]
 */

#ifndef MIPMAP_H_INCLUDED
#define MIPMAP_H_INCLUDED

#include "aqsis.h"

#include <boost/shared_ptr.hpp>

#include "aqsismath.h"
#include "cachedfilter.h"
#include "sampleaccum.h"
#include "texbufsampler.h"
#include "texturebuffer.h"
#include "wrapmode.h"

namespace Aqsis
{

//------------------------------------------------------------------------------
/** \brief Create a mipmap from the given texture buffer
 *
 * To create each downsampled image level in the mipmap, mipmapDownsample() is
 * called on the previous level to reduce the size by a factor of 2.  The
 * process terminates when the final level has dimensions 1x1.
 *
 * BufferDestT is a type which must have a single function, accept() which
 * accepts arrays of type ArrayT:
 * \code
 *   void accept(const ArrayT& buf)
 * \endcode
 * dest.accept() is called as each downsampled image is generated.
 *
 * \param srcBuf - input texture buffer.
 * \param dest - destination for downsampled images.
 * \param filterInfo - information about which filter type and size to use
 * \param wrapModes - specifies how the texture will be wrapped at the edges.
 */
template<typename ArrayT, typename BufferDestT>
void createMipmap(ArrayT& srcBuf, BufferDestT& dest,
		const SqFilterInfo& filterInfo, const SqWrapModes wrapModes);

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
boost::shared_ptr<ArrayT> mipmapDownsample(const ArrayT& srcBuf,
		const SqFilterInfo& filterInfo, const SqWrapModes& wrapModes);


//==============================================================================
// Implementation details
//==============================================================================
template<typename ArrayT, typename BufferDestT>
void createMipmap(ArrayT& srcBuf, BufferDestT& dest,
		const SqFilterInfo& filterInfo, const SqWrapModes wrapModes)
{
	boost::shared_ptr<ArrayT> buf(&srcBuf, nullDeleter);

	dest.accept(*buf);
	while(buf->width() > 1 || buf->height() > 1)
	{
		buf = mipmapDownsample(*buf, filterInfo, wrapModes);
		dest.accept(*buf);
	}
}

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
boost::shared_ptr<ArrayT> mipmapDownsampleNonseperable(
		const ArrayT& srcBuf, TqInt mipmapRatio,
		CqCachedFilter& filterWeights, const SqWrapModes& wrapModes)
{
	TqInt newWidth = lceil(TqFloat(srcBuf.width())/mipmapRatio);
	TqInt newHeight = lceil(TqFloat(srcBuf.height())/mipmapRatio);
	TqInt numChannels = srcBuf.numChannels();
	CqTexBufSampler<ArrayT> srcBufSampler(srcBuf);
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
			srcBufSampler.applyFilter(accumulator, filterWeights.support(),
					wrapModes.sWrap, wrapModes.tWrap );
			destBuf->setPixel(x, y, &accumBuf[0]);
		}
	}
	return destBuf;
}

} // namespace detail


template<typename ArrayT>
boost::shared_ptr<ArrayT> mipmapDownsample(const ArrayT& srcBuf,
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
	return detail::mipmapDownsampleNonseperable(srcBuf, mipmapRatio, weights, wrapModes);
}

} // namespace Aqsis

#endif // MIPMAP_H_INCLUDED
