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
 * \brief Implementation of class encapsulating information about image channels
 *
 * \author Chris Foster
 */

#include "channellist.h"

#include <iostream>
#include <algorithm>

#include <boost/format.hpp>

namespace Aqsis {

//------------------------------------------------------------------------------
// CqChannelList implementation
CqChannelList CqChannelList::displayChannels()
{
    CqChannelList displayChannels;
    displayChannels.addChannel(SqChannelInfo("r", Channel_Unsigned8));
    displayChannels.addChannel(SqChannelInfo("g", Channel_Unsigned8));
    displayChannels.addChannel(SqChannelInfo("b", Channel_Unsigned8));
	return displayChannels;
}

void CqChannelList::addChannel(const SqChannelInfo& newChan)
{
	m_channels.push_back(newChan);
	m_offsets.push_back(m_bytesPerPixel);
	m_bytesPerPixel += newChan.bytesPerPixel();
}

EqChannelType CqChannelList::sharedChannelType() const
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
bool chanHasName(const SqChannelInfo& chInfo, const TqChar* name)
{
	return chInfo.name == name;
}

void CqChannelList::reorderChannels()
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

void CqChannelList::addUnnamedChannels(EqChannelType chanType, TqInt numToAdd)
{
	for(TqInt i = 1; i <= numToAdd; ++i)
		addChannel( SqChannelInfo((boost::format("?%02d") % i).str(), chanType) );
}

TqInt CqChannelList::findChannelIndexImpl(const std::string& name) const
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

void CqChannelList::recomputeByteOffsets()
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
std::ostream& operator<<(std::ostream& out, const CqChannelList& channelList)
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
