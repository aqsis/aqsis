#ifndef MICROQUAD_H_INCLUDED
#define MICROQUAD_H_INCLUDED

#include <cassert>

#include "invbilin.h"
#include "options.h"
#include "sample.h"
#include "util.h"


/// Class for the point-in-quadrilateral test
///
/// Testing is done using the edge equations; an edge equation is a linear
/// function of position which is positive on one side of the line and negative
/// on the other.  If a and b are endpoints of an edge, the associated edge
/// equation is
///
///   cross(b-a, x-p) >= 0       [CCW ordering]
///
/// for some point p on the edge.  The >= is chosen so that the condition will
/// be true for points x which are inside a micropolygon with _counterclockwise_
/// ordering of the vertices a,b,c,d.
///
/// Convex quadrilaterals simply require four such edge tests, while nonconvex
/// quads can be tested using a pair of point in triangle tests (three edge
/// tests each).
///
/// For numerical robustness, we choose p to be equal to one of the edge
/// endpoints.  This choice is very important since it ensures that the given
/// endpoints of the line actually lie _on_ the line according to the resulting
/// edge equation.  Having this property helps avoid cracks and overlap at the
/// _corners_ of adjacent micropolygons.  Finally, it ensures that bounding
/// boxes calculated from the positions of the vertices are correct.
///
/// Unfortunately, choosing p := a is not always exactly equivalent to p := b
/// due to floating point errors, so one final adjustment is desirable to
/// prevent cracks between adjacent micropolygons.  Consider the pair of
/// micropolys:
///
///   c---b---f
///   | 1 | 2 |
///   d---a---e
///
/// With anticlockwise orientation, the vertices a,b are specified in opposite
/// order: ab for micropoly 1 and ba for micropoly 2.  For the edge equation for
/// ab to be consistent between micropolygons 1 and 2 we need to choose either a
/// or b as the point on the line, and we need to make the same choice for 1 and
/// 2.  This is the function of the "flipEnds" flag used in the setupEdge()
/// function below.
///
class PointInQuad
{
    private:
        // Edge equation coefficients.  nx,ny is the edge normal; px,py is one
        // of the edge endpoints.
        // 
        // coefficients 0-3 are for the edges of a convex quad.  For nonconvex
        // quads, coeffs 0-2 and 3-5 hold edge tests for a pair of triangles.
        float m_nx[6];
        float m_ny[6];
        float m_px[6];
        float m_py[6];

        // Indicates whether the polygon is convex
        bool m_convex;

        /// Set up the ith edge equation
        ///
        /// a and b are the edge endpoints.  If flipEnds is true, point a is
        /// used as the edge equation "point on the line"; otherwise point b is
        /// used.
        inline void setupEdge(int i, Vec2 a, Vec2 b, bool flipEnds = true)
        {
            Vec2 e = b-a;
            m_nx[i] = -e.y;
            m_ny[i] = e.x;
            // Use one of the end points of the line as the point on the line.
            // Very important, as discussed above.
            if(flipEnds)
            {
                m_px[i] = a.x;
                m_py[i] = a.y;
            }
            else
            {
                m_px[i] = b.x;
                m_py[i] = b.y;
            }
        }

        // Set up edge equations for an "arrow head" non-convex microquad
        void setupArrowEdgeEqs(Vec2 v[4], int signs[4])
        {
            // Find index, i of the concave vertex
            //      2       .
            //     / \      .
            //    /.i.\     .
            //   3'   '1    .
            int i = 0;
            while(i < 4)
            {
                if(!signs[i])
                    break;
                ++i;
            }
            // Set up edge equations for two triangles, by cutting the arrow
            // head in half down the middle.  Note, I haven't bothered with the
            // "flipEnds" behaviour here.
            int i0 = i, i1 = (i+1)%4, i2 = (i+2)%4, i3 = (i+3)%4;
            setupEdge(0, v[i0], v[i1]);
            setupEdge(1, v[i1], v[i2]);
            setupEdge(2, v[i2], v[i0]);
            setupEdge(3, v[i0], v[i2]);
            setupEdge(4, v[i2], v[i3]);
            setupEdge(5, v[i3], v[i0]);
        }

        // Set up edge equations for a "bow tie" non-convex microquad
        void setupBowtieEdgeEqns(Vec2 v[4], int signs[4])
        {
            // Find the index i such that v[i] is > 180, but v[i+1] is < 180 deg.
            int i = 0;
            while(i < 4)
            {
                if(!signs[i] && signs[(i+1)%4])
                    break;
                ++i;
            }
            // Set up edge equations for two triangles, one for each side of
            // the bow tie
            //
            //     i2--i1
            //       \/    <-- CCW triangle
            //       /\    <-- CW triangle
            //     i0--i3
            int i0 = i, i1 = (i+1)%4, i2 = (i+2)%4, i3 = (i+3)%4;
            setupEdge(0, v[i0], v[i1]);
            setupEdge(1, v[i1], v[i2]);
            setupEdge(2, v[i2], v[i3]);
            setupEdge(3, v[i0], v[i3]);
            setupEdge(4, v[i3], v[i2]);
            setupEdge(5, v[i1], v[i0]);
        }

    public:
        /// Initialize the edge equations.
        ///
        /// Uses a cyclic vertex order:
        ///
        ///   d---c
        ///   |   |
        ///   a---b
        ///
        /// If flipEnds is true, the trailing vertex of each edge will be used
        /// as the point on the edge, otherwise the leading vertex will be
        /// used.  For robustness adjacent micropolygons should have opposite
        /// flipEnds values.  (think black vs white squares in a checkerboard
        /// pattern).
        void init(Vec2 a, Vec2 b, Vec2 c, Vec2 d, bool flipEnds)
        {
            // Vectors along edges.
            Vec2 e[4] = {b-a, c-b, d-c, a-d};

            // The signs of cross products between edges indicate clockwise (-)
            // vs counter-clockwise (+) rotation at the vertex between them
            int s[4] = {cross(e[3], e[0]) > 0, cross(e[0], e[1]) > 0,
                        cross(e[1], e[2]) > 0, cross(e[2], e[3]) > 0};

            // Classify the quad according to the number of counter-clockwise
            // convex vertices.  This tells us the convexity, orientation and
            // self-intersection of the quad.
            switch(s[0] + s[1] + s[2] + s[3])
            {
                case 0: // convex, CW: flip edges to resemble CCW case.
                    m_convex = true;
                    setupEdge(0, b, a, flipEnds);
                    setupEdge(1, c, b, flipEnds);
                    setupEdge(2, d, c, flipEnds);
                    setupEdge(3, a, d, flipEnds);
                    break;
                case 4: // convex, CCW
                    m_convex = true;
                    setupEdge(0, a, b, flipEnds);
                    setupEdge(1, b, c, flipEnds);
                    setupEdge(2, c, d, flipEnds);
                    setupEdge(3, d, a, flipEnds);
                    break;
                case 2: // Bow-tie (self-intersecting).
                    m_convex = false;
                    {
                        Vec2 v[4] = {a,b,c,d};
                        setupBowtieEdgeEqns(v, s);
                    }
                    break;
                case 1: // Arrow head (CW case)
                    m_convex = false;
                    {
                        // Reorder verts & signs into CCW order.
                        Vec2 vccw[4] = {a,d,c,b};
                        int sccw[4] = {!s[0], !s[3], !s[2], !s[1]};
                        setupArrowEdgeEqs(vccw, sccw);
                    }
                    break;
                case 3:
                    // CCW arrow head.
                    m_convex = false;
                    {
                        Vec2 v[4] = {a,b,c,d};
                        setupArrowEdgeEqs(v, s);
                    }
                    break;
            }
        }

        /// Do-nothing constructor.  Use init() to make the state valid.
        PointInQuad() {}

        /// Set up edge equations from the given vertices
        /// \see init
        PointInQuad(Vec2 a, Vec2 b, Vec2 c, Vec2 d, bool flipEnds)
        {
            init(a,b,c,d, flipEnds);
        }

        /// Point-in-polygon test
        inline bool operator()(const Sample& samp)
        {
            float x = samp.p.x;
            float y = samp.p.y;
            if(m_convex)
            {
                return m_nx[0]*(x - m_px[0]) + m_ny[0]*(y - m_py[0]) >= 0
                    && m_nx[1]*(x - m_px[1]) + m_ny[1]*(y - m_py[1]) >= 0
                    && m_nx[2]*(x - m_px[2]) + m_ny[2]*(y - m_py[2]) > 0
                    && m_nx[3]*(x - m_px[3]) + m_ny[3]*(y - m_py[3]) > 0;
            }
            else
            {
                // Use a pair of point-in-triangle tests for non-convex cases.
                //
                // TODO: The inequalities here aren't really consistent with
                // the ones above, and therefore some inter-micropolygon
                // cracking on the interior of a grid might result.
                return (   m_nx[0]*(x - m_px[0]) + m_ny[0]*(y - m_py[0]) >= 0
                        && m_nx[1]*(x - m_px[1]) + m_ny[1]*(y - m_py[1]) >= 0
                        && m_nx[2]*(x - m_px[2]) + m_ny[2]*(y - m_py[2]) >= 0 )
                    || (   m_nx[3]*(x - m_px[3]) + m_ny[3]*(y - m_py[3]) >  0
                        && m_nx[4]*(x - m_px[4]) + m_ny[4]*(y - m_py[4]) >  0
                        && m_nx[5]*(x - m_px[5]) + m_ny[5]*(y - m_py[5]) >  0 );
            }
        }
};



/// Quadrilateral micropolygon sampler
///
/// This is designed to be constructed just before sampling time; it's not
/// memory efficient, so should not be a long-lived data structure.
class MicroQuad
{
    private:
        // Grid indices for vertices
        int m_a;
        int m_b;
        int m_c;
        int m_d;
        // Point-in-polygon tests
        PointInQuad m_hitTest;
        // Storage for the micropoly data
        const GridvarStorage& m_storage;
        ConstDataView<Vec3> m_P;

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
        MicroQuad(int a, int b, int c, int d, const GridvarStorage& storage,
                  bool flipEnd)
            : m_a(a), m_b(b), m_c(c), m_d(d),
            m_storage(storage),
            m_P(storage.P()),
            m_flipEnd(flipEnd)
        { }

        Box bound() const
        {
            Box bnd(m_P[m_a]);
            bnd.extendBy(m_P[m_b]);
            bnd.extendBy(m_P[m_c]);
            bnd.extendBy(m_P[m_d]);
            return bnd;
        }

//        float area() const
//        {
//            return 0.5*(
//                std::fabs(cross(vec2_cast(m_b) - vec2_cast(m_a),
//                                vec2_cast(m_d) - vec2_cast(m_a)))
//              + std::fabs(cross(vec2_cast(m_b) - vec2_cast(m_c),
//                                vec2_cast(m_d) - vec2_cast(m_c))) );
//        }

        // Initialize the hit test
        inline void initHitTest()
        {
            m_hitTest.init(vec2_cast(m_P[m_a]), vec2_cast(m_P[m_b]),
                           vec2_cast(m_P[m_c]), vec2_cast(m_P[m_d]),
                           m_flipEnd);
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
                m_invBilin.init(vec2_cast(m_P[m_a]), vec2_cast(m_P[m_b]),
                                vec2_cast(m_P[m_d]), vec2_cast(m_P[m_c]));
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
                return bilerp(m_P[m_a].z, m_P[m_b].z,
                              m_P[m_d].z, m_P[m_c].z, m_uv);
            else
                return m_P[m_a].z;
        }

        inline void interpolateColor(float* col) const
        {
            int CsIdx = m_storage.varList().stdIndices().Cs;
            assert(CsIdx >= 0);
            ConstFvecView Cs = m_storage.get(CsIdx);
            if(m_smoothShading)
            {
                for(int i = 0; i < 3; ++i)
                    col[i] = bilerp(Cs[m_a][i], Cs[m_b][i],
                                    Cs[m_d][i], Cs[m_c][i], m_uv);
            }
            else
            {
                for(int i = 0; i < 3; ++i)
                    col[i] = Cs[m_a][i];
            }
        }

        friend std::ostream& operator<<(std::ostream& out,
                                        const MicroQuad& q)
        {
            out << "{" << q.m_P[q.m_a] << "--" << q.m_P[q.m_b] << " | "
                << q.m_P[q.m_d] << "--" << q.m_P[q.m_c] << "}";
            return out;
        }
};


#endif // MICROQUAD_H_INCLUDED
