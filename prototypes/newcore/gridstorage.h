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

#ifndef AQSIS_GRIDVAR_H_INCLUDED
#define AQSIS_GRIDVAR_H_INCLUDED

#include <boost/scoped_array.hpp>
#include <boost/noncopyable.hpp>

#include "arrayview.h"
#include "primvar.h"
#include "refcount.h"
#include "ustring.h"
#include "varspec.h"

namespace Aqsis {

//------------------------------------------------------------------------------
/// Storage space for variables attached to a grid.
///
/// GridStorage stores a bunch of grid variables together, and allows for fast
/// access to each variable storage based on the index within the grid.  In
/// order to allow for efficient allocation, GridStorage should be constructed
/// via the specialized GridStorageBuilder object.
class GridStorage : public RefCounted
{
    private:
        boost::scoped_array<float> m_storage;
        boost::scoped_array<FvecView> m_views;
        VarSet m_vars;
        int m_nverts;

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
            m_vars(varBegin, varEnd),
            m_nverts(nverts)
        {
            const int nvars = m_vars.size();
            // Compute the total size of the storage array & allocate
            int totStorage = 0;
            for(GinitvarIterT i = varBegin; i != varEnd; ++i)
                totStorage += i->storageSize(nverts);
            m_storage.reset(new float[totStorage]);
            // Cache views for each variable.
            m_views.reset(new FvecView[nvars]);
            int offset = 0;
            GinitvarIterT var = varBegin;
            for(int i = 0; i < nvars; ++i, ++var)
            {
                const int elSize = var->scalarSize();
                const int stride = var->uniform ? 0 : elSize;
                m_views[i] = FvecView(m_storage.get() + offset, elSize, stride);
                offset += var->storageSize(nverts);
            }
        }

    public:
        enum StorClass
        {
            Uniform,
            Varying
        };

        /// Get the set of contained variables
        const VarSet& varSet() const { return m_vars; }

        /// Return the number of shading points
        int nverts() const { return m_nverts; }

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
        FvecView get(StdIndices::Id id)
        {
            int i = m_vars.find(id);
            if(i == VarSet::npos)
                return FvecView();
            else
                return get(i);
        }

        /// Get maximum number of floats to store any variable on this grid.
        ///
        /// Eg, if the following were attached to the grid
        ///
        ///   vector   - 3 floats
        ///   color[2] - 6 floats
        ///   matrix   - 16 floats
        ///
        /// then maxAggregateSize() would return 16.  This function is needed
        /// for convenient allocation of temporary storage during dicing.
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

        /// Convenient access to storage for standard position variable
        DataView<V3f> P()
        {
            int i = m_vars.find(StdIndices::P); assert(i >= 0);
            return DataView<V3f>(m_views[i]);
        }
        ConstDataView<V3f> P() const
        {
            int i = m_vars.find(StdIndices::P); assert(i >= 0);
            return ConstDataView<V3f>(m_views[i]);
        }
        DataView<V3f> N()
        {
            int i = m_vars.find(StdIndices::N); assert(i >= 0);
            return DataView<V3f>(m_views[i]);
        }
};

typedef boost::intrusive_ptr<GridStorage> GridStoragePtr;


//------------------------------------------------------------------------------
/// Object to collect arguments necessary to build a GridStorage object.
///
/// GridStorage holds an arbitrary number of variables, and we'd like to
/// allocate storage for all of those at the same time.  GridStorageBuilder is
/// designed to collect and sort all variables before passing them to the
/// GridStorage constructor.
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
        int m_precedence;

    public:
        GridStorageBuilder()
            : m_vars(),
            m_precedence(0)
        {}

        /// Clear variables & reset builder.  Should NOT be called by geometry!
        void clear()
        {
            m_vars.clear();
            m_precedence = 0;
        }

        /// Set  the precedence for incoming variable specs.
        void setFromGeom() { m_precedence = 1; }

        /// Determine whether the index'th variable was diced by the geometry.
        bool dicedByGeom(const GridStorage& stor, StdIndices::Id id)
        {
            int index = stor.varSet().find(id);
            assert(index > 0);
            return m_vars[index].precedence > 0;
        }

        /// Add a variable to the grid
        void add(const VarSpec& spec, GridStorage::StorClass gridStorClass)
        {
            m_vars.push_back(GvarInitSpec(spec,
                        gridStorClass == GridStorage::Uniform, m_precedence));
        }
        void add(const PrimvarSpec& spec)
        {
            m_vars.push_back(GvarInitSpec(spec, m_precedence));
        }
        void add(const PrimvarSet& primVars)
        {
            for(int i = 0, nvars = primVars.size(); i < nvars; ++i)
                add(primVars[i]);
        }

        /// Build the grid storage using the current set of variables.
        GridStoragePtr build(int nverts)
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
            return GridStoragePtr(
                    new GridStorage(m_vars.begin(), m_vars.end(), nverts));
        }
};


} // namespace Aqsis
#endif // AQSIS_GRIDVAR_H_INCLUDED
