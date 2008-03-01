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
#include "exception.h"
#include "logging.h"
#include "texexception.h"

namespace Aqsis
{

//------------------------------------------------------------------------------
/** \brief Raster coordinate transformation coefficients
 *
 * Raster coordinates held by a filter need to be modified if the filter is
 * built for the level zero mipmap.  A simple scaling and translation of the
 * coordinates is sufficient.  Example transformation:
 *
 *   xNew = xScale * (xOld + xOffset)
 */
struct SqLevelTrans
{
	TqFloat xScale;
	TqFloat xOffset;
	TqFloat yScale;
	TqFloat yOffset;

	/// Default constructor: set scale factors to 1 and offsets to 0
	SqLevelTrans();
	/// Trivial constructor
	SqLevelTrans(TqFloat xScale, TqFloat xOffset,
			TqFloat yScale, TqFloat yOffset);
};


//------------------------------------------------------------------------------
/** \brief Cache for a set of associated texture buffers
 *
 * This class holds a set of associated 2D textures, such as mipmap levels,
 * independently of any of any details of the sampling procedure.
 *
 * Texture buffers in the cache are constructed on-demand.
 * 
 * In addition to holding a set of buffers, this class also caches the default
 * sampling options, as determined from attributes of the texture file.  When
 * these aren't present, we attempt to choose sensible defaults.
 */
template<typename TextureBufferT>
class CqMipmapLevelCache
{
	public:
		/** \brief Construct the cache from an open file.
		 *
		 * \param file - read texture data from here.
		 */
		CqMipmapLevelCache(const boost::shared_ptr<IqMultiTexInputFile>& file);
		/** \brief Get the buffer for a given level.
		 *
		 * \param levelNum - mipmap level to grab
		 */
		const TextureBufferT& level(TqInt levelNum);
		/** \brief Get the basetex-relative transformation for a mipmap level.
		 */
		const SqLevelTrans& levelTrans(TqInt levelNum);
		/// Get the number of levels in this mipmap.
		inline TqInt numLevels() const;
		/** \brief Get the default sample options associated with the texture file.
		 *
		 * \return The default sample options - these include options which
		 * were used when the texture was created (things like the texutre wrap
		 * mode).
		 */
		inline const CqTextureSampleOptions& defaultSampleOptions();
	private:
		/// Initialize all mipmap levels
		void initLevels();

		/// Texture file to retrieve all data from
		boost::shared_ptr<IqMultiTexInputFile> m_texFile;
		/** \brief List of samplers for mipmap levels.  The pointers to these
		 * may be NULL since they are created only on demand.
		 */
		mutable std::vector<boost::shared_ptr<TextureBufferT> > m_levels;
		/// Transformation information for each level.
		std::vector<SqLevelTrans> m_levelTransforms;
		/// Default texture sampling options for the set of mipmap levels.
		CqTextureSampleOptions m_defaultSampleOptions;
};


//==============================================================================
// Implementation details
//==============================================================================
// SqLevelTrans
inline SqLevelTrans::SqLevelTrans()
	: xScale(1),
	xOffset(0),
	yScale(1),
	yOffset(0)
{ }

inline SqLevelTrans::SqLevelTrans(TqFloat xScale, TqFloat xOffset,
		TqFloat yScale, TqFloat yOffset)
	: xScale(xScale),
	xOffset(xOffset),
	yScale(yScale),
	yOffset(yOffset)
{ }


// CqMipmapLevelCache
template<typename TextureBufferT>
CqMipmapLevelCache<TextureBufferT>::CqMipmapLevelCache(
			const boost::shared_ptr<IqMultiTexInputFile>& file)
	: m_texFile(file),
	m_levels(),
	m_levelTransforms(),
	m_defaultSampleOptions()
{
	assert(m_texFile);
	initLevels();
	m_defaultSampleOptions.fillFromFileHeader(m_texFile->header());
}

template<typename TextureBufferT>
void CqMipmapLevelCache<TextureBufferT>::initLevels()
{
	TqInt numLevels = m_texFile->numSubImages();
	m_levels.resize(numLevels);
	m_levelTransforms.reserve(m_texFile->numSubImages());
	m_levelTransforms.push_back(SqLevelTrans());
	// Read in base texture buffer
	m_texFile->setImageIndex(0);
	// Calculated level sizes (init with base texture dimensions.)
	TqInt levelWidth = m_texFile->header().width();
	TqInt levelHeight = m_texFile->header().height();
	// level offsets (in base-texture raster coordinates)
	TqFloat xOffset = 0;
	TqFloat yOffset = 0;
	for(TqInt i = 1; i < numLevels; ++i)
	{
		if(levelWidth == 1 && levelHeight == 1)
		{
			m_levels.resize(i);
			break;
		}
		m_texFile->setImageIndex(i);
		// Update offsets for the current level.
		if(levelWidth % 2 == 0)
		{
			// Previous level has an even width; add mipmap offset
			xOffset += 0.5*(1 << (i-1));
		}
		if(levelHeight % 2 == 0)
		{
			// Previous level has an even height; add mipmap offset
			yOffset += 0.5*(1 << (i-1));
		}
		// compute expected level dimensions
		levelWidth = max((levelWidth+1)/2, 1);
		levelHeight = max((levelHeight+1)/2, 1);
		// check expected dimensions against actual dimensions.
		if(levelWidth != m_texFile->header().width()
				|| levelHeight != m_texFile->header().height())
		{
			throw XqBadTexture("Mipmap level has incorrect size", __FILE__, __LINE__);
		}
		// set up scaling and offset transformation for this level.
		TqFloat levelScale = 1.0/(1 << i);
		m_levelTransforms.push_back( SqLevelTrans(
				levelScale, -xOffset,
				levelScale, -yOffset) );
	}
	// Check that we have the expected number of mipmap levels.
	if(levelWidth != 1 || levelHeight != 1)
	{
		Aqsis::log() << warning << "Texture \"" << m_texFile->fileName() << "\" "
			<< "has less than the expected number of mipmap levels. "
			<< "(smallest level: " << levelWidth << "x" << levelHeight << ")\n";
	}
}

template<typename TextureBufferT>
inline TqInt CqMipmapLevelCache<TextureBufferT>::numLevels() const
{
	return m_levels.size();
}

template<typename TextureBufferT>
inline const CqTextureSampleOptions& CqMipmapLevelCache<TextureBufferT>::defaultSampleOptions()
{
	return m_defaultSampleOptions;
}

template<typename TextureBufferT>
const TextureBufferT& CqMipmapLevelCache<TextureBufferT>::level(TqInt levelNum)
{
	assert(levelNum < static_cast<TqInt>(m_levels.size()));
	assert(levelNum >= 0);
	if(!m_levels[levelNum])
	{
		Aqsis::log() << debug << "loaded subtexture " << levelNum
			<< " from texture " << m_texFile->fileName() << "\n";
		// read in requested level if it's not loaded yet.
		m_levels[levelNum].reset(new TextureBufferT());
		m_texFile->setImageIndex(levelNum);
		m_texFile->readPixels(*m_levels[levelNum]);
	}
	return *m_levels[levelNum];
}

template<typename TextureBufferT>
const SqLevelTrans& CqMipmapLevelCache<TextureBufferT>::levelTrans(TqInt levelNum)
{
	assert(levelNum < static_cast<TqInt>(m_levelTransforms.size()));
	assert(levelNum >= 0);
	return m_levelTransforms[levelNum];
}

} // namespace Aqsis

#endif // LEVELSAMPLERCACHE_H_INCLUDED
