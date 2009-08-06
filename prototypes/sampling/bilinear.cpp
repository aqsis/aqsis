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
 *        F=C-A  |   .P        /
 *           |   |            /
 *           |   |           /
 *           |   |          /
 *           |   |         /
 *               A--------B
 *
 *                ------->
 *                E = B-A
 *
 *              u-direction ->
 *
 * \endverbatim
 */


using Aqsis::CqVector2D;
using Aqsis::max;
using Aqsis::clamp;
#define OUT(x) std::cout << #x << " = " << (x) << "\n";

#define noinline __attribute__((noinline))

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
		CqVector2D m_E;
		CqVector2D m_F;
		CqVector2D m_G;
		TqFloat m_inv2GxE;
		TqFloat m_ExF;
		TqFloat m_invFxE;
		TqFloat m_GxE;
		CqVector2D m_4GxEF;

		bool m_linear;
		bool m_linearU;

	public:
		CqInvBilinearDirect(CqVector2D A, CqVector2D B, CqVector2D C, CqVector2D D)
			: m_H(),
			m_E(),
			m_F(),
			m_G(),
			m_inv2GxE(0),
			m_ExF(0),
			m_GxE(0),
			m_4GxEF(),
			m_linear(false),
			m_linearU(false)
		{
			setVertices(A, B, C, D);
		}

		void setVertices(CqVector2D A, CqVector2D B, CqVector2D C, CqVector2D D)
		{
			m_H = 0.25*(A+B+C+D);
			m_E = 0.5*(B-A + D-C);
			m_F = 0.5*(C-A + D-B);
			m_G = A-B-C+D;
			m_ExF = cross(m_E, m_F);
			m_linear = false;
			m_linearU = false;
			const TqFloat patchSize = max(maxNorm(m_F), maxNorm(m_E));
			const TqFloat irregularity = maxNorm(m_G);
			if(irregularity < 1e-2*patchSize)
			{
				m_ExF = 1/m_ExF;
				m_linear = true;
			}
			else
			{
				m_GxE = cross(m_G, m_E);
				if(std::fabs(m_GxE) < 1e-4*patchSize)
				{
					m_linearU = true;
				}
				else
				{
					m_inv2GxE = 0.5/m_GxE;
					m_4GxEF = 4*m_GxE*m_F;
				}
			}
		}

		CqVector2D operator()(CqVector2D P) const
		{
			const CqVector2D PH = P - m_H;
			TqFloat u = 0;
			TqFloat v = 0;
			if(m_linear)
			{
				// NOTE: m_ExF is actually 1/m_ExF here!
				u = -cross(m_F, PH)*m_ExF;
				v = cross(m_E, PH)*m_ExF;
			}
			else
			{
				if(m_linearU)
				{
					// Semi-linear case; the equation for u is missing the
					// quadratic term, and we can also find a linear equation
					// for v.  This happens when two adjacent vertices are the
					// same.
//					u = cross(m_F, PH) / (cross(m_G, PH) + m_ExF);
					v = cross(m_E, PH) / m_ExF;
					u = cross(m_F, PH) / (v*cross(m_F, m_G) - m_ExF);
				}
				else
				{
					const TqFloat t = m_ExF + cross(m_G, PH);
					const TqFloat d = sqrt(t*t + cross(m_4GxEF, PH));
					u = m_inv2GxE*(t + d);
					if(u < -0.5-FLT_EPSILON || u > 0.5+FLT_EPSILON)
						u = m_inv2GxE*(t - d);
					// Now that we have u, find v.  There's several ways to do
					// this but the one involving division here is likely the
					// most numerically stable since it's resistent to cases
					// where GxE is small.  However, it can still fail (!) in
					// cases where vertices are the same :-(
					v = cross(m_E, PH) / (m_ExF - m_GxE*u);
				}
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
		template<bool unsafeInvert>
		static inline CqVector2D solve(CqVector2D M1, CqVector2D M2, CqVector2D b)
		{
			TqFloat det = cross(M1, M2);
			if(unsafeInvert || det != 0) det = 1/det;
			return det * CqVector2D(cross(b, M2), -cross(b, M1));
		}
		inline CqVector2D bilinEval(CqVector2D uv) const
		{
			return m_A + m_E*uv.x() + m_F*uv.y() + m_G*uv.x()*uv.y();
		}

	public:
		CqInvBilinearNewton(CqVector2D A, CqVector2D B, CqVector2D C, CqVector2D D)
			: m_A(A),
			m_E(B-A),
			m_F(C-A),
			m_G(-m_E-C+D),
			m_linear(false)
		{
			TqFloat patchSize = max(maxNorm(m_F), maxNorm(m_E));
			TqFloat irregularity = maxNorm(m_G);
			if(irregularity < 1e-2*patchSize)
				m_linear = true;
		}

		CqVector2D operator()(CqVector2D P) const
		{
			// Start at centre, & do two iterations of Newton's method.
			CqVector2D uv(0.5, 0.5);
			uv -= solve<true>(m_E + m_G*uv.y(), m_F + m_G*uv.x(), bilinEval(uv)-P);
			if(!m_linear)
			{
				uv -= solve<false>(m_E + m_G*uv.y(), m_F + m_G*uv.x(), bilinEval(uv)-P);
//				uv -= solve<false>(m_E + m_G*uv.y(), m_F + m_G*uv.x(), bilinEval(uv)-P);
			}
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
#include <fenv.h>

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
		bool operator()(CqVector2D v1, CqVector2D v2)
		{
			return (*this)(v1.x(), v2.x()) && (*this)(v1.y(), v2.y());
		}
};

static void checkLookup(std::vector<CqVector2D> uvInList,
						CqVector2D A, CqVector2D B, CqVector2D C, CqVector2D D,
						// The following two tolerances work for the analytical
						// version, but are more accurate than we need.
//						TqFloat relTol = 10*FLT_EPSILON,
//						TqFloat absTol = 10*FLT_EPSILON)
						// The following two tolerances are about right for the
						// version using Newton's method.
						TqFloat relTol = 1e-3,
						TqFloat absTol = 1e-3)
{
	feenableexcept (FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
	IsCloseRelAbs close(relTol, absTol);
//	typedef CqInvBilinearDirect InvBilin;
	typedef CqInvBilinearNewton InvBilin;
	InvBilin invBilerp(A,B,C,D);
	std::greater_equal<TqFloat> ge;
	std::less_equal<TqFloat> le;
	for(int i = 0; i < static_cast<TqInt>(uvInList.size()); ++i)
	{
		// Compute interpolated position on patch
		CqVector2D uvIn = uvInList[i];
		CqVector2D P = bilerp(A,B,C,D, uvIn.x(), uvIn.y());
		// Invert that to get the parameter values back.
		CqVector2D uvOut = invBilerp(P);
		// Check whether it worked.
		BOOST_CHECK_PREDICATE(close, (uvIn) (uvOut));
		BOOST_CHECK_PREDICATE(ge, (uvOut.x()) (-absTol));
		BOOST_CHECK_PREDICATE(le, (uvOut.x()) (1+absTol));
	}
}


struct UVFixture
{
	std::vector<CqVector2D> uvIn;

	UVFixture(TqFloat uExclude = -1, TqFloat vExclude = -1)
	{
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
		// Exclude potentially problematic regions in u and v.
		for(int i = 0; i < sizeof(uvInInit)/sizeof(TqFloat); i += 2)
		{
			TqFloat u = uvInInit[i];
			TqFloat v = uvInInit[i+1];
			if( (uExclude == -1 || std::fabs(uExclude-u) > 1e-2) &&
				(vExclude == -1 || std::fabs(vExclude-v) > 1e-2) )
			{
				uvIn.push_back(CqVector2D(u,v));
			}
		}
	}
};


BOOST_AUTO_TEST_CASE(InvBilinear_convex_irregular)
{
	UVFixture f;
	CqVector2D A(0.1, 0.1),   B(1.1, 0),
			   C(-0.1, 1.5),  D(1, 1);
	checkLookup(f.uvIn, A, B, C, D);
}

BOOST_AUTO_TEST_CASE(InvBilinear_convex_irregular_rot90)
{
	UVFixture f;
	CqVector2D A(-0.1, 1.5), B(0.1, 0.1),
			   C(1, 1),      D(1.1, 0);
	checkLookup(f.uvIn, A, B, C, D);
}

BOOST_AUTO_TEST_CASE(InvBilinear_convex_irregular_rot180)
{
	UVFixture f;
	CqVector2D A(1, 1),   B(-0.1, 1.5),
			   C(1.1, 0), D(0.1, 0.1);
	checkLookup(f.uvIn, A, B, C, D);
}

BOOST_AUTO_TEST_CASE(InvBilinear_convex_irregular_rot270)
{
	UVFixture f;
	CqVector2D A(1.1, 0),   B(1, 1),
			   C(0.1, 0.1), D(-0.1, 1.5);
	checkLookup(f.uvIn, A, B, C, D);
}

BOOST_AUTO_TEST_CASE(InvBilinear_large_offset)
{
	UVFixture f;
	CqVector2D A(0, 0),  B(2, 0),  C(0, 1),  D(2, 1);
	CqVector2D offset(1000, 2000);
	A += offset; B += offset; C += offset; D += offset;
	// Even for the "exact" analytical solution, we loose about 4 digits of
	// precision here due to cancellation errors...  so cannot expect better
	// than ~2000*eps accuracy for relTol
	//
	// Similarly, absTol has to be quite large.
//	checkLookup(f.uvIn, A, B, C, D, 2000*FLT_EPSILON, 4000*FLT_EPSILON);
	checkLookup(f.uvIn, A, B, C, D);
}

BOOST_AUTO_TEST_CASE(InvBilinear_exactly_rectangular)
{
	UVFixture f;
	CqVector2D A(0, 0),  B(2, 0),  C(0, 1),  D(2, 1);
	checkLookup(f.uvIn, A, B, C, D);
}

BOOST_AUTO_TEST_CASE(InvBilinear_almost_rectangular)
{
	UVFixture f;
	CqVector2D A(0.0001, 0.000005), B(2, 0), C(0, 1),  D(2, 1);
	// The almost-rectangular case uses the lower-precision but _fast_ method
	// of assuming that the system of equations to be solved are exactly
	// linear.  This means an increase in relTol and absTol.
	checkLookup(f.uvIn, A, B, C, D,  2e-4, 1e-4);
}

BOOST_AUTO_TEST_CASE(InvBilinear_degenerate_u_verts)
{
	// Exclude all test (u,v) coords along the line v = 0 where the points A
	// and B are coincident.
	UVFixture f(-1,0);
	CqVector2D A(0, 0),  B(0, 0),  C(0, 1),  D(1, 1.5);
	checkLookup(f.uvIn, A, B, C, D);
}

BOOST_AUTO_TEST_CASE(InvBilinear_degenerate_u_verts2)
{
	// Exclude all test (u,v) coords along the line v = 1 where the points C
	// and D are coincident.
	UVFixture f(-1,1);
	CqVector2D A(0, 0),  B(1.1, 0),  C(0, 1),  D(0, 1);
	checkLookup(f.uvIn, A, B, C, D);
}

BOOST_AUTO_TEST_CASE(InvBilinear_degenerate_v_verts)
{
	// Exclude all test (u,v) coords along the line u = 0 where the points A
	// and C are coincident.
	UVFixture f(0,-1);
	CqVector2D A(0, 0),  B(1, 0),  C(0, 0),  D(1, 1.5);
	checkLookup(f.uvIn, A, B, C, D);
}

BOOST_AUTO_TEST_CASE(InvBilinear_parallel_adjacent_edges)
{
	// Adjacent parallel sides.  Newton's method is really rather slow to
	// converge here, so we can't expect the accuracy of two iterations to be
	// any good :-(
	{
		UVFixture f(1,0);
		CqVector2D A(0, 0),  B(1, 0),  C(1, 1.5),  D(2, 0.01);
		checkLookup(f.uvIn, A, B, C, D, 0.03, 0.03);
	}
	{
		UVFixture f;
		CqVector2D A(0, 0),  B(1, 0),  C(1, 1.5),  D(2, 0.01);
		checkLookup(f.uvIn, A, B, C, D, 0.2, 0.2);
	}
}

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
// analytical method is about the same speed as two iterations of Newton's
// method.  Both methods can be optimized for the case when the micropolygons
// are almost rectangular.
//
// Newton's method is significantly more robust however, with possible divisions
// by zero only occurring near one location which is easy to avoid.  Newton's
// method converges slowly when two adjacent sides are parallel.

// testLoop is the speed testing function.  It's declared as noinline so as to
// isolate it from the constant known values of A,B,C,D and prevent
// un-representative optimizations.
noinline
CqVector2D testLoop(CqVector2D A, CqVector2D B, CqVector2D C, CqVector2D D,
		const int numIters)
{
//	typedef CqInvBilinearDirect InvBilin;       //##bench direct
//	typedef CqInvBilinearNewton InvBilin;       //##bench newton
	CqVector2D uvSum;
	CqVector2D P = bilerp(A,B,C,D, 0.123, 0.456);
	InvBilin invBilin(A,B,C,D);
	for(int i = 0; i < numIters; ++i)
	{
//		InvBilin invBilin(A+CqVector2D(i*0.00000000001,0),B,C,D);

		// Increment P.x and add to the uvSum to fool the optimizer into
		// actually doing the lookup inside the loop.
		P.x() += 1e-9;
		uvSum += invBilin(P);
	}
	return uvSum;
}

int main()
{
	// irregular patch
	CqVector2D A(0.1, 0.1),  B(1.1, 0),  C(-0.1, 1.5),  D(1, 1);
	// almost-rectangular patch
//	CqVector2D A(0.0001, 0.000005), B(2, 0), C(0, 1),  D(2, 1);
	testLoop(A,B,C,D, 40000000);
	return 0;
}


//##description direct Symmetric analytical scheme for inverse bilinear lookup
//##description newton Two iterations of Newton's method


//##CXXFLAGS -O3 -Wall -DSPEED_TEST -I$AQSIS_INCLUDE

#endif // ifndef SPEED_TEST
