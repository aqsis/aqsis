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
