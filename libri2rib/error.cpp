// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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
 *  \brief Error reporting class implementation.
 *  \author Lionel J. Lacour (intuition01@online.fr)
 */
#include <iostream>
#include <stdlib.h>
#include "error.h"

RtInt RiLastError;

RtVoid RiErrorIgnore( RtInt cd, RtInt sev, const char *msg) {}
RtVoid RiErrorPrint( RtInt cd, RtInt sev, const char *msg) {}
RtVoid RiErrorAbort( RtInt cd, RtInt sev, const char *msg) {}

using namespace libri2rib;

RtVoid CqError::manage ()
{
    RiLastError=code;
    std::cerr <<"RI2RIB: "<< message << std::endl;
    if (severity==RIE_SEVERE) exit(EXIT_FAILURE);

    if (to_rib==TqTrue) {
	switch (severity) {
	case RIE_INFO: message.insert(0,"INFO: "); break;
	case RIE_WARNING: message.insert(0, "WARNING: "); break;
	case RIE_ERROR: message.insert(0, "ERROR: "); break;
	default: break;
	}
	RiArchiveRecord(RI_COMMENT, const_cast<char *> (message.c_str()));
    }
}
