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

#include <limits>
#include <map>
#include <vector>

#include <boost/intrusive_ptr.hpp>
#include <tiffio.h>

#include "aqsis.h"
#include "memorysentry.h"


namespace Aqsis {


//------------------------------------------------------------------------------
/** \brief Very simple class providing reference counting machinery via
 * boost::intrusive_ptr.
 *
 * Classes to be counted with an boost::intrusive_ptr should inherit from this class. 
 */
class CqIntrusivePtrCounted
{
	public:
		/// Get the number of references count for this object
		inline TqUint referenceCount();
	protected:
		/// Construct a reference counted object
		inline CqIntrusivePtrCounted();
		virtual ~CqIntrusivePtrCounted();
	private:
		/// Increase the reference count; required for boost::intrusive_ptr
		friend inline void intrusive_ptr_add_ref(CqIntrusivePtrCounted* ptr);
		/// Decrease the reference count; required for boost::intrusive_ptr
		friend inline void intrusive_ptr_release(CqIntrusivePtrCounted* ptr);
		/// reference count for use with boost::intrusive_ptr
		TqUint m_referenceCount;
};


//------------------------------------------------------------------------------
/** \brief A minimal vector interface to the colour samples held by another object.
 *
 * The object which holds 
 *
 * The idea of this class is to provide the absolute minimal interface to array
 * data managed by another object, but in a way which ensures that the data
 * which this class points to (but does not own) is not deallocated by another
 * thread.
 */
template<typename T>
class CqSampleVector
{
	public:
		/** \brief Construct a sample vector.
		 *
		 * \param data - the raw vector sample data.
		 * \param parentTile - the array tile which the sample data belongs to.
		 */
		inline CqSampleVector(const T* data, boost::intrusive_ptr<CqIntrusivePtrCounted>& parentTile);
		/** \brief Indexing operator for this vector of data.
		 *
		 * If the vector holds integer data, this function normalises the data
		 * by the maximum integer value.  Therefore, for (unsigned) integers the
		 * range returned by operator[] is between 0 and 1.  For floating point
		 * data, no extra normalisation is used.
		 *
		 * \param index - the index at which to obtain a sample.
		 */
		inline TqFloat operator[](TqUint index) const;
	private:
		/// Raw pointer to the data held by this vector
		const T* m_data;
		/** A pointer to the parent tile; this is simply to protect the tile
		 *  data from deallocation in the multithreaded case.
		 */
		boost::intrusive_ptr<CqIntrusivePtrCounted> m_parentTile;
};


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
		virtual void setValue(const TqUint x, const TqUint y, const std::vector<TqFloat>& newValue) = 0;

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
/** \brief A class to hold the data from a single texture tile.
 *
 * This class is a container for tiles of texture data for which each pixel
 * holds the same number of samples as every other pixel.
 *
 * Currently it's minimally tied to the TIFF format
 */
template<typename T>
class CqTextureTile : public CqIntrusivePtrCounted
{
	public:
		/** \brief Construct a texture tile
		 *
		 * \todo Investigate the use of an allocator or such for the TIFF data.
		 *
		 * \param data - raw tile data, CqTextureTile takes responsibility for
		 *   freeing this memory; it's assumed to have been allocated with
		 *   _TIFFmalloc.
		 * \param width - tile width
		 * \param height - tile height
		 * \param topLeftX - top left pixel X-position in the larger array
		 * \param topLeftY - top left pixel Y-position in the larger array
		 * \param samplesPerPixel - number of samples of type T per pixel.
		 */
		CqTextureTile(boost::shared_array<T> data,
				const TqUint width, const TqUint height,
				const TqUint topLeftX, const TqUint topLeftY,
				const TqUint samplesPerPixel);

		/** \brief Destructor
		 */
		~CqTextureTile();

		/** \brief Get a reference to the tile samples at the given position.
		 *
		 * Positions are in image coordinates, not tile coordinates, counting
		 * from zero in the top-left
		 *
		 * \param x - pixel index in width direction (column index)
		 * \param y - pixel index in height direction (row index)
		 * \return a lightweight vector holding a reference to the sample data
		 */
		inline CqSampleVector<T> value(const TqUint x, const TqUint y) const;

		/** \brief Set the value of the tile samples at the given position.
		 *
		 * If the template parameter T is an integral type, renormalise the
		 * new values so that an input value of 1.0 corresponds to the maximum
		 * of the integral type, and clamp.  If T is floating point, simply
		 * copy the new values.
		 *
		 * Positions are in image coordinates, not tile coordinates, counting
		 * from zero in the top-left.
		 *
		 * \todo Is there a good way to make sure that only finished tiles are
		 * written to disk?
		 *
		 * \param x - pixel index in width direction (column index)
		 * \param y - pixel index in height direction (row index)
		 * \param newValue - new value for the samples at this pixel
		 */
		//void setValue(const TqUint x, const TqUint y, const std::vector<TqFloat>& newValue);

		/** \brief Get tile width
		 *
		 * \return tile width
		 */
		inline TqUint width() const;

		/** \brief Get tile height
		 *
		 * \return tile height
		 */
		inline TqUint height() const;

		/** \brief Get top left X-position of tile in the larger array
		 *
		 * \return top left pixel X-position in the larger array
		 */
		inline TqUint topLeftX() const;

		/** \brief Get top left Y-position of tile in the larger array
		 *
		 * \return top left pixel Y-position in the larger array
		 */
		inline TqUint topLeftY() const;

		/** \brief Get the number of samples per pixel
		 *
		 * \return number of samples per pixel
		 */
		inline TqUint samplesPerPixel() const;

