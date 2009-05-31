// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
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


/** \file
		\brief Implements the entrypoint for the shader compiler.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#ifndef LIBSLPARSE_H_INCLUDED
#define LIBSLPARSE_H_INCLUDED

#include <aqsis/aqsis.h>

#include <iostream>
#include <string>

namespace Aqsis {

struct IqParseNode;

/// Parses an input stream, using the supplied callback object and sending
/// error data to the supplied output stream
AQSIS_SLCOMP_SHARE bool Parse(std::istream& InputStream, const std::string& StreamName,
		   std::ostream& ErrorStream );

/// Resets the state of the parser, clearing any symbol tables, etc.
AQSIS_SLCOMP_SHARE void ResetParser();

AQSIS_SLCOMP_SHARE IqParseNode* GetParseTree();

} // namespace Aqsis

#endif //LIBSLPARSE_H_INCLUDED
