#ifndef VARSPEC_H_INCLUDED
#define VARSPEC_H_INCLUDED

#include <vector>

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

    friend bool operator<(const VarSpec& l, const VarSpec& r)
    {
        return l.name < r.name
            && l.type < r.type
            && l.arraySize < r.arraySize;
    }

    friend bool operator==(const VarSpec& l, const VarSpec& r)
    {
        return l.name == r.name
            && l.type == r.type
            && l.arraySize == r.arraySize;
    }
    friend bool operator!=(const VarSpec& l, const VarSpec& r)
    {
        return !(l == r);
    }
};


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
