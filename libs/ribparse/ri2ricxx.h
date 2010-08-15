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

#ifndef AQSIS_RI2RICXX_H_INCLUDED
#define AQSIS_RI2RICXX_H_INCLUDED

#include "ricxx.h"

#include <boost/shared_ptr.hpp>

namespace Aqsis {

/// Initialize the conversion interface.
///
/// ourData is an opaque smart pointer in which conversion interface data
/// should be held.
void riToRiCxxBegin(Ri::RendererServices* services,
                    boost::shared_ptr<void>& ourData);

/// Context switching to the given context and conversion interface state
void riToRiCxxContext(Ri::RendererServices* services,
                      boost::shared_ptr<void>& ourData);

/// Reset the conversion interface to the initial state.
void riToRiCxxEnd();

} // namespace Aqsis

#endif // AQSIS_RI2RICXX_H_INCLUDED
// vi: set et:
