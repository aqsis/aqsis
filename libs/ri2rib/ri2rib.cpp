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
/// \brief RI frontend for rib writer: context handling.
/// \author Chris Foster [chris42f (at) g mail (d0t) com]

#include <aqsis/ri/ri.h>

#include <fstream>
#include <iostream>
#include <boost/shared_ptr.hpp>

#include <aqsis/riutil/ri2ricxx.h>
#include <aqsis/riutil/ribwriter.h>
#include <aqsis/riutil/risyms.h>

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

RibWriterOptions g_writerOpts;
std::ostream* g_ostream;

}

namespace Aqsis {
// Collect any configuration arguments which happen to be passed before
// RiBegin.
//
// Here are the supported configuration options:
//
// "RI2RIB_Output", "Type", "Ascii"
// "RI2RIB_Output", "Type", "Binary"
// "RI2RIB_Output", "Compression", "None"
// "RI2RIB_Output", "Compression", "Gzip"
// "RI2RIB_Indentation", "Type", "None"
// "RI2RIB_Indentation", "Type", "Space"
// "RI2RIB_Indentation", "Type", "Tab"
// "RI2RIB_Indentation", "Size", sz  (sz = num chars per indent, an integer)
//
// Note that a previous version of libri2rib also supported the following
// option:
//
//   RiOption("RI2RIB_Output", "PipeHandle", h);
//
// where the integer h was a open file descriptor.  So far this isn't supported
// here, since the new "OStream" option is more general.  (By using
// boost::iostream::file_descriptor_sink, it's possible to construct a
// iostreams compatible stream which writes to a pipe.)
void riToRiCxxOptionPreBegin(RtToken name, RtInt count, RtToken* tokens,
                             RtPointer* values)
{
    for(int i = 0; i < count; ++i)
    {
        if(!strcmp(name, "RI2RIB_Output"))
        {
            if(!strcmp(tokens[i], "Type"))
            {
                const char* value = *static_cast<RtToken*>(values[i]);
                if(!strcmp(value, "Ascii"))
                    g_writerOpts.useBinary = false;
                else if(!strcmp(value, "Binary"))
                    g_writerOpts.useBinary = true;
            }
            else if(!strcmp(tokens[i], "Compression"))
            {
                const char* value = *static_cast<RtToken*>(values[i]);
                if(!strcmp(value, "None"))
                    g_writerOpts.useGzip = false;
                else if(!strcmp(value, "Gzip"))
                    g_writerOpts.useGzip = true;
            }
            else if(!strcmp(tokens[i], "OStream"))
                g_ostream = static_cast<std::ostream*>(values[i]);
        }
        else if(!strcmp(name, "RI2RIB_Indentation"))
        {
            if(!strcmp(tokens[i], "Type"))
            {
                const char* value = *static_cast<RtToken*>(values[i]);
                if(!strcmp(value, "None"))
                    g_writerOpts.indentStep = 0;
                else if(!strcmp(value, "Space"))
                    g_writerOpts.indentChar = ' ';
                else if(!strcmp(value, "Tab"))
                    g_writerOpts.indentChar = '\t';
            }
            else if(!strcmp(tokens[i], "Size"))
                g_writerOpts.indentStep = *static_cast<int*>(values[i]);
        }
    }
}
}

//--------------------------------------------------
// Ri* functions which deal with context handling
extern "C"
AQSIS_RI_SHARE RtVoid RiBegin(RtToken name)
{
    g_context = new RiToRibContext();
    std::ostream* outStream = &std::cout;
    if(g_ostream)
        outStream = g_ostream;
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
    g_context->writerServices.reset(createRibWriter(*outStream, g_writerOpts));
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


//--------------------------------------------------
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


//--------------------------------------------------
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


//--------------------------------------------------
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
