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
		CqTileArray( const boost::shared_ptr<IqTiledTexInputFile>& inFile,
				const boost::shared_ptr<CqMemorySentry>& memSentry
					= boost::shared_ptr<CqMemorySentry>() );

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
		 * Algorithms which need to act on the whole image may need access to
		 * the underlying tiles for efficiency (for instance, mipmap
		 * generation).  Unless efficiency is really an issue, the value()
		 * function should be used instead.
		 *
		 * \param x - index in width direction (column index)
		 * \param y - index in height direction (row index)
		 *
		 * \return The tile holding the underlying data at the given indices.
		 */
		TileT& tileForIndex(const TqInt x, const TqInt y) const;

		/** Allocate a new texture tile from the file.
		 */
		void allocateTile();

		boost::scoped_array<boost::intrusive_ptr<CqTileHolder> > m_tiles;
		/// Underlying texture file.
		boost::shared_ptr<IqTiledTexInputFile> m_inFile;
		/// Width of the array
		TqInt m_width;
		/// Height of the array
		TqInt m_height;
		/// Width of the tiles making up the array
		TqInt m_tileWidth;
		/// Height of tiles making up the the array
		TqInt m_tileHeight;
		/// Number of channels per pixel
		TqInt m_numChannels;
};



//==============================================================================
// Implementation details
//==============================================================================

/// Key to use when finding tiles in std::maps.
struct TileKey
{
	TqInt x;
	TqInt y;
	bool operator<(const TileKey& rhs)
	{
		return x < rhs.x || y < rhs.y;
	}
};

//------------------------------------------------------------------------------
// CqTileArray Implementation
template<typename TileT>
CqTileArray<TileT>::CqTileArray( const boost::shared_ptr<IqTiledTexInputFile>& inFile,
		const boost::shared_ptr<CqMemorySentry>& memSentry)
	: CqMemoryMonitored(memSentry)
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
CqSampleVector<TileT> CqTileArray<TileT>::operator()(const TqInt x, const TqInt y) const
{
	return (*tileForIndex(x/m_tileWidth, y/m_tileHeight))(x,y);
}


template<typename TileT>
TileT& CqTileArray<TileT>::tileForIndex(const TqInt x, const TqInt y) const
{
	/// \todo Implementation
}

template<typename TileT>
CqMemorySentry::TqMemorySize CqTileArray<TileT>::zapMemory()
{
	/// \todo Implementation
	return 0;
}

template<typename TileT>
CqTileArray<TileT>::~CqTileArray()
{
	/// \todo Implementation
}

//------------------------------------------------------------------------------
} // namespace Aqsis

#endif // TILE_ARRAY_H_INCLUDED
