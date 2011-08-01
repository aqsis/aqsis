// Aqsis
// Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the software's owners nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// (This is the New BSD license)

/**
 * \file
 *
 * \brief Declare a texture buffer class and associated machinery.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 */


#ifndef TEXTUREBUFFER_H_INCLUDED
#define TEXTUREBUFFER_H_INCLUDED

#include <aqsis/aqsis.h>

#include <boost/shared_array.hpp>

#include <aqsis/tex/buffers/channellist.h>
#include <aqsis/tex/buffers/samplevector.h>
#include <aqsis/tex/buffers/filtersupport.h>

#include "randomtable.h"

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief Holds homogeneous texture data in a flat buffer.
 *
 * This class holds a flat buffer of texture data, in which the channel types
 * are all the same.
 */
template<typename T>
class CqTextureBuffer
{
	public:
		/// Pixel iterator class
		class CqIterator;
		/// Stochastic pixel iterator class
		class CqStochasticIterator;

		typedef CqIterator TqIterator;
		typedef CqStochasticIterator TqStochasticIterator;
		/// Sample vector type returned by operator()
		typedef CqSampleVector<T> TqSampleVector;

		/// Construct a texture buffer with width = height = 0
		CqTextureBuffer();

		/** \brief Construct a texture buffer
		 *
		 * \param width - buffer width
		 * \param height - buffer height
		 * \param numChannels - number of channels of type T per pixel.
		 */
		CqTextureBuffer(const TqInt width, const TqInt height,
				const TqInt numChannels);

		/** \brief Wrap a texture buffer around preexisting data
		 *
		 * \param pixelData - array of pixel data of the specified size
		 * \param width - buffer width
		 * \param height - buffer height
		 * \param numChannels - number of channels of type T per pixel.
		 */
		CqTextureBuffer(boost::shared_array<T> pixelData,
				const TqInt width, const TqInt height,
				const TqInt numChannels);

		/** \brief Copy constructor
		 *
		 * This constructor creates a copy of rhs, but possibly with a
		 * different type of underlying pixel type.  The copying semantics is
		 * defined by operator=().
		 */
		template<typename T2>
		CqTextureBuffer(const CqTextureBuffer<T2>& rhs);

		//--------------------------------------------------
		/// \name Modifiers for pixel data
		//@{
		/** \brief Copy the given buffer onto this one.
		 *
		 * The copy operation resizes the current buffer to be the size of the
		 * rhs buffer if necessary.  Conversion between the pixel data formats
		 * is simply by calling setPixel(x, y, rhs(x, y)) for every pixel (x,y)
		 * in the buffer.
		 */
		template<typename T2>
		CqTextureBuffer<T>& operator=(const CqTextureBuffer<T2>& rhs);
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
		 * \param pixelValue - vector type which may be indexed with operator[]
		 *                     to obtain floating point values for the pixel
		 *                     (may be TqFloat*)
		 */
		template<typename PixelVectorT>
		void setPixel(const TqInt x, const TqInt y, const PixelVectorT& pixelValue);
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
		/** \brief Resize the buffer
		 *
		 * \param width - new width
		 * \param height - new height
		 * \param numChannels - new number of channels per pixel.
		 */
		void resize(TqInt width, TqInt height, TqInt numChannels);
		//@}

		//--------------------------------------------------
		/// \name Pixel access
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
		const TqSampleVector operator()(const TqInt x, const TqInt y) const;
		/** \brief 2D Indexing - access to the underlying typed channel data.
		 *
		 * \param x - pixel index in width direction (column index)
		 * \param y - pixel index in height direction (row index)
		 * \return a pointer to the channel data for the given pixel
		 */
		T* value(const TqInt x, const TqInt y);
		/// 2D indexing of typed channel data, const version.
		const T* value(const TqInt x, const TqInt y) const;
		/** \brief Access to pixels through a pixel iterator
		 *
		 * The pixel iterator will iterate through all the pixels the provided
		 * support.  See the iterator class for more detail.
		 *
		 * \param support - support to iterate over
		 */
		TqIterator begin(const SqFilterSupport& support) const;
		/** \brief Stochastic iterator access to pixels in the given support.
		 *
		 * A stochastic support iterator aims to choose a fixed number of
		 * samples from the support randomly.  The randomness choices should
		 * ideally be spread evenly over the support with minimal bunching.
		 *
		 * \param support - support to iterate over
		 * \param numSamples - number of samples to choose in the support.
		 */
		TqStochasticIterator beginStochastic(const SqFilterSupport& support,
				TqInt numSamples) const;
		//@}

		//--------------------------------------------------
		/// \name Access to buffer dimensions & metadata
		//@{
		/** \brief Get the list of channels for the buffer.
		 *
		 * \return a (synthetic) list of channels with the correct type and
		 * number for the buffer.
		 */
		CqChannelList channelList() const;
		/// Get the buffer width
		TqInt width() const;
		/// Get the buffer height
		TqInt height() const;
		/// Get the number of channels per pixel
		TqInt numChannels() const;
		//@}

		//--------------------------------------------------
		/// \name Access to raw pixel data
		//@{
		/// Get a pointer to the underlying raw data
		TqUint8* rawData();
		/// Get a pointer to the underlying raw data (const version)
		const TqUint8* rawData() const;
		//@}

	private:
		boost::shared_array<T> m_pixelData;	///< Pointer to the underlying pixel data.
		TqInt m_width;				///< Width of the buffer
		TqInt m_height;				///< Height of the buffer
		TqInt m_numChannels;		///< Number of channels per pixel
};


/** \brief A simple pixel iterator for CqTextureBuffer
 *
 * A pixel iterator iterates over all pixels in some region, but abstracts away
 * the order of iteration.  This means that arrays with non-scanline orderings
 * can be supported easily (for example, tiled arrays).
 */
template<typename T>
class CqTextureBuffer<T>::CqIterator
{
	public:
		/// Go to the next pixel in the support
		CqIterator& operator++();

		/** \brief Test whether the iterator is still inside the support.
		 *
		 * The equivilant of inSupport() in the standard library is to test
		 * against an ending iterator.  However, we ensue that convention here
		 * for efficiency and convenience.
		 */
		bool inSupport() const;
		/// Return the x-position of the currently pointed to pixel
		TqInt x() const;
		/// Return the y-position of the currently pointed to pixel
		TqInt y() const;
		/// Return the pixel sample data at the current location.
		const typename CqTextureBuffer<T>::TqSampleVector operator*() const;

		friend class CqTextureBuffer<T>;
	private:
		/** \brief Construct a pixel iterator for a given buffer and region.
		 *
		 * This is a private constructor, since we only want CqTextureBuffer to
		 * be able to construct pixel iterators.
		 *
		 * \param buf - buffer to iterate over.
		 * \param support - region of the buffer to iterate over.
		 */
		CqIterator(const CqTextureBuffer<T>& buf, const SqFilterSupport& support);

		/// Reference to the underlying buffer.
		const CqTextureBuffer<T>* m_buf;
		/// Support region to iterate over
		SqFilterSupport m_support;
		/// current x-position
		TqInt m_x;
		/// current y-position
		TqInt m_y;
};

/** \brief A stochastic pixel iterator for CqTextureBuffer
 *
 * This pixel iterator covers only a subset of pixels in a given filter
 * support.  It selects those points using a randomized halton sequence as
 * defined by Cq2dQuasiRandomTable.
 */
template<typename T>
class CqTextureBuffer<T>::CqStochasticIterator
{
	public:
		/// Construct a null iterator (dereferencing the result is an error)
		CqStochasticIterator();

		/// Go to the next pixel in the support
		CqStochasticIterator& operator++();

		/** \brief Test whether the iterator is still inside the support.
		 *
		 * The equivilant of inSupport() in the standard library is to test
		 * against an ending iterator.  However, we ensue that convention here
		 * for efficiency and convenience.
		 */
		bool inSupport() const;
		/// Return the x-position of the currently pointed to pixel
		TqInt x() const;
		/// Return the y-position of the currently pointed to pixel
		TqInt y() const;
		/// Return the pixel sample data at the current location.
		const typename CqTextureBuffer<T>::TqSampleVector operator*() const;

