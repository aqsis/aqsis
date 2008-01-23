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

#ifndef LEVELSAMPLERCACHE_H_INCLUDED
#define LEVELSAMPLERCACHE_H_INCLUDED

#include "aqsis.h"

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "itexinputfile.h"
#include "texturesampleoptions.h"

namespace Aqsis
{

//------------------------------------------------------------------------------
/** \brief Cache for a set of associated texture buffers
 *
 * This class holds a set of associated 2D textures, such as mipmap levels,
 * independently of any of any details of the sampling procedure.
 *
 * Samplers for textures in the cache are constructed on-demand.
 * 
 * In addition to holding a set of samplers, this class also caches the default
 * sampling options, as determined from attributes of the texture file.  When
 * these aren't present, we attempt to choose sensible defaults.
 */
template<typename TextureBufferT>
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
		CqLevelSamplerCache(const boost::shared_ptr<IqMultiTexInputFile>& file);
		/** \brief Get the texture sampler for a given level.
		 *
		 * \param levelNum - mipmap level to grab
		 */
		const TextureBufferT& level(TqInt levelNum);
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
		mutable std::vector<boost::shared_ptr<TextureBufferT> > m_levels;
		boost::shared_ptr<IqMultiTexInputFile> m_texFile;
};


//==============================================================================
// Implementation details
//==============================================================================
// CqLevelSamplerCache
template<typename TextureBufferT>
CqLevelSamplerCache<TextureBufferT>::CqLevelSamplerCache(
			const boost::shared_ptr<IqMultiTexInputFile>& file)
	: m_defaultSampleOptions(),
	m_levels(),
	m_texFile(file)
{
	if(m_texFile)
	{
		/// \todo verify that we do indeed have a mipmap.
		//
		m_levels.resize(m_texFile->numSubImages());
		/// \todo Defer the texture loading below to read-time.
		for(TqInt i = 0; i < static_cast<TqInt>(m_levels.size()); ++i)
		{
			m_texFile->setImageIndex(i);
			m_levels[i].reset(new TextureBufferT());
			m_texFile->readPixels(*m_levels[i]);
		}
		m_defaultSampleOptions.fillFromFileHeader(m_texFile->header());
	}
	else
	{
		m_levels.resize(1);
		// A dummy texture; 1x1 with 1 channel.
		m_levels[0].reset(new TextureBufferT(1,1,1));
	}
}

template<typename TextureBufferT>
inline TqInt CqLevelSamplerCache<TextureBufferT>::numLevels() const
{
	return m_levels.size();
}

template<typename TextureBufferT>
inline const CqTextureSampleOptions& CqLevelSamplerCache<TextureBufferT>::defaultSampleOptions()
{
	return m_defaultSampleOptions;
}

template<typename TextureBufferT>
const TextureBufferT& CqLevelSamplerCache<TextureBufferT>::level(TqInt levelNum)
{
	assert(levelNum < static_cast<TqInt>(m_levels.size()));
	assert(levelNum >= 0);
	TextureBufferT* sampler = m_levels[levelNum].get();
//	if(!sampler)
//	{
//		m_levels[levelNum] = TextureBufferT::create(m_texFile);
//		sampler = m_levels[levelNum].get();
//	}
	return *sampler;
}

} // namespace Aqsis

#endif // LEVELSAMPLERCACHE_H_INCLUDED
