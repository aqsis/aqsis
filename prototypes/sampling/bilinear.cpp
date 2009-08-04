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


#include <iostream>
#include <sstream>
#include <vector>
#include <cfloat>

#include <aqsis/aqsis.h>
#include <aqsis/math/vector2d.h>


/** Prototype for inverse of bilinear transformation.
 *
 * Forward bilinear interpolation is extremely staightforward: Given four
 * corners of a bilinear patch, A,B,C and D, we define the bilinear surface
 * between them at parameter value (u,v) to be
 *
 * P(u,v) := (1-v) * ((1-u)*A + u*B)  +  v * ((1-u)*C + u*D)
 *
 * That is, linear interpolation in the u-direction along opposite edges,
 * followed by linear interpolation between the two resulting points with v as
 * the parameter.
 *
 * So, we can get a point P easily from the (u,v) coordinates.  The problem
 * solved here is the reverse: get (u,v) from the coordinates of P.  These
 * classes assume that the user wants to do this lookup operation many times
 * per micropolygon, so it holds a cache of coefficients for the lookup
 * function.
 *
 * Here's a schematic showing the bilinear patch ABCD, along with two important
 * edge vectors E and F used in the calculation.
 *
 * \verbatim
 *
 *               C-----------------D
 * v-        ^   |                /
 * direction |   |               /
 *           |   |              /
 *        E=C-A  |   .P        /
 *           |   |            /
 *           |   |           /
 *           |   |          /
 *           |   |         /
 *               A--------B
 *
 *                ------->
 *                F = B-A
 *
 *              u-direction ->
 *
 * \endverbatim
 */


using Aqsis::CqVector2D;
using Aqsis::max;
using Aqsis::clamp;
#define OUT(x) std::cout << #x << " = " << (x) << "\n";

// Used for testing how much inlining improves the speed (gcc specifc):
//#define noinline __attribute__((noinline))
#define noinline

inline TqFloat cross(const CqVector2D a, const CqVector2D b)
{
	return a.x()*b.y() - a.y()*b.x();
}

inline TqFloat maxNorm(CqVector2D v)
{
	return max(std::fabs(v.x()), std::fabs(v.y()));
}

template<typename T>
inline T bilerp(T A, T B, T C, T D, TqFloat u, TqFloat v)
{
	return (1-v)*((1-u)*A + u*B) + v*((1-u)*C + u*D);
}


//------------------------------------------------------------------------------
class CqInvBilinearDirect
{
	private:
		// Cached coefficients for lookup.
		CqVector2D m_H;
		CqVector2D m_F;
		CqVector2D m_E;
		CqVector2D m_G;
		TqFloat m_inv2GxF;
		TqFloat m_FxE;
		TqFloat m_invFxE;
		TqFloat m_FxG;
		CqVector2D m_4GxFE;

		bool m_linear;

	public:
		CqInvBilinearDirect(CqVector2D A, CqVector2D B, CqVector2D C, CqVector2D D)
			: m_H(),
			m_E(),
			m_F(),
			m_G(),
			m_inv2GxF(0),
			m_FxE(0),
			m_FxG(0),
			m_4GxFE(),
			m_linear(false)
		{
			setVertices(A, B, C, D);
		}

		void setVertices(CqVector2D A, CqVector2D B, CqVector2D C, CqVector2D D)
		{
			m_H = 0.25*(A+B+C+D);
			m_E = 0.5*(C-A + D-B);
			m_F = 0.5*(B-A + D-C);
			m_G = A-B-C+D;
			m_FxE = cross(m_F, m_E);

			m_linear = false;
			const TqFloat patchSize = max(maxNorm(m_E), maxNorm(m_F));
			const TqFloat irregularity = maxNorm(m_G);
			if(irregularity < 1e-3*patchSize)
			{
				m_FxE = 1/m_FxE;
				m_linear = true;
			}
			else
			{
				m_FxG = cross(m_F, m_G);
				m_inv2GxF = -0.5/m_FxG;
				m_4GxFE = -4*m_FxG*m_E;
			}
		}

		noinline CqVector2D operator()(CqVector2D P) const
		{
			const CqVector2D PH = P - m_H;
			TqFloat u = 0;
			TqFloat v = 0;
			if(m_linear)
			{
				// NOTE: m_FxE is actually 1/m_FxE here!
				u = -cross(m_E, PH)*m_FxE;
				v = cross(m_F, PH)*m_FxE;
			}
			else
			{
				const TqFloat t = m_FxE + cross(m_G, PH);
				const TqFloat d = sqrt(t*t + cross(m_4GxFE, PH));
				u = m_inv2GxF*(t + d);
				if(u < -0.5-FLT_EPSILON || u > 0.5+FLT_EPSILON)
					u = m_inv2GxF*(t - d);
				// Now that we have u, find v.  There's several ways to do this
				// but the one involving division here is likely the most
				// numerically stable, since it's resistent to cases where FxG
				// is small.
				v = cross(m_F, PH) / (m_FxE + m_FxG*u);
			}
			u += 0.5;
			v += 0.5;

			// We could clamp u and v here to between 0 and 1 here, but for
			// shading interpolation this is probably a waste of effort - it's
			// enough that they're 'fairly close' to the correct range.
			return CqVector2D(u, v);
		}
};


//------------------------------------------------------------------------------
class CqInvBilinearNewton
{
	private:
		CqVector2D m_A;
		CqVector2D m_E;
		CqVector2D m_F;
		CqVector2D m_G;
		bool m_linear;

		/// Solve M*x = b  with the matrix M = [M1 M2]
		static inline CqVector2D solve(CqVector2D M1, CqVector2D M2, CqVector2D b)
		{
			TqFloat det = 1/(M1.x()*M2.y() - M1.y()*M2.x());
			return det*CqVector2D(b.x()*M2.y() - b.y()*M2.x(),
					              b.y()*M1.x() - b.x()*M1.y());
		}
		inline CqVector2D bilinEval(CqVector2D uv) const
		{
			return m_A + m_F*uv.x() + m_E*uv.y() + m_G*uv.x()*uv.y();
		}

	public:
		CqInvBilinearNewton(CqVector2D A, CqVector2D B, CqVector2D C, CqVector2D D)
			: m_A(A),
			m_E(C-A),
			m_F(B-A),
			m_G(-m_F-C+D),
			m_linear(false)
		{
			TqFloat patchSize = max(maxNorm(m_E), maxNorm(m_F));
			TqFloat irregularity = maxNorm(m_G);
			if(irregularity < 1e-3*patchSize)
				m_linear = true;
		}

		noinline CqVector2D operator()(CqVector2D P) const
		{
			CqVector2D PA = P - m_A;

			// Start at centre, & do two iterations of Newton's method.
			CqVector2D uv(0.5, 0.5);
			uv -= solve(m_F + m_G*uv.y(), m_E + m_G*uv.x(), bilinEval(uv)-P);
			if(!m_linear)
				uv -= solve(m_F + m_G*uv.y(), m_E + m_G*uv.x(), bilinEval(uv)-P);
			return uv;
		}
};


//------------------------------------------------------------------------------
// Unit testing code; link with -lboost_unit_test_framework

#ifndef SPEED_TEST

#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <algorithm>


// Custom predicate functor for BOOST_CHECK_PREDICATE.
//
// This is a combined absolute and relative closeness check.  It's true when
// the either the absolute difference between left and right is below a given
// threshold, *or* the relative difference is.
class IsCloseRelAbs
{
	private:
		TqFloat m_relTol;
		TqFloat m_absTol;
	public:
		IsCloseRelAbs(
			TqFloat relTol = 2*FLT_EPSILON,
			TqFloat absTol = 2*FLT_EPSILON)
			: m_relTol(relTol), m_absTol(absTol)
		{}
		bool operator()(TqFloat f1, TqFloat f2)
		{
			TqFloat diff = std::fabs(f1-f2);
			return diff < m_absTol
				|| diff < m_relTol*std::fabs(f1)
				|| diff < m_relTol*std::fabs(f2);
		}
};

static void checkLookup(std::vector<TqFloat> uvIn,
						CqVector2D A, CqVector2D B, CqVector2D C, CqVector2D D,
						TqFloat relTol = 10*FLT_EPSILON,
						TqFloat absTol = 10*FLT_EPSILON)
{
	IsCloseRelAbs close(relTol, absTol);
	CqInvBilinearDirect invBilerp(A,B,C,D);
	std::greater_equal<TqFloat> ge;
	std::less_equal<TqFloat> le;
	for(int i = 0; i < static_cast<TqInt>(uvIn.size()); i+=2)
	{
		// Compute interpolated position on patch
		TqFloat u = uvIn[i];
		TqFloat v = uvIn[i+1];
		CqVector2D P = bilerp(A,B,C,D, u, v);
		// Invert that to get the parameter values back.
		CqVector2D uvOut = invBilerp(P);
		// Check whether it worked.
		BOOST_CHECK_PREDICATE(close, (u) (uvOut.x()));
		BOOST_CHECK_PREDICATE(close, (v) (uvOut.y()));
		BOOST_CHECK_PREDICATE(ge, (uvOut.x()) (-absTol));
		BOOST_CHECK_PREDICATE(le, (uvOut.x()) (1+absTol));
//		BOOST_CHECK_CLOSE(u, uvOut.x(), 0.1);
//		BOOST_CHECK_CLOSE(v, uvOut.y(), 0.1);
	}
}



const TqFloat e = 1e-6;
const TqFloat uvInInit[] = {
	// Some "random" locations inside the patch
	0.5, 0.5,
	0.12345, 0.67891,
	0.42042, 0.42042,
	0.3141592, 0.2718281,
	// Corners of the patch
	0, 0,
	1, 0,
	0, 1,
	1, 1,
	// Very close to patch corners
	e, e,
	1-e, e,
	e, 1-e,
	1-e, 1-e,
	// On patch edges
	0.5, 0,
	0.5, 1,
	0, 0.5,
	1, 0.5,
	// Very close to patch edges
	0.5, e,
	0.5, 1-e,
	e, 0.5,
	1-e, 0.5,
};
const std::vector<TqFloat> uvIn(uvInInit, uvInInit+sizeof(uvInInit)/sizeof(TqFloat));;


