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
 * \brief Interface to texture buffer sampling machinery.
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#include <aqsis/tex/filtering/itexturesampler.h>

#ifdef USE_OPENEXR
#	include <half.h>
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
