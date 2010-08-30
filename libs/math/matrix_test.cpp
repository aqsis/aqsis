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

#include <aqsis/math/matrix.h>
#include <aqsis/math/vector3d.h>

#define BOOST_TEST_DYN_LINK

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

BOOST_AUTO_TEST_SUITE(matrix4d_tests)


static bool equals(const Aqsis::CqMatrix& a, const Aqsis::CqMatrix& b)
{
	return isClose(a, b);
}

#define CHECK_MATRIX_CLOSE(m1, m2) BOOST_CHECK_PREDICATE(equals, (m1)(m2))

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

	CHECK_MATRIX_CLOSE(mat_a + mat_b, result);
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
		
	CHECK_MATRIX_CLOSE(mat_a - mat_b, result);
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
		
	CHECK_MATRIX_CLOSE(mat_a * mat_b, result);
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
	
	CHECK_MATRIX_CLOSE(mat_a.PreMultiply(mat_b), result);
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
	
	CHECK_MATRIX_CLOSE(mat_a, result);
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
	
	CHECK_MATRIX_CLOSE(mat_a, result);
}

BOOST_AUTO_TEST_CASE(CqMatrix_Rotate_test)
{
	Aqsis::CqMatrix mat_a(  1.0f, 0.0f, 1.0f, 1.0f, 
							2.0f, 3.0f, 0.0f, 0.0f, 
							0.0f, 2.0f, 0.0f, 1.0f, 
							0.0f, 1.0f, 1.0f, 1.0f);
		
	Aqsis::CqMatrix result( 1,-0.8939966559,-0.4480736256,1,
							2,-1.344220877,2.681989908,0,
							0,-0.8961472511,1.787993312,1,
							0,-1.342070341,0.4459230304,1);

	mat_a.Rotate(90.0f, Aqsis::CqVector3D(1.0f, 0.0f, 0.0f));
	
	CHECK_MATRIX_CLOSE(mat_a, result);
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
	
	CHECK_MATRIX_CLOSE(mat_a, result);
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
	
	CHECK_MATRIX_CLOSE(mat_a, result);
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
	
	CHECK_MATRIX_CLOSE(mat_a, result);
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
	
	CHECK_MATRIX_CLOSE(mat_a, result);
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
	
	CHECK_MATRIX_CLOSE(mat_a, result);
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
	
	CHECK_MATRIX_CLOSE(mat_a, result);
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
	
	CHECK_MATRIX_CLOSE(mat_a, result);
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

	CHECK_MATRIX_CLOSE(mat_a, result);
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
	
	CHECK_MATRIX_CLOSE(mat_a.Transpose(), result);
}

BOOST_AUTO_TEST_CASE(CqMatrix_Determinant_test)
{
	Aqsis::CqMatrix mat_a(  1.0f, 0.0f, 1.0f, 1.0f, 
							2.0f, 3.0f, 0.0f, 0.0f, 
							0.0f, 2.0f, 0.0f, 1.0f, 
							0.0f, 1.0f, 1.0f, 1.0f);
		
	BOOST_CHECK_EQUAL(mat_a.Determinant(), -5.0f);
}

BOOST_AUTO_TEST_CASE(matrix_isClose)
{
	const Aqsis::CqMatrix m1(1.3, -2.4, 1.0, 1.02, 
							 1.6, 1.1, 0.03, 1.0, 
							 0.4, 2.0, 1.204, 1.5, 
							 1.2, 1.4, 1.30, 0.8);
	// matrix 2-norm of m1 above is approx 5.2.  Keep this in mind when testing
	// closeness...
	BOOST_CHECK(isClose(m1, m1));
	BOOST_CHECK(isClose(m1, (m1*100.0) * (1/100.0)));

	const Aqsis::CqMatrix mEps( 0, 0, 0, 0,
								0, 0, 0, 0,
								0, 0, 0, 0,
								0, 0, 0, 1e-4);
	BOOST_CHECK(!isClose(m1, m1 + mEps));
	BOOST_CHECK(!isClose(m1, m1 + mEps, 1e-5));
	BOOST_CHECK(isClose(m1, m1 + mEps, 1e-4));
	BOOST_CHECK(isClose(m1, m1 - mEps, 1e-4));

	const Aqsis::CqMatrix mEps2( 1e-6, 1e-6, 1e-6, 1e-6,
								1e-6, 1e-6, 1e-6, 1e-6,
								1e-6, 1e-6, 1e-4, 1e-6,
								1e-6, 1e-6, 1e-6, 1e-6);
	BOOST_CHECK(!isClose(m1, m1 + mEps2));
	BOOST_CHECK(!isClose(m1, m1 + mEps2, 1e-5));
	BOOST_CHECK(isClose(m1, m1 + mEps2, 1e-4));
	BOOST_CHECK(isClose(m1, m1 - mEps2, 1e-4));

	// Special case tests for matrices which are marked as the identity.
	Aqsis::CqMatrix m2;
	Aqsis::CqMatrix m3;
	BOOST_CHECK(isClose(m2, m3));
	m3.SetfIdentity(false);
	BOOST_CHECK(isClose(m2, m3));

	// Zero matrices should also be equal
	BOOST_CHECK(isClose(Aqsis::CqMatrix(0.0f), Aqsis::CqMatrix(0.0f)));
}

BOOST_AUTO_TEST_SUITE_END()
