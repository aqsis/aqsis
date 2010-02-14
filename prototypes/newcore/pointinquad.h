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

#ifndef POINTINQUAD_H_INCLUDED
#define POINTINQUAD_H_INCLUDED

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
        /// Do-nothing constructor.  Use init() to make the state valid.
        PointInQuad()
        {
            // Initialize m_nx etc. here to avoid bogus gcc compiler warnings.
            //
            // Note: this seems to have a small but measurable performance
            // impact if a PointInQuad object is initialized once per poly.
            // Initializing once per grid avoids this problem.
            m_nx[4] = m_ny[4] = m_px[4] = m_py[4] = 0;
            m_nx[5] = m_ny[5] = m_px[5] = m_py[5] = 0;
        }

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
                // Note: The inequalities here aren't really consistent with
                // the ones above, and therefore some inter-micropolygon
                // cracking on the interior of a grid might result (todo?)
                return (   m_nx[0]*(x - m_px[0]) + m_ny[0]*(y - m_py[0]) >= 0
                        && m_nx[1]*(x - m_px[1]) + m_ny[1]*(y - m_py[1]) >= 0
                        && m_nx[2]*(x - m_px[2]) + m_ny[2]*(y - m_py[2]) >= 0 )
                    || (   m_nx[3]*(x - m_px[3]) + m_ny[3]*(y - m_py[3]) >  0
                        && m_nx[4]*(x - m_px[4]) + m_ny[4]*(y - m_py[4]) >  0
                        && m_nx[5]*(x - m_px[5]) + m_ny[5]*(y - m_py[5]) >  0 );
            }
        }
};


#endif // POINTINQUAD_H_INCLUDED
