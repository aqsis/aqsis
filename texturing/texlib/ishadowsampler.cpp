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
 * \brief Interface to shadow texture sampling machinery.
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#include "ishadowsampler.h"
#include "texexception.h"
#include "shadowsampler.h"
#include "itexinputfile.h"

namespace Aqsis {

// IqShadowSampler implementation

boost::shared_ptr<IqShadowSampler> IqShadowSampler::create(
		const boost::shared_ptr<IqTexInputFile>& file, const CqMatrix& camToWorld)
{
	switch(file->header().channelList().sharedChannelType())
	{
		case Channel_Float32:
			{
				boost::shared_ptr<IqShadowSampler>
					sampler(new CqShadowSampler(file, camToWorld));
				return sampler;
			}
		case Channel_Unsigned32:
		case Channel_Signed32:
		case Channel_Float16:
		case Channel_Unsigned16:
		case Channel_Signed16:
		case Channel_Unsigned8:
		case Channel_Signed8:
			throw XqBadTexture("Attempt to use use non-floating point texture"
					"as a shadowmap", __FILE__, __LINE__);
		case Channel_TypeUnknown:
			break;
	}
	// shut up compiler warnings - return a null texture
	assert(0);
	return boost::shared_ptr<IqShadowSampler>();
}

boost::shared_ptr<IqShadowSampler> IqShadowSampler::create(
		const char* fileName, const CqMatrix& camToWorld)
{
	boost::shared_ptr<IqTexInputFile> inFile = IqTexInputFile::open(fileName);
	return IqShadowSampler::create(inFile, camToWorld);
}

const CqShadowSampleOptions& IqShadowSampler::defaultSampleOptions() const
{
	static const CqShadowSampleOptions defaultOptions;
	return defaultOptions;
}

} // namespace Aqsis
