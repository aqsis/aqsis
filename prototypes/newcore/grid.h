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

#ifndef GRID_H_INCLUDED
#define GRID_H_INCLUDED

#include "gridstorage.h"
#include "util.h"

enum GridType
{
    GridType_Quad,
    GridType_QuadSimple
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
        virtual void calculateNormals(DataView<Vec3> N,
                                      ConstDataView<Vec3> P) const = 0;

        /// Project positions into raster space.
        virtual void project(const Mat4& toRaster) = 0;

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

        virtual void calculateNormals(DataView<Vec3> N,
                                      ConstDataView<Vec3> P) const
        {
            for(int v = 0; v < m_nv; ++v)
                for(int u = 0; u < m_nu; ++u, ++P, ++N)
                    *N = diff(P, u, m_nu) % diff(slice(P, m_nu), v, m_nv);
        }

        int nu() const { return m_nu; }
        int nv() const { return m_nv; }

        Iterator begin() const;

        virtual void project(const Mat4& m)
        {
            DataView<Vec3> P = m_storage->P();
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

#endif // GRID_H_INCLUDED
