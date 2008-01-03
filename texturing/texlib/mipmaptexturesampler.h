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
#include "levelsamplercache.h"
#include "ewafilter.h"
#include "texturebuffer.h" // remove when using tiled textures.
#include "sampleaccum.h"
#include "aqsismath.h"

namespace Aqsis {

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
class CqMipmapTextureSampler : public IqTextureSampler
{
	private:
		typedef CqLevelSamplerCache<CqTextureBuffer<T> > TqCacheType;
	public:
		CqMipmapTextureSampler(const boost::shared_ptr<TqCacheType>& levels);

		// from IqTextureSampler
		virtual void filter(const SqSampleQuad& sampleQuad,
				const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const;
		virtual const CqTextureSampleOptions& defaultSampleOptions() const;
	private:
		boost::shared_ptr<TqCacheType> m_levels;
};


//==============================================================================
// Implementation details
//==============================================================================

template<typename T>
CqMipmapTextureSampler<T>::CqMipmapTextureSampler(
		const boost::shared_ptr<TqCacheType>& levels)
	: m_levels(levels)
{ }

template<typename T>
void CqMipmapTextureSampler<T>::filter(const SqSampleQuad& sampleQuad,
		const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	SqSampleQuad quad = sampleQuad;
	sampleOpts.adjustSampleQuad(quad);
	quad.remapPeriodic(sampleOpts.sWrapMode() == WrapMode_Periodic,
			sampleOpts.tWrapMode() == WrapMode_Periodic);

	const CqTextureBuffer<T>& baseBuf = m_levels->level(0);
	// Construct weights
	CqEwaFilterWeights weights(quad, baseBuf.width(),
			baseBuf.height(), sampleOpts.sBlur(), sampleOpts.tBlur());
	// Select mipmap level to use
	const TqFloat minFilterWidth = 2;
	// log2(x) = log(x)/log(2) = log(x) * 1.4426950408889633...
	TqInt level = lfloor(std::log(weights.minorAxisWidth()/minFilterWidth)*1.4426950408889633);
	level = clamp(level, 0, m_levels->numLevels());

	// filter the texture.
	CqSampleAccum<CqEwaFilterWeights> accumulator(weights,
			sampleOpts.startChannel(), sampleOpts.numChannels(), outSamps);

	if(level > 0)
	{
		const CqTextureBuffer<T>& sampleBuf = m_levels->level(level);
		weights.adjustTextureScale(
				static_cast<TqFloat>(sampleBuf.width())/baseBuf.width(),
				static_cast<TqFloat>(sampleBuf.height())/baseBuf.height() );
		sampleBuf.applyFilter(accumulator, weights.support(),
				sampleOpts.sWrapMode(), sampleOpts.tWrapMode());
	}
	else
	{
		baseBuf.applyFilter(accumulator, weights.support(),
				sampleOpts.sWrapMode(), sampleOpts.tWrapMode());
	}

	// Debug: insert an indicator of the mipmap level...
	//outSamps[level%3] += 0.5;
}

template<typename T>
const CqTextureSampleOptions& CqMipmapTextureSampler<T>::defaultSampleOptions() const
{
	return m_levels->defaultSampleOptions();
}

} // namespace Aqsis

#endif // MIPMAPTEXTURESAMPLER_H_INCLUDED
