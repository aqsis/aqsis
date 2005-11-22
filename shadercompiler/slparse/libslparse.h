// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#ifndef LIBSLPARSE_H_INCLUDED
#define LIBSLPARSE_H_INCLUDED

#include <iostream>
#include <string>

#include	"aqsis.h"
#include	"sstring.h"


START_NAMESPACE( Aqsis )

struct IqParseNode;

/// Parses an input stream, using the supplied callback object and sending error data to the supplied output stream
TqBool Parse( std::istream& InputStream, const CqString StreamName, std::ostream& ErrorStream );
/// Resets the state of the parser, clearing any symbol tables, etc.
void ResetParser();

IqParseNode* GetParseTree();

END_NAMESPACE( Aqsis )

#endif //LIBSLPARSE_H_INCLUDED
