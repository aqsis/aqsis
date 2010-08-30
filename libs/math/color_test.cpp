// Aqsis
// Copyright (C) 1997 - 2007, Paul C. Gregory
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
 *
 * \brief Unit tests for CqColor and related stuff
 * \author Chris Foster
 */
#ifndef AQSIS_OPTIMIZATION_TEST
//------------------------------------------------------------------------------
// Unit tests

#include <aqsis/math/color.h>

#define BOOST_TEST_DYN_LINK

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

BOOST_AUTO_TEST_SUITE(color_tests)

using Aqsis::CqColor;

namespace {

bool equal(const CqColor& c1, const CqColor& c2)
{
	return Aqsis::isClose(c1, c2);
}

}

#define CHECK_COL_CLOSE(c1, c2) BOOST_CHECK_PREDICATE(equal, (c1)(c2))

BOOST_AUTO_TEST_CASE(color_min)
{
	const CqColor cMin(0.3,0.4,0.5);
	const CqColor cMax(0.4,0.5,0.6);
	CHECK_COL_CLOSE(Aqsis::min(cMin, cMax), cMin);
	CHECK_COL_CLOSE(Aqsis::min(cMax, cMin), cMin);
	CHECK_COL_CLOSE(Aqsis::min(CqColor(1,0.5,0), CqColor(0.2,0.5,0.8)), CqColor(0.2,0.5,0));
}

BOOST_AUTO_TEST_CASE(color_max)
{
	const CqColor cMin(0.3,0.4,0.5);
	const CqColor cMax(0.4,0.5,0.6);
	CHECK_COL_CLOSE(Aqsis::max(cMin, cMax), cMax);
	CHECK_COL_CLOSE(Aqsis::max(cMax, cMin), cMax);
	CHECK_COL_CLOSE(Aqsis::max(CqColor(1,0.5,0), CqColor(0.2,0.5,0.8)), CqColor(1,0.5,0.8));
}

BOOST_AUTO_TEST_CASE(color_clamp)
{
	const CqColor cMin(0.3,0.4,0.5);
	const CqColor cMax(0.4,0.5,0.6);
	CHECK_COL_CLOSE(Aqsis::clamp(CqColor(0,0,0), cMin, cMax), cMin);
	CHECK_COL_CLOSE(Aqsis::clamp(CqColor(1,1,1), cMin, cMax), cMax);
	const CqColor cMiddle = (cMin+cMax)/2;
	CHECK_COL_CLOSE(Aqsis::clamp(cMiddle, cMin, cMax), cMiddle);
	CHECK_COL_CLOSE(Aqsis::clamp(CqColor(0.9, 0.45, 0.1), cMin, cMax), CqColor(0.4, 0.45, 0.5));
}

BOOST_AUTO_TEST_CASE(color_lerp)
{
	const CqColor cMin(0.1, 0.2, 0.3);
	const CqColor cMax(0.1, 0.05, 1);
	CHECK_COL_CLOSE(Aqsis::lerp(0.2f, cMin, cMax), 0.8f*cMin + 0.2f*cMax);
	CHECK_COL_CLOSE(Aqsis::lerp(0.0f, cMin, cMax), cMin);
	CHECK_COL_CLOSE(Aqsis::lerp(1.0f, cMin, cMax), cMax);
}

BOOST_AUTO_TEST_CASE(color_isClose)
{
	const CqColor c1(1.5, 2.5, 3.5);
	BOOST_CHECK(isClose(c1, c1));
	BOOST_CHECK(isClose(c1, (c1*100.0)/100.0));

	const CqColor cEps(0, 1e-4, 0);

	BOOST_CHECK(!isClose(c1, c1 + cEps));
	BOOST_CHECK(!isClose(c1, c1 + cEps, 1e-5));
	BOOST_CHECK(isClose(c1, c1 + cEps, 1e-4));
	BOOST_CHECK(isClose(c1, c1 - cEps, 1e-4));

	const CqColor cEps2(1e-4, -1e-5, -1e-5);
	BOOST_CHECK(!isClose(c1, c1 + cEps2));
	BOOST_CHECK(!isClose(c1, c1 + cEps2, 1e-5));
	BOOST_CHECK(isClose(c1, c1 + cEps2, 1e-4));
	BOOST_CHECK(isClose(c1, c1 - cEps2, 1e-4));
}

BOOST_AUTO_TEST_SUITE_END()

//------------------------------------------------------------------------------
#else // AQSIS_OPTIMIZATION_TEST

// Efficiency tests.

#include <aqsis/math/color.h>

// This is a duplicate of the generic lerp(), but with a different name to
// allow comparisons with the specialised version.
template<typename T, typename V>
inline V lerp2(const T t, const V x0, const V x1)
{
	return (1-t)*x0 + t*x1;
}

int main()
{
	const TqLong numIters = 1000000000;

	Aqsis::CqColor a(1,0,0);
	Aqsis::CqColor b(0,0,1);
	Aqsis::CqColor c;
	TqFloat f = 0.2;

	for(TqLong i = 0; i < numIters; ++i)
	{
		c = lerp(f, a, b);
		//c = lerp2(f, a, b);
		//c = (1-f)*a + f*b;  // version that the original LERP macro would have produced.
	}
	return 0;
}

#endif // AQSIS_OPTIMIZATION_TEST
