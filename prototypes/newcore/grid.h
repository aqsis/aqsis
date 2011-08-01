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

#ifndef AQSIS_GRID_H_INCLUDED
#define AQSIS_GRID_H_INCLUDED

#include "gridstorage.h"
#include "refcount.h"
#include "util.h"

namespace Aqsis {

enum GridType
{
    GridType_Quad
};

class Grid : public RefCounted
{
    private:
        GridType m_type;
    public:
        Grid(GridType type) : m_type(type) {}

        /// Return the grid type.
        GridType type() const { return m_type; }

        /// Get a ref to the grid storage.
        virtual GridStorage& storage() = 0;

        /// Calculate normal values from P & store in N.
        virtual void calculateNormals(DataView<V3f> N,
                                      ConstDataView<V3f> P) const = 0;

        /// Project positions into raster space.
        virtual void project(const M44f& toRaster) = 0;

        /// Cache bounds for the micropolygons comprising the grid.
        virtual void cacheBounds(std::vector<Box3f>& bounds) = 0;

        /// Compute the bounding box for the grid
        virtual Box3f bound() const = 0;

        virtual ~Grid() {}
};

typedef boost::intrusive_ptr<Grid> GridPtr;


//------------------------------------------------------------------------------
/// A 2D grid of quadrilateral micro polygons
class QuadGrid : public Grid
{
    private:
        int m_nu;
        int m_nv;
        GridStoragePtr m_storage;

    public:
        class Iterator;

        QuadGrid(int nu, int nv, const GridStoragePtr& storage)
            : Grid(GridType_Quad),
            m_nu(nu),
            m_nv(nv),
            m_storage(storage)
        { }

        virtual GridStorage& storage() { return *m_storage; }
        virtual const GridStorage& storage() const { return *m_storage; }

        virtual void calculateNormals(DataView<V3f> N,
                                      ConstDataView<V3f> P) const
        {
            for(int v = 0; v < m_nv; ++v)
                for(int u = 0; u < m_nu; ++u, ++P, ++N)
                    *N = diff(P, u, m_nu) % diff(slice(P, m_nu), v, m_nv);
        }

        int nu() const { return m_nu; }
        int nv() const { return m_nv; }

        Iterator begin() const;

        virtual void project(const M44f& m)
        {
            DataView<V3f> P = m_storage->P();
            for(int i = 0, iend = m_nu*m_nv; i < iend; ++i)
            {
                // Project all points, but restore z afterward.  TODO: This
                // can be done a little more efficiently.
                //
                // TODO: This is rather specialized; maybe it shouldn't go in
                // the Grid class at all?  How about allowing visitor functors
                // which act on all the primvars held on a grid?
                float z = P[i].z;
                P[i] = P[i]*m;
                P[i].z = z;
            }
        }

        virtual void cacheBounds(std::vector<Box3f>& bounds)
        {
            DataView<V3f> P = m_storage->P();
            bounds.resize((m_nv-1)*(m_nu-1));
            for(int v = 0; v < m_nv-1; ++v)
            for(int u = 0; u < m_nu-1; ++u)
            {
                Box3f& b = bounds[v*(m_nu-1) + u];
                b.extendBy(P[v*m_nu + u]);
                b.extendBy(P[(v+1)*m_nu + u]);
                b.extendBy(P[v*m_nu + u+1]);
                b.extendBy(P[(v+1)*m_nu + u+1]);
            }
        }

        virtual Box3f bound() const
        {
            Box3f bound;
            ConstDataView<V3f> P = m_storage->P();
            for(int i = 0, iend = m_nu*m_nv; i < iend; ++i)
                bound.extendBy(P[i]);
            return bound;
        }
};


struct MicroQuadInd
{
    int a,b,c,d;

    MicroQuadInd(int a, int b, int c, int d) : a(a), b(b), c(c), d(d) {}
};


class QuadGrid::Iterator
{
    public:
        Iterator(const QuadGrid& grid)
            : m_grid(&grid),
            m_u(0),
            m_v(0),
            m_uEnd(grid.nu()-1),
            m_vEnd(grid.nv()-1)
            // go up to one less than the end in u & v.
        { }

        bool valid() { return m_v < m_vEnd; }

        Iterator& operator++()
        {
//            if(m_v == 0) ++m_u;
//            if(m_u >= m_uEnd || m_v > 0) { m_u = 0; ++m_v;}
//            return *this;
            ++m_u;
            if(m_u >= m_uEnd)
            {
                m_u = 0;
                ++m_v;
            }
            return *this;
        }

        MicroQuadInd operator*() const
        {
            int nu = m_grid->nu();
            return MicroQuadInd(nu*m_v + m_u,       nu*m_v + m_u+1,
                                nu*(m_v+1) + m_u+1, nu*(m_v+1) + m_u);
        }

        int u() const { return m_u; }
        int v() const { return m_v; }
        const QuadGrid& grid() const { return *m_grid; }

    private:
        const QuadGrid* m_grid;
        int m_u;
        int m_v;
        int m_uEnd;
        int m_vEnd;
};


//==============================================================================

inline QuadGrid::Iterator QuadGrid::begin() const
{
    return Iterator(*this);
}


} // namespace Aqsis

#endif // AQSIS_GRID_H_INCLUDED
