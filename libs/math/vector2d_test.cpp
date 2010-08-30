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
 * \brief Unit tests for CqVector2Vector2DD and related stuff
 * \author Tobias Sauerwein
 */

//------------------------------------------------------------------------------
// Unit tests

#include <aqsis/math/vector2d.h>

#define BOOST_TEST_DYN_LINK

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

BOOST_AUTO_TEST_SUITE(vector2d_tests)


static bool equal(const Aqsis::CqVector2D& a, const Aqsis::CqVector2D& b)
{
	return isClose(a, b);
}

#define CHECK_VEC_CLOSE(v1, v2) BOOST_CHECK_PREDICATE(equal, (v1)(v2))

using Aqsis::CqVector2D;

BOOST_AUTO_TEST_CASE(vector2d_magnitude)
{
	const CqVector2D vec(0.3f, 0.4f);
	
	BOOST_CHECK_EQUAL(vec.Magnitude(), 0.5f);
}

BOOST_AUTO_TEST_CASE(vector2d_magnitude2)
{
	const CqVector2D vec(0.3f, 0.4f);
	
	BOOST_CHECK_EQUAL(vec.Magnitude2(), 0.25f);
}

BOOST_AUTO_TEST_CASE(vector2d_unit)
{
	CqVector2D vec(0.3f, 0.4f);
	
	vec.Unit();
	
	BOOST_CHECK_EQUAL(vec, CqVector2D(0.6f, 0.8f));
}

BOOST_AUTO_TEST_CASE(vector2d_operator_addition_vector)
{
	const CqVector2D vec1(0.3f, 0.4f);
	const CqVector2D vec2(1.2f, 0.3f);
	
	CHECK_VEC_CLOSE(vec1 + vec2, CqVector2D(1.5f, 0.7f));
}

BOOST_AUTO_TEST_CASE(vector2d_operator_addition_float_to_vector)
{
	const CqVector2D vec(0.3f, 0.4f);
	
	CHECK_VEC_CLOSE(vec + 2.4f, CqVector2D(2.7f, 2.8f));
}

BOOST_AUTO_TEST_CASE(vector2d_operator_addition_vector_to_float)
{
	const CqVector2D vec(0.3f, 0.4f);
	
	CHECK_VEC_CLOSE(2.4f + vec, CqVector2D(2.7f, 2.8f));
}

BOOST_AUTO_TEST_CASE(vector2d_operator_substraction_vector)
{
	const CqVector2D vec1(0.3f, 0.4f);
	const CqVector2D vec2(1.2f, 0.3f);
	
	CHECK_VEC_CLOSE(vec1 - vec2, CqVector2D(-0.9f, 0.1f));
}

BOOST_AUTO_TEST_CASE(vector2d_operator_substraction_float_from_vector)
{
	const CqVector2D vec(3.3f, 1.9f);
	
	CHECK_VEC_CLOSE(vec - 2.4f, CqVector2D(0.9f, -0.5f));
}

BOOST_AUTO_TEST_CASE(vector2d_operator_substraction_vector_from_float)
{
	const CqVector2D vec(3.3f, 1.9f);
	
	CHECK_VEC_CLOSE(2.4f - vec, CqVector2D(-0.9f, 0.5f));
}

BOOST_AUTO_TEST_CASE(vector2d_operator_multiplication_vector_with_float)
{
	const CqVector2D vec(0.3f, 0.4f);
		
	CHECK_VEC_CLOSE(vec * 1.3f, CqVector2D(0.39f, 0.52f));
}

BOOST_AUTO_TEST_CASE(vector2d_operator_multiplication_float_with_vector)
{
	const CqVector2D vec(0.3f, 0.4f);
		
	CHECK_VEC_CLOSE(1.3f * vec, CqVector2D(0.39f, 0.52f));
}

BOOST_AUTO_TEST_CASE(vector2d_operator_multiplication_vector_with_vector)
{
	const CqVector2D vec1(4.0f, 7.0f);
	const CqVector2D vec2(5.0f, 1.0f);
		
	BOOST_CHECK_EQUAL((vec1 * vec2), 27.0f);
}

BOOST_AUTO_TEST_CASE(vector2d_operator_division_vector_by_float)
{
	const CqVector2D vec(1.625f, 3.25f);
		
	CHECK_VEC_CLOSE(vec / 1.3f, CqVector2D(1.25f, 2.5f));
}

BOOST_AUTO_TEST_CASE(vector2d_operator_division_float_by_vector)
{
	const CqVector2D vec(0.8f, 0.4f);
		
	CHECK_VEC_CLOSE(1.3f / vec, CqVector2D(1.625f, 3.25f));
}

BOOST_AUTO_TEST_CASE(vector2d_operator_division_vector_by_vector)
{
	const CqVector2D vec1(0.6f, 0.2f);
	const CqVector2D vec2(0.3f, 0.4f);
		
	CHECK_VEC_CLOSE(vec1 / vec2, CqVector2D(2.0f, 0.5f));
}

BOOST_AUTO_TEST_CASE(CqVector2D_isClose)
{
	BOOST_CHECK(isClose(CqVector2D(0,0), CqVector2D(0,0)));

	const CqVector2D v1(1.5, 2.5);
	BOOST_CHECK(isClose(v1, v1));
	BOOST_CHECK(isClose(v1, (v1*100.0)/100.0));

	const CqVector2D eps(0, 1e-4);

	BOOST_CHECK(!isClose(v1, v1 + eps));
	BOOST_CHECK(!isClose(v1, v1 + eps, 1e-5));
	BOOST_CHECK(isClose(v1, v1 + eps, 1e-4));
	BOOST_CHECK(isClose(v1, v1 - eps, 1e-4));

	const CqVector2D eps2(1e-4, -1e-5);
	BOOST_CHECK(!isClose(v1, v1 + eps2));
	BOOST_CHECK(!isClose(v1, v1 + eps2, 1e-5));
	BOOST_CHECK(isClose(v1, v1 + eps2, 1e-4));
	BOOST_CHECK(isClose(v1, v1 - eps2, 1e-4));
}

BOOST_AUTO_TEST_SUITE_END()
