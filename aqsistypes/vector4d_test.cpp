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

#include "vector4d.h"

#ifdef AQSIS_SYSTEM_MACOSX
    #define BOOST_AUTO_TEST_MAIN
	#include <boost/test/included/unit_test.hpp>
#else
	#include <boost/test/auto_unit_test.hpp>
#endif

#include <boost/test/floating_point_comparison.hpp>



static bool equals(const Aqsis::CqVector4D& a, const Aqsis::CqVector4D& b)
{
	if ((a.x() > (b.x() + 0.000001f)
			|| a.x() < (b.x() - 0.000001f)) ||
		(a.y() > (b.y() + 0.000001f)
			|| a.y() < (b.y() - 0.000001f)) ||
		(a.z() > (b.z() + 0.000001f)
			|| a.z() < (b.z() - 0.000001f)) ||
		(a.h() > (b.h() + 0.000001f)
			|| a.h() < (b.h() - 0.000001f)))
			return false;
	return true;
}



using Aqsis::CqVector4D;

BOOST_AUTO_TEST_CASE(vector4d_magnitude)
{
	const CqVector4D vec(0.3f, 0.4f, 1.6f, 2.0f);
	
	BOOST_CHECK_EQUAL(vec.Magnitude(), 0.838152766f);
}

BOOST_AUTO_TEST_CASE(vector4d_magnitude2)
{
	const CqVector4D vec(0.3f, 0.4f, 1.6f, 2.0f);
	
	BOOST_CHECK_EQUAL(vec.Magnitude2(), 0.702500045f);
}

BOOST_AUTO_TEST_CASE(vector4d_unit)
{
	CqVector4D vec(0.3f, 0.4f, 0.6f, 1.0f);	
	
	vec.Unit();
	
	BOOST_CHECK_PREDICATE(equals, (vec)(CqVector4D(0.3f, 0.4f, 0.6f, 0.781025f)));
}

BOOST_AUTO_TEST_CASE(vector4d_operator_addition_vector)
{
	const CqVector4D vec1(0.3f, 0.4f, 2.3f, 0.8f);
	const CqVector4D vec2(1.2f, 0.3f, 1.6f, 0.2f);
	
	BOOST_CHECK_PREDICATE(equals, (vec1 + vec2)(CqVector4D(5.1f, 1.6f, 8.7f, 0.8f)));
}

BOOST_AUTO_TEST_CASE(vector4d_operator_substraction_vector)
{
	const CqVector4D vec1(0.3f, 0.4f, 2.3f, 0.8f);
	const CqVector4D vec2(1.2f, 0.3f, 1.6f, 0.2f);
	
	BOOST_CHECK_PREDICATE(equals, (vec1 - vec2)(CqVector4D(-4.5f, -0.8f, -4.1f, 0.8f)));
}

BOOST_AUTO_TEST_CASE(vector4d_operator_multiplication_vector_with_float)
{
	const CqVector4D vec(0.3f, 0.4f, 2.3f, 0.8f);
		
	BOOST_CHECK_PREDICATE(equals, (vec * 1.3f)(CqVector4D(0.39f, 0.52f, 2.99f, 0.8f)));
}

BOOST_AUTO_TEST_CASE(vector4d_operator_multiplication_float_with_vector)
{
	const CqVector4D vec(0.3f, 0.4f, 2.3f, 0.8f);
		
	BOOST_CHECK_PREDICATE(equals, (1.3f * vec)(CqVector4D(0.39f, 0.52f, 2.99f, 0.8f)));
}

BOOST_AUTO_TEST_CASE(vector4d_operator_multiplication_vector_with_vector)
{
	const CqVector4D vec1(0.3f, 0.4f, 2.3f, 0.8f);
	const CqVector4D vec2(1.2f, 0.3f, 1.6f, 0.2f);
		
	BOOST_CHECK_EQUAL((vec1 * vec2), 26.0f);
}

BOOST_AUTO_TEST_CASE(vector4d_operator_division_vector_by_float)
{
	const CqVector4D vec(1.625f, 3.25f, 1.0f, 1.0f);
		
	BOOST_CHECK_PREDICATE(equals, (vec / 1.3f)(CqVector4D(1.625f, 3.25f, 1.0f, 1.3f)));
}

BOOST_AUTO_TEST_CASE(vector4d_operator_division_float_by_vector)
{
	const CqVector4D vec(0.8f, 0.4f, 1.0f, 1.0f);
		
	BOOST_CHECK_PREDICATE(equals, (1.3f / vec)(CqVector4D(0.8f, 0.4f, 1.0f, 1.3f)));
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
