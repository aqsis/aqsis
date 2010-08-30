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

#include <aqsis/math/cellnoise.h>
#include <aqsis/math/vector3d.h>

#define BOOST_TEST_DYN_LINK

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

BOOST_AUTO_TEST_SUITE(cellnoise_tests)

const TqFloat epsilon = 0.1f; 


static bool equal(const Aqsis::CqVector3D& a, const Aqsis::CqVector3D& b)
{
	return Aqsis::isClose(a, b);
}

#define CHECK_VEC_CLOSE(v1, v2) BOOST_CHECK_PREDICATE(equal, (v1)(v2))


BOOST_AUTO_TEST_CASE(CqCellNoise_1D_float_cellnoise_test)
{
	Aqsis::CqCellNoise cn;
	
	BOOST_CHECK_CLOSE(cn.FCellNoise1(1.0f), 0.832858741f, epsilon);
}

BOOST_AUTO_TEST_CASE(CqCellNoise_2D_float_cellnoise_test)
{
	Aqsis::CqCellNoise cn;
	
	BOOST_CHECK_CLOSE(cn.FCellNoise2(1.0f, 1.0f), 0.00305117574f, epsilon);
}

BOOST_AUTO_TEST_CASE(CqCellNoise_3D_float_cellnoise_test)
{
	Aqsis::CqCellNoise cn;
	
	BOOST_CHECK_CLOSE(cn.FCellNoise3(Aqsis::CqVector3D(1.0f, 1.0f, 1.0f)), 0.251191825f, epsilon);
}

BOOST_AUTO_TEST_CASE(CqCellNoise_4D_float_cellnoise_test)
{
	Aqsis::CqCellNoise cn;
	
	BOOST_CHECK_CLOSE(cn.FCellNoise4(Aqsis::CqVector3D(1.0f, 1.0f, 1.0f), 1.0f), 0.670210421f, epsilon);
}

BOOST_AUTO_TEST_CASE(CqCellNoise_1D_point_cellnoise_test)
{
	Aqsis::CqCellNoise cn;
	
	CHECK_VEC_CLOSE(cn.PCellNoise1(1.0f),
			Aqsis::CqVector3D(0.832859f, 0.782803f, 0.0354029f));
}

BOOST_AUTO_TEST_CASE(CqCellNoise_2D_point_cellnoise_test)
{
	Aqsis::CqCellNoise cn;
	
	CHECK_VEC_CLOSE(cn.PCellNoise2(1.0f, 1.0f),
			Aqsis::CqVector3D(0.00305118f, 0.327067f, 0.917067f));
}

BOOST_AUTO_TEST_CASE(CqCellNoise_3D_point_cellnoise_test)
{
	Aqsis::CqCellNoise cn;
	
	CHECK_VEC_CLOSE(cn.PCellNoise3(Aqsis::CqVector3D(1.0f, 1.0f, 1.0f)),
			Aqsis::CqVector3D(0.251192f, 0.524417f ,0.0602106f));
}

BOOST_AUTO_TEST_CASE(CqCellNoise_4D_point_cellnoise_test)
{
	Aqsis::CqCellNoise cn;
	
	CHECK_VEC_CLOSE(cn.PCellNoise4(Aqsis::CqVector3D(1.0f, 1.0f, 1.0f), 1.0f),
			Aqsis::CqVector3D(0.67021f, 0.930112f, 0.82147f));
}

BOOST_AUTO_TEST_SUITE_END()
