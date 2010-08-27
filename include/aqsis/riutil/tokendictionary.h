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

/** \file
 *
 * \brief A dictionary in which to look up primitive variable tokens.
 * \author Chris Foster [chris42f (at) g mail (d0t) com]
 */

#ifndef TOKENDICTIONARY_H_INCLUDED
#define TOKENDICTIONARY_H_INCLUDED

#include <aqsis/aqsis.h>

#include <map>

#include <aqsis/util/exception.h>
#include <aqsis/riutil/primvartoken.h>

namespace Aqsis
{

/// A dictionary associating variable names with types.
class AQSIS_RIUTIL_SHARE TokenDict
{
    public:
        /// Construct token dict, with all the standard variables pre-declared.
        TokenDict();

        /// Associate a variable name with a type string
        ///
        /// The type string should be in the format
        ///   [ class ]  type  [ '[' count ']' ]
        void declare(const char* name, const char* type);
        /// Associate a variable name with the given type
        void declare(const char* name, const Ri::TypeSpec& type);

        /// Parse a token string, extracting the type and name.
        ///
        /// If token is an "inline declaration" containing the type, that type
        /// is returned.  Otherwise the name is looked up in the dictionary to
        /// find the type.  If the name isn't found, an XqValidation exception
        /// is thrown.
        ///
        /// If the optional parameters nameBegin and nameEnd are non-null, on
        /// return they delimit the variable name stored in the token string.
        Ri::TypeSpec lookup(const char* token,
                            const char** nameBegin = 0,
                            const char** nameEnd = 0) const;

        /// Parse token string, extract type and name.
        ///
        /// This is a more convenient version of lookup(), to be used when
        /// you're going to convert the name to a std::string (and hence may
        /// allocate memory, in contrast to the alternative version).
        Ri::TypeSpec lookup(const char* token, std::string* name) const;

    private:
        typedef std::map<std::string, Ri::TypeSpec> Dict;
        Dict m_dict;
};


/** \brief Get the list of standard predefined primitive variables.
 *
 * The returned vector includes predefined token declarations for tokens
 * involving:
 *   - Standard shader instance variables (eg, "Ka")
 *   - Standar primvars (eg, "P")
 *   - Arguments for standard attributes and options (eg, "gridsize")
 *   - Some aqsis-specific attributes and options.
 *
 * Note that the RISpec says nothing about which variables should be
 * predefined, so this list may be missing some variables.
 *
 * \return A vector of CqPrimvarTokens representing standard predefined tokens.
 */
AQSIS_RIUTIL_SHARE const std::vector<CqPrimvarToken>& standardPrimvars();


} // namespace Aqsis

#endif // TOKENDICTIONARY_H_INCLUDED
// vi: set et:
