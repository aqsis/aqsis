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

#ifndef INVBILIN_H_INCLUDED
#define INVBILIN_H_INCLUDED

#include "util.h"

/** \brief Functor for inverse bilinear mapping
 *
 * Forward bilinear interpolation is extremely staightforward: Given four
 * corners of a bilinear patch, A,B,C and D, we define the bilinear surface
 * between them at parameter value (u,v) to be
 *
 * P(u,v) := (1-v)*((1-u)*A + u*B) + v*((1-u)*C + u*D)
 *
 * That is, linear interpolation in the u-direction along opposite edges,
 * followed by linear interpolation in the v-direction between the two
 * resulting points.
 *
 * So, we can get a point P easily from the (u,v) coordinates.  The problem
 * solved here is the reverse: get (u,v) from the (x,y) coordinates of P.  This
 * class assumes that the user may want to do this lookup operation many times
 * per micropolygon, so it holds a cache of coefficients for the lookup
 * function.
 *
 * Here's a schematic showing the bilinear patch ABCD, along with two important
 * edge vectors E and F used in the calculation.
 *
 * \verbatim
 *
 *                 C-----------------D
 *   v-        ^   |                /
 *   direction |   |               /
 *             |   |              /
 *          F=C-A  |   .P        /
 *             |   |            /
 *             |   |           /
 *             |   |          /
 *             |   |         /
 *                 A--------B
 *  
 *                  ------->
 *                  E = B-A
 *  
 *                u-direction ->
 *
 * \endverbatim
 *
 * The inverse of the equation for P(u,v) may be calculated analytically, but
 * there's a bunch of special cases which result in numerical instability, each
 * of which needs to be carefully coded in.  Here we use two iterations of
 * Newton's method instead to find (u,v) such that the residual function
 *
 *   f(u,v) := (1-v)*((1-u)*A + u*B) + v*((1-u)*C + u*D) - P
 *
 * is zero.  This works very reliably when ABCD is pretty much rectangular with
 * two iterations giving accuracy to three or four decimal places, and only
 * appears to converge slowly when two adacent sides are parallel.
 *
 * Tests show that the analytical solution to the equations and two iterations
 * of Newton's method are very similar in speed, so we take the latter for
 * implementation simplicity.  (See the prototypes directory for both
 * implementations.)
 *
 * NOTE: The speed of this code is pretty critical for the micropolygon
 * sampling implementation, since it's inside one of the the innermost loops.
 * The accuracy however probably isn't so much of a big deal since
 * micropolygons typically are very small on screen.
 */
class InvBilin
{
    public:
        /// Construct a lookup functor with zeros for the vertices.
        InvBilin();

        /** \brief Reset the micropolygon vertices.
         *
         * The ordering of vertices is as follows:
         *
         *   C---D
         *   |   |
         *   A---B
         */
        void init(Vec2 A, Vec2 B, Vec2 C, Vec2 D);

        /** \brief Perform the inverse bilinear mapping
         *
         * The mapping takes P = (x,y) and returns the original (u,v)
         * parameters of the bilinear patch which would map to P under the
         * usual bilinear interpolation scheme.
         */
        Vec2 operator()(Vec2 P) const;

    private:
        template<bool unsafeInvert>
        static Vec2 solve(Vec2 M1, Vec2 M2, Vec2 b);

        Vec2 bilinEval(Vec2 uv) const;

        Vec2 m_A;
        Vec2 m_E;
        Vec2 m_F;
        Vec2 m_G;
        bool m_linear;
};


//==============================================================================
// InvBilin implementation.
inline InvBilin::InvBilin()
    // Note that we initialize m_A etc. to zero here only to avoid a bogus gcc
    // compiler warning.  These should be properly initialized via the init()
    // function.
    : m_A(0),
    m_E(0),
    m_F(0),
    m_G(0),
    m_linear(false)
{}

inline void InvBilin::init(Vec2 A, Vec2 B, Vec2 C, Vec2 D)
{
    m_A = A,
    m_E = B-A;
    m_F = C-A;
    m_G = -m_E-C+D;
    m_linear = false;
    // Determine whether the micropolygon is almost-rectangular.  If it is, we
    // set the m_linear flag as a hint to the solver that we only need to solve
    // linear equations (requiring only one iteration of Newton's method).
    float patchSize = max(maxNorm(m_F), maxNorm(m_E));
    float irregularity = maxNorm(m_G);
    if(irregularity < 1e-2*patchSize)
        m_linear = true;
}

inline Vec2 InvBilin::operator()(Vec2 P) const
{
    // Start at centre of the micropoly & do one or two iterations of Newton's
    // method to solve for (u,v).
    Vec2 uv(0.5, 0.5);
    uv -= solve<true>(m_E + m_G*uv.y, m_F + m_G*uv.x, bilinEval(uv)-P);
    if(!m_linear)
    {
        // The second iteration is only used if we know that the micropolygon
        // is non-rectangular.
        uv -= solve<false>(m_E + m_G*uv.y, m_F + m_G*uv.x, bilinEval(uv)-P);
    }
    // TODO: Investigate exact solution for InvBilin again!  Newton's method
    // can fail to produce the right results sometimes, hence the clamping
    // below.
    //
    // Note that the clamping below has a measurable performance effect (a few
    // percent) when rendering z-buffers.
    if(uv.x < 0) uv.x = 0;
    if(uv.x > 1) uv.x = 1;
    if(uv.y < 0) uv.y = 0;
    if(uv.y > 1) uv.y = 1;
    return uv;
}

/** \brief Solve the linear equation M*x = b  with the matrix M = [M1 M2]
 *
 * The unsafeInvert parameter indicates whether we should check that an inverse
 * exists before trying to find it.  If it doesn't exist, we return the zero
 * vector.
 */
template<bool unsafeInvert>
inline Vec2 InvBilin::solve(Vec2 M1, Vec2 M2, Vec2 b)
{
    float det = cross(M1, M2);
    if(unsafeInvert || det != 0) det = 1/det;
    return det * Vec2(cross(b, M2), -cross(b, M1));
}

/// Evaluate the bilinear function at the coordinates (u,v)
inline Vec2 InvBilin::bilinEval(Vec2 uv) const
{
    return m_A + m_E*uv.x + m_F*uv.y + m_G*uv.x*uv.y;
}


#endif // INVBILIN_H_INCLUDED
