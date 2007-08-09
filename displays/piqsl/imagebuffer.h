// Aqsis
// Copyright (C) 1997 - 2007, Paul C. Gregory
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
#include <limits>

#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/utility.hpp>
#include <tiffio.h>

#include "aqsismath.h"
#include "exception.h"


namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief Hold channel information; similar in purpose to PtDspyDevFormat from ndspy.h.
 */
struct SqChannelInfo
{
	std::string name;  ///< name of the channel (eg, "r", "g"...)
	TqUint type;       ///< Dspy type for the channel (eg, PkDspyUnsigned8)
	/// Trivial constructor
	inline SqChannelInfo(const std::string& name, TqUint type);
	/** \brief Get the number of bytes per pixel for this channel data.
	 *
	 * \return channel size in bytes
	 */
	TqUint bytesPerPixel() const;
};


//------------------------------------------------------------------------------
/** \brief Class holding an ordered list of image channels.
 *
 * This class describes the structure of a pixel in hetrogenous images (ie,
 * images made up of possibly different types for each channel inside a pixel).
 */
class CqChannelInfoList
{
	public:
		/// The underlying container type holding the SqChannelInfo.
		typedef std::vector<SqChannelInfo> TqListType;
		typedef TqListType::const_iterator const_iterator;

		/// Constructor
		inline CqChannelInfoList();
		/// \note We use the default copy constructor,destructor and assignment operator.

		/// Get an iterator to the start of the channel list.
		inline const_iterator begin() const;
		/// Get an iterator to the end of the channel list.
		inline const_iterator end() const;
		/// Get the number of channels in the list
		inline TqUint numChannels() const;
		/** \brief Get the channel at a given index.
		 * \note Range-checked!
		 */
		inline const SqChannelInfo& operator[](TqUint index) const;
		/** \brief Add a channel to the end of the list.
		 */
		void addChannel(const SqChannelInfo& newChan);
		/** \brief Get an index for the given channel name
		 * \throw XqInternal if the channel name isn't in the list.
		 * \return the index for the channel, if it exists.
		 */
		inline TqUint findChannelIndex(const std::string& name) const;
		/// \brief Check whether the list of channels contains the given channel name
		inline bool hasChannel(const std::string& name) const;
		/** \brief Get the byte offset of the indexed channel inside a pixel
		 */
		inline TqUint channelByteOffset(TqUint index) const;
		/** Return the number of bytes required to store a pixel of the
		 * contained channel list.
		 */
		inline TqUint bytesPerPixel() const;
		/** \brief Reorder channels to the "expected" order (rgba)
		 *
		 * A helper function to reorder the channels that Aqsis sends to ensure
		 * that they are in the expected format for display by piqsl, and
		 * subsequent saving to TIFF.
		 */
		void reorderChannels();
		/** \brief Clone the current channels, but set all types to PkDspyUnsigned8
		 * \return A channel list with 8 bits per channel.
		 */
		CqChannelInfoList cloneAs8Bit() const;
	private:
		/** \brief Get an index for the given channel name
		 * \return the channel index, or -1 if not found.
		 */
		TqInt findChannelIndexImpl(const std::string& name) const;
		/** \brief Recompute the cached channel byte offsets.
		 */
		void recomputeByteOffsets();

		TqListType m_channels;  		///< underlying vector of SqChannelInfo
		std::vector<TqUint> m_offsets;  ///< vector of byte offsets into the channels.
		TqUint m_bytesPerPixel;			///< bytes per pixel needed to store the channels.
};

//------------------------------------------------------------------------------
/** \brief A thin wrapper around channel data held in a CqImageBuffer
 *
 * CqImageChannel wraps around a single channel of a subregion of a hetrogenous
 * array:
 *
 * A subregion looks like:
 *
 *  [ o o o o ]  
 *  [ o X X o ]  ^
 *  [ o X X o ]  | height
 *  [ o X X o ]  v
 *      <->
 *     width
 *
 *    <----->
 *    full_width
 *
 * where each pixel (o or X) possibly contains hetrogenous data types,
 * eg, X = [0xRR 0xGGGG] for a 8 bit red channel, and a 16 bit green channel.
 *
 * To do this we need to store
 * - A pointer to the start of the data
 * - width and height of the region
 * - the number of bytes to skip between entries ("stride")
 * - the number of pixels which are skipped at the end of each row (rowSkip =
 *   full_width - width)
 *
 * Operations which need access to the type of the channel data (eg,
 * conversions) are performed in the subclass CqImageChannelTyped
 */
