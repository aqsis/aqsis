#ifndef SIMPLE_H_INCLUDED
#define SIMPLE_H_INCLUDED

#include "geometry.h"
#include "options.h"
#include "util.h"

//------------------------------------------------------------------------------

/// Quadrilateral micropolygon sampler
///
/// This is designed to be constructed just before sampling time; it's not
/// memory efficient, so should not be a long-lived data structure.
class MicroQuadSimple
{
    private:
        // Vertex positions
        Vec3 m_a;
        Vec3 m_b;
        Vec3 m_c;
        Vec3 m_d;
        // Point-in-polygon tests
        PointInQuad m_hitTest;

        // Shading interpolation
        InvBilin m_invBilin;
        // uv coordinates of current interpolation point
        Vec2 m_uv;
        // Whether to use smooth shading or not.
        bool m_smoothShading;

        // Which point-on-edge to use in edge tests
        bool m_flipEnd;

    public:
        // Cyclic vertex order:
        // a -- b
        // |    |
        // d -- c
        MicroQuadSimple(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d,
                bool flipEnd)
            : m_flipEnd(flipEnd)
        {
            m_a = a;
            m_b = b;
            m_c = c;
            m_d = d;
        }

        Box bound() const
        {
            Box bnd(m_a);
            bnd.extendBy(m_b);
            bnd.extendBy(m_c);
            bnd.extendBy(m_d);
            return bnd;
        }

        float area() const
        {
            return 0.5*(
                std::fabs(cross(vec2_cast(m_b) - vec2_cast(m_a),
                                vec2_cast(m_d) - vec2_cast(m_a)))
              + std::fabs(cross(vec2_cast(m_b) - vec2_cast(m_c),
                                vec2_cast(m_d) - vec2_cast(m_c))) );
        }

        // Initialize the hit test
        inline void initHitTest()
        {
            m_hitTest.init(vec2_cast(m_a), vec2_cast(m_b),
                           vec2_cast(m_c), vec2_cast(m_d), m_flipEnd);
        }
        // Returns true if the sample is contained in the polygon
        inline bool contains(const Sample& samp)
        {
            return m_hitTest(samp);
        }

        // Initialize the shading interpolator
        inline void initInterpolator(const Options& opts)
        {
            m_smoothShading = opts.smoothShading;
            if(m_smoothShading)
            {
                m_invBilin.init(vec2_cast(m_a), vec2_cast(m_b),
                                vec2_cast(m_d), vec2_cast(m_c));
            }
        }

        inline void interpolateAt(const Sample& samp)
        {
            if(m_smoothShading)
                m_uv = m_invBilin(samp.p);
        }

        inline float interpolateZ() const
        {
            if(m_smoothShading)
                return bilerp(m_a.z, m_b.z, m_d.z, m_c.z, m_uv);
            else
                return m_a.z;
        }

        inline void interpolateColor(float* col) const
        {
            // We can't support this with the simple grid type!
            col[0] = interpolateZ();
            col[1] = 0;
            col[2] = 0;
        }

        friend std::ostream& operator<<(std::ostream& out,
                                        const MicroQuadSimple& q)
        {
            out << "{" << q.m_a << "--" << q.m_b << " | "
                << q.m_d << "--" << q.m_c << "}";
            return out;
        }
};


//------------------------------------------------------------------------------
/// A 2D grid of quadrilateral micro polygons
class QuadGridSimple : public Grid
{
    public:
        class Iterator;

        typedef MicroQuadSimple UPoly;

        QuadGridSimple(int nu, int nv)
            : m_nu(nu),
            m_nv(nv),
            m_P(nu*nv)
        { }

        virtual GridType type() { return GridType_QuadSimple; }

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


class QuadGridSimple::Iterator
{
    public:
        Iterator(const QuadGridSimple& grid)
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

        MicroQuadSimple operator*() const
        {
            int nu = m_grid->nu();
            return MicroQuadSimple(m_grid->m_P[nu*m_v + m_u],
                             m_grid->m_P[nu*m_v + m_u+1],
                             m_grid->m_P[nu*(m_v+1) + m_u+1],
                             m_grid->m_P[nu*(m_v+1) + m_u],
                             (m_u+m_v)%2);
        }

        int u() const { return m_u; }
        int v() const { return m_v; }

