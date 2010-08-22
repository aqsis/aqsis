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

#include <aqsis/riutil/ricxx.h>

#include <string>
#include <boost/shared_ptr.hpp>

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
        : interpolateArchives(true),
        useBinary(false),
        useGzip(false),
        indentStep(4),
        indentChar(' '),
        archivePath(".")
    { }
};

/// Create an object which serializes Ri::Renderer calls into a RIB stream.
RibWriterServices* createRibWriter(std::ostream& out,
                                   const RibWriterOptions& opts);

} // namespace Aqsis

#endif // AQSIS_RIBWRITER_H_INCLUDED
// vi: set et:
