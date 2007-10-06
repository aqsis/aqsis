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

void CqChannelList::reorderChannels()
{
	// If there are "r", "g", "b" and "a" channels, ensure they
	// are in the expected order.
	const char* elements[] = { "r", "g", "b", "a" };
	TqInt numElements = sizeof(elements) / sizeof(elements[0]);
	for(int elementIndex = 0; elementIndex < numElements; ++elementIndex)
	{
		for(TqListType::iterator channel = m_channels.begin(); channel != m_channels.end(); ++channel)
		{
			// If this entry in the channel list matches one in the expected list, 
			// move it to the right point in the channel list.
			if(channel->name == elements[elementIndex])
			{
				const SqChannelInfo temp = m_channels[elementIndex];
				m_channels[elementIndex] = *channel;
				*channel = temp;
				break;
			}
		}
	}
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


} // namespace Aqsis
