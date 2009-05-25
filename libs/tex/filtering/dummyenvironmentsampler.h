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
 * \brief Dummy environment sampler for missing files
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef DUMMYENVIRONMENTSAMPLER_H_INCLUDED
#define DUMMYENVIRONMENTSAMPLER_H_INCLUDED

#include <aqsis/tex/filtering/ienvironmentsampler.h>

namespace Aqsis {

/** \brief An environment sampler to stand in for missing files.
 *
 * This sampler is intended to make missing environment maps as visible as
 * possible, while also providing useful information on orientation.
 */
class AQSIS_TEX_SHARE CqDummyEnvironmentSampler : public IqEnvironmentSampler
{
	public:
		// From IqEnvironmentSampler
		virtual void sample(const Sq3DSamplePllgram& samplePllgram,
				const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const;
};

} // namespace Aqsis

#endif // DUMMYENVIRONMENTSAMPLER_H_INCLUDED
