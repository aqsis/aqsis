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
 * \brief Declare a texture buffer class and associated machinery.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 */


#ifndef TEXTUREBUFFER_H_INCLUDED
#define TEXTUREBUFFER_H_INCLUDED

#include "aqsis.h"

#include <boost/shared_array.hpp>
#include <boost/intrusive_ptr.hpp>

#include "smartptr.h"
#include "samplevector.h"
#include "channellist.h"
#include "filtersupport.h"
#include "texturesampleoptions.h" // for EqWrapMode; factor this out somehow?

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief A class to hold the data from a single texture tile.
 *
 * This class is a container for tiles of texture data for which each pixel
 * holds the same number of channels as every other pixel.
 *
 * WARNING: Never allocate this class on the stack, especially without calling
 * intrusive_ptr_add_ref() on it immediately afterward.  Doing so and using the
 * indexing operator() which returns a CqSampleVector will result in multiple
 * deallocations.
 */
template<typename T>
class CqTextureBuffer // : public CqIntrusivePtrCounted
{
	public:
		/** \brief The pointer type for use with texture tiles.
		 *
		 * We use intrusive_ptr here rather than shared_ptr since it's lightweight for
		 * the construction of CqSampleVector objects.
		 */
		typedef boost::intrusive_ptr<CqTextureBuffer<T> > TqPtr;

		/// Construct a texture tile with width = height = 0
		CqTextureBuffer();

		/** \brief Construct a texture tile
		 *
		 * \todo Investigate the use of an allocator or such for the TIFF data.
		 *
		 * \param width - tile width
		 * \param height - tile height
		 * \param numChannels - number of channels of type T per pixel.
		 */
		CqTextureBuffer(const TqInt width, const TqInt height,
				const TqInt numChannels);

		/** \brief Resize the buffer
		 *
		 * \param width  - new width
		 * \param height - new height
		 * \param channelList - new channel list for the buffer.
		 *
		 * \throw XqInternal If the new required channel list has channel types
		 * which are incompatible with this texture buffer.
		 */
		void resize(TqInt width, TqInt height, const CqChannelList& channelList);

		/// Get a pointer to the underlying raw data
		inline TqUchar* rawData();

		/// \name Indexing operations
		//@{
		/** \brief 2D Indexing operator - floating point pixel interface.
		 *
		 * The returned vector is a lightweight view onto the underlying pixel,
		 * which presents the data as floating point values.
		 *
		 * \param x - pixel index in width direction (column index)
		 * \param y - pixel index in height direction (row index)
		 * \return a lightweight vector holding a reference to the channels data
		 */
		inline const CqSampleVector<T> operator()(const TqInt x, const TqInt y) const;
		/** \brief Set the value of a pixel in the underlying data
		 *
		 * This function converts floating point values to the underlying
		 * channel type.  If the underlying type is an integral type, the
		 * function first clamps the given values between 0 and 1, followed by
		 * multiplication by the maximum value for the range.  If the
		 * underlying type is floating point, a trivial conversion is
		 * performed.
		 *
		 * \param x - pixel index in width direction (column index)
		 * \param y - pixel index in height direction (row index)
		 * \param pixelValue - array of floating point values for the pixel.
		 */
		inline void setPixel(const TqInt x, const TqInt y, const TqFloat* pixelValue);
		/** \brief 2D Indexing - access to the underlying typed channel data.
		 *
		 * \param x - pixel index in width direction (column index)
		 * \param y - pixel index in height direction (row index)
		 * \return a pointer to the channel data for the given pixel
		 */
		inline T* value(const TqInt x, const TqInt y);
		/// 2D indexing of typed channel data, const version.
		inline const T* value(const TqInt x, const TqInt y) const;
		//@}

		/** \brief Filter kernel to apply 
		 *
		 * \param filterKer
		 * \param resultBuf
		 */
		template<typename FilterKernelT>
		void applyFilter(const FilterKernelT& filterKer, TqFloat* resultBuf,
				EqWrapMode xWrapMode, EqWrapMode yWrapMode);

		/// Get the buffer width
		inline TqInt width() const;

		/// Get the buffer height
		inline TqInt height() const;

		/// Get the number of channels per pixel
		inline TqInt numChannels() const;
	private:
		/** \brief Apply a filter on the internal part of the buffer
		 *
		 * \param filterKer - filter to apply
		 * \param support - support for the filter; assumed to be fully
		 *                  contained withing the image!
		 * \param resultBuf - buffer where the results of the filtering
		 *                    operation should be placed.
		 */
		template<typename FilterKernelT>
		void applyFilterInternal(const FilterKernelT& filterKer,
				const SqFilterSupport& support, TqFloat* resultBuf);

		boost::shared_array<T> m_pixelData;	///< Pointer to the underlying pixel data.
		TqInt m_width;				///< Width of the buffer
		TqInt m_height;				///< Height of the buffer
		TqInt m_numChannels;	///< Number of channels per pixel
};



//==============================================================================
// Implementation of inline functions and templates
//==============================================================================

// Inline functions/templates for CqTextureBuffer

template<typename T>
CqTextureBuffer<T>::CqTextureBuffer()
	: m_pixelData(),
	m_width(0),
	m_height(0),
	m_numChannels(0)
{ }

