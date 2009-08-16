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

#include <aqsis/aqsis.h>

#include <boost/shared_ptr.hpp>

#include "ewafilter.h"
#include <aqsis/tex/filtering/itexturesampler.h>
#include "mipmap.h"

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief A plain texture sampler class.
 *
 * This class implements the IqTextureSampler, providing efficient filtered
 * texture access using a mipmap.
 */
template<typename LevelCacheT>
class AQSIS_TEX_SHARE CqTextureSampler : public IqTextureSampler
{
	public:
		/** \brief Construct a sampler from the provided set of mipmap levels.
		 */
		CqTextureSampler(const boost::shared_ptr<LevelCacheT>& levels);

		// from IqTextureSampler
		virtual void sample(const SqSamplePllgram& samplePllgram,
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
void CqTextureSampler<LevelCacheT>::sample(const SqSamplePllgram& samplePllgram,
		const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	// Scale width if necessary
	SqSamplePllgram pllgram(samplePllgram);
	pllgram.scaleWidth(sampleOpts.sWidth(), sampleOpts.tWidth());
	// Remap onto the main part of the texture if periodic.
	pllgram.remapPeriodic(sampleOpts.sWrapMode() == WrapMode_Periodic,
			sampleOpts.tWrapMode() == WrapMode_Periodic);

	// Construct EWA filter factory
	CqEwaFilterFactory ewaFactory(pllgram, m_levels->width0(), m_levels->height0(),
			ewaBlurMatrix(sampleOpts.sBlur(), sampleOpts.tBlur()),
			-sampleOpts.logTruncAmount());

	// Call through to the mipmap class to do the main filtering work.
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
