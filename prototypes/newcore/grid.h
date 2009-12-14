#ifndef GRID_H_INCLUDED
#define GRID_H_INCLUDED

#include <vector>
#include "util.h"

#include "microquad.h"

enum GridType
{
    GridType_Quad
};

class Grid
{
    public:
        /// Return the grid type.
        virtual GridType type() = 0;

        virtual ~Grid() {}
};

/// A 2D grid of quadrilateral micro polygons
class QuadGrid : public Grid
{
    public:
        class Iterator;

        typedef MicroQuad UPoly;

        QuadGrid(int nu, int nv)
            : m_nu(nu),
            m_nv(nv),
            m_P(nu*nv)
        { }

        virtual GridType type() { return GridType_Quad; }

        Vec3& vertex(int u, int v) { return m_P[m_nu*v + u]; }
        Vec3 vertex(int u, int v) const { return m_P[m_nu*v + u]; }

        int nu() const { return m_nu; }
        int nv() const { return m_nv; }

        Iterator begin();

        Vec3* P(int v) { assert(v >= 0 && v < m_nv); return &m_P[m_nu*v]; }

        void project(Mat4 m)
        {
            for(int i = 0, iend = m_P.size(); i < iend; ++i)
            {
                // Project all points, but restore z afterward.  TODO: This
                // can be done a little more efficiently.
                //
                // TODO: This is rather specialized; maybe it shouldn't go in
                // the Grid class at all?  How about allowing visitor functors
                // which act on all the primvars held on a grid?
                float z = m_P[i].z;
                m_P[i] = m_P[i]*m;
                m_P[i].z = z;
            }
        }

    private:
        int m_nu;
        int m_nv;
        std::vector<Vec3> m_P;
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

        MicroQuad operator*() const
        {
            int nu = m_grid->nu();
            return MicroQuad(m_grid->m_P[nu*m_v + m_u],
                             m_grid->m_P[nu*m_v + m_u+1],
                             m_grid->m_P[nu*(m_v+1) + m_u+1],
                             m_grid->m_P[nu*(m_v+1) + m_u],
                             (m_u+m_v)%2);
        }

        int u() const { return m_u; }
        int v() const { return m_v; }

    private:
        const QuadGrid* m_grid;
        int m_u;
        int m_v;
        int m_uEnd;
        int m_vEnd;
};


//==============================================================================

inline QuadGrid::Iterator QuadGrid::begin()
{
    return Iterator(*this);
}

#endif // GRID_H_INCLUDED
