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

#ifndef AQSIS_RICXX2RI_INCLUDED
#define AQSIS_RICXX2RI_INCLUDED

#include <aqsis/riutil/ricxx.h>

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

namespace Aqsis {

class RiCxxToRi;
class RibParser;

/// Callback function type for passing comments found in the RIB.
typedef boost::function<void (const char*, const char*)> ArchiveRecordCallback;

/// Converter from Ri::Renderer to the traditional C API
///
/// We need to add an extra function or two here to implement some C API
/// stuff; in particular setArchiveRecordCallback(), which doesn't seem like it
/// should be part of the public RendererServices API.
class RiCxxToRiServices : public Ri::RendererServices
{
    public:
        RiCxxToRiServices();

        /// Parse a RIB stream, with archive records going to the given
        /// callback.
        ///
        /// This extra version of parseRib with the callback is necessary to
        /// implement the callback parameter in the traditional RiReadArchive.
        virtual void parseRib(std::istream& ribStream, const char* name,
                              const ArchiveRecordCallback& callback);

        // Stuff from Ri::RendererServices:
        virtual Ri::ErrorHandler& errorHandler() { return *m_errorHandler; }

        virtual RtFilterFunc     getFilterFunc(RtConstToken name) const;
        virtual RtConstBasis*    getBasis(RtConstToken name) const;
        virtual RtErrorFunc      getErrorFunc(RtConstToken name) const;
        virtual RtProcSubdivFunc getProcSubdivFunc(RtConstToken name) const;

        virtual Ri::TypeSpec getDeclaration(RtConstToken token,
                                            const char** nameBegin = 0,
                                            const char** nameEnd = 0) const;

        virtual void addFilter(const char* name,
                               const Ri::ParamList& filterParams
                                = Ri::ParamList());
        virtual Ri::Renderer& firstFilter();

        virtual void parseRib(std::istream& ribStream, const char* name,
                              Ri::Renderer& context);

    private:
        /// Converter from Ri::Renderer to the C API
        boost::shared_ptr<RiCxxToRi> m_renderer;
        /// Aqsis::log error handler
        boost::shared_ptr<Ri::ErrorHandler> m_errorHandler;
        /// Chain of filters
        std::vector<boost::shared_ptr<Ri::Renderer> > m_filterChain;
        /// RIB parser
        boost::shared_ptr<RibParser> m_parser;
};

}

#endif // AQSIS_RICXX2RI_INCLUDED
// vi: set et:
