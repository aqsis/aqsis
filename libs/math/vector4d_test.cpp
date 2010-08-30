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
 * \brief Unit tests for CqVector4D and related stuff
 * \author Tobias Sauerwein
 */

//------------------------------------------------------------------------------
// Unit tests

#include <aqsis/math/vector4d.h>

#define BOOST_TEST_DYN_LINK

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

BOOST_AUTO_TEST_SUITE(vector4d_tests)

static bool equal(const Aqsis::CqVector4D& a, const Aqsis::CqVector4D& b)
{
	return isClose(a, b);
}

#define CHECK_VEC_CLOSE(v1, v2) BOOST_CHECK_PREDICATE(equal, (v1)(v2))


using Aqsis::CqVector4D;

BOOST_AUTO_TEST_CASE(vector4d_magnitude)
{
	const CqVector4D vec(0.3f, 0.4f, 1.6f, 2.0f);
	
	BOOST_CHECK_CLOSE(vec.Magnitude(), 0.838152766f, 1e-5f);
}

BOOST_AUTO_TEST_CASE(vector4d_magnitude2)
{
	const CqVector4D vec(0.3f, 0.4f, 1.6f, 2.0f);
	
	BOOST_CHECK_CLOSE(vec.Magnitude2(), 0.702500045f, 1e-5f);
}

BOOST_AUTO_TEST_CASE(vector4d_unit)
{
	CqVector4D vec(0.3f, 0.4f, 0.6f, 1.0f);	
	
	vec.Unit();
	
	CHECK_VEC_CLOSE(vec, CqVector4D(0.3f, 0.4f, 0.6f, 0.781025f));
}

BOOST_AUTO_TEST_CASE(vector4d_operator_addition_vector)
{
	const CqVector4D vec1(0.3f, 0.4f, 2.3f, 0.8f);
	const CqVector4D vec2(1.2f, 0.3f, 1.6f, 0.2f);
	
	CHECK_VEC_CLOSE(vec1 + vec2, CqVector4D(5.1f, 1.6f, 8.7f, 0.8f));
}

BOOST_AUTO_TEST_CASE(vector4d_operator_substraction_vector)
{
	const CqVector4D vec1(0.3f, 0.4f, 2.3f, 0.8f);
	const CqVector4D vec2(1.2f, 0.3f, 1.6f, 0.2f);
	
	CHECK_VEC_CLOSE(vec1 - vec2, CqVector4D(-4.5f, -0.8f, -4.1f, 0.8f));
}

BOOST_AUTO_TEST_CASE(vector4d_operator_multiplication_vector_with_float)
{
	const CqVector4D vec(0.3f, 0.4f, 2.3f, 0.8f);
		
	CHECK_VEC_CLOSE(vec * 1.3f, CqVector4D(0.39f, 0.52f, 2.99f, 0.8f));
}

BOOST_AUTO_TEST_CASE(vector4d_operator_multiplication_float_with_vector)
{
	const CqVector4D vec(0.3f, 0.4f, 2.3f, 0.8f);
		
	CHECK_VEC_CLOSE(1.3f * vec, CqVector4D(0.39f, 0.52f, 2.99f, 0.8f));
}

BOOST_AUTO_TEST_CASE(vector4d_operator_multiplication_vector_with_vector)
{
	const CqVector4D vec1(0.3f, 0.4f, 2.3f, 0.8f);
	const CqVector4D vec2(1.2f, 0.3f, 1.6f, 0.2f);
		
	BOOST_CHECK_CLOSE((vec1 * vec2), 26.0f, 1e-5f);
}

BOOST_AUTO_TEST_CASE(vector4d_operator_division_vector_by_float)
{
	const CqVector4D vec(1.625f, 3.25f, 1.0f, 1.0f);
		
	CHECK_VEC_CLOSE(vec / 1.3f, CqVector4D(1.625f, 3.25f, 1.0f, 1.3f));
}

BOOST_AUTO_TEST_CASE(vector4d_operator_division_float_by_vector)
{
	const CqVector4D vec(0.8f, 0.4f, 1.0f, 1.0f);
		
	CHECK_VEC_CLOSE(1.3f / vec, CqVector4D(0.8f, 0.4f, 1.0f, 1.3f));
}

BOOST_AUTO_TEST_CASE(vector4d_operator_compare_greater)
{
	const CqVector4D vec1(1.2f, 0.5f, 3.6f, 0.801f);
	const CqVector4D vec2(0.3f, 0.4f, 2.3f, 0.8f);
		
	BOOST_CHECK(vec1 > vec2);
}

BOOST_AUTO_TEST_CASE(vector4d_operator_compare_greater_or_equal)
{
	const CqVector4D vec1(1.2f, 0.5f, 3.6f, 0.801f);
	const CqVector4D vec2(0.3f, 0.4f, 2.3f, 0.8f);
	const CqVector4D vec3(0.3f, 0.4f, 2.3f, 0.8f);
		
	BOOST_CHECK(vec1 >= vec2);
	BOOST_CHECK(vec1 >= vec3);
}

BOOST_AUTO_TEST_CASE(vector4d_operator_less)
{
	const CqVector4D vec1(0.3f, 0.4f, 2.3f, 0.8f);
	const CqVector4D vec2(1.2f, 0.5f, 3.6f, 0.801f);
		
	BOOST_CHECK(vec1 < vec2);
}

BOOST_AUTO_TEST_CASE(vector4d_operator_less_or_equal)
{
	const CqVector4D vec1(0.3f, 0.4f, 2.3f, 0.8f);
	const CqVector4D vec2(1.2f, 0.5f, 3.6f, 0.801f);
	const CqVector4D vec3(0.3f, 0.4f, 2.3f, 0.8f);
		
	BOOST_CHECK(vec1 <= vec2);
	BOOST_CHECK(vec1 >= vec3);
}

BOOST_AUTO_TEST_CASE(CqVector4D_isClose)
{
	// Remeber that these are homogenous vectors, hence the 1 always in the
	// last place!
	const CqVector4D v1(1.5, 2.5, 1.1, 1);
	BOOST_CHECK(isClose(v1, v1));
	BOOST_CHECK(isClose(v1, (v1*100.0)/100.0));

	const CqVector4D eps(0, 1e-4, 0, 1);

	BOOST_CHECK(!isClose(v1, v1 + eps));
	BOOST_CHECK(!isClose(v1, v1 + eps, 1e-5));
	BOOST_CHECK(isClose(v1, v1 + eps, 1e-4));
	BOOST_CHECK(isClose(v1, v1 - eps, 1e-4));

	const CqVector4D eps2(1e-4, -1e-5, 1e-5, 1);
	BOOST_CHECK(!isClose(v1, v1 + eps2));
	BOOST_CHECK(!isClose(v1, v1 + eps2, 1e-5));
	BOOST_CHECK(isClose(v1, v1 + eps2, 1e-4));
	BOOST_CHECK(isClose(v1, v1 - eps2, 1e-4));

	// Special check for homogenous vectors...
	const CqVector4D v2(3.0, 5.0, 2.2, 2);
	BOOST_CHECK(isClose(v1, v2));

	BOOST_CHECK(isClose(CqVector4D(0,0,0,1), CqVector4D(0,0,0,1)));
}

BOOST_AUTO_TEST_SUITE_END()
