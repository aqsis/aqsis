// Aqsis
// Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the software's owners nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// (This is the New BSD license)

/// \file
///
/// \brief Functions to access standard RI symbols by name
/// \author Chris Foster

#ifndef AQSIS_RISYMS_H_INCLUDED
#define AQSIS_RISYMS_H_INCLUDED

#include <aqsis/ri/ritypes.h>
#include <aqsis/riutil/ricxx.h> // for RtConstBasis

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