		friend class CqTextureBuffer<T>;
	private:
		/** \brief Construct a pixel iterator for a given buffer and region.
		 *
		 * This is a private constructor, since we only want CqTextureBuffer to
		 * be able to construct pixel iterators.
		 *
		 * \param buf - buffer to iterate over.
		 * \param support - region of the buffer to iterate over.
		 * \param numSamples - number of samples to take inside the support.
		 */
		CqStochasticIterator(const CqTextureBuffer<T>& buf,
				const SqFilterSupport& support, TqInt numSamples);

		/// Reference to the underlying buffer.
		const CqTextureBuffer<T>* m_buf;
		/// Support region to iterate over
		SqFilterSupport m_support;
		/// current x-position
		TqInt m_x;
		/// current y-position
		TqInt m_y;
		/// Total number of samples to take.
		TqInt m_numSamples;
		/// Current sample number
		TqInt m_sampleNum;
};

//==============================================================================
// Implementation of inline functions and templates
//==============================================================================

// Inline functions/templates for CqTextureBuffer

template<typename T>
inline CqTextureBuffer<T>::CqTextureBuffer()
	: m_pixelData(),
	m_width(0),
	m_height(0),
	m_numChannels(0)
{ }

template<typename T>
inline CqTextureBuffer<T>::CqTextureBuffer(TqInt width, TqInt height, TqInt numChannels)
	: m_pixelData(new T[width * height * numChannels]),
	m_width(width),
	m_height(height),
	m_numChannels(numChannels)
{ }

template<typename T>
inline CqTextureBuffer<T>::CqTextureBuffer(boost::shared_array<T> pixelData,
		const TqInt width, const TqInt height,
		const TqInt numChannels)
	: m_pixelData(pixelData),
	m_width(width),
	m_height(height),
	m_numChannels(numChannels)
{ }

template<typename T>
template<typename T2>
CqTextureBuffer<T>::CqTextureBuffer(const CqTextureBuffer<T2>& rhs)
	: m_pixelData(),
	m_width(0),
	m_height(0),
	m_numChannels(0)
{
	*this = rhs;
}

template<typename T>
inline const typename CqTextureBuffer<T>::TqSampleVector
CqTextureBuffer<T>::operator()(const TqInt x, const TqInt y) const
{
	return TqSampleVector(value(x,y));
}

template<typename T>
template<typename PixelVectorT>
inline void CqTextureBuffer<T>::setPixel(const TqInt x, const TqInt y,
		const PixelVectorT& pixelValue)
{
	T* pixel = value(x,y);
	if(std::numeric_limits<T>::is_integer)
	{
		// T is an integral type; need to rescale channel...
		//
		/// \todo Need to think about how this should work for signed integer channels.
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
inline typename CqTextureBuffer<T>::TqIterator CqTextureBuffer<T>::begin(
		const SqFilterSupport& support) const
{
	return TqIterator(*this, intersect(SqFilterSupport(0, m_width, 0, m_height),
				support));
}

template<typename T>
inline typename CqTextureBuffer<T>::TqStochasticIterator
CqTextureBuffer<T>::beginStochastic(const SqFilterSupport& support,
		TqInt numSamples) const
{
	return TqStochasticIterator(*this,
			intersect(SqFilterSupport(0, m_width, 0, m_height), support),
			numSamples);
}

template<typename T>
void CqTextureBuffer<T>::resize(TqInt width, TqInt height, const CqChannelList& channelList)
{
	if(channelList.sharedChannelType() != getChannelTypeEnum<T>())
	{
		AQSIS_THROW_XQERROR(XqInternal, EqE_Bug, "CqTextureBuffer channel type is"
				"incompatible with new channel type requested");
	}
	resize(width, height, channelList.bytesPerPixel()/sizeof(T));
}

template<typename T>
inline void CqTextureBuffer<T>::resize(TqInt width, TqInt height, TqInt numChannels)
{
	TqInt newSize = width * height * numChannels;
	if(newSize != m_width * m_height * m_numChannels)
		m_pixelData.reset(new T[newSize]);
	// Set new buffer sizes
	m_width = width;
	m_height = height;
	m_numChannels = numChannels;
}

template<typename T>
inline CqChannelList CqTextureBuffer<T>::channelList() const
{
	CqChannelList chanList;
	chanList.addUnnamedChannels(getChannelTypeEnum<T>(), m_numChannels);
	return chanList;
}

template<typename T>
inline TqUint8* CqTextureBuffer<T>::rawData()
{
	return reinterpret_cast<TqUint8*>(m_pixelData.get());
}

template<typename T>
inline const TqUint8* CqTextureBuffer<T>::rawData() const
{
	return reinterpret_cast<const TqUint8*>(m_pixelData.get());
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

template<typename T>
template<typename T2>
CqTextureBuffer<T>& CqTextureBuffer<T>::operator=(const CqTextureBuffer<T2>& rhs)
{
	resize(rhs.width(), rhs.height(), rhs.numChannels());
	for(TqInt y = 0; y < height(); ++y)
	{
		for(TqInt x = 0; x < width(); ++x)
		{
			setPixel(x, y, rhs(x,y));
		}
	}
	return *this;
}


//------------------------------------------------------------------------------
// CqTextureBuffer<T>::CqIterator implementation

template<typename T>
typename CqTextureBuffer<T>::CqIterator& CqTextureBuffer<T>::CqIterator::operator++()
{
	++m_x;
	if(m_x >= m_support.sx.end)
	{
		m_x = m_support.sx.start;
		++m_y;
	}
	return *this;
}

template<typename T>
bool CqTextureBuffer<T>::CqIterator::inSupport() const
{
	return m_y < m_support.sy.end;
}

template<typename T>
TqInt CqTextureBuffer<T>::CqIterator::x() const
{
	return m_x;
}

template<typename T>
TqInt CqTextureBuffer<T>::CqIterator::y() const
{
	return m_y;
}

template<typename T>
const typename CqTextureBuffer<T>::TqSampleVector
CqTextureBuffer<T>::CqIterator::operator*() const
{
	return (*m_buf)(m_x, m_y);
}

template<typename T>
CqTextureBuffer<T>::CqIterator::CqIterator(const CqTextureBuffer<T>& buf,
		const SqFilterSupport& support)
	: m_buf(&buf),
	m_support(support),
	m_x(m_support.sx.start),
	m_y(m_support.sx.isEmpty() ? m_support.sy.end : m_support.sy.start)
{ }

//------------------------------------------------------------------------------
// CqTextureBuffer<T>::CqStochasticIterator implementation

template<typename T>
typename CqTextureBuffer<T>::CqStochasticIterator&
CqTextureBuffer<T>::CqStochasticIterator::operator++()
{
	++m_sampleNum;
	m_x = m_support.sx.start
		+ lfloor(m_support.sx.range()*detail::g_randTab.x(m_sampleNum));
	m_y = m_support.sy.start
		+ lfloor(m_support.sy.range()*detail::g_randTab.y(m_sampleNum));
	return *this;
}

template<typename T>
bool CqTextureBuffer<T>::CqStochasticIterator::inSupport() const
{
	return m_sampleNum < m_numSamples;
}

template<typename T>
const typename CqTextureBuffer<T>::TqSampleVector
CqTextureBuffer<T>::CqStochasticIterator::operator*() const
{
	return (*m_buf)(m_x, m_y);
}

template<typename T>
TqInt CqTextureBuffer<T>::CqStochasticIterator::x() const
{
	return m_x;
}

template<typename T>
TqInt CqTextureBuffer<T>::CqStochasticIterator::y() const
{
	return m_y;
}

template<typename T>
CqTextureBuffer<T>::CqStochasticIterator::CqStochasticIterator()
	: m_buf(0),
	m_support(),
	m_x(0),
	m_y(0),
	m_numSamples(0),
	m_sampleNum(0)
{ }

template<typename T>
CqTextureBuffer<T>::CqStochasticIterator::CqStochasticIterator(
		const CqTextureBuffer<T>& buf, const SqFilterSupport& support,
		TqInt numSamples)
	: m_buf(&buf),
	m_support(support),
	m_x(0),
	m_y(0),
	m_numSamples(numSamples),
	m_sampleNum(-1)
{
	// Randomize the table for the next point.
	detail::g_randTab.randomize();
	// Call operator++ to generate valid initial sample positions.
	++(*this);
}


//------------------------------------------------------------------------------

} // namespace Aqsis

#endif // TEXTUREBUFFER_H_INCLUDED
