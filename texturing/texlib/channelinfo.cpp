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
 * \brief Implement data structures to hold info about image channels.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 *
 */

#include "channelinfo.h"

#include <iostream>

namespace Aqsis {

//------------------------------------------------------------------------------
// Free functions
TqInt bytesPerPixel(EqChannelType type)
{
	switch(type)
	{
		case Channel_Unsigned32:
		case Channel_Signed32:
		case Channel_Float32:
			return 4;
			break;
		case Channel_Unsigned16:
		case Channel_Signed16:
		case Channel_Float16:
			return 2;
		case Channel_Signed8:
		case Channel_Unsigned8:
		default:
			return 1;
	}
}


std::ostream& operator<<(std::ostream& out, EqChannelType chanType)
{
	switch(chanType)
	{
		case Channel_Unsigned32:
			out << "uint32";
			break;
		case Channel_Signed32:
			out << "int32";
			break;
		case Channel_Float32:
			out << "float32";
			break;
		case Channel_Unsigned16:
			out << "uint16";
			break;
		case Channel_Signed16:
			out << "int16";
			break;
		case Channel_Float16:
			out << "float16";
			break;
		case Channel_Signed8:
			out << "int8";
			break;
		case Channel_Unsigned8:
			out << "uint8";
			break;
		default:
			out << "?type";
			break;
	}
	return out;
}

std::ostream& operator<<(std::ostream& out, const SqChannelInfo& info)
{
	out << info.name << "-" << info.type;
	return out;
}

} // namespace Aqsis
