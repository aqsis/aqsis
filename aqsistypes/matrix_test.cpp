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
 * \brief Unit tests for CqMatrix
 * \author Tobias Sauerwein
 */

#include "matrix.h"
#include "vector3d.h"


#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>



bool equals(const Aqsis::CqMatrix& a, const Aqsis::CqMatrix& b)
{
	for (int i=0; i<4; ++i)
	{
		for (int j=0; j<4; ++j)
		{
			if (a.Element(i, j) > (b.Element(i, j) + 0.000001f)
				|| a.Element(i, j) < (b.Element(i, j) - 0.000001f))
				return false;
		}
	}
	return true;
}


BOOST_AUTO_TEST_CASE(CqMatrix_SetElement_test)
{
	Aqsis::CqMatrix mat(  0, 0, 0, 0,
						0, 0, 0, 0,
						0, 0, 0, 0,
						0, 0, 0, 0);
	
	mat.SetElement(0, 1, 0.5432f);
	
	BOOST_CHECK_CLOSE(mat.Element(0, 1), 0.5432f, 0.1f);
}

BOOST_AUTO_TEST_CASE(CqMatrix_Operator_addition_test)
{
	Aqsis::CqMatrix mat_a(  1.3f, -2.4f, 1.0f, 1.02f, 
							2.6f, 3.1f, 0.03f, 14.0f, 
							0.4f, 2.0f, 1.204f, 1.5f, 
							1.2f, 1.4f, 1.30f, 2.0f);
						
	Aqsis::CqMatrix mat_b(  0.4f, 2.0f, 1.204f, 1.5f, 
							1.2f, 1.4f, 1.30f, 2.0f, 
							1.3f, -2.4f, 1.0f, 1.02f, 
							2.6f, 3.1f, 0.03f, 14.0f);
							
	Aqsis::CqMatrix result(  1.7f, -0.4f, 2.204f, 2.52f, 
							 3.8f,  4.5f, 1.33f,  16.0f, 
							 1.7f, -0.4f,  2.204f, 2.52f, 
							 3.8f,  4.5f,  1.33f, 16.0f);

	BOOST_CHECK_PREDICATE(equals, (mat_a + mat_b)(result));
}

BOOST_AUTO_TEST_CASE(CqMatrix_Operator_substraction_test)
{
	Aqsis::CqMatrix mat_a(  1.3f, -2.4f, 1.0f, 1.02f, 
							2.6f, 3.1f, 0.03f, 14.0f, 
							0.4f, 2.0f, 1.204f, 1.5f, 
							1.2f, 1.4f, 1.30f, 2.0f);
	
	Aqsis::CqMatrix mat_b(  0.4f, 2.0f, 1.204f, 1.5f, 
							1.2f, 1.4f, 1.30f, 2.0f, 
							1.3f, -2.4f, 1.0f, 1.02f, 
							2.6f, 3.1f, 0.03f, 14.0f);
	
	Aqsis::CqMatrix result(  0.9f, -4.4f, -0.204f, -0.48f, 
							 1.4f,  1.7f, -1.27f,  12.0f, 
							-0.9f,  4.4f,  0.204f, 0.48f, 
							-1.4f, -1.7f,  1.27f, -12.0f);
		
	BOOST_CHECK_PREDICATE(equals, (mat_a - mat_b)(result));
}

BOOST_AUTO_TEST_CASE(CqMatrix_Operator_multiplication_test)
{
	Aqsis::CqMatrix mat_a(  1.0f, 2.0f, 1.0f, 1.0f, 
							2.0f, 3.0f, 2.0f, 4.0f, 
							0.0f, 2.0f, 1.0f, 1.0f, 
							1.0f, 1.0f, 1.0f, 2.0f);
	
	Aqsis::CqMatrix mat_b(  0.0f, 2.0f, 1.0f, 1.0f, 
							1.0f, 1.0f, 1.0f, 2.0f, 
							1.0f, 2.0f, 1.0f, 1.0f, 
							2.0f, 3.0f, 0.0f, 1.0f);

	Aqsis::CqMatrix result(  5.0f,  9.0f, 6.0f, 11.0f, 
							 5.0f,  9.0f, 6.0f, 10.0f, 
							 6.0f, 11.0f, 7.0f, 12.0f, 
							 9.0f, 14.0f, 9.0f, 16.0f);
		
	BOOST_CHECK_PREDICATE(equals, (mat_a * mat_b)(result));
}