class CqImageChannel
{
	public:
		/** \brief Construct an image channel.
		 *
		 * \param chanInfo - channel information (name & type)
		 * \param data - raw pointer to the start of the channel data
		 * \param width - width of the channel in pixels
		 * \param height - height of the channel in pixels
		 * \param stride - stride between pixels in bytes
		 * \param rowSkip - number of pixels which are skipped at the end of
		 *                  each row (allows for support of rectangular subregions).
		 */
		CqImageChannel(const SqChannelInfo& chanInfo, TqUchar* data,
				TqUint width, TqUint height, TqUint stride, TqUint rowSkip = 0);
		virtual inline ~CqImageChannel();
		/// Get the name of the channel
		inline const std::string& name() const;
		/// Get the channel type (eg, PkDspyUnsigned16)
		inline TqUint type() const;

		/** \brief Copy data from the source channel, replacing the data in the
		 * current channel.
		 *
		 * \param source - channel which the data should come from.
		 */
		void copyFrom(const CqImageChannel& source);

		//void compositeFrom(const CqImageChannel& source, const CqImageChannel& srcAlpha);

		/// \note Use default copy constructor and assignment ops.
	protected:
		// We need to access the protected member data of CqImageChannel from
		// CqImageChannelTyped inside copyFromSameType.
		template<typename T>
		friend class CqImageChannelTyped;

		// Make CqImageBuffer a friend so it can use the low-level row-based
		// interface to manipulate channel data when necessary.
		friend class CqImageBuffer;

		/** \brief Floating point type used to do conversions.
		 *
		 * We include this as it's slightly possible that 32bit floating point
		 * formats won't be entirely ideal - they only inexactly represent
		 * 32bit floating point for example.
		 */
		typedef TqFloat TqFloatConv;

		/** \brief Optimized version of copyFrom for the case where the types are the same.
		 */
		virtual void copyFromSameType(const CqImageChannel& source) = 0;
		/** \brief Copy a row of data into the buffer provided
		 *
		 * Performing any necessary type conversions based on the type of this channel.
		 *
		 * \param row - image row to take data from
		 * \param buf - buffer to fill
		 */
		virtual void fillRowBuffer(TqUint row, TqFloatConv* buf) const = 0 ;
		/** \brief Replace a row in the current channel
		 *
		 * Performs the necessary type conversion from TqFloatConv to
		 * the type of this channel.
		 *
		 * \param 
		 */
		virtual void replaceRow(TqUint row, TqFloatConv* buf) = 0;

		SqChannelInfo m_chanInfo;
		TqUchar* m_data;
		TqUint m_width;
		TqUint m_height;
		TqUint m_stride;
		TqUint m_rowSkip;
};


//------------------------------------------------------------------------------
/** \brief Override some methods of CqImageChannel with type-specific
 * conversion capabilities.
 */
template<typename T>
class CqImageChannelTyped : public CqImageChannel
{
	public:
		/** \brief Constructor
		 * \see CqImageChannel::CqImageChannel
		 */
		inline CqImageChannelTyped(const SqChannelInfo& chanInfo, TqUchar* data,
				TqUint width, TqUint height, TqUint stride, TqUint rowSkip = 0);
	private:
		/// Inherited from CqImageChannel
		virtual void copyFromSameType(const CqImageChannel& source);
		/// Inherited from CqImageChannel
		virtual void fillRowBuffer(TqUint row, TqFloatConv* buf) const;
		/// Inherited from CqImageChannel
		virtual void replaceRow(TqUint row, TqFloatConv* buf);

