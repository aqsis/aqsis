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

#include "autobuffer.h"
#include "aqsismath.h"
#include "ewafilter.h"
#include "filtertexture.h"
#include "itexturesampler.h"
#include "mipmaplevelcache.h"
#include "sampleaccum.h"

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief A mulitresolution (mipmapped) texture sampler class
 *
 * This class uses a set of power-of-two downsampled images (a typical
 * mipmap) to make texture filtering efficient.  When filtering, a mipmap level
 * is chosen based on the extent of the filter quadrilateral.  We choose the
 * mipmap level to be as small as possible, subject to the filter falling
 * across a sufficient number of pixels.
 */
template<typename LevelCacheT>
class AQSISTEX_SHARE CqMipmapTextureSampler : public IqTextureSampler
{
	public:
		/** \brief Construct a sampler from the provided set of mipmap levels.
		 */
		CqMipmapTextureSampler(const boost::shared_ptr<LevelCacheT>& levels);

		// from IqTextureSampler
		virtual void sample(const SqSampleQuad& sampleQuad,
				const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const;
		virtual const CqTextureSampleOptions& defaultSampleOptions() const;
	private:
		boost::shared_ptr<LevelCacheT> m_levels;

		/** \brief Filter the given mipmap level into a sample array.
		 *
		 * \param level - level to filter
		 * \param ewaFactory - factory containing parameters for creating ewa
		 *                     filter weights.
		 * \param sampleOpts - sample options
		 * \param outSamps - destination array for samples.
		 */
		void filterLevel(TqInt level, const CqEwaFilterFactory& ewaFactory,
				const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const;
};


//==============================================================================
// Implementation details
//==============================================================================
// CqMipmapTextureSampler implementation
template<typename LevelCacheT>
CqMipmapTextureSampler<LevelCacheT>::CqMipmapTextureSampler(
		const boost::shared_ptr<LevelCacheT>& levels)
	: m_levels(levels)
{ }

template<typename LevelCacheT>
void CqMipmapTextureSampler<LevelCacheT>::sample(const SqSampleQuad& sampleQuad,
		const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	SqSampleQuad quad = sampleQuad;
	quad.scaleWidth(sampleOpts.sWidth(), sampleOpts.tWidth());
	quad.remapPeriodic(sampleOpts.sWrapMode() == WrapMode_Periodic,
			sampleOpts.tWrapMode() == WrapMode_Periodic);

	// Construct EWA filter factory
	CqEwaFilterFactory ewaFactory(quad, m_levels->width0(),
			m_levels->height0(), sampleOpts.sBlur(), sampleOpts.tBlur());
	bool usingBlur = sampleOpts.sBlur() != 0 || sampleOpts.tBlur() != 0;
	// Select mipmap level to use.
	//
	// The minimum filter width is the minimum number of pixels over which the
	// shortest length scale of the filter should extend.
	TqFloat minFilterWidth = 2;
	// Blur ratio ranges from 0 at no blur to 1 for a "lot" of blur.
	TqFloat blurRatio = 0;
	if(sampleOpts.lerp() != Lerp_Auto)
	{
		// If we're not using automatic detection of when a lerp between mipmap
		// levels is necessary, reset the filter support width.
		minFilterWidth = 4;
	}
	else if(usingBlur)
	{
		// When using blur, the minimum filter width needs to be increased.
		//
		// Experiments show that for large blur factors minFilterWidth should
		// be about 4 for good results.
		TqFloat maxBlur = max(sampleOpts.sBlur()*m_levels->width0(),
				sampleOpts.tBlur()*m_levels->height0());
		// To estimate how much to increase the blur, we take the ratio of the
		// the blur to the computed width of the minor axis of the filter.
		// This should be near 0 for blur which doesn't effect the filtering
		// much, and a asymptote to a positive constant when the blur is the
		// dominant factor.
		blurRatio = clamp(2*maxBlur/ewaFactory.minorAxisWidth(), 0.0f, 1.0f);
		minFilterWidth += 2*blurRatio;
	}
	TqFloat levelCts = log2(ewaFactory.minorAxisWidth()/minFilterWidth);
	TqInt level = clamp<TqInt>(lfloor(levelCts), 0, m_levels->numLevels()-1);

	filterLevel(level, ewaFactory, sampleOpts, outSamps);

	// Sometimes we might want to interpolate between the filtered result
	// already computed above and the next lower mipmap level.  We do that now
	// if necessary.
	if( ( sampleOpts.lerp() == Lerp_Always
		|| (sampleOpts.lerp() == Lerp_Auto && usingBlur && blurRatio > 0.2) )
		&& level < m_levels->numLevels()-1 )
	{
		// Use linear interpolation between the results of filtering on two
		// different mipmap levels.  This should only be necessary if using
		// filter blur, however the user can also turn it on explicitly using
		// the "lerp" option.
		//
		// Experiments with large amounts of blurring show that some form of
		// interpolation near level transitions is necessary to ensure that
		// they're smooth and invisible.
		//
		// Such interpolation is mainly necessary when large regions of the
		// output image arise from filtering over a small part of a high mipmap
		// level - something which only occurs with artifically large filter
		// widths such as those arising from lots of blur.
		//
		// Since this extra interpolation isn't really needed for small amounts
		// of blur, we only do the interpolation when the blur ratio is large
		// enough to make it worthwhile.

		// Filter second level into tmpSamps.
		CqAutoBuffer<TqFloat, 16> tmpSamps(sampleOpts.numChannels());
		filterLevel(level+1, ewaFactory, sampleOpts, tmpSamps.get());

		// Mix outSamps and tmpSamps.
		TqFloat levelInterp = levelCts - level;
		// We square levelInterp here in order to bias the interpolation toward
		// the higher resolution mipmap level, since the filtered result on the
		// higher level is more accurate.
		levelInterp *= levelInterp;
		for(TqInt i = 0; i < sampleOpts.numChannels(); ++i)
			outSamps[i] = (1-levelInterp) * outSamps[i] + levelInterp*tmpSamps[i];
	}
}

template<typename LevelCacheT>
const CqTextureSampleOptions&
CqMipmapTextureSampler<LevelCacheT>::defaultSampleOptions() const
{
	return m_levels->defaultSampleOptions();
}

template<typename LevelCacheT>
void CqMipmapTextureSampler<LevelCacheT>::filterLevel(
		TqInt level, const CqEwaFilterFactory& ewaFactory,
		const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	// Create filter weights for chosen level.
	const SqLevelTrans& trans = m_levels->levelTrans(level);
	CqEwaFilter weights = ewaFactory.createFilter(
		trans.xScale, trans.xOffset,
		trans.yScale, trans.yOffset
	);
	// Create an accumulator for the samples.
	CqSampleAccum<CqEwaFilter> accumulator(
		weights,
		sampleOpts.startChannel(),
		sampleOpts.numChannels(),
		outSamps,
		sampleOpts.fill()
	);
	// filter the texture
	filterTexture(
		accumulator,
		m_levels->level(level),
		weights.support(),
		SqWrapModes(sampleOpts.sWrapMode(), sampleOpts.tWrapMode())
	);
}

} // namespace Aqsis

#endif // MIPMAPTEXTURESAMPLER_H_INCLUDED
