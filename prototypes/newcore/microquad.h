#ifndef MICROQUAD_H_INCLUDED
#define MICROQUAD_H_INCLUDED

#include <cassert>

#include "util.h"
#include "sample.h"

// Class for the point-in-quadrilateral test
//
//
class PointInQuad
{
    private:
        // Hit test coefficients
		// 
		// coefficients 0-3 are for the edges of a convex quad.  For nonconvex
		// quads, coeffs 0-2 and 3-5 hold edge tests for a pair of triangles.
        float m_xmul[6];
        float m_ymul[6];
        float m_offset[6];

		// Indicates whether the polygon is convex
		bool m_convex;

        inline void setupEdgeEq(int i, const Vec2& e, const Vec2& p)
        {
            m_offset[i] = cross(p, e);
            m_xmul[i] = e.y;
            m_ymul[i] = -e.x;
        }

		// Set up edge equations for an "arrow head" non-convex microquad:
		//    x
		//   / \
		//  /.x.\
		// x'   'x
		void setupArrowEdgeEqs(Vec2 v[4], Vec2 e[4], int signs[4])
		{
			// Find index of concave vertex
			int i = 0;
			while(i < 4)
			{
				if(!signs[i])
					break;
				++i;
			}
			// Set up edge equations for two triangles, by cutting the arrow
			// head in half down the middle.
			Vec2 emid = v[i] - v[(i+2)%4];
			setupEdgeEq(0, e[i], v[i]);
			int j = (i+1)%4;
			setupEdgeEq(1, e[j], v[j]);
			setupEdgeEq(2, emid, v[i]);
			j = (i+2)%4;
			setupEdgeEq(3, e[j], v[j]);
			j = (i+3)%4;
			setupEdgeEq(4, e[j], v[j]);
			setupEdgeEq(5, -emid, v[i]);
		}

		// Set up edge equations for a "bow tie" non-convex microquad:
		//  x--x
		//   \/
		//   /\
		//  x--x
		void setupBowtieEdgeEqns(Vec2 v[4], Vec2 e[4], int signs[4])
		{
			// Find an index i such that v[i] is > 180, but v[i+1] is < 180 deg.
			int i = 0;
			while(i < 4)
			{
				if(!signs[i] && signs[(i+1)%4])
					break;
				++i;
			}
			// Set up edge equations for two triangles, one for each side of
			// the bow tie
			int j = i;    setupEdgeEq(0, e[j], v[j]);
			j = (i+1)%4;  setupEdgeEq(1, e[j], v[j]);
			j = (i+2)%4;  setupEdgeEq(2, e[j], v[j]);
			j = (i+2)%4;  setupEdgeEq(3, -e[j], v[j]);
			j = (i+3)%4;  setupEdgeEq(4, -e[j], v[j]);
			j = i;        setupEdgeEq(5, -e[j], v[j]);
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
			int s[4] = {cross(e[3], e[0]) > 0,
			         cross(e[0], e[1]) > 0,
			         cross(e[1], e[2]) > 0,
			         cross(e[2], e[3]) > 0};

			// Classify the quad according to the number of counter-clockwise
			// convex vertices.  This tells us the convexity, orientation and
			// self-intersection of the quad.
			switch(s[0] + s[1] + s[2] + s[3])
			{
				case 0: // convex, CW: flip edges to resemble CCW case.
					e[0] = -e[0];  e[1] = -e[1];  e[2] = -e[2];  e[3] = -e[3];
					//<< intentional case fallthrough
				case 4: // convex, CCW
					m_convex = true;
					setupEdgeEq(0, e[0], a);
					setupEdgeEq(1, e[1], b);
					setupEdgeEq(2, e[2], c);
					setupEdgeEq(3, e[3], d);
					break;
				case 2: // Bow-tie, self-intersecting.
					m_convex = false;
					{
						Vec2 v[4] = {a,b,c,d};
						setupBowtieEdgeEqns(v, e, s);
					}
					break;
				case 1: // Arrow head (CW case)
					m_convex = false;
					{
						// Reorder verts & edges into CCW order.
						Vec2 eccw[4] = {-e[3], -e[2], -e[1], -e[0]};
						Vec2 vccw[4] = {a,d,c,b};
						int sccw[4] = {!s[0], !s[3], !s[2], !s[1]};
						setupArrowEdgeEqs(vccw, eccw, sccw);
					}
					break;
				case 3:
					// CCW arrow head.
					m_convex = false;
					{
						Vec2 v[4] = {a,b,c,d};
						setupArrowEdgeEqs(v, e, s);
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
				return  m_xmul[0]*x + m_ymul[0]*y <= m_offset[0]
					 && m_xmul[1]*x + m_ymul[1]*y <= m_offset[1]
					 && m_xmul[2]*x + m_ymul[2]*y <  m_offset[2]
					 && m_xmul[3]*x + m_ymul[3]*y <  m_offset[3];
			}
			else
			{
				// Use a pair of point-in-triangle tests
				return (   m_xmul[0]*x + m_ymul[0]*y <= m_offset[0]
					    && m_xmul[1]*x + m_ymul[1]*y <= m_offset[1]
					    && m_xmul[2]*x + m_ymul[2]*y <= m_offset[2])
					|| (   m_xmul[3]*x + m_ymul[3]*y <  m_offset[3]
					    && m_xmul[4]*x + m_ymul[4]*y <  m_offset[4]
					    && m_xmul[5]*x + m_ymul[5]*y <  m_offset[5]);
			}
        }
};


#endif // MICROQUAD_H_INCLUDED