		/// Convert the type held by this channel into a float.
		static inline TqFloatConv convertToFloat(T t);
		/// Convert from a float to the type held by this channel.
		static inline T convertFromFloat(TqFloatConv f);
};


//------------------------------------------------------------------------------
/** \brief A wrapper for an array of hetrogenous image data
 *
 * This class wraps around raw image data, composed of multiple channels, and
 * provides interfaces for accessing the channels individually.
 *
 * A central design feature is the ability to place channels with different
 * bit widths contiguously in the same pixel data.  This behaviour was
 * extracted from the CqImage class during refactoring, but things would
 * probably simplify somewhat if we decided we didn't really need it.
 *
 * Rectangular subregions of individual image channels can be selected out of
 * one buffer, and copied without restriction into an arbitrary channel of   
 * another buffer, provided the width and height of the regions match.  The
 * usage is simple:
 *   CqImageBuffer buf1(some_args...);
 *   CqImageBuffer buf2(some_args2...);
 *   // copy a subregion of the buf1 green channel onto buf2 red channel:
 *   buf2.channel("r")->copyFrom(*buf1.channel("g", topLeftX, topLeftY, width, height));
 *
 * This class is noncopyable currently, but only because the default copy
 * operation may not make sense.  It would be extremly easy to implement some
 * sensible copying behaviour.
 *
 * \todo: I just discovered that this is a duplicate name - there is a
 * CqImageBuffer in renderer/render !  This class needs a new name.
 */
class CqImageBuffer : boost::noncopyable
{
	public:
		/** \brief Construct an image buffer with the given channels per pixel.
		 */
		CqImageBuffer(const CqChannelInfoList& channels, TqUint width, TqUint height);
		/** \brief Construct an image buffer with the given channels per pixel.
		 */
		CqImageBuffer(const CqChannelInfoList& channels, boost::shared_array<TqUchar> data,
				TqUint width, TqUint height);

		/** \brief Factory function loading a buffer from a tiff file.
		 *
		 * \param fileName - file to load.
		 *
		 * \return a new image buffer representing the data in the file.
		 */
		static boost::shared_ptr<CqImageBuffer> loadFromTiff(TIFF* tif);

		/** \brief Save the buffer to a TIFF file.
		 */
		void saveToTiff(TIFF* pOut) const;

		/** \brief Quantize the current buffer for an 8 bit display.
		 *
		 * This attempts to make a sensible guess as to how the image data
		 * should be mapped onto 8bits per channel, given no extra information.
		 * A good extension would be to allow custom requantization based on
		 * some given (min, max) range.
		 *
		 * Another good extension might be to allow channels in this image to
		 * be masked out so they don't appear in the output channels.
		 *
		 * \return A new image buffer with 8 bits per channel.
		 */
		boost::shared_ptr<CqImageBuffer> quantizeForDisplay() const;

		/** \brief Set all channels to a fixed value.
		 */
		void clearBuffer(TqFloat f = 0.0f);

		/** \brief Initialise the buffer to a checkerboard to show alpha.
		 *
		 * \param tileSize - size of the square checker "tiles" in pixels
		 */
		void initToCheckerboard(TqUint tileSize = 16);

		/** \brief Copy a source buffer onto this one.
		 *
		 * \param source - source buffer
		 * \param topLeftX - x coordinate where the top left of the source
		 *                   buffer will map onto.
		 * \param topLeftY - y coordinate where the top left of the source
		 *                   buffer will map onto.
		 */
		void copyFrom(const CqImageBuffer& source, TqUint topLeftX = 0, TqUint topLeftY = 0);

		/** \brief Composite the given image buffer on top of this one.
		 */
//		static void CqImageBuffer::compositeFrom(boost::shared_ptr<CqImageBuffer> source,
//				TqUint topLeftX, TqUint topLeftY);

