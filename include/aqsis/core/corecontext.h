// Aqsis
// Copyright (C) 1997 - 2010, Paul C. Gregory
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

/// \file
///
/// \brief Ri::RendererServices object for the core renderer.
/// \author Chris Foster [chris42f (at) g mail (d0t) com]
///

#ifndef AQSIS_CORECONTEXT_H_INCLUDED
#define AQSIS_CORECONTEXT_H_INCLUDED

#include <aqsis/riutil/ricxx.h>
#include <aqsis/config.h>

namespace Aqsis {

/// Get the core renderer context object.
///
/// This must be called after RiBegin().
AQSIS_CORE_SHARE
Ri::RendererServices* cxxRenderContext();

}

#endif // AQSIS_CORECONTEXT_H_INCLUDED
