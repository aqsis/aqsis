// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
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


/** \file
 * \brief Forward and reverse bilinear mappings.
 * \author Paul C. Gregory (pgregory@aqsis.org)
 * \author Chris Foster
 */

//? Is .h included already?
#ifndef BILINEAR_H_INCLUDED
#define BILINEAR_H_INCLUDED 1

#include	<aqsis/aqsis.h>
#include	<aqsis/math/vector2d.h>

namespace Aqsis {


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
class CqInvBilinear
{
	public:
		/// Construct a lookup functor with zeros for the vertices.
		CqInvBilinear();

		/** Use the given vertices for inverse bilinear lookups.
		 *
		 * \see setVertices for the vertex ordering.
		 */
		CqInvBilinear(CqVector2D A, CqVector2D B, CqVector2D C, CqVector2D D);

		/** \brief Reset the micropolygon vertices.
		 *
		 * The ordering of vertices is as follows:
		 *
		 *   C---D
		 *   |   |
		 *   A---B
		 */
		void setVertices(CqVector2D A, CqVector2D B, CqVector2D C, CqVector2D D);

		/** \brief Perform the inverse bilinear mapping
		 *
		 * The mapping takes P = (x,y) and returns the original (u,v)
		 * parameters of the bilinear patch which would map to P under the
		 * usual bilinear interpolation scheme.
		 */
		CqVector2D operator()(CqVector2D P) const;

	private:
		template<bool unsafeInvert>
		static CqVector2D solve(CqVector2D M1, CqVector2D M2, CqVector2D b);

		CqVector2D bilinEval(CqVector2D uv) const;

		CqVector2D m_A;
		CqVector2D m_E;
		CqVector2D m_F;
		CqVector2D m_G;
		bool m_linear;
};


//---------------------------------------------------------------------
/** Bilinearly evalute the four specified values at the specified intervals.
 * \attention The type to be interpolated must support operator+, operator- and operator*.
 * \param A min u min v.
 * \param B max u min v.
 * \param C min u max v.
 * \param D max u max v.
 * \param s the fraction in the u direction.
 * \param t the fraction in the v direction.
 * \return the interpolated value.
 */

template <class T>
T BilinearEvaluate( const T& A, const T& B, const T& C, const T& D, TqFloat s, TqFloat t );


/** \brief Bilinear interpolation, fast version.
 *
 * Bilinear interpolation over a "quadrilateral" of values arranged in the
 * standard order as follows:
 *
 * \verbatim
 *
 *   C---D    ^
 *   |   |    | v-direction [ uv.y() ]
 *   A---B    |
 *
 *   u-direction -->
 *   [ uv.x() ]
 *
 * \endverbatim
 *
 * This version performs no checks for whether u and v are between 0 and 1 and
 * may give odd results if they're not.
 *
 * \param A,B,C,D - quadrilateral corners.
 * \param uv - Parametric (u,v) coordinates, should be in the interval [0,1]
 */
template<typename T>
inline T bilerp(T A, T B, T C, T D, CqVector2D uv);



//==============================================================================
// Implementation details.
//==============================================================================
template <class T>
inline T BilinearEvaluate( const T& A, const T& B, const T& C, const T& D, TqFloat s, TqFloat t )
{
	T AB, CD;
	// Work out where the u points are first, then linear interpolate the v value.
	if ( s <= 0.0 )
	{
		AB = A;
		CD = C;
	}
	else
	{
		if ( s >= 1.0 )
		{
			AB = B;
			CD = D;
		}
		else
		{
			AB = static_cast<T>( ( B - A ) * s + A );
			CD = static_cast<T>( ( D - C ) * s + C );
		}
	}

	T R;
	if ( t <= 0.0 )
		R = AB;
	else
	{
		if ( t >= 1.0 )
			R = CD;
		else
			R = static_cast<T>( ( CD - AB ) * t + AB );
	}

	return ( R );
}


template<typename T>
inline T bilerp(T A, T B, T C, T D, CqVector2D uv)
{
    TqFloat w0 = (1-uv.y())*(1-uv.x());
    TqFloat w1 = (1-uv.y())*uv.x();
    TqFloat w2 = uv.y()*(1-uv.x());
    TqFloat w3 = uv.y()*uv.x();
    return w0*A + w1*B + w2*C + w3*D;
}


//------------------------------------------------------------------------------
// CqInvBilinear implementation
inline CqInvBilinear::CqInvBilinear()
	: m_A(),
	m_E(),
	m_F(),
	m_G(),
	m_linear(false)
{}

inline CqInvBilinear::CqInvBilinear(CqVector2D A, CqVector2D B,
									CqVector2D C, CqVector2D D)
	: m_A(),
	m_E(),
	m_F(),
	m_G(),
	m_linear(false)
{
	setVertices(A,B,C,D);
}

inline void CqInvBilinear::setVertices(CqVector2D A, CqVector2D B,
									   CqVector2D C, CqVector2D D)
{
	m_A = A,
	m_E = B-A;
	m_F = C-A;
	m_G = -m_E-C+D;
	m_linear = false;
	// Determine whether the micropolygon is almost-rectangular.  If it is, we
	// set the m_linear flag as a hint to the solver that we only need to solve
	// linear equations (requiring only one iteration of Newton's method).
	TqFloat patchSize = max(maxNorm(m_F), maxNorm(m_E));
	TqFloat irregularity = maxNorm(m_G);
	if(irregularity < 1e-2*patchSize)
		m_linear = true;
}

inline CqVector2D CqInvBilinear::operator()(CqVector2D P) const
{
	// Start at centre of the micropoly & do one or two iterations of Newton's
	// method to solve for (u,v).
	CqVector2D uv(0.5, 0.5);
	uv -= solve<true>(m_E + m_G*uv.y(), m_F + m_G*uv.x(), bilinEval(uv)-P);
	if(!m_linear)
	{
		// The second iteration is only used if we know that the micropolygon
		// is non-rectangular.
		uv -= solve<false>(m_E + m_G*uv.y(), m_F + m_G*uv.x(), bilinEval(uv)-P);
	}
	return uv;
}

/** \brief Solve the linear equation M*x = b  with the matrix M = [M1 M2]
 *
 * The unsafeInvert parameter indicates whether we should check that an inverse
 * exists before trying to find it.  If it doesn't exist, we return the zero
 * vector.
 */
template<bool unsafeInvert>
inline CqVector2D CqInvBilinear::solve(CqVector2D M1, CqVector2D M2,
											  CqVector2D b)
{
	TqFloat det = cross(M1, M2);
	if(unsafeInvert || det != 0) det = 1/det;
	return det * CqVector2D(cross(b, M2), -cross(b, M1));
}
/// Evaluate the bilinear function at the coordinates (u,v)
inline CqVector2D CqInvBilinear::bilinEval(CqVector2D uv) const
{
	return m_A + m_E*uv.x() + m_F*uv.y() + m_G*uv.x()*uv.y();
}


} // namespace Aqsis

#endif	// !BILINEAR_H_INCLUDED
