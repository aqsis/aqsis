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
 * \brief A texture cache interface
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef ITEXTURECACHE_H_INCLUDED
#define ITEXTURECACHE_H_INCLUDED

#include <aqsis/aqsis.h>

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

namespace Aqsis {

class IqTextureSampler;
class IqShadowSampler;
class IqOcclusionSampler;
class IqEnvironmentSampler;
class IqTiledTexInputFile;
class CqTexFileHeader;
class CqMatrix;

/** \brief A cache interface for managing the various types of texture
 * samplers.
 */
struct AQSIS_TEX_SHARE IqTextureCache
{
	/// Type for holding a search-path callback.
	typedef boost::function<const char* ()> TqSearchPathCallback;

	/** \brief Construct an empty texture cache.
	 *
	 * \param searchPathCallback - Function used to return the current search
	 * path when searching for textures.
	 */
	static boost::shared_ptr<IqTextureCache> create(
			TqSearchPathCallback searchPathCallback);

	virtual ~IqTextureCache() {}

	//--------------------------------------------------
	// \name Sampler access
	//@{
	/** \brief Find a texture sampler in the cache or load from file if not
	 * found.
	 *
	 * If any problems are encountered in opening the texture, issue a warning
	 * to Aqsis::log(), and return a dummy sampler.
	 *
	 * \param name - the texture file name.
	 */
	virtual IqTextureSampler& findTextureSampler(const char* name) = 0;
	virtual IqEnvironmentSampler& findEnvironmentSampler(const char* name) = 0;
	virtual IqShadowSampler& findShadowSampler(const char* name) = 0;
	virtual IqOcclusionSampler& findOcclusionSampler(const char* name) = 0;
	//@}

	//--------------------------------------------------
	/// Delete all textures from the cache
	virtual void flush() = 0;

	/** \brief Return the texture file attributes for the named file.
	 *
	 * If the file is not found or is otherwise invalid, return 0.
	 *
	 * \param name - file name 
	 */
	virtual const CqTexFileHeader* textureInfo(const char* name) = 0;

	/** \brief Set the current -> world transformation
	 *
	 * This transformation is used by shadow samplers in order to transform
	 * the provided shading coordinates into the coordinate system of the
	 * shadowed light.
	 *
	 * \param currToWorld - current -> world transformation.
	 */
	virtual void setCurrToWorldMatrix(const CqMatrix& currToWorld) = 0;
};


} // namespace Aqsis

#endif // ITEXTURECACHE_H_INCLUDED
