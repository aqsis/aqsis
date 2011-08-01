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
/// \brief Converter from plain RI calls to the Ri::Renderer interface.
/// \author Chris Foster [chris42f (at) g mail (d0t) com]
///
/// ri2ricxx is a conversion wrapper from plain RI calls to the Ri::Renderer
/// interface.  It should be initialized via riToRiCxxBegin() with an object
/// to which calls will be passed.

#ifndef AQSIS_RI2RICXX_H_INCLUDED
#define AQSIS_RI2RICXX_H_INCLUDED

#include <aqsis/ri/ritypes.h>

namespace Aqsis {

namespace Ri { class RendererServices; }

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
