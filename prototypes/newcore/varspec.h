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

#include <algorithm>
#include <cassert>
#include <iostream>
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
};


// Comparison operators for sorting VarSpecs.
inline bool operator<(const VarSpec& l, const VarSpec& r)
{
    if(l.name < r.name)
        return true;
    else if(l.name == r.name)
    {
        if(l.type < r.type)
            return true;
        else if(l.type == r.type && l.arraySize < r.arraySize)
            return true;
    }
    return false;
}

inline bool operator==(const VarSpec& l, const VarSpec& r)
{
    return l.name == r.name
        && l.type == r.type
        && l.arraySize == r.arraySize;
}

inline bool operator!=(const VarSpec& l, const VarSpec& r)
{
    return l.name != r.name
        || l.type != r.type
        || l.arraySize != r.arraySize;
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
    extern const VarSpec s;
    extern const VarSpec t;
    extern const VarSpec I;
    extern const VarSpec N;

    extern const VarSpec st;
    extern const VarSpec z;
}


struct StdvarIndices
{
    int P;
    int Cs;
    int Ci;
    int Os;
    int Oi;
    int s;
    int t;
    int I;
    int N;

    StdvarIndices()
        : P(-1), Cs(-1), Ci(-1), Os(-1), Oi(-1), s(-1), t(-1), I(-1), N(-1) {}

    void add(int index, const VarSpec& var)
    {
        if(var == Stdvar::P)
            P = index;
        else if(var == Stdvar::Cs)
            Cs = index;
        else if(var == Stdvar::Ci)
            Ci = index;
        else if(var == Stdvar::Os)
            Os = index;
        else if(var == Stdvar::Oi)
            Oi = index;
        else if(var == Stdvar::s)
            s = index;
        else if(var == Stdvar::t)
            t = index;
        else if(var == Stdvar::I)
            I = index;
        else if(var == Stdvar::N)
            N = index;
    }
};


class VarSet
{
    private:
        VarList m_vars;
        StdvarIndices m_stdIndices;

        /// Return true if the list of vars is sorted
        static bool isSorted(const VarList& vars)
        {
            bool sorted = true;
            for(int i = 0, iend = vars.size()-1; i < iend; ++i)
                sorted &= vars[i] < vars[i+1];
            return sorted;
        }

        /// Update the standard indices
        void updateStdIndices()
        {
            for(int i = 0, iend = m_vars.size(); i < iend; ++i)
                m_stdIndices.add(i, m_vars[i]);
        }

    public:
        typedef VarList::const_iterator const_iterator;
        static const int npos = -1;

        template<typename VarIterT>
        VarSet(VarIterT b, VarIterT e)
            : m_vars(b, e)
        {
            assert(isSorted(m_vars));
            updateStdIndices();
        }

        const StdvarIndices& stdIndices() const { return m_stdIndices; }

        /// Iterator access and indexing.
        const_iterator begin() const { return m_vars.begin(); }
        const_iterator end() const { return m_vars.end(); }
        int size() const { return m_vars.size(); }
        const VarSpec& operator[](int i) const { return m_vars[i]; }

        int find(const VarSpec& var) const
        {
            const_iterator i = std::lower_bound(m_vars.begin(),
                                                m_vars.end(), var);
            if(i != m_vars.end() && *i == var)
                return i - m_vars.begin();
            else
                return npos;
        }

        bool contains(const VarSpec& var) const
        {
            return std::binary_search(m_vars.begin(), m_vars.end(), var);
        }
};


#endif // VARSPEC_H_INCLUDED
