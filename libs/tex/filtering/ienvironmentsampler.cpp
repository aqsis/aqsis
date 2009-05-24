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
