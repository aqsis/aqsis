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

#ifndef TEXTURESAMPLER_H_INCLUDED
#define TEXTURESAMPLER_H_INCLUDED

#include "aqsis.h"

#include <boost/shared_ptr.hpp>

#include "ewafilter.h"
#include "itexturesampler.h"
#include "mipmap.h"

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
class AQSISTEX_SHARE CqTextureSampler : public IqTextureSampler
{
	public:
		/** \brief Construct a sampler from the provided set of mipmap levels.
		 */
		CqTextureSampler(const boost::shared_ptr<LevelCacheT>& levels);

		// from IqTextureSampler
		virtual void sample(const SqSampleQuad& sampleQuad,
				const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const;
		virtual const CqTextureSampleOptions& defaultSampleOptions() const;
	private:
		boost::shared_ptr<LevelCacheT> m_levels;
};


//==============================================================================
// Implementation details
//==============================================================================
// CqTextureSampler implementation
template<typename LevelCacheT>
CqTextureSampler<LevelCacheT>::CqTextureSampler(
		const boost::shared_ptr<LevelCacheT>& levels)
	: m_levels(levels)
{ }

template<typename LevelCacheT>
void CqTextureSampler<LevelCacheT>::sample(const SqSampleQuad& sampleQuad,
		const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	SqSampleQuad quad = sampleQuad;
	quad.scaleWidth(sampleOpts.sWidth(), sampleOpts.tWidth());
	quad.remapPeriodic(sampleOpts.sWrapMode() == WrapMode_Periodic,
			sampleOpts.tWrapMode() == WrapMode_Periodic);

	// Construct EWA filter factory
	CqEwaFilterFactory ewaFactory(quad, m_levels->width0(),
			m_levels->height0(), sampleOpts.sBlur(), sampleOpts.tBlur());

	// Call through to the mipmap class to do the filtering proper.
	m_levels->applyFilter(ewaFactory, sampleOpts, outSamps);
}


template<typename LevelCacheT>
const CqTextureSampleOptions&
CqTextureSampler<LevelCacheT>::defaultSampleOptions() const
{
	return m_levels->defaultSampleOptions();
}

} // namespace Aqsis

#endif // TEXTURESAMPLER_H_INCLUDED
