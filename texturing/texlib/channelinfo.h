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

#include <string>
#include <iosfwd>

#ifdef USE_OPENEXR
#include <half.h>
#endif

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

/** \brief Get the number of bytes per pixel required to store the given
 * channel type.
 */
TqInt bytesPerPixel(EqChannelType type);

/** \brief Get the EqChannelType for the given template type
 *
 * This function is specialized for each type represented in EqChannelType.
 * For all other types it will return Channel_TypeUnknown.
 */
template<typename T>
inline EqChannelType getChannelTypeEnum();


/** \brief Stream insertion operator for EqChannelType
 *
 * Inserts a human-readable representation of the type to the stream.
 *
 * \param out - stream to write to
 * \param chanType - type to output.
 */
std::ostream& operator<<(std::ostream& out, EqChannelType chanType);

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
	inline TqInt bytesPerPixel() const;
};

/** \brief Stream insertion operator for SqChannelInfo.
 *
 * Inserts a human-readable representation of the channel info to the ostream.
 *
 * \param out - stream to write to
 * \param info - output this.
 */
std::ostream& operator<<(std::ostream& out, const SqChannelInfo& info);

/** Comparison operator for SqChannelInfo
 *
 * \return true if the name and type of the channels are the same
 */
inline bool operator==(const SqChannelInfo& info1, const SqChannelInfo& info2);

//==============================================================================
// Implementation of inline functions and templates
//==============================================================================

//------------------------------------------------------------------------------
// SqChannelInfo
inline SqChannelInfo::SqChannelInfo(const std::string& name, EqChannelType type)
	: name(name),
	type(type)
{ }

inline TqInt SqChannelInfo::bytesPerPixel() const
{
	return Aqsis::bytesPerPixel(type);
}

//------------------------------------------------------------------------------
// Free functions

// getChannelTypeEnum - generic implementation
template<typename T> inline EqChannelType getChannelTypeEnum() { return Channel_TypeUnknown; }

// specializations for getChannelTypeEnum
template<> inline EqChannelType getChannelTypeEnum<TqFloat>() { return Channel_Float32; }
template<> inline EqChannelType getChannelTypeEnum<TqUint>() { return Channel_Unsigned32; }
template<> inline EqChannelType getChannelTypeEnum<TqInt>() { return Channel_Signed32; }
#ifdef USE_OPENEXR
// Need access to the half data type from OpenEXR here.
template<> inline EqChannelType getChannelTypeEnum<half>() { return Channel_Float16; }
#endif
template<> inline EqChannelType getChannelTypeEnum<TqUshort>() { return Channel_Unsigned16; }
template<> inline EqChannelType getChannelTypeEnum<TqShort>() { return Channel_Signed16; }
template<> inline EqChannelType getChannelTypeEnum<TqUchar>() { return Channel_Unsigned8; }
template<> inline EqChannelType getChannelTypeEnum<TqChar>() { return Channel_Signed8; }


inline bool operator==(const SqChannelInfo& info1, const SqChannelInfo& info2)
{
	return info1.name == info2.name && info1.type == info2.type;
}

} // namespace Aqsis

#endif // CHANNELINFO_H_INCLUDED
