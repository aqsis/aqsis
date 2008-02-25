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
 * \brief Declare a multiresolution texture sampler
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef MIPMAPTEXTURESAMPLER_H_INCLUDED
#define MIPMAPTEXTURESAMPLER_H_INCLUDED

#include "aqsis.h"

#include <boost/shared_ptr.hpp>
#include <cmath>

#include "itexturesampler.h"
#include "mipmaplevelcache.h"
#include "ewafilter.h"
#include "texbufsampler.h" // remove when using tiled textures.
#include "texturebuffer.h" // remove when using tiled textures.
#include "sampleaccum.h"
#include "aqsismath.h"

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief A mulitresolution (mipmapped) texture sampler class
 *
 * This class uses a set of power-of-two downsampled images (a typical
 * mipmap) to make texture filtering efficient.  When filtering, a mipmap level
 * is chosen based on the extent of the filter quadrilateral.  Here we make a
 * choice which eliminates aliasing and blurring: we choose a mipmap level such
 * that the thinnest direction of the sampling quad falls across more than one
 * pixel.
 *
 * \todo: Investigate optimal mipmap level selection.
 */
template<typename T>
class AQSISTEX_SHARE CqMipmapTextureSampler : public IqTextureSampler
{
	private:
		typedef CqMipmapLevelCache<CqTextureBuffer<T> > TqCacheType;
	public:
		CqMipmapTextureSampler(const boost::shared_ptr<TqCacheType>& levels);

		// from IqTextureSampler
		virtual void sample(const SqSampleQuad& sampleQuad,
				const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const;
		virtual const CqTextureSampleOptions& defaultSampleOptions() const;
	private:
		boost::shared_ptr<TqCacheType> m_levels;
};

//==============================================================================
// Implementation details
//==============================================================================
namespace detail
{

/** Class for scaling filter weights by a fixed scale amount
 *
 * This is useful for interpolating between two mipmap levels;
 * we want something like
 *
 * result = (1-interp) * map1_accumulated_samples + interp * map2_accumulated_samples
 *
 * And this can be achieved by scaling the samples during accumulation by
 * setting the scale factor in CqScaledWeights to (1-interp) and interp for
 * map1 and map2 respectively.
 */
template<typename WeightsT>
class CqScaledWeights
{
	private:
		const WeightsT& m_weights;
		TqFloat m_scale;
	public:
		CqScaledWeights(const WeightsT& weights, TqFloat scale)
			: m_weights(weights),
			m_scale(scale)
		{ }
		TqFloat operator()(TqInt i, TqInt j) const
		{
			return m_scale*m_weights(i,j);
		}
		static bool isNormalized()
		{
			// Almost certainly not normalized.
			return false;
		}
		void setWeightScale(TqFloat scale)
		{
			m_scale = scale;
		}
};

} // namespace detail

//------------------------------------------------------------------------------
// CqMipmapTextureSampler implementation
template<typename T>
CqMipmapTextureSampler<T>::CqMipmapTextureSampler(
		const boost::shared_ptr<TqCacheType>& levels)
	: m_levels(levels)
{ }

template<typename T>
void CqMipmapTextureSampler<T>::sample(const SqSampleQuad& sampleQuad,
		const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	SqSampleQuad quad = sampleQuad;
	quad.scaleWidth(sampleOpts.sWidth(), sampleOpts.tWidth());
	quad.remapPeriodic(sampleOpts.sWrapMode() == WrapMode_Periodic,
			sampleOpts.tWrapMode() == WrapMode_Periodic);

	const CqTextureBuffer<T>& baseBuf = m_levels->level(0);
	// Construct weights
	CqEwaFilterWeights weights(quad, baseBuf.width(),
			baseBuf.height(), sampleOpts.sBlur(), sampleOpts.tBlur());
	bool usingBlur = sampleOpts.sBlur() != 0 || sampleOpts.tBlur() != 0;
	// Select mipmap level to use.
	//
	// The minimum filter width is the minimum number of pixels over which the
	// shortest length scale of the filter should extend.
	TqFloat minFilterWidth = 2;
	// Blur ratio ranges from 0 at no blur to 1 for a "lot" of blur.
	TqFloat blurRatio = 0;
	if(usingBlur)
	{
		// When using blur, the minimum filter width needs to be increased.
		//
		// Experiments show that for large blur factors minFilterWidth should
		// be about 4 for good results.
		TqFloat maxBlur = max(sampleOpts.sBlur()*baseBuf.width(),
				sampleOpts.tBlur()*baseBuf.height());
		// To estimate how much to increase the blur, we take the ratio of the
		// the blur to the computed width of the minor axis of the filter.
		// This should be near 0 for blur which doesn't effect the filtering
		// much, and a asymptote to a positive constant when the blur is the
		// dominant factor.
		blurRatio = clamp(2*maxBlur/weights.minorAxisWidth(), 0.0f, 1.0f);
		minFilterWidth += 2*blurRatio;
		// std::cout << minFilterWidth << "\n";
	}
	// Note: log2(x) = log(x)/log(2) ~= 1.4426950408889633 * log(x)
	TqFloat levelCts = 1.4426950408889633
		* std::log(weights.minorAxisWidth()/minFilterWidth);
	TqInt level = clamp<TqInt>(lfloor(levelCts), 0, m_levels->numLevels()-1);

	if(level > 0)
	{
		// Adjust the filter coefficients for the raster coordinates of the
		// chosen level
		const SqLevelTrans& trans = m_levels->levelTrans(level);
		weights.adjustTextureScale(trans.xScale, trans.xOffset,
				trans.yScale, trans.yOffset);
	}

	if(usingBlur && level < m_levels->numLevels()-1 && blurRatio > 0.2)
	{
		// Sample with blur: Use linear interpolation between the results of
		// filtering on two different mipmap levels.  Experiments with large
		// amounts of blurring show that some form of interpolation near level
		// transitions is necessary to ensure that they're smooth and invisible.
		//
		// Such interpolation is only necessary when large regions of the
		// output image arise from filtering over a small part of a high mipmap
		// level - something which only occurs with artifically large filter
		// widths such as those arising from lots of blur.
		//
		// Since this extra interpolation isn't really needed for small amounts
		// of blur, we only do the interpolation when the blur Ratio is large
		// enough to make it worthwhile.

		// renormalize levelInterp into the range [0,1]
		TqFloat levelInterp = levelCts - level;
		typedef detail::CqScaledWeights<CqEwaFilterWeights> TqScaledWeights;
		TqScaledWeights scaledWeights(weights, 1-levelInterp);
		CqSampleAccum<TqScaledWeights> accumulator(scaledWeights,
				sampleOpts.startChannel(), sampleOpts.numChannels(),
				outSamps, sampleOpts.fill());
		// Filter first mipmap level
		CqTexBufSampler<CqTextureBuffer<T> >(m_levels->level(level)).applyFilter(
				accumulator, weights.support(),
				sampleOpts.sWrapMode(), sampleOpts.tWrapMode());

		// Adjust filter weight for the linear interpolation
		scaledWeights.setWeightScale(levelInterp);
		// Adjust weight coordinate system.
		const SqLevelTrans& trans1 = m_levels->levelTrans(level);
		const SqLevelTrans& trans2 = m_levels->levelTrans(level+1);
		weights.adjustTextureScale(
				trans2.xScale/trans1.xScale,
				trans1.xScale*(trans2.xOffset - trans1.xOffset),
				trans2.yScale/trans1.yScale,
				trans1.yScale*(trans2.yOffset - trans1.yOffset)
				);
		// Filter second mipmap level
		CqTexBufSampler<CqTextureBuffer<T> >(m_levels->level(level+1)).applyFilter(
				accumulator, weights.support(),
				sampleOpts.sWrapMode(), sampleOpts.tWrapMode());
	}
	else
	{
		// Sample without blur
		CqSampleAccum<CqEwaFilterWeights> accumulator(weights,
				sampleOpts.startChannel(), sampleOpts.numChannels(),
				outSamps, sampleOpts.fill());
		CqTexBufSampler<CqTextureBuffer<T> >(m_levels->level(level)).applyFilter(
				accumulator, weights.support(),
				sampleOpts.sWrapMode(), sampleOpts.tWrapMode());
	}
}

template<typename T>
const CqTextureSampleOptions& CqMipmapTextureSampler<T>::defaultSampleOptions() const
{
	return m_levels->defaultSampleOptions();
}


} // namespace Aqsis

#endif // MIPMAPTEXTURESAMPLER_H_INCLUDED
