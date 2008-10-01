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
 * \brief Declare class encapsulating information about image channels.
 *
 * \author Chris Foster
 */

#ifndef CHANNELLIST_H_INCLUDED
#define CHANNELLIST_H_INCLUDED

#include "aqsis.h"

#include <iosfwd>
#include <vector>

#include "channelinfo.h"
#include "exception.h"

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief Class holding an ordered list of image channels.
 *
 * This class describes the structure of a pixel in hetrogenous images (ie,
 * images made up of possibly different types for each channel inside a pixel).
 */
class AQSISTEX_SHARE CqChannelList
{
	private:
		/// The underlying container type holding the SqChannelInfo.
		typedef std::vector<SqChannelInfo> TqListType;
	public:
		/// Iterator for channels
		typedef TqListType::const_iterator const_iterator;

		/// Construct an empty channel list.
		CqChannelList();

		/** \brief Construct a channel list with the given number of unnamed channels
		 *
		 * \see addUnnamedChannels
		 *
		 * \param chanType - type for the new channels
		 * \param numChans - number of channels in the constructed channel list.
		 */
		CqChannelList(EqChannelType chanType, TqInt numChans);

		/// Factory function for standard 8bpp RGB display channel list
		static CqChannelList displayChannels();
		/// We use the default copy constructor,destructor and assignment operator.

		//----------------------------------------------------------------------
		/// \name Functions for comparing channel lists.
		//@{
		/** \brief Equality operator
		 *
		 * Two channel lists are equal when their channels match in both name
		 * and type.
		 *
		 * \return true if channels are equal
		 */
		bool operator==(const CqChannelList& other) const;
		/** \brief Inequality operator
		 *
		 * \return !(*this == other)
		 */
		bool operator!=(const CqChannelList& other) const;
		/** \brief Determine whether the channel types of two channel lists
		 * match.
		 *
		 * \param other - other channel list to match with this one.
		 * \return true if the number and types of all channels match.
		 *   Channel names are not considered.
		 */
		bool channelTypesMatch(const CqChannelList& other) const;
		//@}

		//----------------------------------------------------------------------
		/// \name Standard iterator interface
		//@{
		/// Get an iterator to the start of the channel list.
		const_iterator begin() const;
		/// Get an iterator to the end of the channel list.
		const_iterator end() const;
		//@}

		//----------------------------------------------------------------------
		/// \name Vector interface
		//@{
		/// Add a channel to the end of the list.
		void addChannel(const SqChannelInfo& newChan);
		/// Get the number of channels in the list
		TqInt numChannels() const;
		/** \brief Get the channel at a given index.
		 */
		const SqChannelInfo& operator[](TqInt index) const;
		//@}

		//----------------------------------------------------------------------
		/// \name Functions to deal with channel names
		//@{
		/** \brief Get an index for the given channel name
		 * \throw XqInternal if the channel name isn't in the list.
		 * \return the index for the channel, if it exists.
		 */
		TqInt findChannelIndex(const std::string& name) const;
		/// \brief Check whether the list of channels contains the given channel name
		bool hasChannel(const std::string& name) const;
		/** \brief Return true if the channel list has at least one channel of
		 * RGB colour data.
		 *
		 * This is determined by examining the channel names for "r" "g" or "b"
		 * channels.
		 */
		bool hasRgbChannel() const;
		/** \brief Return true if the channel list has a intensity channel 
		 *
		 * This is determined by examining the channel names for a "y" channel.
		 */
		bool hasIntensityChannel() const;
		//@}

		//----------------------------------------------------------------------
		/// \name Access to low-level pixel layout
		//@{
		/** \brief Get the byte offset for the given channel number
		 *
		 * When channel data is packed contiguously in memory, the byte offset
		 * is the number of bytes the indexed channel is away from the position
		 * of the first channel inside a pixel.
		 */
		TqInt channelByteOffset(TqInt index) const;
		/// Number of bytes required to store all channels in a pixel
		TqInt bytesPerPixel() const;
		/** \brief Get the shared channel type code if it exists.
		 *
		 * \return the channel type which is shared by all channels, or
		 * Channel_TypeUnknown if the channels aren't all identical or there
		 * are no channels present.
		 */
		EqChannelType sharedChannelType() const;
		//@}

