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

#include <map>
#include <vector>

#include <boost/intrusive_ptr.hpp>
#include <boost/shared_array.hpp>
#include <tiffio.h>

#include "memorysentry.h"
#include "texturebuffer.h"


namespace Aqsis {

class IqTiledTexInputFile;
struct TileKey;

//------------------------------------------------------------------------------
/** \brief 2D array interface for tiled data - aimed at tiled textures.
 *
 * This class specifies an array interface to 2D tiled data.  The design is
 * based on the premise that each position in the array contains a short fixed
 * length vector of samples.  Getting a reference to this sample vector
 * requires hunting through sets of tiles, and as such may be slow.
 *
 * A primary design requirement is that samples should be as fast as possible
 * to obtain.  If the current implementation doesn't suffice, a possible
 * optimization when accessing blocks of texture (such as during filtering) is
 * to return a list of tiles which cover the block in question.  The filtering
 * code could then perform an explicit loop over the pixels in each tile, which
 * would save the overhead incurred by tile search.  Something like this might
 * be very necessary for reasonable performance in a multithreaded environment
 * where two threads could contend for the same texture.
 */
template<typename T>
class AQSISTEX_SHARE CqTileArray : public CqMemoryMonitored
{
	public:
		/** \brief Construct a tiled texture array connected to a file
		 *
		 * The constructor throws an error if the file cannot be read.
		 *
		 */
		CqTileArray( const boost::shared_ptr<IqTiledTexInputFile>& texFile,
				const boost::shared_ptr<CqMemorySentry>& memSentry
					= boost::shared_ptr<CqMemorySentry>() );

		/** \brief Get the value of the array at the given position
		 *
		 * This function provides access to the underlying sample data at a
		 * particular position in the array.  
		 *
		 * Positions count from zero in the top-left corner of the image.
		 *
		 * \param x - index in width direction (column index)
		 * \param y - index in height direction (row index)
		 * \return a lightweight vector holding a reference to the sample data
		 */
		CqSampleVector<T> operator()(const TqInt x, const TqInt y) const;

		/** \brief Set the value of the array at a particular position
		 *
		 * \todo It mightn't be desirable to support this operation within the
		 * context of this class - perhaps it should go in derived classes for
		 * which it makes sense.  Further investigation required.
		 *
		 * Positions count from zero in the top-left corner of the image.
		 *
		 * \param x - index in width direction (column index)
		 * \param y - index in height direction (row index)
		 * \param newValue - New value for the
		 */
		//void setValue(const TqInt x, const TqInt y, const std::vector<TqFloat>& newValue) = 0;

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
		boost::intrusive_ptr<CqTextureBuffer<T> >& tileForIndex(const TqInt x, const TqInt y) const;

		/// Get the number of samples per pixel
		TqInt samplesPerPixel() const;

		/// Get array width
		TqInt width() const;
		/// Get array height
		TqInt height() const;

		// Inherited from CqMemoryMonitored
		virtual CqMemorySentry::TqMemorySize zapMemory();

		/// Destructor
		virtual ~CqTileArray();
	private:
		/** Allocate a new texture tile from the file.
		 */
		void allocateTile();

		/// Width of the array
		TqInt m_width;
		/// Height of the array
		TqInt m_height;
		/// Width of the tiles making up the array
		TqInt m_tileWidth;
		/// Height of tiles making up the the array
		TqInt m_tileHeight;
		/// Number of samples per pixel
		TqInt m_samplesPerPixel;
		/// The tile which was last accessed.
		boost::intrusive_ptr<CqTextureBuffer<T> > m_lastUsedTile;
		/** A list holding recently used or "hot" tiles in the order of
		 * hotness.  The same tiles are also held in hotMap.
		 */
		std::list<boost::intrusive_ptr<CqTextureBuffer<T> > > m_hotList;
		/** A map to hold recently used or "hot" tiles.  The same tiles are
		 * also held in hotList.
		 */
		std::map<TileKey, boost::intrusive_ptr<CqTextureBuffer<T> > > m_hotMap;
		/// A map holding tiles which haven't been used for a while; "cold tiles"
		std::map<TileKey, boost::intrusive_ptr<CqTextureBuffer<T> > > m_coldMap;
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
template<typename T>
CqTileArray<T>::CqTileArray( const boost::shared_ptr<IqTiledTexInputFile>& texFile,
		const boost::shared_ptr<CqMemorySentry>& memSentry)
	: CqMemoryMonitored(memSentry)
{ }

template<typename T>
inline TqInt CqTileArray<T>::width() const
{
	return m_width;
}

template<typename T>
inline TqInt CqTileArray<T>::height() const
{
	return m_height;
}

template<typename T>
inline TqInt CqTileArray<T>::samplesPerPixel() const
{
	return m_samplesPerPixel;
}

template<typename T>
CqSampleVector<T> CqTileArray<T>::operator()(const TqInt x, const TqInt y) const
{
	return (*tileForIndex(x/m_tileWidth, y/m_tileHeight))(x,y);
}


template<typename T>
boost::intrusive_ptr<CqTextureBuffer<T> >& CqTileArray<T>::tileForIndex(const TqInt x, const TqInt y) const
{
	/// \todo Implementation
	if(m_lastUsedTile) 
	return boost::intrusive_ptr<CqTextureBuffer<T> > (0); // dodgy; get the stub to compile...
}

template<typename T>
CqMemorySentry::TqMemorySize CqTileArray<T>::zapMemory()
{
	/// \todo Implementation
	return 0;
}

template<typename T>
CqTileArray<T>::~CqTileArray()
{
	/// \todo Implementation
}

//------------------------------------------------------------------------------
} // namespace Aqsis

#endif // TILE_ARRAY_H_INCLUDED
