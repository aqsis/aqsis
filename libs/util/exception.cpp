// Aqsis
// Copyright 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
//
// This program is free software; you can redistribute it and/or
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
 * \brief Declares classes for the aqsis exception heiarchy
 */

#include <aqsis/util/exception.h>

#include <ostream>
#include <boost/filesystem/path.hpp>

namespace Aqsis
{

std::ostream& operator<<(std::ostream& o, const XqException& e)
{
	o << e.description () << " (" << boost::filesystem::path(e.where().first).leaf() << ", " << e.where().second << ")";
	o <<": " << e.what();

	return o;
}

} // namespace Aqsis
