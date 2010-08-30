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
 * \brief Unit tests for aqsis math functions.
 * \author Chris Foster
 */

#include <aqsis/math/math.h>
#include "limits.h"

#define BOOST_TEST_DYN_LINK

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

BOOST_AUTO_TEST_SUITE(base_math_tests)

BOOST_AUTO_TEST_CASE(lfloor_test)
{
	BOOST_CHECK_EQUAL(Aqsis::lfloor<float>(1.0f), 1);
	BOOST_CHECK_EQUAL(Aqsis::lfloor<float>(-1.0f), -1);
	BOOST_CHECK_EQUAL(Aqsis::lfloor<float>(1.1f), 1);
	BOOST_CHECK_EQUAL(Aqsis::lfloor<float>(-1.1f), -2);
}

BOOST_AUTO_TEST_CASE(lceil_test)
{
	BOOST_CHECK_EQUAL(Aqsis::lceil<float>(1.0f), 1);
	BOOST_CHECK_EQUAL(Aqsis::lceil<float>(-1.0f), -1);
	BOOST_CHECK_EQUAL(Aqsis::lceil<float>(1.1f), 2);
	BOOST_CHECK_EQUAL(Aqsis::lceil<float>(-1.1f), -1);
}

BOOST_AUTO_TEST_CASE(lround_test)
{
	BOOST_CHECK_EQUAL(Aqsis::lround<float>(1.0f), 1);
	BOOST_CHECK_EQUAL(Aqsis::lround<float>(1.2f), 1);
	BOOST_CHECK_EQUAL(Aqsis::lround<float>(1.5f), 2);
	BOOST_CHECK_EQUAL(Aqsis::lround<float>(1.7f), 2);

	BOOST_CHECK_EQUAL(Aqsis::lround<float>(-1.0f), -1);
	BOOST_CHECK_EQUAL(Aqsis::lround<float>(-1.2f), -1);
	BOOST_CHECK_EQUAL(Aqsis::lround<float>(-1.5f), -1);
	BOOST_CHECK_EQUAL(Aqsis::lround<float>(-1.7f), -2);
}

BOOST_AUTO_TEST_CASE(round_test)
{
	BOOST_CHECK_CLOSE(Aqsis::round<float>(1.0f), 1.0f, 0.00001f);
	BOOST_CHECK_CLOSE(Aqsis::round<float>(1.2f), 1.0f, 0.00001f);
	BOOST_CHECK_CLOSE(Aqsis::round<float>(1.5f), 2.0f, 0.00001f);
	BOOST_CHECK_CLOSE(Aqsis::round<float>(1.7f), 2.0f, 0.00001f);
	BOOST_CHECK_CLOSE(Aqsis::round<float>(2*float(LONG_MAX) + 0.1), 2*float(LONG_MAX), 0.00001f);

	BOOST_CHECK_CLOSE(Aqsis::round<float>(-1.0f), -1.0f, 0.00001f);
	BOOST_CHECK_CLOSE(Aqsis::round<float>(-1.2f), -1.0f, 0.00001f);
	BOOST_CHECK_CLOSE(Aqsis::round<float>(-1.5f), -1.0f, 0.00001f);
	BOOST_CHECK_CLOSE(Aqsis::round<float>(-1.7f), -2.0f, 0.00001f);
	BOOST_CHECK_CLOSE(Aqsis::round<float>(-2*float(LONG_MAX) + 0.1), -2*float(LONG_MAX), 0.00001f);
}

BOOST_AUTO_TEST_CASE(lerp_test)
{
	BOOST_CHECK_CLOSE((Aqsis::lerp(0.1f, 1.0f, 2.0f)), 1.1f, 0.00001f);
	BOOST_CHECK_CLOSE((Aqsis::lerp(0.9f, 1.0f, 2.0f)), 1.9f, 0.00001f);
}

BOOST_AUTO_TEST_CASE(clamp_test)
{
	BOOST_CHECK_EQUAL(Aqsis::clamp<float>(-4.0f, -1.0f, 2.0f), -1.0f);
	BOOST_CHECK_EQUAL(Aqsis::clamp<float>(1.0f, -1.0f, 2.0f), 1.0f);
	BOOST_CHECK_EQUAL(Aqsis::clamp<float>(4.0f, -1.0f, 2.0f), 2.0f);
}

BOOST_AUTO_TEST_CASE(min_test)
{
	BOOST_CHECK_EQUAL(Aqsis::min<float>(1.0f, 2.0f), 1.0f);
}

BOOST_AUTO_TEST_CASE(max_test)
{
	BOOST_CHECK_EQUAL(Aqsis::max<float>(1.0f, 2.0f), 2.0f);
}

BOOST_AUTO_TEST_CASE(degToRad_test)
{
	BOOST_CHECK_CLOSE(Aqsis::degToRad(180.0f), float(M_PI), 0.00001f);
}

BOOST_AUTO_TEST_CASE(radToDeg_test)
{
	BOOST_CHECK_CLOSE(Aqsis::radToDeg(M_PI), 180.0f, 0.00001f);
}

BOOST_AUTO_TEST_CASE(ceilPow2_test)
{
	BOOST_CHECK_EQUAL(Aqsis::ceilPow2(0x100), TqUint(0x100));
	BOOST_CHECK_EQUAL(Aqsis::ceilPow2(0x110), TqUint(0x200));
}

BOOST_AUTO_TEST_CASE(log2_test)
{
	BOOST_CHECK_CLOSE(Aqsis::log2(1), 0.0f, 0.00001f);
	BOOST_CHECK_CLOSE(Aqsis::log2(8), 3.0f, 0.00001f);
}

BOOST_AUTO_TEST_CASE(isClose_test)
{
	BOOST_CHECK(Aqsis::isClose(0,0));

	const TqFloat f1 = 1.5;
	BOOST_CHECK(Aqsis::isClose(f1, f1));
	BOOST_CHECK(Aqsis::isClose(f1, (f1*100.0)/100.0));

	const TqFloat eps = 1e-4;

	BOOST_CHECK(!Aqsis::isClose(f1, f1 + eps));
	BOOST_CHECK(!Aqsis::isClose(f1, f1 + eps, 1e-5));
	BOOST_CHECK(Aqsis::isClose(f1, f1 + eps, 1e-4));
	BOOST_CHECK(Aqsis::isClose(f1, f1 - eps, 1e-4));
}

BOOST_AUTO_TEST_SUITE_END()
