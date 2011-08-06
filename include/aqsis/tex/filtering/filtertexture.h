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

/**
 * \file
 *
 * \brief Facilities for applying filters to texture buffers.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 */

#ifndef FILTERTEXTURE_H_INCLUDED
#define FILTERTEXTURE_H_INCLUDED

#include <aqsis/aqsis.h>

#include <aqsis/util/autobuffer.h>
#include <aqsis/tex/buffers/filtersupport.h>
#include <aqsis/tex/filtering/wrapmode.h>

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief Filter a texture buffer over the supplied region.
 *
 * This function filters a texture buffer by iterating over all pixels in the
 * filter support region provided, and accumulating the pixel samples from that
 * region into the sample accumulator.  Wrap modes must be provided to
 * determine the behaviour for parts of the filter support which lie outside
 * the bounds of the texture.  Parts of the buffer is remapped onto these
 * regions when using the "periodic" or "clamp" wrap modes, while for the
 * "black" wrap mode, a pixel of black is accumulated for each pixel location
 * lying outside the filter support.
 *
 * SampleAccumT - A model of the SampleAccumulatorConcept.
 * ArrayT - A texture buffer array type.  This needs to have a pixel iterator
 *          of type ArrayT::TqIterator for filter support regions, accessible
 *          via a begin() method.  It also needs width(), height() and
 *          numChannels() methods.  See CqTextureBuffer for a model which
 *          satisfies the requirements.
 *
 * \todo Properly document the array concept which ArrayT must satisfy.
 *
 * \param sampleAccum - pixel samples from the support are accumulated into here.
 * \param buffer - texture buffer from which the samples will be obtained.
 * \param support - rectangular filter support region from which to accumulate
 *                  pixel samples.
 * \param wrapModes - specify how regions outside the buffer should be wrapped
 *                    back onto the valid region.
 */
template<typename SampleAccumT, typename ArrayT>
void filterTexture(SampleAccumT& sampleAccum, const ArrayT& buffer,
		const SqFilterSupport& support, const SqWrapModes wrapModes);

/** \brief Filter a texture without wrapping at the edges.
 *
 * This function works just like filterTexture(), except that instead of
 * wrapping the texture at the edges, the support is simply truncated to lie
 * wholly within the provided texture buffer.
 *
 * \see filterTexture() for more details.
 *
 * \param sampleAccum - pixel samples from the support are accumulated into here.
 * \param buffer - texture buffer from which the samples will be obtained.
 * \param support - rectangular filter support region from which to accumulate
 *                  pixel samples.
 */
template<typename SampleAccumT, typename ArrayT>
void filterTextureNowrap(SampleAccumT& sampleAccum, const ArrayT& buffer,
		const SqFilterSupport& support);

/** \brief Filter a texture stochastically without wrapping.
 *
 * Stochastic filtering of a texture is like normal filtering, except that not
 * all points inside filter support are considered.  Instead we choose a subset
 * of the points randomly.  This allows the time required for large filter
 * supports to be bounded by the time required to filter a fixed subset of the
 * filter points.
 *
 * \see filterTexture() for more details.
 *
 * \param sampleAccum - pixel samples from the support are accumulated into here.
 * \param buffer - texture buffer from which the samples will be obtained.
 * \param support - rectangular filter support region from which to accumulate
 *                  pixel samples.
 * \param numSamples - number of samples across the support to use.
 */
template<typename SampleAccumT, typename ArrayT>
void filterTextureNowrapStochastic(SampleAccumT& sampleAccum, const ArrayT& buffer,
		const SqFilterSupport& support, TqInt numSamples);



//==============================================================================
// Implementation details
//==============================================================================

