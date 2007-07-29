// Aqsis
// Copyright (C) 1997 - 2007, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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

/** \file
 *
 * \brief Define an image buffer class for manipulating raw image data.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 *
 */

#ifndef IMAGEBUFFER_H_INCLUDED
#define IMAGEBUFFER_H_INCLUDED

#include "aqsis.h"

#include <vector>
#include <string>

#include <boost/shared_array.hpp>

#include "aqsismath.h"


namespace Aqsis {

/** \brief Very simple class for encapsulating information about image channels
 */
class CqImageChannel
{
	public:
		/// Constructor
		inline CqImageChannel(const std::string& name, TqUint type);
		/// Get the name of the channel
		inline const std::string& name() const;
		/// Get the channel type (eg, PkDspyUnsigned16)
		inline TqUint type() const;
		/// Get the number of bytes taken up by a value of this channel type
		inline TqUint numBytes() const;

		/// Set the channel type
		inline void setType(TqUint type);
		/// Set the name of the channel
		inline void setName(const std::string& name);
	private:
		/** \brief Get the size for a given channel type.
		 *
		 * \param channel type  (eg, PkDspyUnsigned16)
		 * \return channel size in bytes
		 */
		TqUint channelSize(TqInt type);

		std::string m_name;
		TqUint m_type;
		TqUint m_numBytes;
};

/// List of channels used to represent one pixel.
typedef	std::vector<CqImageChannel> TqChannelList;

/** \brief A wrapper for image channel data
 *
 * This class wraps around raw image data, and provides interfaces for
 * quantizing and compositing images data etc.
 */
class CqImageBuffer
{
	public:
		/** \brief Return the total number of bytes needed for a pixel containing
		 * the given channels.
		 */
		static TqUint bytesPerPixel(const TqChannelList& channels);

		/** \brief Transfer the data from the real buffer to thd display buffer.
		 *
		 * This takes into accound the format of the channels in the real data and
		 * will attempt to make a good guess at the intended quantisation for 8bit
		 * display.
		 *
		 * \param src - source buffer
		 * \param dest - destination buffer
		 */
		static void CqImageBuffer::quantizeForDisplay(const TqUchar* src, TqUchar* dest,
				const TqChannelList& srcChannels, TqUint width, TqUint height);

		/** \brief Re-quantize a single channel from a buffer into an 8bit
		 * destination buffer.
		 *
		 * It's possible to quantize an arbitrary rectangle using this function; if
		 * the rectangle is smaller than the width of the image the numRows,
		 * srcRowSkip destRowSkip variables must be used.
		 *
		 * If you want to convert a rectangle of image data which is equal to the
		 * width of the image, the last three parameters can be ignored, and
		 * pixelsPerRow should be the total number of pixels in the rectangle.
		 *
		 * T is the numeric type of the source data.
		 *
		 * \param src - source buffer
		 * \param dest - destination buffer
		 * \param srcStride - stride for the source buffer in bytes.
		 * \param destStride - stride for the destination buffer in bytes.
		 * \param pixelsPerRow - number of pixels to be converted per row.
		 * \param numRows - number of pixel rows (same for src & dest)
		 * \param srcRowSkip - number of bytes to skip at the end of a source row.
		 * \param destRowSkip - number of bytes to skip at the end of a destination row.
		 */
		template<typename T>
		static void quantize8bitChannelStrided(const TqUchar* src, TqUchar* dest,
				TqUint srcStride, TqUint destStride, TqUint pixelsPerRow,
				TqUint numRows = 1, TqUint srcRowSkip = 0, TqUint destRowSkip = 0);

		/** \brief Requantieze an integer type for 8bit displays
		 *
		 * \param val - a variable of either signed or unsigned integral type
		 *
		 * \return A TqUchar representing the input.
		 */
		template<typename T>
		static inline TqUchar quantize8bit(T val);

		/** \brief Quantize a TqFloat for 8bit displays
		 *
		 * \param float to remap
		 *
		 * \return A TqUchar representing the input.
		 */
		static inline TqUchar quantize8bit(TqFloat val);
};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Inline function and template implementations
//
//------------------------------------------------------------------------------
// CqImageChannel

inline CqImageChannel::CqImageChannel(const std::string& name, TqUint type)
	: m_name(name),
	m_type(type),
	m_numBytes(channelSize(type))
{ }

inline const std::string& CqImageChannel::name() const
{
	return m_name;
}

inline TqUint CqImageChannel::type() const
{
	return m_type;
}

inline TqUint CqImageChannel::numBytes() const
{
	return m_numBytes;
}

inline void CqImageChannel::setType(TqUint type)
{
	m_type = type;
}

inline void CqImageChannel::setName(const std::string& name)
{
	m_name = name;
}

//------------------------------------------------------------------------------
// CqImageBuffer
template<typename T>
void CqImageBuffer::quantize8bitChannelStrided( const TqUchar* src, TqUchar* dest,
		TqUint srcStride, TqUint destStride, TqUint pixelsPerRow, TqUint numRows,
		TqUint srcRowSkip, TqUint destRowSkip)
{
	for(TqUint row = 0; row < numRows; ++row)
	{
		for(TqUint i = 0; i < pixelsPerRow; ++i)
		{
			*dest = quantize8bit(*reinterpret_cast<const T*>(src));
			src += srcStride;
			dest += destStride;
		}
		src += srcRowSkip;
		dest += destRowSkip;
	}
}

template<typename T>
inline TqUchar CqImageBuffer::quantize8bit(T val)
{
	return static_cast<TqUchar>(
			static_cast<TqUlong>(val - std::numeric_limits<T>::min()) >>
			( std::numeric_limits<TqUchar>::digits*(sizeof(T) - sizeof(TqUchar)) )
		);
}

inline TqUchar CqImageBuffer::quantize8bit(TqFloat val)
{
	return static_cast<TqUchar>(clamp(val, 0.0f, 1.0f)*255);
}

} // namespace Aqsis

#endif // !IMAGEBUFFER_H_INCLUDED
