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
