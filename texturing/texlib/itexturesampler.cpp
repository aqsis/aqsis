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
 * \brief Texture buffer sampling machinery.
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#include "itexturesampler.h"

#ifdef USE_OPENEXR
#	include <half.h>
#endif

#include "dummytexturesampler.h"
#include "itexinputfile.h"
#include "mipmaptexturesampler.h"
#include "texturebuffer.h"
#include "tilearray.h"

namespace Aqsis {

//------------------------------------------------------------------------------
namespace {

// Helper functions.

template<typename T>
boost::shared_ptr<IqTextureSampler> createMipmapSampler(
		const boost::shared_ptr<IqMultiTexInputFile>& file)
{
	boost::shared_ptr<CqMipmapLevelCache<CqTextureBuffer<T> > >
		levels(new CqMipmapLevelCache<CqTextureBuffer<T> >(file));
	boost::shared_ptr<IqTextureSampler>
		sampler(new CqMipmapTextureSampler<T>(levels));
	return sampler;
}

} // unnamed namespace


//------------------------------------------------------------------------------
// IqTextureSampler implementation

boost::shared_ptr<IqTextureSampler> IqTextureSampler::create(
		const boost::shared_ptr<IqTiledTexInputFile>& file)
{
	assert(0);
	return createDummy();
}

boost::shared_ptr<IqTextureSampler> IqTextureSampler::create(
		const boost::shared_ptr<IqMultiTexInputFile>& file)
{
	if(!file)
		AQSIS_THROW(XqInvalidFile, "Cannot create texture sampler from null file handle");
	/// \todo We really should use proper types of guarenteed bit-widths here.
	switch(file->header().channelList().sharedChannelType())
	{
		case Channel_Float32:
			return createMipmapSampler<TqFloat>(file);
		case Channel_Unsigned32:
			return createMipmapSampler<TqUint>(file);
		case Channel_Signed32:
			return createMipmapSampler<TqInt>(file);
		case Channel_Float16:
#			ifdef USE_OPENEXR
			return createMipmapSampler<half>(file);
#			endif
			break;
		case Channel_Unsigned16:
			return createMipmapSampler<TqUshort>(file);
		case Channel_Signed16:
			return createMipmapSampler<TqShort>(file);
		case Channel_Unsigned8:
			return createMipmapSampler<TqUchar>(file);
		case Channel_Signed8:
			return createMipmapSampler<TqChar>(file);
		case Channel_TypeUnknown:
			break;
	}
	AQSIS_THROW(XqBadTexture, "Could not create a texture sampler for file \"" 
			<< file->fileName() << "\"");
	return createDummy();
}

boost::shared_ptr<IqTextureSampler> IqTextureSampler::createDummy()
{
	return boost::shared_ptr<IqTextureSampler>(new CqDummyTextureSampler());
}

const CqTextureSampleOptions& IqTextureSampler::defaultSampleOptions() const
{
	static const CqTextureSampleOptions defaultOptions;
	return defaultOptions;
}

} // namespace Aqsis
