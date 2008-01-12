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
 * \brief Texture cache implementation.
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#include "texturecache.h"

#include "exception.h"
#include "texturemap.h"
#include "logging.h"

namespace Aqsis {

//------------------------------------------------------------------------------
// CqTextureCache

inline CqTextureCache::CqTextureCache()
		//const boost::shared_ptr<CqFilePathList>& searchPaths)
	: m_cache()//, m_searchPaths(searchPaths)
{ }

inline boost::shared_ptr<IqTextureMap> CqTextureCache::findTexture(
		const std::string& name)
{
	TqUlong hash = CqString::hash(name.c_str());
	TqCacheMap::const_iterator texIter = m_cache.find(hash);
	if(texIter != m_cache.end())
		return boost::shared_ptr<IqTextureMap>(new CqTextureMap(texIter->second));
	else
		return boost::shared_ptr<IqTextureMap>(new CqTextureMap(addTexture(name)));
}

boost::shared_ptr<IqTextureSampler> CqTextureCache::addTexture(
		const std::string& name)
{
	boost::shared_ptr<IqTextureSampler> newTex;
	try
	{
		//newTex.reset(new IqTextureSampler(m_searchPaths->findFile()));
		newTex = IqTextureSampler::create(name.c_str());
	}
	catch(XqInvalidFile& e)
	{
        Aqsis::log() << warning
            << "Could not open file: \"" << name << "\": " << e.what() << "\n";
		/// \todo Put some kind of dummy implementation of IqTextureSampler to
		// in here.
		assert(0);
	}
	m_cache[CqString::hash(name.c_str())] = newTex;
	return newTex;
}


} // namespace Aqsis
