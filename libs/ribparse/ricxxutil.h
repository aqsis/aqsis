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
/// \brief Utilities to make working with the Ri types nicer
/// \author Chris Foster [chris42f (at) g mail (d0t) com]
///

#ifndef AQSIS_RICXXUTIL_H_INCLUDED
#define AQSIS_RICXXUTIL_H_INCLUDED

#include "ricxx.h"

#include <cassert>

#include <aqsis/riutil/primvartoken.h>

namespace Aqsis {

/// A class for convenient building of Ri::ParamList instances
///
/// Example:
/// \code
///     float dist = 0;
///     const char* name = "a string";
///     std::vector<int> frames(10);
///
///     // Construct the paramer list (this can also be done inline)
///     ParamListBuilder pList();
///     pList("float dist", &dist)
///          (Ri::TypeSpec(Ri::TypeSpec::String, "name", &name)
///          ("int frames", frames);
///
///     // Can assign to a ParamList, or pass to a function taking one such
///     Ri::ParamList realList = pList;
/// \endcode
///
class ParamListBuilder
{
    public:
        /// Add a single value, v to the parameter list
        template<typename T>
        ParamListBuilder& operator()(const char* token, T* v);
        /// Add a single value, v to the parameter list
        template<typename T>
        ParamListBuilder& operator()(const Ri::TypeSpec& spec,
                                     const char* name, T* v);

        /// Add an array, v to the parameter list
        template<typename T>
        ParamListBuilder& operator()(const char* token,
                                     const std::vector<T>& v);
        /// Add an array, v to the parameter list
        template<typename T>
        ParamListBuilder& operator()(const Ri::TypeSpec& spec,
                                     const char* name,
                                     const std::vector<T>& v);

        /// Implicity convert to Ri::ParamList
        operator Ri::ParamList();

    private:
        std::vector<Ri::Param> m_paramStorage;
};


//==============================================================================
// implementation details

template<typename T>
ParamListBuilder& ParamListBuilder::operator()(const char* token, T* v)
{
    const char* nameBegin = 0;
    const char* nameEnd = 0;
    Ri::TypeSpec spec = parseDeclaration(token, &nameBegin, &nameEnd);
    assert(*nameEnd == '\0');
    m_paramStorage.push_back(
        Ri::Param(spec, nameBegin, Ri::Array<T>(v, 1)));
    return *this;
}

template<typename T>
ParamListBuilder& ParamListBuilder::operator()(const char* token,
                                               const std::vector<T>& v)
{
    const char* nameBegin = 0;
    const char* nameEnd = 0;
    Ri::TypeSpec spec = parseDeclaration(token, &nameBegin, &nameEnd);
    assert(*nameEnd == '\0');
    m_paramStorage.push_back(
        Ri::Param(spec, nameBegin,
                    Ri::Array<T>(v.empty() ? 0 : &v[0], v.size())));
    return *this;
}

template<typename T>
ParamListBuilder& ParamListBuilder::operator()(const Ri::TypeSpec& spec,
                                               const char* name, T* v)
{
    m_paramStorage.push_back(
        Ri::Param(spec, name, Ri::Array<T>(v, 1)));
    return *this;
}

template<typename T>
ParamListBuilder& ParamListBuilder::operator()(const Ri::TypeSpec& spec,
                                               const char* name,
                                               const std::vector<T>& v)
{
    m_paramStorage.push_back(
        Ri::Param(spec, name,
                    Ri::Array<T>(v.empty() ? 0 : &v[0], v.size())));
    return *this;
}

ParamListBuilder::operator Ri::ParamList()
{
    if(m_paramStorage.empty())
        return Ri::ParamList();
    return Ri::ParamList(&m_paramStorage[0], m_paramStorage.size());
}


}

#endif // AQSIS_RICXXUTIL_H_INCLUDED
// vi: set et:
