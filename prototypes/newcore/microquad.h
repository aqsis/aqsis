#ifndef MICROQUAD_H_INCLUDED
#define MICROQUAD_H_INCLUDED

#include <cassert>

#include "util.h"
#include "sample.h"


// Class for the point-in-quadrilateral test
//
// Testing is done using the edge equations; an edge equation is a linear
// function of position which is positive on one side of the line and negative
// on the other.  If a and b are endpoints of an edge, the associated edge
// equation is
//
//   cross(b-a, x-p) >= 0       [CCW ordering]
//
// for some point p on the edge.  The >= is chosen so that the condition will
// be true for points x which are inside a micropolygon with _counterclockwise_
// ordering of the vertices a,b,c,d.
//
// For numerical robustness we need to choose p to be the edge midpoint,
// p = 0.5(a+b), to try to ensure that the edge equations for adjacent
// micropolygons on either side of an edge come out exactly the same.  If we
// don't do this then samples might fall through the holes.  For this choice of
// p, the edge equation becomes particularly simple:
//
//   cross(b-a, x) >= cross(b, a)   [CCW ordering, true inside]
//
// Convex quadrilaterals simply require four edge tests, while nonconvex quads
// can be tested using a pair of point in triangle tests (three edge tests
// each).
//
class PointInQuad
{
    private:
        // Edge equation coefficients
        // 
        // coefficients 0-3 are for the edges of a convex quad.  For nonconvex
        // quads, coeffs 0-2 and 3-5 hold edge tests for a pair of triangles.
        float m_xmul[6];
        float m_ymul[6];
        float m_offset[6];

        // Indicates whether the polygon is convex
        bool m_convex;

        inline void setupEdge(int i, Vec2 a, Vec2 b)
        {
            Vec2 e = b-a;
            m_xmul[i] = -e.y;
            m_ymul[i] = e.x;
            m_offset[i] = cross(b, a);
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
            // head in half down the middle.
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
        // Cyclic vertex order:
        // a -- b
        // |    |
        // d -- c
        PointInQuad(Vec2 a, Vec2 b, Vec2 c, Vec2 d)
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
                    setupEdge(0, b, a);
                    setupEdge(1, c, b);
                    setupEdge(2, d, c);
                    setupEdge(3, a, d);
                    break;
                case 4: // convex, CCW
                    m_convex = true;
                    setupEdge(0, a, b);
                    setupEdge(1, b, c);
                    setupEdge(2, c, d);
                    setupEdge(3, d, a);
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

        // point-in-polygon test
        inline bool operator()(const Sample& samp)
        {
            float x = samp.p.x;
            float y = samp.p.y;
            if(m_convex)
            {
                return  m_xmul[0]*x + m_ymul[0]*y >= m_offset[0]
                     && m_xmul[1]*x + m_ymul[1]*y >= m_offset[1]
                     && m_xmul[2]*x + m_ymul[2]*y >  m_offset[2]
                     && m_xmul[3]*x + m_ymul[3]*y >  m_offset[3];
            }
            else
            {
                // Use a pair of point-in-triangle tests for non-convex cases.
                //
                // TODO: The inequalities here aren't really consistent with
                // the ones above, and therefore some inter-micropolygon
                // cracking on the interior of a grid result.
                return (   m_xmul[0]*x + m_ymul[0]*y >= m_offset[0]
                        && m_xmul[1]*x + m_ymul[1]*y >= m_offset[1]
                        && m_xmul[2]*x + m_ymul[2]*y >= m_offset[2])
                    || (   m_xmul[3]*x + m_ymul[3]*y >  m_offset[3]
                        && m_xmul[4]*x + m_ymul[4]*y >  m_offset[4]
                        && m_xmul[5]*x + m_ymul[5]*y >  m_offset[5]);
            }
        }
};



// Simple quadrilateral micropolygon container.
//
// This is designed to be constructed just before sampling time, and can bound
// itself or return a point-in-polygon testing functor.
class MicroQuad
{
    private:
        Vec3 m_a;
        Vec3 m_b;
        Vec3 m_c;
        Vec3 m_d;
    public:
        // Cyclic vertex order:
        // a -- b
        // |    |
        // d -- c
        MicroQuad(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d)
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

        inline PointInQuad hitTest() const
        {
            return PointInQuad(vec2_cast(m_a), vec2_cast(m_b),
                               vec2_cast(m_c), vec2_cast(m_d));
        }

        float z() const { return m_a.z; }

        friend std::ostream& operator<<(std::ostream& out,
                                        const MicroQuad& q)
        {
            out << "{" << q.m_a << "--" << q.m_b << " | "
                << q.m_d << "--" << q.m_c << "}";
            return out;
        }
};


#endif // MICROQUAD_H_INCLUDED