template<typename T>
CqTextureBuffer<T>::CqTextureBuffer(TqInt width, TqInt height, TqInt numChannels)
	: m_pixelData(new T[width * height * numChannels]),
	m_width(width),
	m_height(height),
	m_numChannels(numChannels)
{ }

template<typename T>
inline const CqSampleVector<T> CqTextureBuffer<T>::operator()(const TqInt x, const TqInt y) const
{
	return CqSampleVector<T>(value(x,y));
	/// \todo Have to think about how to do the below for the multi-threaded case...
//	return CqSampleVector<T>(value(x,y),
//			boost::intrusive_ptr<const CqIntrusivePtrCounted>(this));
}

template<typename T>
inline void CqTextureBuffer<T>::setPixel(const TqInt x, const TqInt y,
		const TqFloat* pixelValue)
{
	T* pixel = value(x,y);
	if(std::numeric_limits<T>::is_integer)
	{
		// T is an integral type; need to rescale channel...
		for(TqInt chan = 0; chan < m_numChannels; ++chan)
		{
			pixel[chan] = static_cast<T>(std::numeric_limits<T>::max()
					* clamp(pixelValue[chan], 0.0f, 1.0f));
		}
	}
	else
	{
		// floating point; do simple conversion.
		for(TqInt chan = 0; chan < m_numChannels; ++chan)
			pixel[chan] = static_cast<T>(pixelValue[chan]);
	}
}

template<typename T>
inline T* CqTextureBuffer<T>::value(const TqInt x, const TqInt y)
{
	return const_cast<T*>(value(x,y));
}

template<typename T>
inline const T* CqTextureBuffer<T>::value(const TqInt x, const TqInt y) const
{
	assert(x >= 0);
	assert(x < m_width);
	assert(y >= 0);
	assert(y < m_height);
	return m_pixelData.get() + (y*m_width + x)*m_numChannels;
}

template<typename T>
void CqTextureBuffer<T>::resize(TqInt width, TqInt height, const CqChannelList& channelList)
{
	if(channelList.sharedChannelType() != getChannelTypeEnum<T>())
		throw XqInternal("Channel type is incompatible with new channel info list",
				__FILE__, __LINE__);
	TqInt numChannels = channelList.bytesPerPixel()/sizeof(T);
	TqInt newSize = width * height * numChannels;
	if(newSize != m_width * m_height * m_numChannels);
		m_pixelData.reset(new T[newSize]);
	// Set new buffer sizes
	m_width = width;
	m_height = height;
	m_numChannels = numChannels;
}

template<typename T>
inline TqUchar* CqTextureBuffer<T>::rawData()
{
	return reinterpret_cast<TqUchar*>(m_pixelData.get());
}

template<typename T>
template<typename FilterKernelT>
void CqTextureBuffer<T>::applyFilter(const FilterKernelT& filterKer,
		TqFloat* resultBuf, EqWrapMode xWrapMode, EqWrapMode yWrapMode)
{
	/// \todo Modify to use a specifiable number of channels.
	// Clear the result buffer.
	for(TqInt chan = 0; chan < m_numChannels; ++chan)
		resultBuf[chan] = 0;
	SqFilterSupport support = filterKer.support();
	/// \todo Put the following code in the filter support file & fix it.
	// Remap start and end coordinates if using a periodic coordinate system.
	/*if(xWrapMode == WrapMode_Periodic)
	{
		TqInt offset = lfloor(TqFloat(support.startX)/m_width)*m_width;
		support.startX -= offset;
		support.endX -= offset;
		remapCoordsPeriodic(startX, endX);
	}
	if(yWrapMode == WrapMode_Periodic)
		remapCoordsPeriodic(startY, endY);
	*/
	// Return black if outside texture in black mode
	// Apply filter
	if( support.startX >= 0 && support.endX <= m_width
		&& support.startY >= 0 && support.endY <= m_height )
	{
		// The bounds for the filter support are all inside the texture; do
		// simple filtering.
		applyFilterInternal(filterKer, support, resultBuf);
	}
	else
	{
		// The bounds for the filter support cross the boundary of the texture;
		// have to use a version with boundary checking.
		//applyFilterBoundary(filterKer, resultBuf, xWrapMode, yWrapMode);
		resultBuf[0] = 1;
		for(TqInt chan = 1; chan < m_numChannels; ++chan)
			resultBuf[chan] = 0;
	}
}

template<typename T>
template<typename FilterKernelT>
void CqTextureBuffer<T>::applyFilterInternal(const FilterKernelT& filterKer,
		const SqFilterSupport& support, TqFloat* resultBuf)
{
	for(TqInt y = support.startY; y < support.endY; ++y)
	{
		for(TqInt x = support.startX; x < support.endX; ++x)
		{
			TqFloat weight = filterKer(x, y);
			// Some filters are likely to return a lot of zeros (eg, sinc,
			// EWA), so we check that the weight is nonzero before doing any
			// filtering.
			if(weight != 0)
			{
				const CqSampleVector<T> samples = (*this)(x,y);
				for(TqInt chan = 0; chan < m_numChannels; ++chan)
					resultBuf[chan] += weight * samples[chan];
			}
		}
	}
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
inline TqInt CqTextureBuffer<T>::numChannels() const
{
	return m_numChannels;
}

} // namespace Aqsis

#endif // TEXTUREBUFFER_H_INCLUDED
