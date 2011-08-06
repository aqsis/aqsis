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

/** \file
 *
 * \brief Declare class encapsulating information about image channels.
 *
 * \author Chris Foster
 */

#ifndef CHANNELLIST_H_INCLUDED
#define CHANNELLIST_H_INCLUDED

#include <aqsis/aqsis.h>

#include <iosfwd>
#include <vector>

#include <boost/format.hpp>

#include <aqsis/tex/buffers/channelinfo.h>
#include <aqsis/tex/texexception.h>

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief Class holding an ordered list of image channels.
 *
 * This class describes the structure of a pixel in hetrogenous images (ie,
 * images made up of possibly different types for each channel inside a pixel).
 */
class CqChannelList
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
AQSIS_TEX_SHARE std::ostream& operator<<(std::ostream& out, const CqChannelList& channelList);

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
		AQSIS_THROW_XQERROR(XqInternal, EqE_Bug,
			"Cannot find image channel with name \"" << name << "\"");
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

inline CqChannelList CqChannelList::displayChannels()
{
    CqChannelList displayChannels;
    displayChannels.addChannel(SqChannelInfo("r", Channel_Unsigned8));
    displayChannels.addChannel(SqChannelInfo("g", Channel_Unsigned8));
    displayChannels.addChannel(SqChannelInfo("b", Channel_Unsigned8));
	return displayChannels;
}

inline bool CqChannelList::channelTypesMatch(const CqChannelList& other) const
{
	if(numChannels() != other.numChannels())
		return false;
	for(TqInt i = 0; i < numChannels(); ++i)
	{
		if(m_channels[i].type != other.m_channels[i].type)
			return false;
	}
	return true;
}

inline void CqChannelList::addChannel(const SqChannelInfo& newChan)
{
	m_channels.push_back(newChan);
	m_offsets.push_back(m_bytesPerPixel);
	m_bytesPerPixel += newChan.bytesPerPixel();
}

inline EqChannelType CqChannelList::sharedChannelType() const
{
	if(m_channels.empty())
		return Channel_TypeUnknown;
	EqChannelType sharedType = m_channels[0].type;
	for(TqListType::const_iterator channel = m_channels.begin(); channel != m_channels.end(); ++channel)
	{
		if(channel->type != sharedType)
			return Channel_TypeUnknown;
	}
	return sharedType;
}

/// Predicate: true if chInfo has the given name.
inline bool chanHasName(const SqChannelInfo& chInfo, const TqChar* name)
{
	return chInfo.name == name;
}

inline void CqChannelList::reorderChannels()
{
	const TqChar* desiredNames[] = { "r", "g", "b", "a" };
	TqInt numNames = sizeof(desiredNames) / sizeof(desiredNames[0]);
	TqInt numChans = m_channels.size();
	// Return if channels are already in the correct order.
	if(numChans <= 1 || std::equal(m_channels.begin(), m_channels.begin()
			+ std::min(numNames, numChans), desiredNames, chanHasName) )
		return;

	// Reorder the channels
	TqListType oldChannels;
	m_channels.swap(oldChannels);
	// Put any of the standard channels from "desiredNames" in the correct
	// order at the beginning of the channel list.
	for(TqInt j = 0; j < numNames; ++j)
	{
		for(TqListType::iterator i = oldChannels.begin(); i != oldChannels.end(); ++i)
		{
			if(i->name == desiredNames[j])
			{
				m_channels.push_back(*i);
				oldChannels.erase(i);
				break;
			}
		}
	}
	// Add the remaining channels back into the 
	std::copy(oldChannels.begin(), oldChannels.end(), std::back_inserter(m_channels));
	recomputeByteOffsets();
}

inline void CqChannelList::addUnnamedChannels(EqChannelType chanType, TqInt numToAdd)
{
	for(TqInt i = 1; i <= numToAdd; ++i)
		addChannel( SqChannelInfo((boost::format("?%02d") % i).str(), chanType) );
}

inline TqInt CqChannelList::findChannelIndexImpl(const std::string& name) const
{
	TqInt index = 0;
	TqListType::const_iterator ichan = m_channels.begin();
	while(ichan != m_channels.end() && ichan->name != name)
	{
		++ichan;
		++index;
	}
	if(ichan == m_channels.end())
		return -1;
	return index;
}

inline void CqChannelList::recomputeByteOffsets()
{
	m_offsets.clear();
	TqInt offset = 0;
	for(TqListType::const_iterator chInfo = m_channels.begin();
			chInfo != m_channels.end(); ++chInfo)
	{
		m_offsets.push_back(offset);
		offset += chInfo->bytesPerPixel();
	}
	m_bytesPerPixel = offset;
}

//------------------------------------------------------------------------------
// Free functions
inline std::ostream& operator<<(std::ostream& out, const CqChannelList& channelList)
{
	EqChannelType sharedChanType = channelList.sharedChannelType();
	if(sharedChanType != Channel_TypeUnknown)
	{
		out << "(";
		for(CqChannelList::const_iterator chan = channelList.begin(), end = channelList.end();
							chan != end; ++chan)
		{
			out << chan->name;
			if(chan+1 != end)
				out << ",";
		}
		out << ")-" << sharedChanType;
	}
	else
	{
		for(CqChannelList::const_iterator chan = channelList.begin(), end = channelList.end();
							chan != end; ++chan)
		{
			out << *chan;
			if(chan+1 != end)
				out << ",";
		}
	}
	return out;
}


} // namespace Aqsis

#endif // CHANNELLIST_H_INCLUDED
