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
 * \brief Dummy texture sampler for missing files
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef DUMMYTEXTURESAMPLER_H_INCLUDED
#define DUMMYTEXTURESAMPLER_H_INCLUDED

#include <aqsis/tex/filtering/itexturesampler.h>

namespace Aqsis {

/** \brief A texture sampler to stand in for missing files.
 *
 * This sampler is intended to make missing textures as visible as possible.
 */
class AQSIS_TEX_SHARE CqDummyTextureSampler : public IqTextureSampler
{
	public:
		/** \brief Point sample a dummy cross-shaped texture
		 *
		 * As a clear indicator for when a texture cannot be found, this
		 * sampler produces an ugly unfiltered black-on-white cross shape.  (A
		 * bit like the dreaded red crosses which often appear in powerpoint
		 * presentations ;-)
		 */
		virtual void sample(const SqSamplePllgram& samplePllgram,
			const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const;
};

} // namespace Aqsis

#endif // DUMMYTEXTURESAMPLER_H_INCLUDED