		/// Destructor
		virtual ~CqTextureTile();
	private:
		/** \brief Get a pointer to the sample at the given coordinates.
		 *
		 * Positions are in image coordinates, not tile coordinates, counting
		 * from zero in the top-left
		 *
		 * \param x - pixel index in width direction (column index)
		 * \param y - pixel index in height direction (row index)
		 */
		inline T* samplePtr(TqUint x, TqUint y) const;

		boost::shared_array<T> m_data;	///< Pointer to the underlying data.
		TqUint m_width;				///< Width of the tile
		TqUint m_height;			///< Height of the tile
		TqUint m_topLeftX;			///< Column index of the top left of the tile in the full array
		TqUint m_topLeftY;			///< Row index of the top left of the tile in the full array
		TqUint m_samplesPerPixel;	///< Number of samples per pixel
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
		virtual void setValue(const TqUint x, const TqUint y, const std::vector<TqFloat>& newValue);

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
//------------------------------------------------------------------------------
// Inline functions for CqIntrusivePtrCounted
//
inline CqIntrusivePtrCounted::CqIntrusivePtrCounted()
	: m_referenceCount(0)
{ }

// pure virtual destructors need an implementation :-/
CqIntrusivePtrCounted::~CqIntrusivePtrCounted()
{ }

/** \todo: Threading: the following two functions are not thread-safe.  Using
 * an intrusive pointer is far more efficient/lightweight than a shared_ptr
 * when these two functions have the non-threadsafe implementaion below.  If a
 * threadsafe version turns out not to be very efficient, it might be worth
 * going back to a shared_ptr instead.
 */
inline void intrusive_ptr_add_ref(CqIntrusivePtrCounted* ptr)
{
    ++ptr->m_referenceCount;
}

inline void intrusive_ptr_release(CqIntrusivePtrCounted* ptr)
{
    if(--ptr->m_referenceCount == 0)
        delete ptr;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Inline functions for CqSampleVector
//------------------------------------------------------------------------------
template<typename T>
inline CqSampleVector<T>::CqSampleVector<T>(const T* data,
		boost::intrusive_ptr<CqIntrusivePtrCounted>& parentTile)
	: m_data(data),
	m_parentTile(parentTile)
{
}

template<typename T>
inline TqFloat CqSampleVector<T>::operator[](TqUint index) const
{
	// The following if statement should be optimised away at compile time.
	if(std::numeric_limits<T>::is_integer)
		return static_cast<TqFloat>(m_data[index]) / std::numeric_limits<T>::max();
	else
		return m_data[index];
}


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
//------------------------------------------------------------------------------
// Implementation of CqTextureTile
//------------------------------------------------------------------------------
// Inline functions for CqTextureTile
//
template<typename T>
inline CqSampleVector<T> CqTextureTile<T>::value(const TqUint x, const TqUint y) const
{
	return CqSampleVector<T>(samplePtr(x,y),
			boost::intrusive_ptr<CqIntrusivePtrCounted>(*this));
}

template<typename T>
inline TqUint CqTextureTile<T>::width() const
{
	return m_width;
}

template<typename T>
inline TqUint CqTextureTile<T>::height() const
{
	return m_height;
}

template<typename T>
inline TqUint CqTextureTile<T>::topLeftX() const
{
	return m_topLeftX;
}

template<typename T>
inline TqUint CqTextureTile<T>::topLeftY() const
{
	return m_topLeftY;
}

template<typename T>
inline TqUint CqTextureTile<T>::samplesPerPixel() const
{
	return m_samplesPerPixel;
}

template<typename T>
inline T* CqTextureTile<T>::samplePtr(TqUint x, TqUint y) const
{
	assert(x >= m_topLeftX);
	assert(x < m_topLeftX+m_width);
	assert(y >= m_topLeftY);
	assert(y < m_topLeftY+m_height);
	return m_data.get() + ((y-m_topLeftY)*m_width + (x-m_topLeftX))*m_samplesPerPixel;
}

//------------------------------------------------------------------------------
// Template implementations for CqTextureTile
//
template<typename T>
CqTextureTile<T>::CqTextureTile<T>(boost::shared_array<T> data, TqUint width, TqUint height,
		TqUint topLeftX, TqUint topLeftY, TqUint samplesPerPixel)
	: m_data(data),
	m_width(width),
	m_height(height),
	m_topLeftX(topLeftX),
	m_topLeftY(topLeftY),
	m_samplesPerPixel(samplesPerPixel)
{ }

//------------------------------------------------------------------------------
template<typename T>
void CqTextureTile<T>::setValue(const TqUint x, const TqUint y, const std::vector<TqFloat>& newValue)
{
	T* samplePtr = samplePtr(x,y);
	if(std::numeric_limits<T>::is_integer)
	{
		// For integer data types, renormalise the new values so that a input
		// value of 1 becomes the maximum representable integer.
		for(TqUint i = 0; i < m_samplesPerPixel; i++)
			samplePtr[i] = static_cast<T>(
					clamp<TqFloat>(newValue[i]*std::numeric_limits<T>::max(),
					std::numeric_limits<T>::min(), std::numeric_limits<T>::max()) );
	}
	else
	{
		for(TqUint i = 0; i < m_samplesPerPixel; i++)
			samplePtr[i] = static_cast<T>(newValue[i]);
	}
}

//------------------------------------------------------------------------------
template<typename T>
CqTextureTile<T>::~CqTextureTile<T>()
{
	// We assume that m_data was allocated via _TIFFmalloc().
	_TIFFfree(reinterpret_cast<tdata_t>(m_data));
}


//------------------------------------------------------------------------------
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

template<typename T>
void CqTextureTileArray<T>::setValue(const TqUint x, const TqUint y, const std::vector<TqFloat>& newValue)
{
	/// \todo Implementation
}

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
//------------------------------------------------------------------------------
} // namespace Aqsis

#endif // TILE_ARRAY_H_INCLUDED
