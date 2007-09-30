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


#ifndef TEXTUREBUFFER_H_INCLUDED
#define TEXTUREBUFFER_H_INCLUDED

#include "aqsis.h"

#include <boost/shared_array.hpp>

#include "smartptr.h"

namespace Aqsis {

class CqSampleVector;

//------------------------------------------------------------------------------
class CqTextureBufferBase : public CqIntrusivePtrCounted
{
	public:
		CqTextureBufferBase(EqChannelType pixelType);

		virtual void resize(TqInt width, TqInt height, TqInt channelsPerPixel) = 0;
	protected:
		EqChannelType m_pixelType;
};

//------------------------------------------------------------------------------
/** \brief A class to hold the data from a single texture tile.
 *
 * This class is a container for tiles of texture data for which each pixel
 * holds the same number of channels as every other pixel.
 */
template<typename T>
class CqTextureBuffer : public CqTextureBufferBase
{
	public:
		/** \brief The pointer type for use with texture tiles.
		 *
		 * We use intrusive_ptr here rather than shared_ptr since it's lightweight for
		 * the construction of CqSampleVector objects.
		 */
		typedef boost::intrusive_ptr<CqTextureBuffer<T> > TqPtr;

		/** \brief Construct a texture tile
		 *
		 * \todo Investigate the use of an allocator or such for the TIFF data.
		 *
		 * \param width - tile width
		 * \param height - tile height
		 * \param channelsPerPixel - number of channels of type T per pixel.
		 */
		CqTextureBuffer(const TqInt width, const TqInt height,
				const TqInt channelsPerPixel);

		/** \brief Resize the buffer
		 */
		virtual void resize(TqInt width, TqInt height, TqInt channelsPerPixel);

		/** \brief Get a reference to the tile channels at the given position.
		 *
		 * Positions are in image coordinates, not tile coordinates, counting
		 * from zero in the top-left
		 *
		 * \param x - pixel index in width direction (column index)
		 * \param y - pixel index in height direction (row index)
		 * \return a lightweight vector holding a reference to the channels data
		 */
		inline const CqSampleVector<T> operator()(const TqInt x, const TqInt y) const;

		/// Get the buffer width
		inline TqInt width() const;

		/// Get the buffer height
		inline TqInt height() const;

		/// Get the number of channels per pixel
		inline TqInt channelsPerPixel() const;
	private:
		/** \brief Get a pointer to the sample at the given coordinates.
		 *
		 * Positions are in image coordinates, not tile coordinates, counting
		 * from zero in the top-left
		 *
		 * \param x - pixel index in width direction (column index)
		 * \param y - pixel index in height direction (row index)
		 */
		inline T* samplePtr(TqInt x, TqInt y) const;

		boost::shared_array<T> m_pixelData;	///< Pointer to the underlying pixel data.
		TqInt m_width;				///< Width of the buffer
		TqInt m_height;				///< Height of the buffer
		TqInt m_channelsPerPixel;	///< Number of channels per pixel
};


//------------------------------------------------------------------------------
/** \brief A minimal vector interface to the colour channels held by another object.
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
		inline CqSampleVector(const T* sampleData,
				const boost::intrusive_ptr<const CqIntrusivePtrCounted>& parentTile);
		/** \brief Indexing operator for this vector of data.
		 *
		 * If the vector holds integer data, this function normalises the data
		 * by the maximum integer value.  Therefore, for (unsigned) integers the
		 * range returned by operator[] is between 0 and 1.  For floating point
		 * data, no extra normalisation is used.
		 *
		 * \param index - the index at which to obtain a sample.
		 */
		inline TqFloat operator[](TqInt index) const;
	private:
		/// Raw pointer to the data held by this vector
		const T* m_sampleData;
		/** A pointer to the parent tile; this is simply to protect the tile
		 *  data from deallocation in the multithreaded case.
		 */
		const boost::intrusive_ptr<const CqIntrusivePtrCounted> m_parentTile;
};



//==============================================================================
// Implementation of inline functions and templates
//==============================================================================

//------------------------------------------------------------------------------
// Inline functions for CqTextureBufferBase
//------------------------------------------------------------------------------
CqTextureBufferBase(EqChannelType pixelType)
	: m_pixelType(pixelType)
{ }

//------------------------------------------------------------------------------
// Inline functions/templates for CqTextureBuffer
//------------------------------------------------------------------------------
template<typename T>
CqTextureBuffer<T>::CqTextureBuffer(TqInt width, TqInt height, TqInt channelsPerPixel)
	: CqTextureBufferBase(getChannelTypeEnum<T>()),
	m_pixelData(new T[width * height * channelsPerPixel]),
	m_width(width),
	m_height(height),
	m_channelsPerPixel(channelsPerPixel)
{ }

template<typename T>
inline const CqSampleVector<T> CqTextureBuffer<T>::operator()(const TqInt x, const TqInt y) const
{
	return CqSampleVector<T>(samplePtr(x,y),
			boost::intrusive_ptr<const CqIntrusivePtrCounted>(this));
}

template<typename T>
void CqTextureBuffer<T>::resize(TqInt width, TqInt height, TqInt channelsPerPixel)
{
	m_pixelData.reset(new T[width * height * channelsPerPixel]);
}

template<typename T>
inline TqInt CqTextureBuffer<T>::width() const
{
	return m_width;
}

template<typename T>
inline TqInt CqTextureBuffer<T>::height() const
{
	return m_height;
}

template<typename T>
inline TqInt CqTextureBuffer<T>::channelsPerPixel() const
{
	return m_channelsPerPixel;
}

template<typename T>
inline T* CqTextureBuffer<T>::samplePtr(TqInt x, TqInt y) const
{
	assert(x >= 0);
	assert(x < m_width);
	assert(y >= 0);
	assert(y < m_height);
	return m_pixelData.get() + (y*m_width + x)*m_channelsPerPixel;
}


//------------------------------------------------------------------------------
// Inline functions for CqSampleVector
//------------------------------------------------------------------------------
template<typename T>
inline CqSampleVector<T>::CqSampleVector(const T* sampleData,
		const boost::intrusive_ptr<const CqIntrusivePtrCounted>& parentTile)
	: m_sampleData(sampleData),
	m_parentTile(parentTile)
{ }

template<typename T>
inline TqFloat CqSampleVector<T>::operator[](TqInt index) const
{
	// The following if statement should be optimised away at compile time.
	if(std::numeric_limits<T>::is_integer)
		return static_cast<TqFloat>(m_sampleData[index])
			/ std::numeric_limits<T>::max();
	else
		return m_sampleData[index];
}


} // namespace Aqsis

#endif // TEXTUREBUFFER_H_INCLUDED