BOOST_AUTO_TEST_CASE(CqMatrix_Operator_premultiplication_test)
{
	Aqsis::CqMatrix mat_a(  1.0f, 2.0f, 1.0f, 1.0f, 
							2.0f, 3.0f, 2.0f, 4.0f, 
							0.0f, 2.0f, 1.0f, 1.0f, 
							1.0f, 1.0f, 1.0f, 2.0f);
	
	Aqsis::CqMatrix mat_b(  0.0f, 2.0f, 1.0f, 1.0f, 
							1.0f, 1.0f, 1.0f, 2.0f, 
							1.0f, 2.0f, 1.0f, 1.0f, 
							2.0f, 3.0f, 0.0f, 1.0f);
	
	Aqsis::CqMatrix result(  5.0f,  9.0f, 4.0f,  7.0f, 
							13.0f, 23.0f, 7.0f, 14.0f, 
							 5.0f,  7.0f, 3.0f,  6.0f, 
							 6.0f, 11.0f, 3.0f,  6.0f);
	
	BOOST_CHECK_PREDICATE(equals, (mat_a.PreMultiply(mat_b))(result));
}

BOOST_AUTO_TEST_CASE(CqMatrix_Equals_test)
{
	Aqsis::CqMatrix mat_a(  1.0f, 2.0f, 1.0f, 1.0f, 
							2.0f, 3.0f, 2.0f, 4.0f, 
							0.0f, 2.0f, 1.0f, 1.0f, 
							1.0f, 1.0f, 1.0f, 2.0f);
	
	Aqsis::CqMatrix mat_a2(  1.0f, 2.0f, 1.0f, 1.0f, 
							2.0f, 3.0f, 2.0f, 4.0f, 
							0.0f, 2.0f, 1.0f, 1.0f, 
							1.0f, 1.0f, 1.0f, 2.0f);
								
	Aqsis::CqMatrix mat_b(  0.0f, 2.0f, 1.0f, 1.0f, 
							2.0f, 3.0f, 2.0f, 4.0f, 
							0.0f, 2.0f, 1.0f, 1.0f, 
							1.0f, 1.0f, 1.0f, 2.0f);

	BOOST_CHECK(mat_a == mat_a2);
	BOOST_CHECK(!(mat_a == mat_b));
}

BOOST_AUTO_TEST_CASE(CqMatrix_Unequals_test)
{
	Aqsis::CqMatrix mat_a(  1.0f, 2.0f, 1.0f, 1.0f, 
							2.0f, 3.0f, 2.0f, 4.0f, 
							0.0f, 2.0f, 1.0f, 1.0f, 
							1.0f, 1.0f, 1.0f, 2.0f);
							
	Aqsis::CqMatrix mat_b(  0.0f, 2.0f, 1.0f, 1.0f, 
							2.0f, 3.0f, 2.0f, 4.0f, 
							0.0f, 2.0f, 1.0f, 1.0f, 
							1.0f, 1.0f, 1.0f, 2.0f);

	Aqsis::CqMatrix mat_c(  1.0f, 2.0f, 1.0f, 1.0f, 
							2.0f, 3.0f, 2.0f, 4.0f, 
							0.0f, 2.0f, 1.0f, 1.0f, 
							1.0f, 1.0f, 1.0f, 2.0f);
	
	BOOST_CHECK(mat_a != mat_b);
	BOOST_CHECK(!(mat_a != mat_c));
}


BOOST_AUTO_TEST_CASE(CqMatrix_Identity_test)
{
	Aqsis::CqMatrix mat_a(  1.0f, 2.0f, 1.0f, 1.0f, 
							2.0f, 3.0f, 2.0f, 4.0f, 
							0.0f, 2.0f, 1.0f, 1.0f, 
							1.0f, 1.0f, 1.0f, 2.0f);
		
	Aqsis::CqMatrix result( 1.0f, 0.0f, 0.0f, 0.0f, 
							0.0f, 1.0f, 0.0f, 0.0f, 
							0.0f, 0.0f, 1.0f, 0.0f, 
							0.0f, 0.0f, 0.0f, 1.0f);
	
	mat_a.Identity();
	
	BOOST_CHECK_PREDICATE(equals, (mat_a)(result));
}

BOOST_AUTO_TEST_CASE(CqMatrix_Scale_test)
{
	Aqsis::CqMatrix mat_a(  1.0f, 2.0f, 1.0f, 1.0f, 
							2.0f, 3.0f, 2.0f, 4.0f, 
							0.0f, 2.0f, 1.0f, 1.0f, 
							1.0f, 1.0f, 1.0f, 2.0f);
		
	Aqsis::CqMatrix result( 2.0f, 4.0f, 2.0f, 1.0f, 
							4.0f, 6.0f, 4.0f, 4.0f, 
							0.0f, 4.0f, 2.0f, 1.0f, 
							2.0f, 2.0f, 2.0f, 2.0f);
	
	mat_a.Scale(2.0f);
	
	BOOST_CHECK_PREDICATE(equals, (mat_a)(result));
}

BOOST_AUTO_TEST_CASE(CqMatrix_Rotate_test)
{
	Aqsis::CqMatrix mat_a(  1.0f, 0.0f, 1.0f, 1.0f, 
							2.0f, 3.0f, 0.0f, 0.0f, 
							0.0f, 2.0f, 0.0f, 1.0f, 
							0.0f, 1.0f, 1.0f, 1.0f);
		
	Aqsis::CqMatrix result( -0.448074f, 0.0f, -0.448074f, 1.0f,
							-0.896147f, -1.34422f, 0.0f, 0.0f,
							0.0f, -0.896147f, 0.0f, 1.0f,
							0.0f, -0.448074f, -0.448074f, 1.0f);
	
	mat_a.Rotate(90.0f, (1.0f, 0.0f, 0.0f));
	
	BOOST_CHECK_PREDICATE(equals, (mat_a)(result));
}

BOOST_AUTO_TEST_CASE(CqMatrix_Translate_by_vectors_test)
{
	Aqsis::CqMatrix mat_a(  1.0f, 0.0f, 1.0f, 1.0f, 
							2.0f, 3.0f, 0.0f, 0.0f, 
							0.0f, 2.0f, 0.0f, 1.0f, 
							0.0f, 1.0f, 1.0f, 1.0f);
		
	Aqsis::CqMatrix result( 3.0f, 1.0f, 4.0f, 1.0f, 
							2.0f, 3.0f, 0.0f, 0.0f, 
							2.0f, 3.0f, 3.0f, 1.0f, 
							2.0f, 2.0f, 4.0f, 1.0f);
	
	mat_a.Translate(Aqsis::CqVector3D(2.0f, 1.0f, 3.0f));
	
	BOOST_CHECK_PREDICATE(equals, (mat_a)(result));
}

BOOST_AUTO_TEST_CASE(CqMatrix_Translate_by_floats_test)
{
	Aqsis::CqMatrix mat_a(  1.0f, 0.0f, 1.0f, 1.0f, 
							2.0f, 3.0f, 0.0f, 0.0f, 
							0.0f, 2.0f, 0.0f, 1.0f, 
							0.0f, 1.0f, 1.0f, 1.0f);
		
	Aqsis::CqMatrix result( 3.0f, 1.0f, 4.0f, 1.0f, 
							2.0f, 3.0f, 0.0f, 0.0f, 
							2.0f, 3.0f, 3.0f, 1.0f, 
							2.0f, 2.0f, 4.0f, 1.0f);
	
	mat_a.Translate(2.0f, 1.0f, 3.0f);
	
	BOOST_CHECK_PREDICATE(equals, (mat_a)(result));
}

BOOST_AUTO_TEST_CASE(CqMatrix_Shear_in_X_test)
{
	Aqsis::CqMatrix mat_a(  1.0f, 2.0f, 2.0f, 1.0f, 
							2.0f, 7.0f, 2.0f, 0.0f, 
							0.0f, 2.0f, 0.0f, 1.0f, 
							0.0f, 1.0f, 1.0f, 1.0f);
		
	Aqsis::CqMatrix result( 1.0f, 4.0f, 3.0f, 1.0f, 
							2.0f, 11.0f, 4.0f, 0.0f, 
							0.0f, 2.0f, 0.0f, 1.0f, 
							0.0f, 1.0f, 1.0f, 1.0f);
	
	mat_a.ShearX(2.0f, 1.0f);
	
	BOOST_CHECK_PREDICATE(equals, (mat_a)(result));
}

BOOST_AUTO_TEST_CASE(CqMatrix_Shear_in_Y_test)
{
	Aqsis::CqMatrix mat_a(  1.0f, 0.0f, 1.0f, 1.0f, 
							2.0f, 3.0f, 0.0f, 0.0f, 
							0.0f, 2.0f, 0.0f, 1.0f, 
							0.0f, 1.0f, 1.0f, 1.0f);
		
	Aqsis::CqMatrix result( 1.0f, 0.0f, 1.0f, 1.0f, 
							8.0f, 3.0f, 3.0f, 0.0f, 
							4.0f, 2.0f, 2.0f, 1.0f, 
							2.0f, 1.0f, 2.0f, 1.0f);
	
	mat_a.ShearY(2.0f, 1.0f);
	
	BOOST_CHECK_PREDICATE(equals, (mat_a)(result));
}

BOOST_AUTO_TEST_CASE(CqMatrix_Shear_in_Z_test)
{
	Aqsis::CqMatrix mat_a(  1.0f, 0.0f, 1.0f, 1.0f, 
							2.0f, 3.0f, 0.0f, 0.0f, 
							0.0f, 2.0f, 0.0f, 1.0f, 
							0.0f, 1.0f, 1.0f, 1.0f);
		
	Aqsis::CqMatrix result( 3.0f, 1.0f, 1.0f, 1.0f, 
							2.0f, 3.0f, 0.0f, 0.0f, 
							0.0f, 2.0f, 0.0f, 1.0f, 
							2.0f, 2.0f, 1.0f, 1.0f);
	
	mat_a.ShearZ(2.0f, 1.0f);
	
	BOOST_CHECK_PREDICATE(equals, (mat_a)(result));
}

BOOST_AUTO_TEST_CASE(CqMatrix_Skew_test)
{
	Aqsis::CqMatrix mat_a(  1.0f, 0.0f, 1.0f, 1.0f, 
							2.0f, 3.0f, 0.0f, 0.0f, 
							0.0f, 2.0f, 0.0f, 1.0f, 
							0.0f, 1.0f, 1.0f, 1.0f);
		
	Aqsis::CqMatrix result( 1.0f, 0.0f, 1.0f, 1.0f, 
							2.0f, 3.0f, 0.0f, 0.0f, 
							0.0f, 2.0f, 0.0f, 1.0f, 
							0.0f, 1.0f, 1.0f, 1.0f);
	
	mat_a.Skew(25.0f, 2.0f, 1.0f, 3.0f, -1.0f, 2.0f, -1.0f);
	
	BOOST_CHECK_PREDICATE(equals, (mat_a)(result));
}

BOOST_AUTO_TEST_CASE(CqMatrix_Normalise_test)
{
	Aqsis::CqMatrix mat_a(  1.0f, 0.0f, 1.0f, 1.0f, 
							2.0f, 3.0f, 0.0f, 0.0f, 
							0.0f, 2.0f, 0.0f, 1.0f, 
							0.0f, 1.0f, 1.0f, 4.0f);
		
	Aqsis::CqMatrix result( 0.25f, 0.0f, 0.25f, 0.25f, 
							0.5f, 0.75f, 0.0f, 0.0f, 
							0.0f, 0.5f, 0.0f, 0.25f, 
							0.0f, 0.25f, 0.25f, 1.0f);
	
	mat_a.Normalise();
	
	BOOST_CHECK_PREDICATE(equals, (mat_a)(result));
}

BOOST_AUTO_TEST_CASE(CqMatrix_Inverse_test)
{
	Aqsis::CqMatrix mat_a(  4.0f, 0.0f, 1.0f, 0.0f, 
							0.0f, 1.0f, 0.0f, 0.0f, 
							3.0f, 0.0f, 1.0f, 0.0f, 
							0.0f, 1.0f, 0.0f, 1.0f);
		
	Aqsis::CqMatrix result( 1.0f, 0.0f, -1.0f, 0.0f, 
							0.0f, 1.0f, 0.0f, 0.0f, 
							-3.0f, 0.0f, 4.0f, 0.0f, 
							0.0f, -1.0f, 0.0f, 1.0f);
		
	mat_a = mat_a.Inverse();

	BOOST_CHECK_PREDICATE(equals, (mat_a)(result));
}

BOOST_AUTO_TEST_CASE(CqMatrix_Transpose_test)
{
	Aqsis::CqMatrix mat_a(  1.0f, 0.0f, 1.0f, 1.0f, 
							2.0f, 3.0f, 0.0f, 0.0f, 
							0.0f, 2.0f, 0.0f, 1.0f, 
							0.0f, 1.0f, 1.0f, 1.0f);
		
	Aqsis::CqMatrix result( 1.0f, 2.0f, 0.0f, 0.0f, 
							0.0f, 3.0f, 2.0f, 1.0f, 
							1.0f, 0.0f, 0.0f, 1.0f, 
							1.0f, 0.0f, 1.0f, 1.0f);	
	
	BOOST_CHECK_PREDICATE(equals, (mat_a.Transpose())(result));
}

BOOST_AUTO_TEST_CASE(CqMatrix_Determinant_test)
{
	Aqsis::CqMatrix mat_a(  1.0f, 0.0f, 1.0f, 1.0f, 
							2.0f, 3.0f, 0.0f, 0.0f, 
							0.0f, 2.0f, 0.0f, 1.0f, 
							0.0f, 1.0f, 1.0f, 1.0f);
		
	BOOST_CHECK_EQUAL(mat_a.Determinant(), -5.0f);
}
