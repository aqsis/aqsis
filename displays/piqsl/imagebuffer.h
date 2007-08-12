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

#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/utility.hpp>
#include <tiffio.h>

#include "aqsismath.h"
#include "exception.h"
#include "imagechannel.h"


namespace Aqsis {

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
