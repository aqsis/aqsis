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

/** \file Unit tests for inverse bilinear lookup.
 *
 * \author Chris Foster
 */

#include "bilinear.h"

#include <vector>
#include <cfloat>

#define BOOST_TEST_DYN_LINK
#include <boost/test/auto_unit_test.hpp>

BOOST_AUTO_TEST_SUITE(bilinear_tests)

using namespace Aqsis;

namespace {
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

void checkLookup(std::vector<CqVector2D> uvInList,
				 CqVector2D A, CqVector2D B, CqVector2D C, CqVector2D D,
				 // The following two tolerances are about right for the
				 // version using Newton's method.
				 TqFloat relTol = 1e-3,
				 TqFloat absTol = 1e-3)
{
	IsCloseRelAbs close(relTol, absTol);
	typedef CqInvBilinear InvBilin;
	InvBilin invBilerp(A,B,C,D);
	std::greater_equal<TqFloat> ge;
	std::less_equal<TqFloat> le;
	for(int i = 0; i < static_cast<TqInt>(uvInList.size()); ++i)
	{
		// Compute interpolated position on patch
		CqVector2D uvIn = uvInList[i];
		CqVector2D P = bilerp(A,B,C,D, uvIn);
		// Invert that to get the parameter values back.
		CqVector2D uvOut = invBilerp(P);
		// Check whether it worked.
		BOOST_CHECK_PREDICATE(close, (uvIn) (uvOut));
		BOOST_CHECK_PREDICATE(ge, (uvOut.x()) (-absTol));
		BOOST_CHECK_PREDICATE(le, (uvOut.x()) (1+absTol));
	}
}

// Fixture struct for holding some (u,v) values to test the inverse mapping at.
struct UVFixture
{
	std::vector<CqVector2D> uvIn;

	// Construct the fixture.
	//
	// if uExclude or vExclude are between 0 and 1, they specify values of u
	// and v to avoid during testing.  This is necessary, since for some
	// suitably pathologically-shaped micropolygons, the inverse doesn't exist
	// on the boundary of the micropoly.
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
		int nuvInit = sizeof(uvInInit)/sizeof(TqFloat);
		for(int i = 0; i < nuvInit; i += 2)
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

} // anon. namespace


// And now, the test cases.  These have been chosen to represent a bunch of
// micropolygon shapes encountered in practise, including some pathalogical
// cases :-)

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
	// than ~2000*eps accuracy for relTol in the analytical version.
	//
	// Similarly, absTol has to be quite large.
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

// TODO: See whether we can get anything sensible for the nonconvex case.
//
//BOOST_AUTO_TEST_CASE(InvBilinear_nonconvex_irregular)
//{
//	CqVector2D A(0.8, 0.8),  B(1.1, 0),  C(-0.1, 1.5),  D(1, 1);
//	checkLookup(uvIn, A, B, C, D);
//}

BOOST_AUTO_TEST_SUITE_END()
