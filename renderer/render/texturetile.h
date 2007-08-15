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
 * \brief Declare a texture tile class and associated machinery.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 */


#ifndef TEXTILE_H_INCLUDED
#define TEXTILE_H_INCLUDED

#include "aqsis.h"

#include <boost/intrusive_ptr.hpp>
#include <boost/shared_array.hpp>

#include "smartptr.h"

namespace Aqsis {

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
		inline CqSampleVector(const T* data, const boost::intrusive_ptr<const CqIntrusivePtrCounted>& parentTile);
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
		const boost::intrusive_ptr<const CqIntrusivePtrCounted> m_parentTile;
};


//------------------------------------------------------------------------------
/** \brief A class to hold the data from a single texture tile.
 *
 * This class is a container for tiles of texture data for which each pixel
 * holds the same number of samples as every other pixel.
 */
template<typename T>
class CqTextureTile : public CqIntrusivePtrCounted
{
	public:
		/** \brief Construct a texture tile
		 *
		 * \todo Investigate the use of an allocator or such for the TIFF data.
		 *
		 * \param data - raw tile data.
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
//------------------------------------------------------------------------------
// Implementation of inline functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Inline functions for CqSampleVector
//------------------------------------------------------------------------------
template<typename T>
inline CqSampleVector<T>::CqSampleVector<T>(const T* data,
		const boost::intrusive_ptr<const CqIntrusivePtrCounted>& parentTile)
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
// Implementation of CqTextureTile
//------------------------------------------------------------------------------
// Inline functions for CqTextureTile
//
template<typename T>
inline CqSampleVector<T> CqTextureTile<T>::value(const TqUint x, const TqUint y) const
{
	return CqSampleVector<T>(samplePtr(x,y),
			boost::intrusive_ptr<const CqIntrusivePtrCounted>(this));
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

/*template<typename T>
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
}*/

template<typename T>
CqTextureTile<T>::~CqTextureTile<T>()
{ }


//------------------------------------------------------------------------------
} // namespace Aqsis

#endif // TEXTILE_H_INCLUDED