    private:
        const QuadGridSimple* m_grid;
        int m_u;
        int m_v;
        int m_uEnd;
        int m_vEnd;
};



//------------------------------------------------------------------------------
class PatchSimple : public Geometry
{
    private:
        Vec3 m_P[4];
        const Options& m_opts;

    public:

        PatchSimple(const Options& opts, Vec3 a, Vec3 b, Vec3 c, Vec3 d)
            : m_opts(opts)
        {
            m_P[0] = a; m_P[1] = b;
            m_P[2] = c; m_P[3] = d;
        }

        virtual void splitdice(const Mat4& splitTrans, RenderQueue& queue) const
        {
            // Project points into "splitting coordinates"
            Vec3 a = m_P[0] * splitTrans;
            Vec3 b = m_P[1] * splitTrans;
            Vec3 c = m_P[2] * splitTrans;
            Vec3 d = m_P[3] * splitTrans;

            // Diceable test: Estimate area as the sum of the areas of two
            // triangles which make up the patch.
            float area = 0.5 * (  ((b-a)%(c-a)).length()
                                + ((b-d)%(c-d)).length() );

            const float maxArea = m_opts.gridSize*m_opts.gridSize
                                  * m_opts.shadingRate;

            // estimate length in a-b, c-d direction
            float lu = 0.5*((b-a).length() + (d-c).length());
            // estimate length in a-c, b-d direction
            float lv = 0.5*((c-a).length() + (d-b).length());

            if(area <= maxArea)
            {
                // When the area (in number of micropolys) is small enough,
                // dice the surface.
                int uRes = 1 + Imath::floor(lu/std::sqrt(m_opts.shadingRate));
                int vRes = 1 + Imath::floor(lv/std::sqrt(m_opts.shadingRate));
                boost::shared_ptr<QuadGridSimple> grid(new QuadGridSimple(uRes, vRes));
                float dv = 1.0f/(vRes-1);
                float du = 1.0f/(uRes-1);
                for(int v = 0; v < vRes; ++v)
                {
                    Vec3 Pmin = Imath::lerp(m_P[0], m_P[2], v*dv);
                    Vec3 Pmax = Imath::lerp(m_P[1], m_P[3], v*dv);
                    Vec3* row = grid->P(v);
                    for(int u = 0; u < uRes; ++u)
                        row[u] = Imath::lerp(Pmin, Pmax, u*du);
                }
                queue.push(grid);
            }
            else
            {
                // Otherwise, split the surface.  The splitting direction is
                // the shortest edge.

                // Split
                if(lu > lv)
                {
                    // split in the middle of the a-b and c-d sides.
                    // a---b
                    // | | |
                    // c---d
                    Vec3 ab = 0.5f*(m_P[0] + m_P[1]);
                    Vec3 cd = 0.5f*(m_P[2] + m_P[3]);
                    queue.push(boost::shared_ptr<Geometry>(new PatchSimple(m_opts,
                                    m_P[0], ab, m_P[2], cd)));
                    queue.push(boost::shared_ptr<Geometry>(new PatchSimple(m_opts,
                                    ab, m_P[1], cd, m_P[3])));
                }
                else
                {
                    // split in the middle of the a-c and b-d sides.
                    // a---b
                    // |---|
                    // c---d
                    Vec3 ac = 0.5f*(m_P[0] + m_P[2]);
                    Vec3 bd = 0.5f*(m_P[1] + m_P[3]);
                    queue.push(boost::shared_ptr<Geometry>(new PatchSimple(m_opts,
                                    m_P[0], m_P[1], ac, bd)));
                    queue.push(boost::shared_ptr<Geometry>(new PatchSimple(m_opts,
                                    ac, bd, m_P[2], m_P[3])));
                }
            }
        }

        virtual void transform(const Mat4& trans)
        {
            m_P[0] *= trans;
            m_P[1] *= trans;
            m_P[2] *= trans;
            m_P[3] *= trans;
        }

        virtual Box bound() const
        {
            Box b(m_P[0]);
            b.extendBy(m_P[1]);
            b.extendBy(m_P[2]);
            b.extendBy(m_P[3]);
            return b;
        }
};


inline QuadGridSimple::Iterator QuadGridSimple::begin()
{
    return Iterator(*this);
}

#endif // SIMPLE_H_INCLUDED
