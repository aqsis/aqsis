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
