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
 * \author Chris Foster
 */

#ifndef MIPMAP_H_INCLUDED
#define MIPMAP_H_INCLUDED

#include "aqsis.h"

#include <boost/shared_ptr.hpp>

#include "aqsismath.h"
#include "texbufsampler.h"
#include "wrapmode.h"
#include "cachedfilter.h"
#include "sampleaccum.h"

#include "logging.h" /// \todo debug: remove later

namespace Aqsis
{

//------------------------------------------------------------------------------
/** \brief Downsample an image to the next smaller mipmap size.
 *
 * The size of the new image is ceil(width/2) x ceil(height/2).  This is one
 * possible choice, the other being to take the floor.  Neither of these is
 * obviously preferable; the essential problem is that non-power of two
 * textures aren't perfect for mipmapping, though they do a fairly good job.
 *
 * \param
 */
template<typename FilterFunctorT, typename ChannelT>
boost::shared_ptr<CqTextureBuffer<ChannelT> > mipmapDownsample(
		const CqTextureBuffer<ChannelT>& inputBuf,
		TqFloat sWidth, TqFloat tWidth,
		EqWrapMode sWrapMode, EqWrapMode tWrapMode);

template<typename ChannelT>
boost::shared_ptr<CqTextureBuffer<ChannelT> > mipmapDownsampleNonseperable(
		const CqTextureBuffer<ChannelT>& srcBuf,
		CqCachedFilter& filterWeights,
		EqWrapMode sWrapMode, EqWrapMode tWrapMode);

//template<typename FilterFunctorT, typename ChannelT>
//boost::shared_ptr<CqTextureBuffer<ChannelT> > mipmapDownsampleSeperable(
//		const CqTextureBuffer<ChannelT>& srcBuf,
//		const std::vector<TqFloat>& filterWeights,
//		EqWrapMode sWrapMode, EqWrapMode tWrapMode);



//==============================================================================
// Implementation details

template<typename FilterFunctorT, typename ChannelT>
boost::shared_ptr<CqTextureBuffer<ChannelT> > mipmapDownsample(
		const CqTextureBuffer<ChannelT>& srcBuf,
		TqFloat sWidth, TqFloat tWidth,
		EqWrapMode sWrapMode, EqWrapMode tWrapMode)
{
	// Amount to scale the image by.  Fixed at a factor of 2 for now.
	TqFloat mipmapRatio = 2;
	TqFloat scale = 1.0f/mipmapRatio;

	/// \todo Optimize for seperable filters.  This should also give some idea
	/// about how to optimize the analogous filtering operations in the core renderer.
	//
	// if(FilterFunctorT::isSeperable) ...

	// General case: Non-seperable filter.
	CqCachedFilter weights(FilterFunctorT(sWidth*scale, tWidth*scale),
			sWidth, tWidth, srcBuf.width() % 2 != 0, srcBuf.height() % 2 != 0,
			scale);
	return mipmapDownsampleNonseperable(srcBuf, mipmapRatio, weights,
			sWrapMode, tWrapMode);
}

template<typename ChannelT>
boost::shared_ptr<CqTextureBuffer<ChannelT> > mipmapDownsampleNonseperable(
		const CqTextureBuffer<ChannelT>& srcBuf,
		TqInt mipmapRatio,
		CqCachedFilter& filterWeights,
		EqWrapMode sWrapMode, EqWrapMode tWrapMode)
{
	Aqsis::log() << filterWeights;
	TqInt newWidth = lceil(TqFloat(srcBuf.width())/mipmapRatio);
	TqInt newHeight = lceil(TqFloat(srcBuf.height())/mipmapRatio);
	TqInt numChannels = srcBuf.numChannels();
	CqTexBufSampler<ChannelT> srcBufSampler(srcBuf);
	boost::shared_ptr<CqTextureBuffer<ChannelT> > destBuf(
			new CqTextureBuffer<ChannelT>(newWidth, newHeight, numChannels) );
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
			srcBufSampler.applyFilter(accumulator, filterWeights.support(), sWrapMode, tWrapMode);
			destBuf->setPixel(x, y, &accumBuf[0]);
		}
	}
	return destBuf;
}

} // namespace Aqsis

#endif // MIPMAP_H_INCLUDED
