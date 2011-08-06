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
 * \brief Implementation of interface to environment map sampling machinery.
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#include <aqsis/tex/filtering/ienvironmentsampler.h>
#include "cubeenvironmentsampler.h"
#include "dummyenvironmentsampler.h"
#include "latlongenvironmentsampler.h"
#include <aqsis/tex/buffers/tilearray.h>

namespace Aqsis {

namespace {

// Helper functions.

template<typename T>
boost::shared_ptr<IqEnvironmentSampler> createEnvSampler(
		const boost::shared_ptr<IqTiledTexInputFile>& file)
{
	typedef CqMipmap<CqTileArray<T> > TqLevelCache;
	boost::shared_ptr<TqLevelCache> levels(new TqLevelCache(file));
	// Note: We need the temporary here, since at least one g++ version, 4.0.1
	// on OSX complains about the expression 
	// file->header().find<Attr::TextureFormat>(TextureFormat_Unknown);
	const CqTexFileHeader& header = file->header();
	switch(header.find<Attr::TextureFormat>(TextureFormat_Unknown))
	{
		case TextureFormat_CubeEnvironment:
			return boost::shared_ptr<IqEnvironmentSampler>(
					new CqCubeEnvironmentSampler<TqLevelCache>(levels));
		case TextureFormat_LatLongEnvironment:
			return boost::shared_ptr<IqEnvironmentSampler>(
					new CqLatLongEnvironmentSampler<TqLevelCache>(levels));
		default:
			AQSIS_THROW_XQERROR(XqBadTexture, EqE_BadFile,
						  "Accessing non-environment texture \""
						  << file->fileName() << "\" as an environment map");
	}
}

} // unnamed namespace

void IqEnvironmentSampler::sample(const Sq3DSampleQuad& sampleQuad,
		const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	// default implementation calls through to the less general version of
	// sample()
	sample(Sq3DSamplePllgram(sampleQuad), sampleOpts, outSamps);
}

const CqTextureSampleOptions& IqEnvironmentSampler::defaultSampleOptions() const
{
	static const CqTextureSampleOptions defaultOptions;
	return defaultOptions;
}

boost::shared_ptr<IqEnvironmentSampler> IqEnvironmentSampler::create(
		const boost::shared_ptr<IqTiledTexInputFile>& file)
{
	assert(file);

	// Create an environment sampler based on the underlying pixel type.
	switch(file->header().channelList().sharedChannelType())
	{
		case Channel_Float32:
			return createEnvSampler<TqFloat>(file);
		case Channel_Unsigned32:
			return createEnvSampler<TqUint32>(file);
		case Channel_Signed32:
			return createEnvSampler<TqInt32>(file);
		case Channel_Float16:
#			ifdef USE_OPENEXR
			return createEnvSampler<half>(file);
#			endif
			break;
		case Channel_Unsigned16:
			return createEnvSampler<TqUint16>(file);
		case Channel_Signed16:
			return createEnvSampler<TqInt16>(file);
		case Channel_Unsigned8:
			return createEnvSampler<TqUint8>(file);
		case Channel_Signed8:
			return createEnvSampler<TqInt8>(file);
		default:
		case Channel_TypeUnknown:
			break;
	}
	AQSIS_THROW_XQERROR(XqBadTexture, EqE_BadFile,
				  "Could not create an environment sampler for file \"" 
				  << file->fileName() << "\"");
	return createDummy();
}

boost::shared_ptr<IqEnvironmentSampler> IqEnvironmentSampler::createDummy()
{
	return boost::shared_ptr<IqEnvironmentSampler>(
			new CqDummyEnvironmentSampler());
}

} // namespace Aqsis
