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
#include "logging.h"
#include "sstring.h"
#include "itexturesampler.h"

namespace Aqsis {

//------------------------------------------------------------------------------
// CqTextureCache

CqTextureCache::CqTextureCache()
		//const boost::shared_ptr<CqFilePathList>& searchPaths)
	: m_cache()//, m_searchPaths(searchPaths)
{ }

IqTextureSampler& CqTextureCache::findTexture(const char* name)
{
	TqUlong hash = CqString::hash(name);
	TqCacheMap::const_iterator texIter = m_cache.find(hash);
	if(texIter != m_cache.end())
		return *(texIter->second);
	else
		return addTexture(name);
}

IqTextureSampler& CqTextureCache::addTexture(const char* name)
{
	boost::shared_ptr<IqTextureSampler> newTex;
	try
	{
		//newTex.reset(new IqTextureSampler(m_searchPaths->findFile()));
		newTex = IqTextureSampler::create(name);
	}
	catch(XqInvalidFile& e)
	{
        Aqsis::log() << warning
            << "Could not open file: \"" << name << "\": " << e.what() << "\n";
		/// \todo Put some kind of dummy implementation of IqTextureSampler to
		// in here.
		assert(0);
	}
	m_cache[CqString::hash(name)] = newTex;
	return *newTex;
}


} // namespace Aqsis
