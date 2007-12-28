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

#include "texturesampler.h"
#include "tilearray.h"
#include "texturebuffer.h"

#include "itexinputfile.h"

namespace Aqsis {

boost::shared_ptr<IqTextureSampler> IqTextureSampler::create(
		const boost::shared_ptr<IqTiledTexInputFile>& file)
{
	assert(0);
	return boost::shared_ptr<IqTextureSampler>(
			new CqTextureSampler<CqTextureBuffer<TqUchar> >(boost::shared_ptr<CqTextureBuffer<TqUchar> >()));
}

boost::shared_ptr<IqTextureSampler> IqTextureSampler::create(
		const boost::shared_ptr<IqTexInputFile>& file)
{
	if(file)
	{
		switch(file->header().channelList().sharedChannelType())
		{
			case Channel_Float32:
			case Channel_Unsigned32:
			case Channel_Signed32:
				assert(0);
				break;
			case Channel_Float16:
			case Channel_Unsigned16:
			case Channel_Signed16:
				assert(0);
				break;
			case Channel_Unsigned8:
				{
					boost::shared_ptr<CqTextureBuffer<TqUchar> > buffer(
						new CqTextureBuffer<TqUchar>() );
					file->readPixels(*buffer);
					return boost::shared_ptr<IqTextureSampler>(
							new CqTextureSampler<CqTextureBuffer<TqUchar> >(buffer) );
				}
			case Channel_Signed8:
			case Channel_TypeUnknown:
				assert(0);
				break;
		}
	}
	return boost::shared_ptr<IqTextureSampler>(
			new CqTextureSampler<CqTextureBuffer<TqUchar> >(boost::shared_ptr<CqTextureBuffer<TqUchar> >()));
}


const CqTextureSampleOptions& IqTextureSampler::defaultSampleOptions() const
{
	static const CqTextureSampleOptions defaultOptions;
	return defaultOptions;
}

} // namespace Aqsis
