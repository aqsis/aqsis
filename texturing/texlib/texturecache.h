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

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include "matrix.h"

namespace Aqsis {

class IqTextureSampler;
class IqShadowSampler;
class IqEnvironmentSampler;
class IqTiledTexInputFile;
class CqTexFileHeader;

/** \brief A cache managing the various types of texture samplers.
 */
class AQSISTEX_SHARE CqTextureCache : boost::noncopyable
{
	public:
		/// Type for holding a search-path callback.
		typedef boost::function<const char* ()> TqSearchPathCallback;

		/** \brief Construct an empty texture cache.
		 */
		CqTextureCache(TqSearchPathCallback searchPathCallback);

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
		IqTextureSampler& findTextureSampler(const char* name);
		/** \brief Find a texture sampler in the cache or load from file if not found.
		 *
		 * If any problems are encountered in opening the texture, issue a
		 * warning to Aqsis::log(), and return a dummy sampler.
		 *
		 * \param name - the texture name.
		 */
		IqEnvironmentSampler& findEnvironmentSampler(const char* name);
		/** \brief Find a shadow sampler in the cache or load from file if not found.
		 *
		 * If any problems are encountered in opening the shadow texture, issue
		 * a warning to Aqsis::log(), and return a dummy sampler.
		 *
		 * \param name - the texture name.
		 */
		IqShadowSampler& findShadowSampler(const char* name);
		/** \brief Delete all textures from the cache
		 */
		void flush();
		//@}

		//--------------------------------------------------
		/** \brief Return the texture file attributes for the named file.
		 *
		 * If the file is not found or is otherwise invalid, return 0.
		 *
		 * \param name - file name 
		 */
		const CqTexFileHeader* textureInfo(const char* name);

		/** \brief Set the current -> world transformation
		 *
		 * This transformation is used by shadow samplers in order to transform
		 * the provided shading coordinates into the coordinate system of the
		 * shadowed light.
		 *
		 * \param currToWorld - current -> world transformation.
		 */
		void setCurrToWorldMatrix(const CqMatrix& currToWorld);

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
		/** \brief Retrive a texture file from the cache, or open it from file.
		 *
		 * First search for the given file name in the cache.  If it's not
		 * there, grab the file from disk (note that this may throw an
		 * XqInvalidFile if it's not found).
		 *
		 * \param name - file name to open.
		 */
		boost::shared_ptr<IqTiledTexInputFile> getTextureFile(const char* name);
		/** \brief Create a sampler of the given type from a file.
		 *
		 * SamplerT - is a sampler type to instantiate.
		 *
		 * \param name - absolute path to file
		 */
		template<typename SamplerT>
		boost::shared_ptr<SamplerT> newSamplerFromFile(
				const boost::shared_ptr<IqTiledTexInputFile>& file);

		/// Cached textures live in here
		std::map<TqUlong, boost::shared_ptr<IqTextureSampler> > m_textureCache;
		std::map<TqUlong, boost::shared_ptr<IqEnvironmentSampler> > m_environmentCache;
		std::map<TqUlong, boost::shared_ptr<IqShadowSampler> > m_shadowCache;
		/// Cached texture files live in here:
		std::map<TqUlong, boost::shared_ptr<IqTiledTexInputFile> > m_texFileCache;
		/// Camera -> world transformation - used for creating shadow maps.
		CqMatrix m_currToWorld;
		/// Callback function to obtain the current texture search path.
		TqSearchPathCallback m_searchPathCallback;
};


} // namespace Aqsis

#endif // TEXTURECACHE_H_INCLUDED
