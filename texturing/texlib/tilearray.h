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
 * \brief Declare array class which holds data as tiles and competes with other
 * arrays for memory via a common cache.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 */

#ifndef TILE_ARRAY_H_INCLUDED
#define TILE_ARRAY_H_INCLUDED

#include "aqsis.h"

#include <limits>
#include <map>
#include <vector>

#include <boost/intrusive_ptr.hpp>
#include <boost/shared_array.hpp>
#include <tiffio.h>

#include "memorysentry.h"
#include "texturetile.h"


namespace Aqsis {


//------------------------------------------------------------------------------
/** \brief 2D array interface for tiled data - aimed at tiled textures.
 *
 * This class specifies an array interface to 2D tiled data.  The design is
 * based on the premise that each position in the array contains a vector of
 * samples.  Getting a reference to this samples vector requires hunting
 * through sets of tiles, and as such may be slow.
 *
 * A primary design requirement is that once the vector of samples is obtained
 * it should be very fast to access.  This is particularly important for large
 * numbers of samples.
 *
 */
template<typename T>
class CqTileArray : public CqMemoryMonitored
{
	public:
		/** \brief constructor
		 *
		 * \param width - array width
		 * \param height - array height
		 */
		CqTileArray(const TqUint width, const TqUint height);

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
		virtual CqSampleVector<T> value(const TqUint x, const TqUint y) const = 0;

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
		//virtual void setValue(const TqUint x, const TqUint y, const std::vector<TqFloat>& newValue) = 0;

		/** \brief Get array width
		 *
		 * \return array width
		 */
		inline TqUint width() const;

		/** \brief Get array height
		 *
		 * \return array height
		 */
		inline TqUint height() const;
	protected:
		/// Destructor
		virtual ~CqTileArray() = 0;

		/// Width of the array
		TqUint m_width;
		/// Height of the array
		TqUint m_height;
};


//------------------------------------------------------------------------------
/** \brief Array class for which the underlying tiles are held in tiles.
 *
 * This class is currently designed to be a wrapper around TIFF files.  It
 * wraps both tiled and strip-based TIFF storage in a uniform manner, as a set
 * of tiles which may be individually allocated and deallocated.
 */
template<typename T>
class CqTextureTileArray : public CqTileArray<T>
{
	public:
		/** \brief Construct a tiled texture array from a tiff file.
		 *
		 * The constructor throws an error if the file cannot be read.
		 *
		 */
		CqTextureTileArray(std::string fileName, TIFF* openTiff, TqUint tiffDirectory = 0);

		// Inherited
		virtual CqSampleVector<T> value(const TqUint x, const TqUint y) const;
		//virtual void setValue(const TqUint x, const TqUint y, const std::vector<TqFloat>& newValue);

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
		boost::intrusive_ptr<CqTextureTile<T> >& tileForIndex(const TqUint x, const TqUint y) const;

		/** \brief Get the number of samples per pixel
		 *
		 * \return number of samples per pixel
		 */
		inline TqUint samplesPerPixel() const;

		// Inherited from CqMemoryMonitored
		virtual CqMemorySentry::TqMemorySize zapMemory();

		/// Destructor
		virtual ~CqTextureTileArray();
	private:
		/// Key to use when finding tiles in std::maps.
		typedef std::pair<TqUint, TqUint> TileKey;

		/** Allocate a new texture tile from the TIFF file.
		 */
		void allocateTile();

		/// Number of samples per pixel
		TqUint m_samplesPerPixel;
		/** A list holding recently used or "hot" tiles in the order of
		 * hotness.  The same tiles are also held in hotMap.
		 */
		std::list<boost::intrusive_ptr<CqTextureTile<T> > > hotList;
		/** A map to hold recently used or "hot" tiles.  The same tiles are
		 * also held in hotList.
		 */
		std::map<TileKey, boost::intrusive_ptr<CqTextureTile<T> > > hotMap;
		/// A map holding tiles which haven't been used for a while; "cold tiles"
		std::map<TileKey, boost::intrusive_ptr<CqTextureTile<T> > > coldMap;
};



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Implementation of inline functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Inline functions for CqTileArray
//------------------------------------------------------------------------------

template<typename T>
inline TqUint CqTileArray<T>::width() const
{
	return m_width;
}

template<typename T>
inline TqUint CqTileArray<T>::height() const
{
	return m_height;
}

template<typename T>
CqTileArray<T>::~CqTileArray<T>()
{ }


//------------------------------------------------------------------------------
// Implementation of CqTextureTileArray
//------------------------------------------------------------------------------
// Inline functions for CqTextureTileArray
//

template<typename T>
inline TqUint CqTextureTileArray<T>::samplesPerPixel() const
{
	return m_samplesPerPixel;
}

//------------------------------------------------------------------------------
// Template Implementations for CqTextureTileArray
//

#if 0
template<typename T>
CqTextureTileArray<T>::CqTextureTileArray<T>(std::string fileName, TqUint tiffDirectory)
	: m_tiffDirectory(tiffDirectory),
	m_fileName(fileName)
{
	/// \todo Implementation
	//CqTileArray<T>()
}
#endif

/* Stuff to go in CqTextureMap
	// Validate the filename
	CqRiFile textureFile(fileName.c_str(), "texture");
	if(!textureFile.IsValid())
	{
		Aqsis::log() << error << "Cannot open texture file '" << fileName << "'; using default texture\n";
		m_validFile = false;
	}
	else
	{

	}
*/

template<typename T>
CqSampleVector<T> CqTextureTileArray<T>::value(const TqUint x, const TqUint y) const
{
	return tileForIndex(x, y)->value();
}

/*template<typename T>
void CqTextureTileArray<T>::setValue(const TqUint x, const TqUint y, const std::vector<TqFloat>& newValue)
{
	/// \todo Implementation
}*/

template<typename T>
boost::intrusive_ptr<CqTextureTile<T> >& CqTextureTileArray<T>::tileForIndex(const TqUint x, const TqUint y) const
{
	/// \todo Implementation
	// if() 
	return boost::intrusive_ptr<CqTextureTile<T> > (0); // dodgy; get the stub to compile...
}

template<typename T>
CqMemorySentry::TqMemorySize CqTextureTileArray<T>::zapMemory()
{
	/// \todo Implementation
	return 0;
}

template<typename T>
CqTextureTileArray<T>::~CqTextureTileArray<T>()
{
	/// \todo Implementation
}

//------------------------------------------------------------------------------
} // namespace Aqsis

#endif // TILE_ARRAY_H_INCLUDED
