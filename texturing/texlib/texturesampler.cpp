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

#include "texturesampler.h"

namespace Aqsis {

boost::shared_ptr<IqTextureSampler> IqTextureSampler::create(
		const boost::shared_ptr<IqTiledTexInputFile>& file)
{
	// \todo write a switch based on the pixel type in file.
	return boost::shared_ptr<IqTextureSampler>(
			new CqTextureSamplerImpl<TqUchar>(boost::shared_ptr<CqTileArray<TqUchar> >()));
}

} // namespace Aqsis
