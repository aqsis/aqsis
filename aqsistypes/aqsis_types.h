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
		\brief Declares typedefs for the basic types.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/


#ifndef	AQSIS_TYPES_INCLUDED
#define	AQSIS_TYPES_INCLUDED

#include <boost/cstdint.hpp>

typedef	char	TqChar;
typedef	unsigned char	TqUchar;
typedef	char*	TqPchar;
typedef	unsigned char*	TqPuchar;

typedef	int	TqInt;
typedef	unsigned int	TqUint;
typedef	long	TqLong;
typedef	unsigned long	TqUlong;

typedef	short	TqShort;
typedef	unsigned short	TqUshort;

typedef	float	TqFloat;
typedef	double	TqDouble;


// Integer types with specific size.
typedef boost::int8_t TqInt8;
typedef boost::int16_t TqInt16;
typedef boost::int32_t TqInt32;
typedef boost::int64_t TqInt64;

typedef boost::uint8_t TqUint8;
typedef boost::uint16_t TqUint16;
typedef boost::uint32_t TqUint32;
typedef boost::uint64_t TqUint64;


#endif	// AQSIS_TYPES_INCLUDED