namespace detail {

/** \brief Accumulate samples from a displaced buffer
 *
 * This function accumulates samples from a buffer which is displaced such that
 * the top left pixel is located at coordinates (tlX, tlY).  For this case, we
 * have to consider the texture wrap modes.
 *
 * \param sampleAccum - pixel samples from the support are accumulated into here.
 * \param buffer - texture buffer from which the samples will be obtained.
 * \param support - rectangular filter support region from which to accumulate
 *                  pixel samples.
 * \param wrapModes - specify how regions outside the buffer should be wrapped
 *                    back onto the valid region.
 * \param tlX - top left x-position of the buffer
 * \param tlY - top left y-position of the buffer
 */
template<typename SampleAccumT, typename ArrayT>
void filterWrappedBuffer(SampleAccumT& sampleAccum, const ArrayT& buffer,
		const SqFilterSupport& support, const SqWrapModes wrapModes,
		const TqInt tlX, const TqInt tlY)
{
	bool wrapX = tlX != 0;
	bool wrapY = tlY != 0;
	assert(wrapX || wrapY);
	// Construct the support of the current buffer tile.
	SqFilterSupport tileSupport = intersect(support, SqFilterSupport(
				tlX, tlX + buffer.width(), tlY, tlY + buffer.height()));
	// Select one of the possible wrap modes / wrapping combinations.
	if((wrapModes.sWrap == WrapMode_Black && wrapX)
			|| (wrapModes.tWrap == WrapMode_Black && wrapY))
	{
		// If the tile is in a black wrapmode region, accumulate black samples.
		CqAutoBuffer<TqFloat, 16> blackSamp(buffer.numChannels(), 0);
		for(TqInt ix = tileSupport.sx.start; ix < tileSupport.sx.end; ++ix)
			for(TqInt iy = tileSupport.sy.start; iy < tileSupport.sy.end; ++iy)
				sampleAccum.accumulate(ix, iy, blackSamp.get());
	}
	else if(wrapModes.sWrap == WrapMode_Clamp && wrapX)
	{
		if(wrapModes.tWrap == WrapMode_Clamp && wrapY)
		{
			// Both directions are clamped.  This requires accumulation of
			// a single corner pixel over the support.
			TqInt xClamp = clamp(tlX, 0, buffer.width()-1);
			TqInt yClamp = clamp(tlY, 0, buffer.height()-1);
			// sampVec is the samples for the corner pixel to be accumulated.
			typename ArrayT::TqSampleVector sampVec = *buffer.begin(SqFilterSupport(
						xClamp, xClamp+1, yClamp, yClamp+1));
			for(TqInt ix = tileSupport.sx.start; ix < tileSupport.sx.end; ++ix)
				for(TqInt iy = tileSupport.sy.start; iy < tileSupport.sy.end; ++iy)
					sampleAccum.accumulate(ix, iy, sampVec);
		}
		else
		{
			// clamped in x direction, but not in y direction.
			// Here we perform a normal iteration over y, but leave 
			TqInt xClamp = clamp(tlX, 0, buffer.width()-1);
			SqFilterSupport clampedSupport(SqFilterSupport1D(xClamp, xClamp+1), tileSupport.sy);
			for(typename ArrayT::TqIterator i = buffer.begin(clampedSupport);
					i.inSupport(); ++i)
			{
				for(TqInt ix = tileSupport.sx.start; ix < tileSupport.sx.end; ++ix)
					sampleAccum.accumulate(ix, i.y(), *i);
			}
		}
	}
	else if(wrapModes.tWrap == WrapMode_Clamp && wrapY)
	{
		// clamped in y direction, but not in x direction.  This is just an
		// inverted (and slightly duplicated :-/ ) version of the code above
		TqInt yClamp = clamp(tlY, 0, buffer.height()-1);
		SqFilterSupport clampedSupport(tileSupport.sx, SqFilterSupport1D(yClamp, yClamp+1));
		for(typename ArrayT::TqIterator i = buffer.begin(clampedSupport);
				i.inSupport(); ++i)
		{
			for(TqInt iy = tileSupport.sy.start; iy < tileSupport.sy.end; ++iy)
				sampleAccum.accumulate(i.x(), iy, *i);
		}
	}
	else
	{
		// The only cases left are periodic wrapping in at least one direction.
		// We treat these cases together.
		SqFilterSupport wrappedSupport(
				tileSupport.sx.start - tlX, tileSupport.sx.end - tlX,
				tileSupport.sy.start - tlY, tileSupport.sy.end - tlY);
		for(typename ArrayT::TqIterator i = buffer.begin(wrappedSupport);
				i.inSupport(); ++i)
		{
			sampleAccum.accumulate(i.x() + tlX, i.y() + tlY, *i);
		}
	}
}

} // namespace detail

