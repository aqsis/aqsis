// Aqsis
// Copyright (C) 1997 - 2010, Paul C. Gregory
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

#ifndef VARSPEC_H_INCLUDED
#define VARSPEC_H_INCLUDED

#include <vector>
#include <iostream>

#include "ustring.h"

struct VarSpec
{
    /// Variable type.
    enum Type
    {
        Float,
        Point,
        Hpoint,
        Vector,
        Normal,
        Color,
        Matrix,
        String
    };

    Type     type;       ///< Variable type
    int      arraySize;  ///< Array size
    ustring  name;       ///< Variable name

    VarSpec() : type(Float), arraySize(1), name() { }

    VarSpec(Type type, int arraySize, ustring name)
        : type(type), arraySize(arraySize), name(name) { }

    /// Get number of scalar values required for the given type
    static int sizeForType(Type type)
    {
        switch(type)
        {
            case Float:  return 1;
            case Point:  return 3;
            case Hpoint: return 4;
            case Vector: return 3;
            case Normal: return 3;
            case Color:  return 3;
            case Matrix: return 16;
            case String: return 1;
        }
    }

    /// Get number of scalar values required for the variable
    ///
    /// Each element of the variable requires a number of scalar values to
    /// represent it; these are floats, except in the case of strings.
    int scalarSize() const
    {
        return sizeForType(type)*arraySize;
    }
};


// Comparison operators for sorting VarSpecs.
inline bool operator<(const VarSpec& l, const VarSpec& r)
{
    return l.name < r.name
        && l.type < r.type
        && l.arraySize < r.arraySize;
}

inline bool operator==(const VarSpec& l, const VarSpec& r)
{
    return l.name == r.name
        && l.type == r.type
        && l.arraySize == r.arraySize;
}

inline bool operator!=(const VarSpec& l, const VarSpec& r)
{
    return !(l == r);
}

inline std::ostream& operator<<(std::ostream& out, const VarSpec& var)
{
    const char* typeNames[] = {
        "float", "point", "hpoint", "vector",
        "normal", "color", "matrix", "string"
    };
    out << typeNames[var.type] << " " << var.name;
    if(var.arraySize != 1)
        out << "[" << var.arraySize << "]";
    return out;
}

typedef std::vector<VarSpec> VarList;


namespace Stdvar {
    extern const VarSpec P;
    extern const VarSpec Cs;
    extern const VarSpec Ci;
    extern const VarSpec Os;
    extern const VarSpec Oi;
    extern const VarSpec st;
    extern const VarSpec I;
    extern const VarSpec N;

    extern const VarSpec z;
}

#endif // VARSPEC_H_INCLUDED
