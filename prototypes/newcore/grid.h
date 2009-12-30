#ifndef GRID_H_INCLUDED
#define GRID_H_INCLUDED

#include "gridvar.h"
#include "util.h"

enum GridType
{
    GridType_Quad,
    GridType_QuadSimple
};

class Grid
{
    public:
        /// Return the grid type.
        virtual GridType type() = 0;

        virtual ~Grid() {}
};


//------------------------------------------------------------------------------
/// A 2D grid of quadrilateral micro polygons
class QuadGrid : public Grid
{
    private:
        int m_nu;
        int m_nv;
        GridvarStorage m_storage;

    public:
        class Iterator;

        QuadGrid(int nu, int nv, boost::shared_ptr<GridvarList> vars)
            : m_nu(nu),
            m_nv(nv),
            m_storage(vars, nu*nv)
        { }

        virtual GridType type() { return GridType_Quad; }

        int nu() const { return m_nu; }
        int nv() const { return m_nv; }

        Iterator begin() const;

        GridvarStorage& storage() { return m_storage; }
        const GridvarStorage& storage() const { return m_storage; }

        void project(Mat4 m)
        {
            DataView<Vec3> P = m_storage.P();
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
    int a;
    int b;
    int c;
    int d;

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
