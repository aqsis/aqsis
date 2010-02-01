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
#include <boost/noncopyable.hpp>

#include "arrayview.h"
#include "primvar.h"
#include "ustring.h"
#include "varspec.h"


class GridStorage
{
    private:
        boost::scoped_array<float> m_storage;
        boost::scoped_array<FvecView> m_views;
        VarSet m_vars;

        friend class GridStorageBuilder;

        /// Allocate grid storage for a sorted set of grid vars.
        ///
        /// The constructor is private so that only GridStorageBuilder can
        /// create one of these.
        ///
        /// GinitvarIterT should point to a type which is assignable to
        /// VarSpec and has a uniform member determining whether the
        /// associated storage is to be allocated as uniform or varying.
        template<typename GinitvarIterT>
        GridStorage(GinitvarIterT varBegin, GinitvarIterT varEnd, int nverts)
            : m_storage(),
            m_views(),
            m_vars(varBegin, varEnd)
        {
            // Compute offsets for each variable into the shared storage, and
            // compute the total size of the storage array.
            const int nvars = m_vars.size();
            // Allocate storage for all grid vars.
            int totStorage = 0;
            for(GinitvarIterT i = varBegin; i != varEnd; ++i)
                totStorage += i->storageSize(nverts);
            m_storage.reset(new float[totStorage]);
            // Cache views for individual vars.
            m_views.reset(new FvecView[nvars]);
            int offset = 0;
            GinitvarIterT var = varBegin;
            for(int i = 0; i < nvars; ++i, ++var)
            {
                const int len = var->scalarSize();
                const int stride = var->uniform ? 0 : len;
                m_views[i] = FvecView(&m_storage[0] + offset, stride, len);
                offset += var->storageSize(nverts);
            }
        }

    public:

        const VarSet& varSet() const { return m_vars; }

        /// Get allocated storage for the ith variable
        FvecView get(int i) { return m_views[i]; }
        ConstFvecView get(int i) const { return m_views[i]; }

        /// Get the storage associated with a VarSpec symbol
        ///
        /// \todo Can we avoid this function?  It seems a pity to be using
        /// find() unless really necessary...
        FvecView get(const VarSpec& spec)
        {
            int i = m_vars.find(spec);
            if(i == VarSet::npos)
                return FvecView();
            else
                return get(i);
        }
        ConstFvecView get(const VarSpec& spec) const
        {
            int i = m_vars.find(spec);
            if(i == VarSet::npos)
                return ConstFvecView();
            else
                return get(i);
        }

        int maxAggregateSize() const
        {
            int maxSize = 0;
            for(int i = 0, nvars = m_vars.size(); i < nvars; ++i)
            {
                int currSize = m_views[i].elSize();
                if(maxSize < currSize)
                    maxSize = currSize;
            }
            return maxSize;
        }

        /// Get storage for standard position variable
        DataView<Vec3> P()
        {
            int Pidx = m_vars.stdIndices().P;
            assert(Pidx >= 0);
            return DataView<Vec3>(m_views[Pidx].storage(),
                                  m_views[Pidx].stride());
        }
        ConstDataView<Vec3> P() const
        {
            int Pidx = m_vars.stdIndices().P;
            assert(Pidx >= 0);
            return ConstDataView<Vec3>(m_views[Pidx].storage(),
                                       m_views[Pidx].stride());
        }
};


class GridStorageBuilder : boost::noncopyable
{
    private:
        struct GvarInitSpec : public VarSpec
        {
            bool uniform;   // true if the var should be constant over the grid
            int precedence; // precedence for resolving duplicates
            GvarInitSpec(const VarSpec& spec, bool uniform, int precedence)
                : VarSpec(spec),
                uniform(uniform),
                precedence(precedence)
            {}
            GvarInitSpec(const PrimvarSpec& spec, int precedence)
                : VarSpec(spec),
                uniform(spec.iclass == PrimvarSpec::Constant ||
                        spec.iclass == PrimvarSpec::Uniform),
                precedence(precedence)
            {
                // No hpoint supported yet!
                assert(spec.type != VarSpec::Hpoint);
            }
            /// Get number of scalars for the variable on a grid with nVerts
            /// vertices.
            int storageSize(int nverts) const
            {
                return (uniform ? 1 : nverts) * scalarSize();
            }
        };
        std::vector<GvarInitSpec> m_vars;
        bool m_fromGeom;

    public:
        GridStorageBuilder()
            : m_vars(),
            m_fromGeom(false)
        {}

        /// Clear variables & reset builder.  Should NOT be called by geometry!
        void clear()
        {
            m_vars.clear();
            m_fromGeom = false;
        }

        /// Set  the precedence for incoming variable specs.
        void setFromGeom() { m_fromGeom = true; }

        void add(const VarSpec& spec, bool uniform)
        {
            m_vars.push_back(GvarInitSpec(spec, uniform, m_fromGeom));
        }
        void add(const PrimvarSpec& spec)
        {
            m_vars.push_back(GvarInitSpec(spec, m_fromGeom));
        }
        void add(const PrimvarList& primVars)
        {
            for(int i = 0, nvars = primVars.size(); i < nvars; ++i)
                add(primVars[i]);
        }

        boost::shared_ptr<GridStorage> build(int nverts)
        {
            std::sort(m_vars.begin(), m_vars.end());
            // Uniqueify the vars.  Any duplicates coming from the geometry
            // should override those coming from the renderer via the
            // precedence.  Note that uniqueness is in the sense of operator==
            // for VarSpec's.
            //
            // The intention of this is to give the geometry precedence in
            // determining the uniform/varying nature of the grid data.
            int nvars = m_vars.size();
            int in = 1;
            while(in < nvars)
            {
                if(m_vars[in-1] == m_vars[in])
                    break;
                ++in;
            }
            if(in < nvars)
            {
                // Remove non-unique vars
                int out = in-1;
                while(in < nvars)
                {
                    if(m_vars[in] != m_vars[out])
                        m_vars[++out] = m_vars[in];
                    else if(m_vars[in].precedence > m_vars[out].precedence)
                        m_vars[out] = m_vars[in];
                    ++in;
                }
                m_vars.erase(m_vars.begin() + out + 1, m_vars.end());
            }
            // Ok, now that we have all our vars, it's possible to create the
            // storage space.
            return boost::shared_ptr<GridStorage>(
                    new GridStorage(m_vars.begin(), m_vars.end(), nverts));
        }
};

#endif // GRIDVAR_H_INCLUDED
