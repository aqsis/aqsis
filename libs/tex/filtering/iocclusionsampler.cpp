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
 * \brief Factory functions etc. for occlusion sampler interface
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#include <aqsis/tex/filtering/iocclusionsampler.h>

#include "dummyocclusionsampler.h"
#include <aqsis/tex/io/itiledtexinputfile.h>
#include "occlusionsampler.h"
#include <aqsis/tex/texexception.h>

namespace Aqsis {

// IqOcclusionSampler implementation

boost::shared_ptr<IqOcclusionSampler> IqOcclusionSampler::create(
		const boost::shared_ptr<IqTiledTexInputFile>& file, const CqMatrix& camToWorld)
{
	assert(file);
	// Create an occlusion sampler if the pixel type is floating point.
	switch(file->header().channelList().sharedChannelType())
	{
		case Channel_Float32:
			{
				boost::shared_ptr<IqOcclusionSampler>
					sampler(new CqOcclusionSampler(file, camToWorld));
				return sampler;
			}
		case Channel_Float16: // float16 may be useful in future...
		default:
			AQSIS_THROW_XQERROR(XqBadTexture, EqE_BadFile,
				"Cannot use non-float32 pixels in texture file \"" << file->fileName()
				<< "\" as an occlusion map.");
			break;
	}
	// shut up compiler warnings - return a null texture
	return createDummy();
}

boost::shared_ptr<IqOcclusionSampler> IqOcclusionSampler::createDummy()
{
	return boost::shared_ptr<IqOcclusionSampler>(new CqDummyOcclusionSampler());
}

const CqShadowSampleOptions& IqOcclusionSampler::defaultSampleOptions() const
{
	static const CqShadowSampleOptions defaultOptions;
	return defaultOptions;
}

} // namespace Aqsis
