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


/** \file
		\brief Declares a class to manage a 2D region of float values with
		arbitrary channel names.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is channelbuffer.h included already?
#ifndef CHANNELBUFFER_H_INCLUDED
//{
#define CHANNELBUFFER_H_INCLUDED 1

#include "aqsis.h"

#include <string>
#include <map>
#include <vector>
#include <stdexcept>
#include <iostream>

#include "iddmanager.h"

namespace Aqsis {

//-----------------------------------------------------------------------
/** \class CqChannelBuffer
 * Class to store a 2D region of float data with arbitrary channel names.
 */

class CqChannelBuffer : public IqChannelBuffer
{
	public:
		CqChannelBuffer();
		virtual ~CqChannelBuffer() {}	

		void clearChannels();
		TqInt addChannel(const std::string& name, TqInt size);
		void allocate(TqInt width, TqInt height);

		TqChannelPtr operator()(TqInt x, TqInt y, TqInt index);

		// Overidden from IqChannelBuffer
		virtual TqInt width() const;
		virtual TqInt height() const;
		virtual TqInt getChannelIndex(const std::string& name) const;
		virtual TqConstChannelPtr operator()(TqInt x, TqInt y, TqInt index) const;
	
	private:
		TqInt	m_width;
		TqInt	m_height;
		TqInt m_elementSize;
		std::map<std::string, std::pair<TqInt, TqInt> >	m_channels;
		TqChannelValues	m_data;
};

//--------------------------------------------------------
// Implementation
//

inline void CqChannelBuffer::clearChannels()
{
	m_channels.clear();
	m_data.clear();
	m_elementSize = 0;
}

inline CqChannelBuffer::CqChannelBuffer() : m_elementSize(0)
{}

inline TqInt CqChannelBuffer::addChannel(const std::string& name, TqInt size)
{
		if(m_channels.find(name) != m_channels.end())
			throw(std::string("Error: channel already exists"));

		m_channels[name] = std::pair<int, int>(m_channels.size()-1, size);
		m_elementSize += size;
		
		return m_channels[name].first;
}

inline TqInt CqChannelBuffer::getChannelIndex(const std::string& name) const
{
	std::map<std::string, std::pair<TqInt, TqInt> >::const_iterator iter;
	if((iter = m_channels.find(name)) != m_channels.end())
		return iter->second.first;
	else
		// \todo: Need to deal with this error
		return 0;
}

inline void CqChannelBuffer::allocate(TqInt width, TqInt height)
{
	m_width = width;
	m_height = height;
	m_data.resize(m_width*m_height*m_elementSize);
}

inline IqChannelBuffer::TqChannelPtr CqChannelBuffer::operator()(TqInt x, TqInt y, TqInt index)
{
	assert(index < static_cast<TqInt>(m_channels.size()));
	assert(x < m_width);
	assert(y < m_height);

	TqInt offset = ( y * m_width * m_elementSize ) + ( x * m_elementSize ) + index;

	assert(offset < static_cast<TqInt>(m_data.size()));

	return m_data.begin() + offset;
}

inline IqChannelBuffer::TqConstChannelPtr CqChannelBuffer::operator()(TqInt x, TqInt y, TqInt index) const
{
	assert(index < static_cast<TqInt>(m_channels.size()));
	assert(x < m_width);
	assert(y < m_height);

	TqUlong offset = ( y * m_width * m_channels.size() ) + ( x * m_channels.size() ) + index;

	assert(offset < m_data.size() );

	return m_data.begin() + offset;
}

inline TqInt CqChannelBuffer::width() const
{
	return m_width;
}

inline TqInt CqChannelBuffer::height() const
{
	return m_height;
}

//-----------------------------------------------------------------------

} // namespace Aqsis

//}  // End of #ifdef CHANNELBUFFER_H_INCLUDED
#endif

