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

#include <map>

#include <boost/shared_ptr.hpp>

#include "matrix.h"

namespace Aqsis {

class CqFilePathList;
class IqTextureSampler;
class IqShadowSampler;
class CqTexFileHeader;

/** \brief A cache managing the various types of texture samplers.
 */
class AQSISTEX_SHARE CqTextureCache
{
	public:
		/** \brief Construct an empty texture cache.
		 */
		CqTextureCache();

		//--------------------------------------------------
		// \name Sampler access
		//@{
		/** \brief Find a texture sampler in the cache or load from file if not found.
		 *
		 * If any problems are encountered in opening the texture, issue a
		 * warning to Aqsis::log(), and return a dummy sampler.
		 *
		 * \param name - the texture name.
		 */
		IqTextureSampler& findTextureSampler(const char* texName);
		/** \brief Find a shadow sampler in the cache or load from file if not found.
		 *
		 * If any problems are encountered in opening the shadow texture, issue
		 * a warning to Aqsis::log(), and return a dummy sampler.
		 *
		 * \param name - the texture name.
		 */
		IqShadowSampler& findShadowSampler(const char* texName);
		//@}

		//--------------------------------------------------
		/** \brief Return the texture file attributes for the named file.
		 *
		 * If the file is not found or is otherwise invalid, return 0.
		 *
		 * \param texName - file name 
		 */
		CqTexFileHeader* textureInfo(const char* texName);

		/** \brief Set the camera -> world transformation
		 *
		 * This transformation is used by shadow samplers in order to transform
		 * the provided shading (camera) coordinates into the coordinate system
		 * of the shadowed light.
		 *
		 * \param camToWorld - camera -> world transformation.
		 */
		void setCamToWorldMatrix(const CqMatrix& camToWorld);

	private:
		/** \brief Find a sampler in the given map, or create one from file if needed.
		 *
		 * If the file isn't found, we issue a warning, and a dummy sampler
		 * should be created instead so that the render can continue.
		 *
		 * \param samplerMap - std::map to find the sampler in.
		 * \param name - name of the texture.
		 */
		template<typename SamplerT>
		SamplerT& findSampler(std::map<TqUlong, boost::shared_ptr<SamplerT> >&
				samplerMap, const char* name);
		/** \brief Create a sampler of the given type from a file.
		 *
		 * SamplerT - is a sampler type to instantiate.
		 *
		 * \param name - absolute path to file
		 */
		template<typename SamplerT>
		boost::shared_ptr<SamplerT> newSamplerFromFile(const char* name);
		/** \brief Create a dummy sampler of the given type.
		 *
		 * This is used when a texture sampler cannot be created from the
		 * requested file name.  A dummy sampler enables the renderer to
		 * continue with a fake texture.
		 *
		 * SamplerT - base sampler type.
		 */
		template<typename SamplerT>
		boost::shared_ptr<SamplerT> newDummySampler();

		/// Cached textures live in here
		std::map<TqUlong, boost::shared_ptr<IqTextureSampler> > m_textureCache;
		std::map<TqUlong, boost::shared_ptr<IqShadowSampler> > m_shadowCache;
		/// Camera -> world transformation - used for creating shadow maps.
		CqMatrix m_camToWorld;
		/// Search path
		/// \todo Get search path working!
		//boost::shared_ptr<CqFilePathList> m_searchPaths;
};


} // namespace Aqsis

#endif // TEXTURECACHE_H_INCLUDED
