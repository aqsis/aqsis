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
 * \brief Implementation of a class which holds a collection of mipmap levels.
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#include "mipmaplevels.h"

#include "itexturesampler.h"

namespace Aqsis
{

//------------------------------------------------------------------------------
// Implementation of CqMipmapLevels

CqMipmapLevels::CqMipmapLevels(const std::string& texName)
	: m_texName(texName),
	m_defaultSampleOptions(),
	m_mipLevels()
{
	// \todo decide how many mipmap levels are needed.
	m_mipLevels.resize(1);
	// \todo: Init default sampling opts from texture file
}

const IqTextureSampler& CqMipmapLevels::level(TqInt levelNum)
{
	assert(levelNum >= 0);
	IqTextureSampler* sampler = m_mipLevels[levelNum].get();
	if(!sampler)
	{
		m_mipLevels[levelNum] = IqTextureSampler::create(
				boost::shared_ptr<IqTiledTexInputFile>());
		sampler = m_mipLevels[levelNum].get();
	}
	return *sampler;
}

} // namespace Aqsis
