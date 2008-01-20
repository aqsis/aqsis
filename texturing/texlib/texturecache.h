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
 * \brief A texture cache
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef TEXTURECACHE_H_INCLUDED
#define TEXTURECACHE_H_INCLUDED

#include "aqsis.h"

#include <string>
#include <map>

#include <boost/shared_ptr.hpp>

#include "itexturesampler.h"

namespace Aqsis {

class CqFilePathList;
class IqTextureSampler;

class CqTextureCache
{
	public:
		/** \brief Construct an empty texture cache.
		 */
		CqTextureCache();

		/** \brief Find a texture in the cache or load from file if not found.
		 *
		 * \param name - the texture name.
		 */
		inline IqTextureSampler& findTexture(const std::string& name);
	private:
		typedef std::map<TqUlong, boost::shared_ptr<IqTextureSampler> > TqCacheMap;

		/** \brief Load a texture from the file with the given name.
		 *
		 * If the file isn't found, we issue a warning, and a dummy texture is
		 * created instead so that the render can continue.
		 */
		IqTextureSampler& addTexture(const std::string& name);

		/// Cached textures live in here
		TqCacheMap m_cache;
		/// Search path
		//boost::shared_ptr<CqFilePathList> m_searchPaths;
};


//==============================================================================
// Implementation details.
//==============================================================================

} // namespace Aqsis

#endif // TEXTURECACHE_H_INCLUDED
