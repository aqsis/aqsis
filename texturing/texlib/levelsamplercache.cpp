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

#include "levelsamplercache.h"

#include "itexturesampler.h"

namespace Aqsis
{

//------------------------------------------------------------------------------
// Implementation of CqLevelSamplerCache

CqLevelSamplerCache::CqLevelSamplerCache(
				const boost::shared_ptr<IqTiledTexInputFile>& file)
	: m_defaultSampleOptions(),
	m_levels(),
	m_texFile(file)
{
	// \todo decide how many mipmap levels are needed.
	m_levels.resize(1);
	// \todo: Init default sampling opts from texture file
}

const IqTextureSampler& CqLevelSamplerCache::level(TqInt levelNum)
{
	IqTextureSampler* sampler = m_levels.at(levelNum).get();
	if(!sampler)
	{
		m_levels[levelNum] = IqTextureSampler::create(m_texFile);
		sampler = m_levels[levelNum].get();
	}
	return *sampler;
}

} // namespace Aqsis
