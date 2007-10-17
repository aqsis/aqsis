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
 * \brief Implement class for dealing with image file metadata
 *
 * \author Chris Foster
 */

#include "texfileheader.h"

#include <boost/format.hpp>
#include <ctime>

namespace Aqsis {

//------------------------------------------------------------------------------
// CqTexFileHeader implementation

/** \brief Get a date-time string in the format expected for TIFF files
 *
 * The tiff date-time format is the 19 characters: "YYYY:MM:DD hh:mm:ss"
 * In the absence (?) of another well-defined format, we use this.
 */
std::string tiffDateTimeString()
{
	time_t long_time;
	// Get time as long integer.
	time( &long_time );
	// Convert to local time.
	struct tm* ct = localtime( &long_time );
	return (boost::format("%04d:%02d:%02d %02d:%02d:%02d")
			% (1900 + ct->tm_year) % (ct->tm_mon + 1) % ct->tm_mday
			% ct->tm_hour % ct->tm_min % ct->tm_sec).str();
}

void CqTexFileHeader::addStandardAttributes()
{
	setAttribute<TqInt>("width", 100);
	setAttribute<TqInt>("height", 100);
	setAttribute<bool>("isTiled", false);
	setAttribute<std::string>("compression", "none");
	setAttribute<CqChannelList>("channels", CqChannelList());
	setAttribute<TqFloat>("pixelAspectRatio", 1.0f);
	setAttribute<std::string>("dateTime", tiffDateTimeString());
}

} // namespace Aqsis