template<typename SampleAccumT, typename ArrayT>
void filterTexture(SampleAccumT& sampleAccum, const ArrayT& buffer,
		const SqFilterSupport& support, const SqWrapModes wrapModes)
{
	if(!sampleAccum.setSampleVectorLength(buffer.numChannels()))
		return;

	// First accumulate samples across the part of the support which lies
	// inside the buffer.  Note that this may be empty; we could check for this
	// first with support.intersectsRange() if it helps performance...
	for(typename ArrayT::TqIterator i = buffer.begin(support); i.inSupport(); ++i)
		sampleAccum.accumulate(i.x(), i.y(), *i);

	// Next, if the support isn't wholly inside the buffer, we need to consider
	// the wrap modes.
	if(!support.inRange(0, buffer.width(), 0, buffer.height()))
	{
		// The texture buffer tiles the plane; we iterate over the tiles which
		// the filter support crosses and perform whatever wrapping operation
		// is necessary for each tile we get to.
		//
		// The "tiling" discussed here is with respect to texture wrapping, and
		// is unrelated to whether the underlying type (ArrayT) uses tiled
		// storage.
		TqInt x0 = lfloor(TqFloat(support.sx.start)/buffer.width()) * buffer.width();
		TqInt y0 = lfloor(TqFloat(support.sy.start)/buffer.height()) * buffer.height();
		// (tlX, tlY) is the top left corner of the translated buffer.
		for(TqInt tlX = x0; tlX < support.sx.end; tlX += buffer.width())
		{
			for(TqInt tlY = y0; tlY < support.sy.end; tlY += buffer.height())
			{
				// The special case of the non-translated source buffer, where
				// tlX = tlY == 0 is already dealt with in the first
				// iteration above.
				if(tlX == 0 && tlY == 0)
					continue;

				detail::filterWrappedBuffer(sampleAccum, buffer, support,
						wrapModes, tlX, tlY);
			}
		}
	}
}

template<typename SampleAccumT, typename ArrayT>
void filterTextureNowrap(SampleAccumT& sampleAccum, const ArrayT& buffer,
		const SqFilterSupport& support)
{
	if(!sampleAccum.setSampleVectorLength(buffer.numChannels()))
		return;
	// Accumulate samples across the part of the support which lies inside the
	// buffer only.  No wrapping is performed, so parts of the support outside
	// the buffer are simply truncated.
	for(typename ArrayT::TqIterator i = buffer.begin(support); i.inSupport(); ++i)
		sampleAccum.accumulate(i.x(), i.y(), *i);
}

template<typename SampleAccumT, typename ArrayT>
void filterTextureNowrapStochastic(SampleAccumT& sampleAccum, const ArrayT& buffer,
		const SqFilterSupport& support, TqInt numSamples)
{
	if(!sampleAccum.setSampleVectorLength(buffer.numChannels()))
		return;
	// Accumulate samples stochastically across the part of the support which
	// lies inside the buffer only.  No wrapping is performed, so parts of the
	// support outside the buffer are simply truncated.
	for(typename ArrayT::TqStochasticIterator i = buffer.beginStochastic(support,
				numSamples); i.inSupport(); ++i)
		sampleAccum.accumulate(i.x(), i.y(), *i);
}

} // namespace Aqsis

#endif // FILTERTEXTURE_H_INCLUDED
