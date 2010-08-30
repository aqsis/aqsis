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
 * \brief Unit tests for SqMatrix2D
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#include <aqsis/math/matrix2d.h>

#define BOOST_TEST_DYN_LINK

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

BOOST_AUTO_TEST_SUITE(matrix2d_tests)

static const TqFloat closeEps = 1e-5f;

BOOST_AUTO_TEST_CASE(SqMatrix2D_inv_test)
{
	Aqsis::SqMatrix2D A(1,2,3,4);
	Aqsis::SqMatrix2D Ainv = A.inv();
	Aqsis::SqMatrix2D I = A*Ainv;
	BOOST_CHECK_CLOSE(I.a, 1.0f, closeEps);
	BOOST_CHECK_SMALL(I.b, closeEps);
	BOOST_CHECK_SMALL(I.c, closeEps);
	BOOST_CHECK_CLOSE(I.d, 1.0f, closeEps);
}

BOOST_AUTO_TEST_CASE(SqMatrix2D_eigenvalue_test)
{
	TqFloat l1, l2;

	Aqsis::SqMatrix2D A(1);
	A.eigenvalues(l1,l2);
	BOOST_CHECK_CLOSE(l1, 1.0f, closeEps);
	BOOST_CHECK_CLOSE(l2, 1.0f, closeEps);

	A = Aqsis::SqMatrix2D(1,2);
	A.eigenvalues(l1,l2);
	BOOST_CHECK_CLOSE(l1, 2.0f, closeEps);
	BOOST_CHECK_CLOSE(l2, 1.0f, closeEps);

	A = Aqsis::SqMatrix2D(1,1, 1,2);
	A.eigenvalues(l1,l2);
	BOOST_CHECK_CLOSE(l1, TqFloat((3+std::sqrt(5.0f))/2), closeEps);
	BOOST_CHECK_CLOSE(l2, TqFloat((3-std::sqrt(5.0f))/2), closeEps);
}

BOOST_AUTO_TEST_CASE(SqMatrix2D_orthogDiagonalize_test)
{
	{
		TqFloat l1, l2;
		Aqsis::SqMatrix2D A = Aqsis::SqMatrix2D(1,2);
		A.eigenvalues(l1,l2);
		Aqsis::SqMatrix2D R = A.orthogDiagonalize(l1,l2);
		Aqsis::SqMatrix2D D = R.transpose()*A*R;
		BOOST_CHECK_CLOSE(D.a, l1, closeEps);
		BOOST_CHECK_CLOSE(D.d, l2, closeEps);
		BOOST_CHECK_SMALL(D.b, closeEps);
		BOOST_CHECK_SMALL(D.c, closeEps);
	}

	{
		TqFloat l1, l2;
		Aqsis::SqMatrix2D A(1,1, 1,2);
		A.eigenvalues(l1,l2);
		Aqsis::SqMatrix2D R = A.orthogDiagonalize(l1,l2);
		Aqsis::SqMatrix2D D = R.transpose()*A*R;
		BOOST_CHECK_CLOSE(D.a, TqFloat((3+std::sqrt(5.0f))/2), closeEps);
		BOOST_CHECK_CLOSE(D.d, TqFloat((3-std::sqrt(5.0f))/2), closeEps);
		BOOST_CHECK_SMALL(D.b, closeEps);
		BOOST_CHECK_SMALL(D.c, closeEps);
	}
}

BOOST_AUTO_TEST_SUITE_END()
