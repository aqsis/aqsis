#include <vector>
#include "util.h"

#include "microquad.h"

/// A 2D grid of quadrilateral micro polygons
class Grid
{
    public:
        class Iterator;

        typedef PointInQuad HitTest;
        typedef MicroQuad UPoly;

        Grid(int nu, int nv)
            : m_nu(nu),
            m_nv(nv),
            m_P(nu*nv)
        { }

        Vec3& vertex(int u, int v) { return m_P[m_nu*v + u]; }
        Vec3 vertex(int u, int v) const { return m_P[m_nu*v + u]; }

        int nu() const { return m_nu; }
        int nv() const { return m_nv; }

        Iterator begin() { return Iterator(*this); }

        std::vector<Vec3>& P() { return m_P; }

    private:
        int m_nu;
        int m_nv;
        std::vector<Vec3> m_P;
};


class Grid::Iterator
{
    public:
        Iterator(const Grid& grid)
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
            ++m_u;
            if(m_u > m_uEnd)
            {
                m_u = 0;
                ++m_v;
            }
            return *this;
        }

        MicroQuad operator*() const
        {
            return MicroQuad(m_grid.m_P[m_uEnd*m_v + m_u],
                             m_grid.m_P[m_uEnd*m_v + m_u+1],
                             m_grid.m_P[m_uEnd*(m_v+1) + m_u+1],
                             m_grid.m_P[m_uEnd*(m_v+1) + m_u]);
        }

    private:
        const Grid* m_grid;
        int m_u;
        int m_v;
        int m_uEnd;
        int m_vEnd;
};


#if 0
// Visitor-like pattern for resolving grid types
class Grid
{
    public:
        virtual void rasterize(Renderer& renderer) const = 0;
};

class QuadGrid : Grid
{
    public:
        virtual void rasterize(Renderer& renderer) const
        {
            renderer.rasterize(*this);
        }
};
#endif
