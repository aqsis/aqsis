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

/// \file
///
/// \brief Converter from plain RI calls to the Ri::Renderer interface.
/// \author Chris Foster [chris42f (at) g mail (d0t) com]
///
/// ri2ricxx is a conversion wrapper from plain RI calls to the Ri::Renderer
/// interface.  It should be initialized via riToRiCxxBegin() with an object
/// to which calls will be passed.

#ifndef AQSIS_RI2RICXX_H_INCLUDED
#define AQSIS_RI2RICXX_H_INCLUDED

#include "ricxx.h"

namespace Aqsis {

/// Initialize the conversion interface.
///
/// \param services provides error reporting and the callback api via
/// services.firstFilter().
///
/// Returns an opaque pointer to data needed by the conversion interface.
void* riToRiCxxBegin(Ri::RendererServices& services);

/// Switching to the given context, previously initialized with
/// riToRiCxxBegin()
void riToRiCxxContext(void* context);

/// Delete the current context.
void riToRiCxxEnd();

/// Function called when RiOption{V} is called before RiBegin.
///
/// The riToRiCxx calling code should define an appropriate implementation of
/// this function (though it may be simply empty).  The arguments correspond
/// directly to those of RiOptionV().
void riToRiCxxOptionPreBegin(RtToken name, RtInt count, RtToken* tokens,
                             RtPointer* values);

} // namespace Aqsis

#endif // AQSIS_RI2RICXX_H_INCLUDED
// vi: set et:
