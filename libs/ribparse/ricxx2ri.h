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

#ifndef AQSIS_RICXX2RI_INCLUDED
#define AQSIS_RICXX2RI_INCLUDED

#include "ricxx.h"

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