		/** \brief Check for the existance of a particular channel name.
		 */
//		bool hasChannel(const std::string& name);

		
		/** \brief Get the vector of channel information describing pixels in
		 * the buffer.
		 */
		inline const CqChannelInfoList& channelsInfo() const;
		/** \brief get the number of channels per pixel in the buffer.
		 */
		inline TqUint numChannels() const;
		/** \brief Get the width of the image buffer in pixels
		 */
		inline TqUint width() const;
		/** \brief Get the height of the image buffer in pixels
		 */
		inline TqUint height() const;
		/** \brief Get a const pointer to the underlying raw data.
		 *
		 * The proper thing here would be to return a shared array of type
		 * boost::shared_array<const TqUchar> 
		 * but this isn't allowed in boost-1.33.1 (though it works fine for shared_ptr)
		 */
		inline const boost::shared_array<TqUchar>& rawData() const;
		/** \brief Get a pointer to the underlying raw data.
		 */
		inline boost::shared_array<TqUchar>& rawData();
		/** \brief Get the number of bytes required per pixel.
		 */
		inline TqUint bytesPerPixel() const;

		/** \brief Retreive the image channel subregion with the given name
		 *
		 * \param name - name of the channel
		 * \param topLeftX - x-coordinate for the region, counting from the top left of the image
		 * \param topLeftY - y-coordinate for the region, counting from the top left of the image
		 * \param width - width of the region.  If zero, use the full image width.
		 * \param height - height of the region.  If zero, use the full image height.
		 *
		 * \return The named channel subregion.
		 */
		inline boost::shared_ptr<CqImageChannel> channel(const std::string& name,
				TqUint topLeftX = 0, TqUint topLeftY = 0, TqUint width = 0,
				TqUint height = 0);
		/** \brief Retreive the image channel subregion with the given index in
		 * the list of channels.
		 *
		 * \see channel(const std::string& name, ...)
		 */
		inline boost::shared_ptr<CqImageChannel> channel(TqUint index,
				TqUint topLeftX = 0, TqUint topLeftY = 0, TqUint width = 0,
				TqUint height = 0);
		/// \see channel(const std::string& name, ...)
		inline boost::shared_ptr<const CqImageChannel> channel(const std::string& name,
				TqUint topLeftX = 0, TqUint topLeftY = 0, TqUint width = 0,
				TqUint height = 0) const;
		/// \see channel(TqUint index, ...)
		inline boost::shared_ptr<const CqImageChannel> channel(TqUint index,
				TqUint topLeftX = 0, TqUint topLeftY = 0, TqUint width = 0,
				TqUint height = 0) const;

	private:
		/** \brief Implmentation for the various possible channel() calls in
		 * the interface.
		 *
		 * \see channel
		 */
		boost::shared_ptr<CqImageChannel> channelImpl(TqUint index, TqUint topLeftX,
				TqUint topLeftY, TqUint width, TqUint height) const;
		/** \brief Compute the byte offsets into each image channel.
		 *
		 * \param channels - list of channel information
		 * \return The byte-offsets for the start of each channel in the image.
		 * This is one element longer than channels; the last element contains
		 * the 
		 */
		static std::vector<TqUint> channelOffsets(const CqChannelInfoList& channels);

		/// Return the index of the first channel with the given name.
		static TqUint findChannelIndex(const std::string& name, const CqChannelInfoList& channelInfo);

	private:
		CqChannelInfoList m_channelsInfo; ///< vector of channel information
		TqUint m_width; ///< buffer width in pixels
		TqUint m_height; ///< buffer height in pixels
		boost::shared_array<TqUchar> m_data; ///< raw image data
};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Inline function and template implementations
//
//------------------------------------------------------------------------------
// SqChannelInfo
inline SqChannelInfo::SqChannelInfo(const std::string& name, TqUint type)
	: name(name),
	type(type)
{ }

//------------------------------------------------------------------------------
// CqChannelInfoList
inline CqChannelInfoList::CqChannelInfoList()
	: m_channels(),
	m_offsets(),
	m_bytesPerPixel(0)
{ }

inline CqChannelInfoList::const_iterator CqChannelInfoList::begin() const
{
	return m_channels.begin();
}

inline CqChannelInfoList::const_iterator CqChannelInfoList::end() const
{
	return m_channels.end();
}

inline TqUint CqChannelInfoList::numChannels() const
{
	return m_channels.size();
}

inline const SqChannelInfo& CqChannelInfoList::operator[](TqUint index) const
{
	return m_channels.at(index);
}

inline TqUint CqChannelInfoList::findChannelIndex(const std::string& name) const
{
	TqInt index = findChannelIndexImpl(name);
	if(index < 0)
		throw XqInternal("Cannot find channel with name \"" + name + "\"", __FILE__, __LINE__);
	return static_cast<TqUint>(index);
}

inline bool CqChannelInfoList::hasChannel(const std::string& name) const
{
	return findChannelIndexImpl(name) >= 0;
}

inline TqUint CqChannelInfoList::channelByteOffset(TqUint index) const
{
	return m_offsets.at(index);
}

inline TqUint CqChannelInfoList::bytesPerPixel() const
{
	return m_bytesPerPixel;
}

//------------------------------------------------------------------------------
// CqImageChannel
inline CqImageChannel::~CqImageChannel()
{ }

inline const std::string& CqImageChannel::name() const
{
	return m_chanInfo.name;
}

inline TqUint CqImageChannel::type() const
{
	return m_chanInfo.type;
}

//------------------------------------------------------------------------------
// CqImageChannelTyped
template<typename T>
inline CqImageChannelTyped<T>::CqImageChannelTyped(const SqChannelInfo& chanInfo,
		TqUchar* data, TqUint width, TqUint height, TqUint stride, TqUint rowSkip)
	: CqImageChannel(chanInfo, data, width, height, stride, rowSkip)
{ }

template<typename T>
void CqImageChannelTyped<T>::copyFromSameType(const CqImageChannel& source)
{
	assert(m_chanInfo.type == source.m_chanInfo.type);
	TqUchar* srcBuf = source.m_data;
	TqUchar* destBuf = m_data;
	for(TqUint row = 0; row < m_height; ++row)
	{
		for(TqUint col = 0; col < m_width; ++col)
		{
			*reinterpret_cast<T*>(destBuf) = *reinterpret_cast<T*>(srcBuf);
			srcBuf += source.m_stride;
			destBuf += m_stride;
		}
		srcBuf += source.m_rowSkip*source.m_stride;
		destBuf += m_rowSkip*m_stride;
	}
}

template<typename T>
void CqImageChannelTyped<T>::fillRowBuffer(TqUint row, CqImageChannel::TqFloatConv* buf) const
{
	TqUchar* srcBuf = m_data + row*m_stride*(m_width + m_rowSkip);
	for(TqUint i = 0; i < m_width; ++i)
	{
		*buf = convertToFloat(*reinterpret_cast<T*>(srcBuf));
		srcBuf += m_stride;
		buf++;
	}
}

template<typename T>
void CqImageChannelTyped<T>::replaceRow(TqUint row, CqImageChannel::TqFloatConv* buf)
{
	TqUchar* destBuf = m_data + row*m_stride*(m_width + m_rowSkip);
	for(TqUint i = 0; i < m_width; ++i)
	{
		*reinterpret_cast<T*>(destBuf) = convertFromFloat(*buf);
		destBuf += m_stride;
		buf++;
	}
}

template<typename T>
inline CqImageChannel::TqFloatConv CqImageChannelTyped<T>::convertToFloat(T src)
{
	if(std::numeric_limits<T>::is_integer)
	{   
		// source = integer
		return ( static_cast<TqFloatConv>(src) -
				static_cast<TqFloatConv>(std::numeric_limits<T>::min()) )
			/ ( static_cast<TqFloatConv>(std::numeric_limits<T>::max())
				- static_cast<TqFloatConv>(std::numeric_limits<T>::min()));
	}
	else
	{
		// source = floating point
		return static_cast<TqFloatConv>(src);
	}
}

