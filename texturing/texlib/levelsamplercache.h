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
 * \brief Declare a class for holding a collection of mipmap levels.
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef MIPMAPLEVELS_H_INCLUDED
#define MIPMAPLEVELS_H_INCLUDED

#include "aqsis.h"

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "texturesampleoptions.h"

namespace Aqsis
{

class IqTextureSampler;
class IqTiledTexInputFile;

//------------------------------------------------------------------------------
/** \brief Cache for a set of associated texture samplers
 *
 * This class holds texture samplers for a set of associated 2D textures, such
 * as mipmap levels.  The class is independent of any of any details of the
 * sampling procedure.
 *
 * Samplers for textures in the cache are constructed only on-demand.
 * 
 * CqLevelSamplerCache is the class which will be held in the high-level
 * texture cache (yes, caches within caches).
 *
 * In addition to holding a set of samplers, this class also caches the default
 * sampling options, as determined from attributes of the texture file.  When
 * these aren't present, we attempt to choose sensible defaults.
 */
class CqLevelSamplerCache
{
	public:
		/** \brief Construct the sampler cache from an open file.
		 *
		 * If the file pointer is null, this class is still constructed
		 * sucessfully.  However, the number of levels is then set to one, and
		 * that level is a dummy texture consisting of a single pixel.  This
		 * allows us to gracefully recover from missing files.
		 *
		 * \param file - read texture data from here.
		 */
		CqLevelSamplerCache(const boost::shared_ptr<IqTiledTexInputFile>& file);
		/** \brief Get the texture sampler for a given level.
		 *
		 * \param levelNum - mipmap level to grab
		 */
		const IqTextureSampler& level(TqInt levelNum);
		/** \brief Get the number of levels in this mipmap.
		 *
		 */
		inline TqInt numLevels() const;
		/** \brief Get the default sample options associated with the texture file.
		 *
		 * \return The default sample options - these include options which
		 * were used when the texture was created (things like the texutre wrap
		 * mode).
		 */
		inline const CqTextureSampleOptions& defaultSampleOptions();
	private:
		/// Default texture sampling options for the set of mipmap levels.
		CqTextureSampleOptions m_defaultSampleOptions;
		/** \brief List of samplers for mipmap levels.  The pointers to these
		 * may be NULL since they are created only on demand.
		 */
		mutable std::vector<boost::shared_ptr<IqTextureSampler> > m_levels;
		boost::shared_ptr<IqTiledTexInputFile> m_texFile;
};


//==============================================================================
// Implementation details
//==============================================================================
inline TqInt CqLevelSamplerCache::numLevels() const
{
	return m_levels.size();
}

inline const CqTextureSampleOptions& CqLevelSamplerCache::defaultSampleOptions()
{
	return m_defaultSampleOptions;
}

} // namespace Aqsis

#endif // MIPMAPLEVELS_H_INCLUDED
