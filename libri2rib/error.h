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
 *  \brief Error messages storage class.
 *  \author Lionel J. Lacour (intuition01@online.fr)
 */

#ifndef RI2RIB_ERROR_H
#define RI2RIB_ERROR_H 1

#include <string>
#include "aqsis.h"
#include "ri.h"

namespace libri2rib {

class CqError
{
private:
    RtInt  code;
    RtInt  severity;
    std::string message;
    TqBool to_rib;

public:
    CqError(RtInt cd, RtInt sev, std::string msg, TqBool tr)
	: code(cd), severity(sev), message(msg), to_rib(tr) {}
    ~CqError() {}
    RtVoid manage();
};

} /* namespace libri2rib */
#endif
