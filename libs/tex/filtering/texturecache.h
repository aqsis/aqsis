// Aqsis
// Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the software's owners nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// (This is the New BSD license)

/** \file
 *
 * \brief A texture cache
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef TEXTURECACHE_H_INCLUDED
#define TEXTURECACHE_H_INCLUDED

#include <aqsis/aqsis.h>

#include <map>

#include <boost/utility.hpp>

#include <aqsis/tex/filtering/itexturecache.h>
#include <aqsis/math/matrix.h>

namespace Aqsis {

class IqTextureSampler;
class IqShadowSampler;
class IqOcclusionSampler;
class IqEnvironmentSampler;
class IqTiledTexInputFile;
class CqTexFileHeader;

/** \brief A cache managing the various types of texture samplers.
 */
#ifdef AQSIS_SYSTEM_WIN32
class AQSIS_TEX_SHARE boost::noncopyable_::noncopyable;
#endif
class AQSIS_TEX_SHARE CqTextureCache : public IqTextureCache, private boost::noncopyable
{
	public:
		/** \brief Construct an empty texture cache.
		 *
		 * \param searchPathCallback - Function used to return the current
		 * search path when searching for textures.
		 */
		CqTextureCache(TqSearchPathCallback searchPathCallback);

		// Inherited from IqTextureCache
		virtual IqTextureSampler& findTextureSampler(const char* name);
		virtual IqEnvironmentSampler& findEnvironmentSampler(const char* name);
		virtual IqShadowSampler& findShadowSampler(const char* name);
		virtual IqOcclusionSampler& findOcclusionSampler(const char* name);
		virtual void flush();
		virtual const CqTexFileHeader* textureInfo(const char* name);
		virtual void setCurrToWorldMatrix(const CqMatrix& currToWorld);

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
		std::map<TqUlong, boost::shared_ptr<IqOcclusionSampler> > m_occlusionCache;
		/// Cached texture files live in here:
		std::map<TqUlong, boost::shared_ptr<IqTiledTexInputFile> > m_texFileCache;
		/// Camera -> world transformation - used for creating shadow maps.
		CqMatrix m_currToWorld;
		/// Callback function to obtain the current texture search path.
		TqSearchPathCallback m_searchPathCallback;
};


} // namespace Aqsis

#endif // TEXTURECACHE_H_INCLUDED
