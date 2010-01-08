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

#ifndef GRIDVAR_H_INCLUDED
#define GRIDVAR_H_INCLUDED

#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>

#include "arrayview.h"
#include "primvar.h"
#include "util.h"
#include "ustring.h"
#include "varspec.h"

inline VarSpec::Type pvarToGvarType(PrimvarSpec::Type type)
{
    switch(type)
    {
        // Following are the same.
        case PrimvarSpec::Float:  return VarSpec::Float;
        case PrimvarSpec::Point:  return VarSpec::Point;
        case PrimvarSpec::Vector: return VarSpec::Vector;
        case PrimvarSpec::Normal: return VarSpec::Normal;
        case PrimvarSpec::Color:  return VarSpec::Color;
        case PrimvarSpec::Matrix: return VarSpec::Matrix;
        case PrimvarSpec::String: return VarSpec::String;
        // Following are different
        case PrimvarSpec::Hpoint: return VarSpec::Point;
    }
}


struct GridvarSpec : public VarSpec
{
    bool uniform; ///< True if the variable is constant over the grid

    GridvarSpec(bool uniform, Type type, int arraySize, ustring name)
        : VarSpec(type, arraySize, name), uniform(uniform) {}
    GridvarSpec(const PrimvarSpec& spec) { *this = spec; }

    /// Get number of scalars for the variable on a grid with nVerts vertices.
    int storageSize(int nVerts) const
    {
        return (uniform ? 1 : nVerts) * sizeForType(type);
    }

    /// Convert primvar spec to gridvar
    GridvarSpec& operator=(const PrimvarSpec& var)
    {
        assert(var.type != PrimvarSpec::Hpoint); // no hpoints yet
        type = pvarToGvarType(var.type);
        arraySize = var.arraySize;
        name = var.name;
        uniform = var.iclass == PrimvarSpec::Constant ||
                  var.iclass == PrimvarSpec::Uniform;
        return *this;
    }
};

class GridvarList
{
    private:
        std::vector<GridvarSpec> m_vars;
        StdVarIndices m_stdIndices;

    public:
        GridvarList(const PrimvarList& primVars)
            : m_vars(),
            m_stdIndices(primVars.stdIndices())
        {
            int nvars = primVars.size();
            m_vars.reserve(nvars);
            for(int i = 0; i < nvars; ++i)
            {
                const PrimvarSpec& var = primVars[i];
                m_vars.push_back(
                    GridvarSpec(var.iclass == PrimvarSpec::Constant ||
                                var.iclass == PrimvarSpec::Uniform,
                                pvarToGvarType(var.type), var.arraySize,
                                var.name) );
            }
        }

        int add(const GridvarSpec& var)
        {
            int index = m_vars.size();
            m_stdIndices.add(index, var);
            m_vars.push_back(var);
            return index;
        }

        int size() const { return m_vars.size(); }

        int maxAggregateSize() const
        {
            int maxSize = 0;
            for(int i = 0, nvars = m_vars.size(); i < nvars; ++i)
            {
                int currSize = m_vars[i].scalarSize();
                if(maxSize < currSize)
                    maxSize = currSize;
            }
            return maxSize;
        }

        const GridvarSpec& operator[](int i) const
        {
            assert(i >= 0 && i < (int)m_vars.size());
            return m_vars[i];
        }

        const StdVarIndices& stdIndices() const { return m_stdIndices; }
};

class GridvarStorage
{
    private:
        boost::scoped_array<float> m_storage;
        boost::scoped_array<FvecView> m_views;
        boost::shared_ptr<GridvarList> m_vars;

        void initStorage(int nverts)
        {
            // Compute offsets for each variable into the shared storage, and
            // compute the total size of the storage array.
            const int nvars = m_vars->size();
            // Allocate storage for all grid vars.
            int totStorage = 0;
            for(int i = 0; i < nvars; ++i)
                totStorage += (*m_vars)[i].storageSize(nverts);
            m_storage.reset(new float[totStorage]);
            // Cache views for individual vars.
            m_views.reset(new FvecView[nvars]);
            int offset = 0;
            for(int i = 0; i < nvars; ++i)
            {
                const int nfloats = (*m_vars)[i].scalarSize();
                m_views[i] = FvecView(&m_storage[0] + offset, nfloats);
                offset += (*m_vars)[i].storageSize(nverts);
            }
        }

    public:
        GridvarStorage(boost::shared_ptr<GridvarList> vars, int nverts)
            : m_storage(),
            m_views(),
            m_vars(vars)
        {
            initStorage(nverts);
        }

        const GridvarList& varList() const { return *m_vars; }

        /// Get allocated storage for the ith variable
        FvecView get(int i) { return m_views[i]; }
        ConstFvecView get(int i) const { return m_views[i]; }

        DataView<Vec3> P()
        {
            int Pidx = m_vars->stdIndices().P;
            assert(Pidx >= 0);
            return DataView<Vec3>(m_views[Pidx].base(),
                                  m_views[Pidx].stride());
        }
        ConstDataView<Vec3> P() const
        {
            int Pidx = m_vars->stdIndices().P;
            assert(Pidx >= 0);
            return ConstDataView<Vec3>(m_views[Pidx].base(),
                                       m_views[Pidx].stride());
        }
};

#endif // GRIDVAR_H_INCLUDED
