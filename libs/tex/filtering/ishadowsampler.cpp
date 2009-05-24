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

#include <aqsis/tex/filtering/ishadowsampler.h>

#include "dummyshadowsampler.h"
#include <aqsis/tex/io/itiledtexinputfile.h>
#include "shadowsampler.h"
#include <aqsis/tex/texexception.h>

namespace Aqsis {

// IqShadowSampler implementation

boost::shared_ptr<IqShadowSampler> IqShadowSampler::create(
		const boost::shared_ptr<IqTiledTexInputFile>& file, const CqMatrix& camToWorld)
{
	assert(file);
	// Create a shadow sampler if the pixel type is floating point.
	switch(file->header().channelList().sharedChannelType())
	{
		case Channel_Float32:
			{
				boost::shared_ptr<IqShadowSampler>
					sampler(new CqShadowSampler(file, camToWorld));
				return sampler;
			}
		case Channel_Float16: // float16 may be useful in future...
		default:
			AQSIS_THROW_XQERROR(XqBadTexture, EqE_BadFile,
				"Cannot use non-float32 pixels in texture file \"" << file->fileName()
				<< "\" as a shadowmap");
			break;
	}
	// shut up compiler warnings - return a null texture
	return createDummy();
}

boost::shared_ptr<IqShadowSampler> IqShadowSampler::createDummy()
{
	return boost::shared_ptr<IqShadowSampler>(new CqDummyShadowSampler());
}

const CqShadowSampleOptions& IqShadowSampler::defaultSampleOptions() const
{
	static const CqShadowSampleOptions defaultOptions;
	return defaultOptions;
}

} // namespace Aqsis
