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
 * \brief Interface to texture buffer sampling machinery.
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#include <aqsis/tex/filtering/itexturesampler.h>

#ifdef USE_OPENEXR
#	include <OpenEXR/half.h>
#endif

#include "dummytexturesampler.h"
#include <aqsis/tex/io/itexinputfile.h>
#include "texturesampler.h"
#include <aqsis/tex/buffers/texturebuffer.h>
#include <aqsis/tex/buffers/tilearray.h>

namespace Aqsis {

//------------------------------------------------------------------------------
namespace {

// Helper functions.

template<typename T>
boost::shared_ptr<IqTextureSampler> createMipmapSampler(
		const boost::shared_ptr<IqTiledTexInputFile>& file)
{
	typedef CqMipmap<CqTileArray<T> > TqLevelCache;
	boost::shared_ptr<TqLevelCache> levels(new TqLevelCache(file));
	boost::shared_ptr<IqTextureSampler> sampler(
			new CqTextureSampler<TqLevelCache>(levels));
	return sampler;
}

} // unnamed namespace


//------------------------------------------------------------------------------
// IqTextureSampler implementation

void IqTextureSampler::sample(const SqSampleQuad& sampleQuad,
		const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	// Default implementation for texture quadrilateral sampling.  This just
	// approximates the quad with a parallelogram and calls through to the
	// other version of sample().
	sample(SqSamplePllgram(sampleQuad), sampleOpts, outSamps);
}

const CqTextureSampleOptions& IqTextureSampler::defaultSampleOptions() const
{
	static const CqTextureSampleOptions defaultOptions;
	return defaultOptions;
}

boost::shared_ptr<IqTextureSampler> IqTextureSampler::create(
		const boost::shared_ptr<IqTiledTexInputFile>& file)
{
	assert(file);
	// Check the texture format and complain if it's not a plain texture
	const CqTexFileHeader& header = file->header();
	switch(header.find<Attr::TextureFormat>(TextureFormat_Unknown))
	{
		case TextureFormat_CubeEnvironment:
		case TextureFormat_LatLongEnvironment:
			Aqsis::log() << warning << "Accessing an environment map as a plain texture\n";
			break;
		case TextureFormat_Shadow:
			Aqsis::log() << warning << "Accessing a shadow map as a plain texture\n";
			break;
		default:
			// no warnings in generic case.
			break;
	}
	// Create a texture sampler based on the underlying pixel type.
	switch(header.channelList().sharedChannelType())
	{
		case Channel_Float32:
			return createMipmapSampler<TqFloat>(file);
		case Channel_Unsigned32:
			return createMipmapSampler<TqUint32>(file);
		case Channel_Signed32:
			return createMipmapSampler<TqInt32>(file);
		case Channel_Float16:
#			ifdef USE_OPENEXR
			return createMipmapSampler<half>(file);
#			endif
			break;
		case Channel_Unsigned16:
			return createMipmapSampler<TqUint16>(file);
		case Channel_Signed16:
			return createMipmapSampler<TqInt16>(file);
		case Channel_Unsigned8:
			return createMipmapSampler<TqUint8>(file);
		case Channel_Signed8:
			return createMipmapSampler<TqInt8>(file);
		default:
		case Channel_TypeUnknown:
			break;
	}
	AQSIS_THROW_XQERROR(XqBadTexture, EqE_BadFile,
		"Could not create a texture sampler for file \"" << file->fileName() << "\"");
	return createDummy();
}

boost::shared_ptr<IqTextureSampler> IqTextureSampler::create(
		const boost::shared_ptr<IqMultiTexInputFile>& file)
{
	return createDummy();
}

boost::shared_ptr<IqTextureSampler> IqTextureSampler::createDummy()
{
	return boost::shared_ptr<IqTextureSampler>(new CqDummyTextureSampler());
}

} // namespace Aqsis