BOOST_AUTO_TEST_CASE(InvBilinear_convex_irregular)
{
	CqVector2D A(0.1, 0.1),   B(1.1, 0),
			   C(-0.1, 1.5),  D(1, 1);
	checkLookup(uvIn, A, B, C, D);
}

BOOST_AUTO_TEST_CASE(InvBilinear_convex_irregular_rot90)
{
	CqVector2D A(-0.1, 1.5), B(0.1, 0.1),
			   C(1, 1),      D(1.1, 0);
	checkLookup(uvIn, A, B, C, D);
}

BOOST_AUTO_TEST_CASE(InvBilinear_convex_irregular_rot180)
{
	CqVector2D A(1, 1),   B(-0.1, 1.5),
			   C(1.1, 0), D(0.1, 0.1);
	checkLookup(uvIn, A, B, C, D);
}

BOOST_AUTO_TEST_CASE(InvBilinear_convex_irregular_rot270)
{
	CqVector2D A(1.1, 0),   B(1, 1),
			   C(0.1, 0.1), D(-0.1, 1.5);
	checkLookup(uvIn, A, B, C, D);
}

BOOST_AUTO_TEST_CASE(InvBilinear_large_offset)
{
	CqVector2D A(0, 0),  B(2, 0),  C(0, 1),  D(2, 1);
	CqVector2D offset(1000, 2000);
	A += offset; B += offset; C += offset; D += offset;
	// We loose about 4 digits of precision here due to cancellation errors...
	// so cannot expect better than ~2000*eps accuracy for relTol
	//
	// Similarly, absTol has to be quite large.
	checkLookup(uvIn, A, B, C, D, 2000*FLT_EPSILON, 4000*FLT_EPSILON);
}

BOOST_AUTO_TEST_CASE(InvBilinear_exactly_rectangular)
{
	CqVector2D A(0, 0),  B(2, 0),  C(0, 1),  D(2, 1);
	checkLookup(uvIn, A, B, C, D);
}

BOOST_AUTO_TEST_CASE(InvBilinear_almost_rectangular)
{
	CqVector2D A(0.0001, 0.000005), B(2, 0), C(0, 1),  D(2, 1);
	// The almost-rectangular case uses the lower-precision but _fast_ method
	// of assuming that the system of equations to be solved are exactly
	// linear.  This means an increase in relTol and absTol.
	checkLookup(uvIn, A, B, C, D,  2e-4, 1e-4);
}

BOOST_AUTO_TEST_CASE(InvBilinear_degenerate_points)
{
	CqVector2D A(0, 0),  B(0, 0),  C(0, 1),  D(1, 1.5);
	checkLookup(uvIn, A, B, C, D);
}


	// Degenerate corner

	// Irregular non-convex

//BOOST_AUTO_TEST_CASE(InvBilinear_nonconvex_irregular)
//{
//	CqVector2D A(0.8, 0.8),  B(1.1, 0),  C(-0.1, 1.5),  D(1, 1);
//	checkLookup(uvIn, A, B, C, D);
//}

#else // ifndef SPEED_TEST



//------------------------------------------------------------------------------
// The following is speed testing code, designed to be run with the cppbench
// utility.
//
// There's three orthogonal things things to test here:
//   1) Performance of the two methods for almost-rectangular quads
//   2) Performance with & without the initialization step for every inverse lookup
//   3) Performance for general vs almost-rectangular grids.
//
// The tests seem to indicate that generally the symmetrically formulated
// analytical method is somewhat faster than two iterations of Newton's method.
// Both methods can be optimized for the case when the micropolygons are almost
// rectangular, and the resulting computation is essentially the same in that
// case.

int main()
{
//	typedef CqInvBilinearDirect InvBilin;       //##bench direct
//	typedef CqInvBilinearNewton InvBilin;       //##bench newton
	// irregular patch
	CqVector2D A(0.1, 0.1),  B(1.1, 0),  C(-0.1, 1.5),  D(1, 1);
	// almost-rectangular patch
//	CqVector2D A(0.0001, 0.000005), B(2, 0), C(0, 1),  D(2, 1);
	CqVector2D uvSum;
	CqVector2D P = bilerp(A,B,C,D, 0.123, 0.456);
	InvBilin invBilin(A,B,C,D);
	for(int i = 0; i < 100000000; ++i)
	{
//		InvBilin invBilin(A+CqVector2D(i*0.0000000001,0),B,C,D);

		// Increment P.x and add to the uvSum to fool the optimizer into
		// actually doing the lookup inside the loop.
		P.x() += 1e-9;
		uvSum += invBilin(P);
	}
	return static_cast<int>(uvSum.x());
}


//##description direct Symmetric analytical scheme for inverse bilinear lookup
//##description newton Two iterations of Newton's method


//##CXXFLAGS -O3 -DSPEED_TEST -I$AQSIS_INCLUDE

#endif // ifndef SPEED_TEST
