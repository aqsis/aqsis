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

#include "color.h"

#define BOOST_TEST_MODULE CqColor_Tests
#define BOOST_AUTO_TEST_MAIN

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

using Aqsis::CqColor;

BOOST_AUTO_TEST_CASE(color_min)
{
	const CqColor cMin(0.3,0.4,0.5);
	const CqColor cMax(0.4,0.5,0.6);
	BOOST_CHECK_EQUAL(Aqsis::min(cMin, cMax), cMin);
	BOOST_CHECK_EQUAL(Aqsis::min(cMax, cMin), cMin);
	BOOST_CHECK_EQUAL(Aqsis::min(CqColor(1,0.5,0), CqColor(0.2,0.5,0.8)), CqColor(0.2,0.5,0));
}

BOOST_AUTO_TEST_CASE(color_max)
{
	const CqColor cMin(0.3,0.4,0.5);
	const CqColor cMax(0.4,0.5,0.6);
	BOOST_CHECK_EQUAL(Aqsis::max(cMin, cMax), cMax);
	BOOST_CHECK_EQUAL(Aqsis::max(cMax, cMin), cMax);
	BOOST_CHECK_EQUAL(Aqsis::max(CqColor(1,0.5,0), CqColor(0.2,0.5,0.8)), CqColor(1,0.5,0.8));
}

BOOST_AUTO_TEST_CASE(color_clamp)
{
	const CqColor cMin(0.3,0.4,0.5);
	const CqColor cMax(0.4,0.5,0.6);
	BOOST_CHECK_EQUAL(Aqsis::clamp(CqColor(0,0,0), cMin, cMax), cMin);
	BOOST_CHECK_EQUAL(Aqsis::clamp(CqColor(1,1,1), cMin, cMax), cMax);
	const CqColor cMiddle = (cMin+cMax)/2;
	BOOST_CHECK_EQUAL(Aqsis::clamp(cMiddle, cMin, cMax), cMiddle);
	BOOST_CHECK_EQUAL(Aqsis::clamp(CqColor(0.9, 0.45, 0.1), cMin, cMax), CqColor(0.4, 0.45, 0.5));
}

BOOST_AUTO_TEST_CASE(color_lerp)
{
	const CqColor cMin(0.1, 0.2, 0.3);
	const CqColor cMax(0.1, 0.05, 1);
	BOOST_CHECK_EQUAL(Aqsis::lerp(0.2f, cMin, cMax), 0.8f*cMin + 0.2f*cMax);
	BOOST_CHECK_EQUAL(Aqsis::lerp(0.0f, cMin, cMax), cMin);
	BOOST_CHECK_EQUAL(Aqsis::lerp(1.0f, cMin, cMax), cMax);
}

//------------------------------------------------------------------------------
#else // AQSIS_OPTIMIZATION_TEST

// Efficiency tests.

#include "color.h"

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