		//----------------------------------------------------------------------
		/// \name Methods to modify all channels
		//@{
		/** \brief Reorder channels to the "expected" order (rgba)
		 *
		 * Reorder the list of channels to be in the standard order - "r", "g",
		 * "b", "a".  Other channel names are ignored.
		 */
		void reorderChannels();
		/// Remove all channels
		void clear();
		/** \brief Add the specified number of "unnamed" channels
		 *
		 * The channels will be named from "?01" up until "?nn" where nn is
		 * numToAdd.
		 *
		 * \param chanType - type of the channels to add
		 * \param numToAdd - number of channels to add
		 */
		void addUnnamedChannels(EqChannelType chanType, TqInt numToAdd);
		//@}

	private:
		/** \brief Get an index for the given channel name
		 * \return the channel index, or -1 if not found.
		 */
		TqInt findChannelIndexImpl(const std::string& name) const;
		/** \brief Recompute the cached channel byte offsets.
		 */
		void recomputeByteOffsets();

		TqListType m_channels;  		///< underlying vector of SqChannelInfo
		std::vector<TqInt> m_offsets;  ///< vector of byte offsets into the channels.
		TqInt m_bytesPerPixel;			///< bytes per pixel needed to store the channels.
};

/** \brief Stream insertion operator for CqChannelList
 *
 * Inserts a human-readable representation of the channels to the stream.
 *
 * \param out - stream to write to
 * \param channelList - channels to output.
 */
AQSISTEX_SHARE std::ostream& operator<<(std::ostream& out, const CqChannelList& channelList);

//==============================================================================
// Implementation of inline functions and templates
//==============================================================================
inline CqChannelList::CqChannelList()
	: m_channels(),
	m_offsets(),
	m_bytesPerPixel(0)
{ }

inline CqChannelList::CqChannelList(EqChannelType chanType, TqInt numChans)
	: m_channels(),
	m_offsets(),
	m_bytesPerPixel(0)
{
	addUnnamedChannels(chanType, numChans);
}

inline bool CqChannelList::operator==(const CqChannelList& other) const
{
	return m_channels == other.m_channels;
}

inline bool CqChannelList::operator!=(const CqChannelList& other) const
{
	return !(*this == other);
}

inline CqChannelList::const_iterator CqChannelList::begin() const
{
	return m_channels.begin();
}

inline CqChannelList::const_iterator CqChannelList::end() const
{
	return m_channels.end();
}

inline TqInt CqChannelList::numChannels() const
{
	return m_channels.size();
}

inline const SqChannelInfo& CqChannelList::operator[](TqInt index) const
{
	assert(index >= 0);
	assert(index < static_cast<TqInt>(m_channels.size()));
	return m_channels[index];
}

inline TqInt CqChannelList::findChannelIndex(const std::string& name) const
{
	TqInt index = findChannelIndexImpl(name);
	if(index < 0)
		AQSIS_THROW(XqInternal, "Cannot find image channel with name \""
				<< name << "\"");
	return static_cast<TqInt>(index);
}

inline bool CqChannelList::hasChannel(const std::string& name) const
{
	return findChannelIndexImpl(name) >= 0;
}

inline bool CqChannelList::hasRgbChannel() const
{
	return hasChannel("r") || hasChannel("g") || hasChannel("b");
}

inline bool CqChannelList::hasIntensityChannel() const
{
	return hasChannel("y");
}

inline TqInt CqChannelList::channelByteOffset(TqInt index) const
{
	assert(index >= 0);
	assert(index < static_cast<TqInt>(m_offsets.size()));
	return m_offsets[index];
}

inline TqInt CqChannelList::bytesPerPixel() const
{
	return m_bytesPerPixel;
}

inline void CqChannelList::clear()
{
	m_channels.clear();
	recomputeByteOffsets();
}

} // namespace Aqsis

#endif // CHANNELLIST_H_INCLUDED