template<typename T>
inline T CqImageChannelTyped<T>::convertFromFloat(CqImageChannel::TqFloatConv src)
{
	if(std::numeric_limits<T>::is_integer)
	{
		// result = integer
		return static_cast<T>(lround(clamp<TqFloatConv>(src, 0, 1) *
					( static_cast<TqFloatConv>(std::numeric_limits<T>::max())
					- static_cast<TqFloatConv>(std::numeric_limits<T>::min()) )
					+ std::numeric_limits<T>::min()
				));
	}
	else
	{
		// result = floating point
		return static_cast<T>(src);
	}
}


/*
// The above are the type-conversion functions.
//
// Here is a massive and messy attempt at a generalized templated format
// conversion function.  Converting between the different integer types turns
// out to be rather a mess becuase of overflow issues, and the fact that the
// behaviour of overflow for signed types is apparently implementation defined.
// ( http://www.boost.org/libs/numeric/conversion/doc/definitions.html#stdconv )
// 
// It's difficult to design clean code to get from a description of the types
// in terms of PkDspy* into calls to template functions based on the type.

template<typename TDest, typename TSrc>
static inline TDest convertChannelFormat(TSrc src)
{       
	if(std::numeric_limits<TSrc>::is_integer)
	{   
		if(std::numeric_limits<TDest>::is_integer)
		{
			// dest = unsigned integer
			const TqInt shiftAmt = std::numeric_limits<unsigned char>::digits
									* (sizeof(TSrc) - sizeof(TDest));
			TqUlong tempUnsigned = 0;
			if(shiftAmt >= 0)
			{
				// Precision of destination is lower than source
				tempUnsigned = static_cast<TqUlong>(src
						- std::numeric_limits<TSrc>::min()) >> shiftAmt;
			}
			else
			{
				// Precision of destination is higher than source.
				//
				// The behaviour in this case isn't ideal, since, for example,
				// for dest = unsigned short, src = unsigned char,
				//
				// src = 255  =>  dest = (255 << 8) == 0xFF00 < 0xFFFF
				//
				// So the maximum possible value of the source doesn't map to
				// the max. possible value for the dest.  but I don't see how
				// to fix it easily.  We're going to loose precision how ever
				// we do it anyway.
				tempUnsigned = (static_cast<TqUlong>(src
						- std::numeric_limits<TSrc>::min()) + 1 )<< -shiftAmt;
			}
			return static_cast<TDest>(tempUnsigned) + std::numeric_limits<TDest>::min();
		}
		else
		{
			// source = integer, dest = floating point
			return ( static_cast<TDest>(src) -
					static_cast<TDest>(std::numeric_limits<TSrc>::min()) )
				/ ( static_cast<TDest>(std::numeric_limits<TSrc>::max())
					- static_cast<TDest>(std::numeric_limits<TSrc>::min()));
		}
	}
	else
	{
		if(std::numeric_limits<TDest>::is_integer)
		{
			// source = floating point, dest = integer 
			return static_cast<TDest>(
				clamp<TSrc>(src, 0, 1)
				* ( static_cast<TSrc>(std::numeric_limits<TDest>::max())
					- static_cast<TSrc>(std::numeric_limits<TDest>::min()) )
				+ static_cast<TSrc>(std::numeric_limits<TDest>::min())
			); 
		}
		else
		{
			// source = floating point, dest = floating point
			return static_cast<TDest>(src);
		}
	}
}       
*/


//------------------------------------------------------------------------------
// CqImageBuffer

inline const CqChannelInfoList& CqImageBuffer::channelsInfo() const
{
	return m_channelsInfo;
}

inline TqUint CqImageBuffer::numChannels() const
{
	return m_channelsInfo.numChannels();
}

inline TqUint CqImageBuffer::width() const
{
	return m_width;
}

inline TqUint CqImageBuffer::height() const
{
	return m_height;
}

inline const boost::shared_array<TqUchar>& CqImageBuffer::rawData() const
{
	return m_data;
}

inline boost::shared_array<TqUchar>& CqImageBuffer::rawData()
{
	return m_data;
}

inline TqUint CqImageBuffer::bytesPerPixel() const
{
	return m_channelsInfo.bytesPerPixel();
}

} // namespace Aqsis

#endif // IMAGEBUFFER_H_INCLUDED
