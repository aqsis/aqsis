// Copyright (C) 2003, Timothy M. Shead
//
// Contact: tshead@k-3d.com
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef	___logging_Loaded___
#define	___logging_Loaded___

#include <iostream>

#include <aqsis/aqsis.h>

namespace Aqsis {

/// Returns an output stream to be used for all logging
AQSIS_UTIL_SHARE std::ostream& log();

// iostream-compatible manipulators - use these
// at the beginning of a message to indicate its priority, e.g.
//
// Aqsis::log() << info << "Informational message" << std::endl;
// Aqsis::log() << critical << "Critical message" << std::endl;

AQSIS_UTIL_SHARE std::ostream& emergency(std::ostream&);
AQSIS_UTIL_SHARE std::ostream& alert(std::ostream&);
AQSIS_UTIL_SHARE std::ostream& critical(std::ostream&);
AQSIS_UTIL_SHARE std::ostream& error(std::ostream&);
AQSIS_UTIL_SHARE std::ostream& warning(std::ostream&);
AQSIS_UTIL_SHARE std::ostream& notice(std::ostream&);
AQSIS_UTIL_SHARE std::ostream& info(std::ostream&);
AQSIS_UTIL_SHARE std::ostream& debug(std::ostream&);

} // namespace Aqsis

#endif //	___logging_Loaded___


