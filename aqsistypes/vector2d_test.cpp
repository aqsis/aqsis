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

#include "vector2d.h"

#ifdef AQSIS_SYSTEM_MACOSX
    #define BOOST_AUTO_TEST_MAIN
	#include <boost/test/included/unit_test.hpp>
#else
	#include <boost/test/auto_unit_test.hpp>
#endif

#include <boost/test/floating_point_comparison.hpp>



static bool equals(const Aqsis::CqVector2D& a, const Aqsis::CqVector2D& b)
{
	if ((a.x() > (b.x() + 0.000001f)
			|| a.x() < (b.x() - 0.000001f)) ||
		(a.y() > (b.y() + 0.000001f)
			|| a.y() < (b.y() - 0.000001f)))
			return false;
	return true;
}



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
	
	BOOST_CHECK_PREDICATE(equals, (vec1 + vec2)(CqVector2D(1.5f, 0.7f)));
}

BOOST_AUTO_TEST_CASE(vector2d_operator_addition_float_to_vector)
{
	const CqVector2D vec(0.3f, 0.4f);
	
	BOOST_CHECK_PREDICATE(equals, (vec + 2.4f)(CqVector2D(2.7f, 2.8f)));
}

BOOST_AUTO_TEST_CASE(vector2d_operator_addition_vector_to_float)
{
	const CqVector2D vec(0.3f, 0.4f);
	
	BOOST_CHECK_PREDICATE(equals, (2.4f + vec)(CqVector2D(2.7f, 2.8f)));
}

BOOST_AUTO_TEST_CASE(vector2d_operator_substraction_vector)
{
	const CqVector2D vec1(0.3f, 0.4f);
	const CqVector2D vec2(1.2f, 0.3f);
	
	BOOST_CHECK_PREDICATE(equals, (vec1 - vec2)(CqVector2D(-0.9f, 0.1f)));
}

BOOST_AUTO_TEST_CASE(vector2d_operator_substraction_float_from_vector)
{
	const CqVector2D vec(3.3f, 1.9f);
	
	BOOST_CHECK_PREDICATE(equals, (vec - 2.4f)(CqVector2D(0.9f, -0.5f)));
}

BOOST_AUTO_TEST_CASE(vector2d_operator_substraction_vector_from_float)
{
	const CqVector2D vec(3.3f, 1.9f);
	
	BOOST_CHECK_PREDICATE(equals, (2.4f - vec)(CqVector2D(-0.9f, 0.5f)));
}

BOOST_AUTO_TEST_CASE(vector2d_operator_multiplication_vector_with_float)
{
	const CqVector2D vec(0.3f, 0.4f);
		
	BOOST_CHECK_PREDICATE(equals, (vec * 1.3f)(CqVector2D(0.39f, 0.52f)));
}

BOOST_AUTO_TEST_CASE(vector2d_operator_multiplication_float_with_vector)
{
	const CqVector2D vec(0.3f, 0.4f);
		
	BOOST_CHECK_PREDICATE(equals, (1.3f * vec)(CqVector2D(0.39f, 0.52f)));
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
		
	BOOST_CHECK_PREDICATE(equals, (vec / 1.3f)(CqVector2D(1.25f, 2.5f)));
}

BOOST_AUTO_TEST_CASE(vector2d_operator_division_float_by_vector)
{
	const CqVector2D vec(0.8f, 0.4f);
		
	BOOST_CHECK_PREDICATE(equals, (1.3f / vec)(CqVector2D(1.625f, 3.25f)));
}

BOOST_AUTO_TEST_CASE(vector2d_operator_division_vector_by_vector)
{
	const CqVector2D vec1(0.6f, 0.2f);
	const CqVector2D vec2(0.3f, 0.4f);
		
	BOOST_CHECK_PREDICATE(equals, (vec1 / vec2)(CqVector2D(2.0f, 0.5f)));
}
