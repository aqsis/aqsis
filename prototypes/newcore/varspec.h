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

//------------------------------------------------------------------------------
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
        assert(0 && "Unknown type"); return 0;
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


namespace Stdvar
{
    // Standard environment variables for shader execution
    extern const VarSpec alpha;  ///< Fractional pixel coverage.
    extern const VarSpec Ci;     ///< Incident color.
    extern const VarSpec Cl;     ///< Light color.
    extern const VarSpec Cs;     ///< Surface color.
    extern const VarSpec dPdu;   ///< Surface derivative dP/du
    extern const VarSpec dPdv;   ///< Surface derivative dP/dv
    extern const VarSpec du;     ///< First derivative in u.
    extern const VarSpec dv;     ///< First derivative in v.
    extern const VarSpec E;      ///< Viewpoint position.
    extern const VarSpec I;      ///< Incident ray direction.
    extern const VarSpec L;      ///< Incoming light direction.
    extern const VarSpec ncomps; ///< Number of color components.
    extern const VarSpec Ng;     ///< Geometric normal.
    extern const VarSpec Ns;     ///< Normal at point being lit.
    extern const VarSpec N;      ///< Surface normal.
    extern const VarSpec Oi;     ///< Incident opacity.
    extern const VarSpec Ol;     ///< Light opacity.
    extern const VarSpec Os;     ///< Surface opacity.
    extern const VarSpec P;      ///< Point being shaded.
    extern const VarSpec Ps;     ///< Point being lit.
    extern const VarSpec s;      ///< Texture s coordinate.
    extern const VarSpec time;   ///< Frame time.
    extern const VarSpec t;      ///< Texture t coordinate.
    extern const VarSpec u;      ///< Surface u coordinate.
    extern const VarSpec v;      ///< Surface v coordinate.

    /// Standard vars used elsewhere in the pipeline.
    extern const VarSpec st;
    extern const VarSpec z;
}


//------------------------------------------------------------------------------
/// Collection of indices for the renderer standard variables.
///
/// This can be used to shortcut any variable lookup when getting the standard
/// variables on a grid.  TODO: Is it overkill to have *all* the standard
/// variables here?  Maybe we only need some of the more important ones?
class StdIndices
{
    public:
        enum Id
        {
            P,
            Cs, Ci,
            Os, Oi,
            s, t,
            u, v,
            I,
            N, Ng,
            VAR_LAST
        };

        StdIndices()
        {
            for(int i = 0; i < VAR_LAST; ++i)
                m_indices[i] = -1;
        }

        void add(int index, const VarSpec& var)
        {
            if(var == Stdvar::P)        m_indices[P]  = index;
            else if(var == Stdvar::Cs)  m_indices[Cs] = index;
            else if(var == Stdvar::Ci)  m_indices[Ci] = index;
            else if(var == Stdvar::Os)  m_indices[Os] = index;
            else if(var == Stdvar::Oi)  m_indices[Oi] = index;
            else if(var == Stdvar::s)   m_indices[s]  = index;
            else if(var == Stdvar::t)   m_indices[t]  = index;
            else if(var == Stdvar::u)   m_indices[u]  = index;
            else if(var == Stdvar::v)   m_indices[v]  = index;
            else if(var == Stdvar::I)   m_indices[I]  = index;
            else if(var == Stdvar::N)   m_indices[N]  = index;
            else if(var == Stdvar::Ng)  m_indices[Ng] = index;
        }

        int get(Id id) const { return m_indices[id]; }
        bool contains(Id id) const { return m_indices[id] != -1; }

    private:
        int m_indices[VAR_LAST];
};


//------------------------------------------------------------------------------
/// An immutable set of variables, represented as a sorted vector.
///
/// This class is designed for fast lookup and compact representation, not
/// insertion/removal (in fact, the set is immutable so variables can't be
/// inserted or removed dynamcially).
template<typename SpecT, typename StdIndT>
class BasicVarSet
{
    private:
        typedef std::vector<SpecT> VarVec;
        VarVec m_vars;
        StdIndT m_stdIndices;

        /// Return true if the list of vars is sorted
        static bool isSorted(const VarVec& vars)
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
        typedef typename VarVec::const_iterator const_iterator;
        static const int npos = -1;

        /// Make an empty varset
        BasicVarSet() : m_vars() { }

        /// Construct a set from a _sorted_ sequence of vars.
        template<typename VarIterT>
        BasicVarSet(VarIterT b, VarIterT e)
            : m_vars(b, e)
        {
            assert(isSorted(m_vars));
            updateStdIndices();
        }

        /// Assign a set of _unsorted_ variables to the set.
        template<typename VarIterT>
        void assign(VarIterT b, VarIterT e)
        {
            m_vars.assign(b, e);
            std::sort(m_vars.begin(), m_vars.end());
            updateStdIndices();
        }

        const StdIndT& stdIndices() const { return m_stdIndices; }

        /// Iterator access and indexing.
        const_iterator begin() const { return m_vars.begin(); }
        const_iterator end() const { return m_vars.end(); }
        int size() const { return m_vars.size(); }
        const SpecT& operator[](int i) const
        {
            assert(i >= 0 && i < static_cast<int>(m_vars.size()));
            return m_vars[i];
        }

        int find(const VarSpec& var) const
        {
            const_iterator i = std::lower_bound(m_vars.begin(),
                                                m_vars.end(), var);
            if(i != m_vars.end() && *i == var)
                return i - m_vars.begin();
            else
                return npos;
        }
        int find(typename StdIndT::Id id) const { return m_stdIndices.get(id); }

        bool contains(const VarSpec& var) const
        {
            return std::binary_search(m_vars.begin(), m_vars.end(), var);
        }
        bool contains(typename StdIndT::Id id) const
        {
            return m_stdIndices.contains(id);
        }
};

typedef BasicVarSet<VarSpec, StdIndices> VarSet;


//==============================================================================
// It's necessary to define VarSet::npos here to force the compiler to make
// actual storage space for npos.  Otherwise it can't be bound to a const
// reference if necessary, eg, using VarSet::npos in BOOST_CHECK_EQUAL won't
// work.  Duh!
template<typename SpecT, typename StdIndT>
const int BasicVarSet<SpecT, StdIndT>::npos;

#endif // VARSPEC_H_INCLUDED
