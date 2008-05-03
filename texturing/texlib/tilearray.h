// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
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

/**
 * \file
 *
 * \brief Declare array class which holds data as tiles.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 */

#ifndef TILE_ARRAY_H_INCLUDED
#define TILE_ARRAY_H_INCLUDED

#include "aqsis.h"

#include <vector>

#include <boost/intrusive_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>

//#include "memorysentry.h"

namespace Aqsis {

class IqTiledTexInputFile;

//------------------------------------------------------------------------------
/** \brief 2D array interface for tiled data aimed at tiled textures.
 */
template<typename TileT>
class AQSISTEX_SHARE CqTileArray : boost::noncopyable //, public CqMemoryMonitored
{
	public:
		/** \brief Construct a tiled texture array connected to a file
		 */
		CqTileArray(const boost::shared_ptr<IqTiledTexInputFile>& inFile);

		//--------------------------------------------------
		/// \name Access to buffer dimensions & metadata
		//@{
		/// Get array width
		TqInt width() const;
		/// Get array height
		TqInt height() const;
		/// Get the number of samples per pixel
		TqInt numChannels() const;
		//@}

		//--------------------------------------------------
		/// \name Pixel access
		//@{
		/** \brief Access to pixels through a pixel iterator
		 *
		 * The pixel iterator will iterate through all the pixels the provided
		 * support.
		 *
		 * \param support - support to iterate over
		 */
		CqIterator begin(const SqFilterSupport& support) const;
		/** \brief Stochastic iterator access to pixels in the given support.
		 *
		 * A stochastic support iterator aims to choose a fixed number of
		 * samples from the support randomly.  The randomness choices should
		 * ideally be spread evenly over the support with minimal bunching.
		 *
		 * \param support - support to iterate over
		 * \param numSamples - number of samples to choose in the support.
		 */
		CqStochasticIterator beginStochastic(const SqFilterSupport& support,
				TqInt numSamples) const;
		//@}

		/// Destructor
		virtual ~CqTileArray();
	private:
		class CqTileHolder
		{
			TqInt refCount;
			boost::scoped_ptr<TileT> tile;

			CqTileHolder(TileT* tile)
				: refCount(0),
				tile(tile)
			{ }
		};

		/** \brief Access to the underlying tiles
		 *
		 * \return The tile holding the underlying data at the given indices.
		 */
		TileT& getTile(const TqInt x, const TqInt y) const;

		/// Underlying texture file.
		boost::shared_ptr<IqTiledTexInputFile> m_inFile;
		/// Width of the array
		TqInt m_width;
		/// Height of the array
		TqInt m_height;
		/// Number of channels per pixel
		TqInt m_numChannels;
		/// Width of the tiles making up the array
		TqInt m_tileWidth;
		/// Height of tiles making up the the array
		TqInt m_tileHeight;
		/// Width of the array
		TqInt m_widthInTiles;
		/// Height of the array
		TqInt m_heightInTiles;
		/// "2D" array of tiles.  Tiles may be founnd in O(1) time using this array.
		boost::scoped_array<boost::intrusive_ptr<CqTileHolder> > m_tiles;
};


//==============================================================================
// Implementation details
//==============================================================================
// CqTileArray Implementation
template<typename TileT>
CqTileArray<TileT>::CqTileArray(const boost::shared_ptr<IqTiledTexInputFile>& inFile)
	: m_inFile(inFile),
	m_width(inFile->header().width()),
	m_height(inFile->header().height()),
	m_numChannels(inFile->header().channels().numChannels()),
	m_tileWidth(inFile->tileInfo().width),
	m_tileHeight(inFile->tileInfo().height),
	m_widthInTiles((m_width-1)/m_tileWidth + 1),
	m_heightInTiles((m_height-1)/m_tileHeight + 1),
	m_tiles(new boost::intrusive_ptr<CqTileHolder>[m_widthInTiles*m_heightInTiles])
{ }

template<typename TileT>
inline TqInt CqTileArray<TileT>::width() const
{
	return m_width;
}

template<typename TileT>
inline TqInt CqTileArray<TileT>::height() const
{
	return m_height;
}

template<typename TileT>
inline TqInt CqTileArray<TileT>::numChannels() const
{
	return m_numChannels;
}

template<typename TileT>
CqTileArray<TileT>::CqIterator CqTileArray<TileT>::begin(
		const SqFilterSupport& support) const
{
}

template<typename TileT>
CqTileArray<TileT>::CqStochasticIterator CqTileArray<TileT>::beginStochastic(
		const SqFilterSupport& support, TqInt numSamples) const
{
}

template<typename TileT>
TileT& CqTileArray<TileT>::getTile(const TqInt x, const TqInt y) const
{
	/// \todo Implementation
}

//------------------------------------------------------------------------------
} // namespace Aqsis

#endif // TILE_ARRAY_H_INCLUDED
