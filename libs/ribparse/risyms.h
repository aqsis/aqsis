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
/// \brief Functions to access standard RI symbols by name
/// \author Chris Foster

#ifndef AQSIS_RISYMS_H_INCLUDED
#define AQSIS_RISYMS_H_INCLUDED

#include <aqsis/ri/ritypes.h>
#include "ricxx.h" // for RtConstBasis

namespace Aqsis {

class RibWriterServices;

/// Get a standard RI filter function from its name
RtFilterFunc getFilterFuncByName(const char* name);
/// Get a standard RI basis from its name
RtConstBasis* getBasisByName(const char* name);
/// Get a standard RI error function from its name
RtErrorFunc getErrorFuncByName(const char* name);
/// Get a standard RI procedural subdivision function from its name
RtProcSubdivFunc getProcSubdivFuncByName(const char* name);

/// Register the standard RI functions with the given RIB writer.
///
/// This ensures that when the writer is passed a function handle - RiBoxFilter
/// for example - it can turn that function handle into the correct string to
/// be printed.
void registerStdFuncs(RibWriterServices& writer);

} // namespace Aqsis

#endif // AQSIS_RISYMS_H_INCLUDED
