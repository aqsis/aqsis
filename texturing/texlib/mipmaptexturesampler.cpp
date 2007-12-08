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
 * \brief Multiresolution texture sampler implementation
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#include "mipmaptexturesampler.h"

namespace Aqsis {

CqMipmapTextureSampler::CqMipmapTextureSampler(
		const boost::shared_ptr<CqLevelSamplerCache>& levels)
	: m_levels(levels)
{ }

void CqMipmapTextureSampler::filter(const SqSampleQuad& sampleQuad,
		const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	SqSampleQuad quad = sampleQuad;
	sampleOpts.adjustSampleQuad(quad);
	m_levels->level(0).filter(quad, sampleOpts, outSamps);
}

const CqTextureSampleOptions& CqMipmapTextureSampler::defaultSampleOptions() const
{
	return m_levels->defaultSampleOptions();
}

} // namespace Aqsis
