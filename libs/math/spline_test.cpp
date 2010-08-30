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
 * \brief Unit tests for CqCubicSpline
 * \author Tobias Sauerwein
 */

#include <aqsis/math/spline.h>

#define BOOST_TEST_DYN_LINK

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

BOOST_AUTO_TEST_SUITE(spline_tests)

BOOST_AUTO_TEST_CASE(CqCubicSpline_pushBack_test)
{
	Aqsis::CqCubicSpline<TqFloat> spline(Aqsis::SplineBasis_CatmullRom);
	spline.pushBack(1.0f);
	spline.pushBack(1.0f);
	BOOST_CHECK_EQUAL(spline.numControlPoints(), 2);
}

BOOST_AUTO_TEST_CASE(CqCubicSpline_index_access_test)
{
	Aqsis::CqCubicSpline<TqFloat> spline(Aqsis::SplineBasis_CatmullRom);
	spline.pushBack(1.0f);
	spline.pushBack(2.0f);
	BOOST_CHECK_CLOSE(spline[0], 1.0f, 1e-5f);
	BOOST_CHECK_CLOSE(spline[1], 2.0f, 1e-5f);
}

BOOST_AUTO_TEST_CASE(CqCubicSpline_clear_test)
{
	Aqsis::CqCubicSpline<TqFloat> spline(Aqsis::SplineBasis_CatmullRom);
	spline.pushBack(1.0f);
	spline.pushBack(2.0f);
	spline.clear();
	BOOST_CHECK_EQUAL(spline.numControlPoints(), 0);
}

BOOST_AUTO_TEST_CASE(CqCubicSpline_Linear_evaluate_test)
{
	Aqsis::CqCubicSpline<TqFloat> spline(Aqsis::SplineBasis_Linear);
	spline.pushBack(1.0f);
	spline.pushBack(2.0f);
	spline.pushBack(4.0f);
	spline.pushBack(5.0f);
	BOOST_CHECK_CLOSE(spline.evaluate(0.5f), 3.0f, 1e-5f);
	BOOST_CHECK_CLOSE(spline.evaluate(0.0f), 2.0f, 1e-5f);
	BOOST_CHECK_CLOSE(spline.evaluate(0.25f), 2.5f, 1e-5f);
	BOOST_CHECK_CLOSE(spline.evaluate(0.75f), 3.5f, 1e-5f);
	BOOST_CHECK_CLOSE(spline.evaluate(0.875f), 3.75f, 1e-5f);
}

BOOST_AUTO_TEST_SUITE_END()
