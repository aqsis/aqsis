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
 * \brief Define data structures to hold info about image channels.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 *
 */

#ifndef CHANNELINFO_H_INCLUDED
#define CHANNELINFO_H_INCLUDED

#include "aqsis.h"

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief Represent the possible image channel types
 */
enum EqChannelType
{
	Channel_Float32,
	Channel_Unsigned32,
	Channel_Signed32,
	Channel_Float16,     // OpenEXR "half" data type.
	Channel_Unsigned16,
	Channel_Signed16,
	Channel_Unsigned8,
	Channel_Signed8,
	Channel_TypeUnknown
};

template<typename T>
EqChannelType getChannelTypeEnum();

//------------------------------------------------------------------------------
/** \brief Hold name and type information about image channels.
 */
struct SqChannelInfo
{
	std::string name;  ///< name of the channel (eg, "r", "g"...)
	EqChannelType type;	///< channel format type
	/// Trivial constructor
	inline SqChannelInfo(const std::string& name, EqChannelType type);
	/** \brief Get the number of bytes per pixel for this channel data.
	 *
	 * \return channel size in bytes
	 */
	TqInt bytesPerPixel() const;
};


//==============================================================================
// Implementation of inline functions and templates
//==============================================================================

// SqChannelInfo
inline SqChannelInfo::SqChannelInfo(const std::string& name, EqChannelType type)
	: name(name),
	type(type)
{ }


template<typename T> EqChannelType getChannelTypeEnum() { return Channel_TypeUnknown; }

template<> EqChannelType getChannelTypeEnum<TqFloat>() { return Channel_Float32; }
template<> EqChannelType getChannelTypeEnum<TqUint>() { return Channel_Unsigned32; }
template<> EqChannelType getChannelTypeEnum<TqInt>() { return Channel_Signed32; }
#if 0
// Need access to the half data type from OpenEXR here.
template<> EqChannelType getChannelTypeEnum<>() { return Channel_Float16; }
#endif
template<> EqChannelType getChannelTypeEnum<TqUshort>() { return Channel_Unsigned16; }
template<> EqChannelType getChannelTypeEnum<TqShort>() { return Channel_Signed16; }
template<> EqChannelType getChannelTypeEnum<TqUchar>() { return Channel_Unigned8; }
template<> EqChannelType getChannelTypeEnum<TqChar>() { return Channel_Signed8; }

} // namespace Aqsis

#endif // CHANNELINFO_H_INCLUDED
