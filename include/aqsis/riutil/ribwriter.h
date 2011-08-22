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

#ifndef AQSIS_RIBWRITER_H_INCLUDED
#define AQSIS_RIBWRITER_H_INCLUDED

#include <aqsis/config.h>
#include <aqsis/riutil/ricxx.h>

#include <string>

namespace Aqsis {

/// RendererServices refinement to allow registration of custom functions.
///
/// The register*Func() methods allow the user define names for their own
/// filter functions, etc, as output into the RIB file.  (Dummy function
/// pointers come pre-registered so that get*Func() work for the default
/// functions, but sometimes it's necessary to override these.)
class RibWriterServices : public Ri::RendererServices
{
    public:
        /// Register a name for a user-defined filter function.
        virtual void registerFilterFunc(RtConstString name,
                                        RtFilterFunc func) = 0;
        /// Register a name for a user-defined subdivision function.
        virtual void registerProcSubdivFunc(RtConstString name,
                                            RtProcSubdivFunc func) = 0;
        /// Register a name for a user-defined error function.
        virtual void registerErrorFunc(RtConstString name,
                                       RtErrorFunc func) = 0;
};

/// Options for RibWriter
struct RibWriterOptions
{
    /// Read and interpolate ReadArchive files into the RIB stream
    bool interpolateArchives;
    /// If true, print the standard procedurals, and free procedural data
    bool handleProcedurals;
    /// Produce binary RIB
    bool useBinary;
    /// Produce gzipped RIB
    bool useGzip;
    /// Number of chars per indent level
    int indentStep;
    /// Character to use for indenting (should be whitespace)
    char indentChar;
    /// Path for finding archive files
    std::string archivePath;

    RibWriterOptions()
        : interpolateArchives(false),
        handleProcedurals(true),
        useBinary(false),
        useGzip(false),
        indentStep(4),
        indentChar(' '),
        archivePath(".")
    { }
};

/// Create an object which serializes Ri::Renderer calls into a RIB stream.
AQSIS_RIUTIL_SHARE
RibWriterServices* createRibWriter(std::ostream& out,
                                   const RibWriterOptions& opts);

} // namespace Aqsis

#endif // AQSIS_RIBWRITER_H_INCLUDED
// vi: set et:
