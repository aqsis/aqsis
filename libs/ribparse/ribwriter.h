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

#ifndef AQSIS_RIBWRITER_H_INCLUDED
#define AQSIS_RIBWRITER_H_INCLUDED

#include "ricxx.h"

#include <boost/shared_ptr.hpp>

namespace Aqsis {

/// Create an object which serializes Ri::Renderer calls into a RIB stream.
boost::shared_ptr<Ri::RendererServices> createRibWriter(
        std::ostream& out, bool interpolateArchives, bool useBinary,
        bool useGzip, int indentStep, char indentChar,
        const std::string& initialArchivePath);


}

#endif // AQSIS_RIBWRITER_H_INCLUDED
// vi: set et:
