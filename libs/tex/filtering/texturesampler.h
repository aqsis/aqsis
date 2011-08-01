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
