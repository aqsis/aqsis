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
/// \brief Aqsis RIB parser interface.
/// \author Chris Foster [chris42f (at) g mail (d0t) com]
///

#ifndef AQSIS_RIBPARSER_H_INCLUDED
#define AQSIS_RIBPARSER_H_INCLUDED

#include <aqsis/aqsis.h>

namespace Aqsis
{

namespace Ri { class Renderer; class RendererServices; }

//------------------------------------------------------------------------------
/// Parser for standard RIB streams.
///
/// An instance of RibParser parses a RIB stream, sending all encountered
/// interface function calls to the supplied callback interface.
class RibParser
{
    public:
        /// Create RIB parser instance
        ///
        /// The services parameter is used by the parser to look up standard
        /// bases from their names, parse string tokens, etc.
        static RibParser* create(Ri::RendererServices& services);

        /// Parse a RIB stream, sending requests to the callback interface
        ///
        /// \param ribStream - RIB stream to be parsed.  May be gzipped.
        /// \param streamName - name of the stream, present in error messages
        /// \param context - parsed interface function calls will be sent here
        virtual void parseStream(std::istream& ribStream,
                                 const std::string& streamName,
                                 Ri::Renderer& context) = 0;

        virtual ~RibParser() {}
};

} // namespace Aqsis

#endif // AQSIS_RIBPARSER_H_INCLUDED
// vi: set et:
