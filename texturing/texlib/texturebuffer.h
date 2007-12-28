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
#include "texturesampleoptions.h" // For texture wrap modes

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

		/** \brief Get the list of channels for the buffer.
		 *
		 * \return a (synthetic) list of channels with the correct type and
		 * number for the buffer.
		 */
		CqChannelList channelList() const;

		/// Get a pointer to the underlying raw data
		inline TqUchar* rawData();
		/// Get a pointer to the underlying raw data (const version)
		inline const TqUchar* rawData() const;

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
		 * \param sampleAccum - accumulator for sample data.  Must implement
		 *                      SampleAccumulatorConcept
		 * \param support - support region to iterate over
		 * \param xWrapMode - wrap behaviour at the x-boundaries of the buffer
		 * \param yWrapMode - wrap behaviour at the y-boundaries of the buffer
		 */
		template<typename SampleAccumT>
		void applyFilter(SampleAccumT& sampleAccum,
				const SqFilterSupport& support,
				EqWrapMode xWrapMode, EqWrapMode yWrapMode) const;

		/// Get the buffer width
		inline TqInt width() const;

		/// Get the buffer height
		inline TqInt height() const;

		/// Get the number of channels per pixel
		inline TqInt numChannels() const;
	private:
		/** \brief Apply a filter on the internal part of the buffer
		 *
		 * \param sampleAccum - accumulator for samples in the support
		 * \param support - support for the filter; assumed to be fully
		 *                  contained withing the image!
		 */
		template<typename SampleAccumT>
		void applyFilterInternal(SampleAccumT& sampleAccum,
				const SqFilterSupport& support) const;
		/** \brief Apply a filter to the buffer near the edge
		 *
		 * \param sampleAccum - accumulator for samples in the support
		 * \param support - support for the filter; may lie (partially)
		 *                  outside the texture boundaries.
		 * \param xWrapMode
		 * \param yWrapMode - wrap modes for texture coordinates off the image
		 *                    edge.
		 */
		template<typename SampleAccumT>
		void applyFilterBoundary(SampleAccumT& sampleAccum,
				const SqFilterSupport& support,
				EqWrapMode xWrapMode, EqWrapMode yWrapMode) const;

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
	// Call the const version
	return const_cast<T*>(
			const_cast<const CqTextureBuffer<T>*>(this)->value(x,y)
			);
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
CqChannelList CqTextureBuffer<T>::channelList() const
{
	CqChannelList chanList;
	chanList.addUnnamedChannels(getChannelTypeEnum<T>(), m_numChannels);
	return chanList;
}

template<typename T>
inline TqUchar* CqTextureBuffer<T>::rawData()
{
	return reinterpret_cast<TqUchar*>(m_pixelData.get());
}

template<typename T>
inline const TqUchar* CqTextureBuffer<T>::rawData() const
{
	return reinterpret_cast<const TqUchar*>(m_pixelData.get());
}

template<typename T>
template<typename SampleAccumT>
void CqTextureBuffer<T>::applyFilter(SampleAccumT& sampleAccum,
		const SqFilterSupport& support,
		EqWrapMode xWrapMode, EqWrapMode yWrapMode) const
{
	if( support.inRange(0, m_width, 0, m_height) )
	{
		// The bounds for the filter support are all inside the texture; do
		// simple and efficient filtering.
		applyFilterInternal(sampleAccum, support);
	}
	else
	{
		SqFilterSupport supportTrunc(support);
		// If we get here, the filter support falls at least partially outside
		// the texture range.
		if(xWrapMode == WrapMode_Black)
		{
			supportTrunc.sx.truncate(0, m_width);
			if(supportTrunc.sx.isEmpty())
				return;
		}
		if(yWrapMode == WrapMode_Black)
		{
			supportTrunc.sy.truncate(0, m_height);
			if(supportTrunc.sy.isEmpty())
				return;
		}
		// Apply a filter using careful boundary checking.
		applyFilterBoundary(sampleAccum, supportTrunc, xWrapMode, yWrapMode);
	}
}

template<typename T>
template<typename SampleAccumT>
void CqTextureBuffer<T>::applyFilterInternal(SampleAccumT& sampleAccum,
		const SqFilterSupport& support) const
{
	for(TqInt y = support.sy.start; y < support.sy.end; ++y)
	{
		for(TqInt x = support.sx.start; x < support.sx.end; ++x)
			sampleAccum.accumulate(x, y, (*this)(x,y));
	}
}

inline TqInt wrapCoord(TqInt x, TqInt width, EqWrapMode wrapMode)
{
	switch(wrapMode)
	{
		case WrapMode_Black:
			break;
		case WrapMode_Periodic:
			return (x + width) % width;
		case WrapMode_Clamp:
			return clamp(x, 0, width-1);
	}
	return x;
}

template<typename T>
template<typename SampleAccumT>
void CqTextureBuffer<T>::applyFilterBoundary(SampleAccumT& sampleAccum,
		const SqFilterSupport& support,
		EqWrapMode xWrapMode, EqWrapMode yWrapMode) const
{
	for(TqInt y = support.sy.start; y < support.sy.end; ++y)
	{
		TqInt yWrap = wrapCoord(y, m_height, yWrapMode);
		for(TqInt x = support.sx.start; x < support.sx.end; ++x)
		{
			TqInt xWrap = wrapCoord(x, m_width, xWrapMode);
			sampleAccum.accumulate(x, y, (*this)(xWrap, yWrap) );
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
