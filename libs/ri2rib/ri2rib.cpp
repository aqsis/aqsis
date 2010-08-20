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
/// \brief RI frontend for rib writer: context handling.
/// \author Chris Foster [chris42f (at) g mail (d0t) com]
///

#include <aqsis/ri/ri.h>

#include <fstream>
#include <iostream>
#include <boost/shared_ptr.hpp>

#include "../ribparse/ribwriter.h"
#include "../ribparse/ri2ricxx.h"

using namespace Aqsis;

namespace {

// Simple context holding RIB writer, output file, and opaque data for
// generic riToRiCxx translation layer.
struct RiToRibContext
{
    std::ofstream outFile;
    boost::shared_ptr<RibWriterServices> writerServices;
    /// Opaque data for RI -> RiCxx
    void* riToRiCxxData;
};

static RiToRibContext* g_context = 0;

void registerStdFuncs(RibWriterServices& writer)
{
    writer.registerFilterFunc("box", RiBoxFilter);
    writer.registerFilterFunc("gaussian", RiGaussianFilter);
    writer.registerFilterFunc("triangle", RiTriangleFilter);
    writer.registerFilterFunc("mitchell", RiMitchellFilter);
    writer.registerFilterFunc("catmull-rom", RiCatmullRomFilter);
    writer.registerFilterFunc("sinc", RiSincFilter);
    writer.registerFilterFunc("bessel", RiBesselFilter);
    writer.registerFilterFunc("disk", RiDiskFilter);

    writer.registerProcSubdivFunc("DelayedReadArchive", RiProcDelayedReadArchive);
    writer.registerProcSubdivFunc("RunProgram", RiProcRunProgram);
    writer.registerProcSubdivFunc("DynamicLoad", RiProcDynamicLoad);

    writer.registerErrorFunc("ignore", RiErrorIgnore);
    writer.registerErrorFunc("print", RiErrorPrint);
    writer.registerErrorFunc("abort", RiErrorAbort);
}

}

// Ri* functions which deal with context handling
extern "C"
AQSIS_RI_SHARE RtVoid RiBegin(RtToken name)
{
    g_context = new RiToRibContext();
    std::ostream* outStream = &std::cout;
    if(name && strcmp(name, "") != 0 && strcmp(name, "stdout") != 0)
    {
        g_context->outFile.open(name, std::ios::out | std::ios::binary);
        if(!g_context->outFile)
        {
            delete g_context;
            return;
        }
        outStream = &g_context->outFile;
    }
    // TODO: Allow these options to be controlled somehow!
    //
    // Here's the options that libri2rib previously allowed via a RiOption
    // call, invoked before RiBegin.  The token was typeless and had to be
    // given exactly as shown.
    //
    // RiOption("RI2RIB_Output", "Type", "Ascii");
    // RiOption("RI2RIB_Output", "Type", "Binary");
    // RiOption("RI2RIB_Output", "Compression", "None");
    // RiOption("RI2RIB_Output", "Compression", "Gzip");
    // RiOption("RI2RIB_Output", "PipeHandle", h); //< int h is an open file descriptor
    // RiOption("RI2RIB_Indentation", "Type", "None");
    // RiOption("RI2RIB_Indentation", "Type", "Space");
    // RiOption("RI2RIB_Indentation", "Type", "Tab");
    // RiOption("RI2RIB_Indentation", "Size", sz); //< int sz - chars per indent
    g_context->writerServices.reset(
        createRibWriter(*outStream, true, false, false, 4, ' ', ".") );
    g_context->writerServices->addFilter("validate");
    registerStdFuncs(*g_context->writerServices);
    g_context->riToRiCxxData = riToRiCxxBegin(*g_context->writerServices);
}

extern "C"
AQSIS_RI_SHARE RtVoid RiEnd()
{
    riToRiCxxEnd();
    delete g_context;
    g_context = 0;
}

extern "C"
AQSIS_RI_SHARE RtContextHandle RiGetContext()
{
    return static_cast<RtContextHandle>(g_context);
}

extern "C"
AQSIS_RI_SHARE RtVoid RiContext(RtContextHandle handle)
{
    g_context = static_cast<RiToRibContext*>(handle);
    riToRiCxxContext(g_context->riToRiCxxData);
}


// Dummy filter func implementations.
//
// Note that these should have distinct bodies:  MSVC7 has been observed to
// optimize them all down to the same function otherwise!
extern "C" AQSIS_RI_SHARE RtFloat RiGaussianFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth) { return 0; }
extern "C" AQSIS_RI_SHARE RtFloat RiBoxFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth) { return 1; }
extern "C" AQSIS_RI_SHARE RtFloat RiMitchellFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth) { return 2; }
extern "C" AQSIS_RI_SHARE RtFloat RiTriangleFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth) { return 3; }
extern "C" AQSIS_RI_SHARE RtFloat RiCatmullRomFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth) { return 4; }
extern "C" AQSIS_RI_SHARE RtFloat RiSincFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth) { return 5; }
extern "C" AQSIS_RI_SHARE RtFloat RiDiskFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth) { return 6; }
extern "C" AQSIS_RI_SHARE RtFloat RiBesselFilter(RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth) { return 7; }


// Dummy procedural funcs
extern "C"
AQSIS_RI_SHARE RtVoid RiProcFree(RtPointer data)
{
    free(data);
}
extern "C"
AQSIS_RI_SHARE RtVoid RiProcDelayedReadArchive(RtPointer data, RtFloat detail)
{
    std::cerr << "ProcDelayedReadArchive unimplemented\n";
}
extern "C"
AQSIS_RI_SHARE RtVoid RiProcRunProgram(RtPointer data, RtFloat detail)
{
    std::cerr << "ProcRunProgram unimplemented\n";
}
extern "C"
AQSIS_RI_SHARE RtVoid RiProcDynamicLoad(RtPointer data, RtFloat detail)
{
    std::cerr << "ProcDynamicLoad unimplemented\n";
}


// TODO: Make standard error handlers work!

// More-or-less dummy error func implementations
extern "C"
AQSIS_RI_SHARE RtVoid RiErrorIgnore(RtInt code, RtInt severity, RtString message)
{ }
extern "C"
AQSIS_RI_SHARE RtVoid RiErrorPrint(RtInt code, RtInt severity, RtString message)
{
    std::cerr << message << "\n";
}
extern "C"
AQSIS_RI_SHARE RtVoid RiErrorAbort(RtInt code, RtInt severity, RtString message)
{
    RiErrorPrint(code, severity, message);
    abort();
}

// vi: set et:
