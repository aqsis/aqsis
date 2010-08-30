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
 * \brief Unit tests for CqVector3D and related stuff
 * \author Chris Foster
 */

#include <aqsis/math/vector3d.h>

#define BOOST_TEST_DYN_LINK

#include <boost/test/auto_unit_test.hpp>
//#include <boost/test/floating_point_comparison.hpp>

BOOST_AUTO_TEST_SUITE(vector3d_tests)

using namespace Aqsis;

//------------------------------------------------------------------------------
// Test tools

// Forwarding functor to test whether two vectors are close to equal.  Needed
// because the overloaded isClose() function doesn't seem to work with
// BOOST_CHECK_PREDICATE.
struct ClosePred
{
	template<typename T1, typename T2>
	bool operator()(const T1& v1, const T2& v2)
	{
		return isClose(v1, v2);
	}
};

#define CHECK_VEC_CLOSE(v1, v2) BOOST_CHECK_PREDICATE(ClosePred(), (v1)(v2))
//------------------------------------------------------------------------------


BOOST_AUTO_TEST_CASE(vector3d_min)
{
	const CqVector3D vMin(0.3,0.4,0.5);
	const CqVector3D vMax(0.4,0.5,0.6);
	CHECK_VEC_CLOSE(min(vMin, vMax), vMin);
	CHECK_VEC_CLOSE(min(vMax, vMin), vMin);
	CHECK_VEC_CLOSE(min(CqVector3D(1,0.5,0), CqVector3D(0.2,0.5,0.8)), CqVector3D(0.2,0.5,0));
}

BOOST_AUTO_TEST_CASE(vector3d_max)
{
	const CqVector3D vMin(0.3,0.4,0.5);
	const CqVector3D vMax(0.4,0.5,0.6);
	CHECK_VEC_CLOSE(max(vMin, vMax), vMax);
	CHECK_VEC_CLOSE(max(vMax, vMin), vMax);
	CHECK_VEC_CLOSE(max(CqVector3D(1,0.5,0), CqVector3D(0.2,0.5,0.8)), CqVector3D(1,0.5,0.8));
}

BOOST_AUTO_TEST_CASE(vector3d_clamp)
{
	const CqVector3D vMin(0.3,0.4,0.5);
	const CqVector3D vMax(0.4,0.5,0.6);
	CHECK_VEC_CLOSE(clamp(CqVector3D(0,0,0), vMin, vMax), vMin);
	CHECK_VEC_CLOSE(clamp(CqVector3D(1,1,1), vMin, vMax), vMax);
	const CqVector3D vMiddle = (vMin+vMax)/2;
	CHECK_VEC_CLOSE(clamp(vMiddle, vMin, vMax), vMiddle);
	CHECK_VEC_CLOSE(clamp(CqVector3D(0.9, 0.45, 0.1), vMin, vMax), CqVector3D(0.4, 0.45, 0.5));
}

BOOST_AUTO_TEST_CASE(vector3d_lerp)
{
	const CqVector3D vMin(0.1, 0.2, 0.3);
	const CqVector3D vMax(0.1, 0.05, 1);
	CHECK_VEC_CLOSE(lerp(0.2f, vMin, vMax), 0.8f*vMin + 0.2f*vMax);
	CHECK_VEC_CLOSE(lerp(0.0f, vMin, vMax), vMin);
	CHECK_VEC_CLOSE(lerp(1.0f, vMin, vMax), vMax);
}

BOOST_AUTO_TEST_CASE(CqVector3D_isClose)
{
	BOOST_CHECK(isClose(CqVector3D(0,0,0), CqVector3D(0,0,0)));

	const CqVector3D v1(1.5, 2.5, 1.1);
	BOOST_CHECK(isClose(v1, v1));
	BOOST_CHECK(isClose(v1, (v1*100.0)/100.0));

	const CqVector3D eps(0, 1e-4, 0);

	BOOST_CHECK(!isClose(v1, v1 + eps));
	BOOST_CHECK(!isClose(v1, v1 + eps, 1e-5));
	BOOST_CHECK(isClose(v1, v1 + eps, 1e-4));
	BOOST_CHECK(isClose(v1, v1 - eps, 1e-4));

	const CqVector3D eps2(1e-4, -1e-5, 1e-5);
	BOOST_CHECK(!isClose(v1, v1 + eps2));
	BOOST_CHECK(!isClose(v1, v1 + eps2, 1e-5));
	BOOST_CHECK(isClose(v1, v1 + eps2, 1e-4));
	BOOST_CHECK(isClose(v1, v1 - eps2, 1e-4));
}


//------------------------------------------------------------------------------
// CqRefVec3 tests.

BOOST_AUTO_TEST_CASE(CqRefVec3_assign_test)
{
	// Test simple assignment from CqVec3 to CqRefVec3
	TqFloat storage[3] = {0,0,0};

	CqVec3 source(1,2,3);

	CqRefVec3 v(storage);
	v = source;

	BOOST_CHECK_EQUAL(source[0], storage[0]);
	BOOST_CHECK_EQUAL(source[1], storage[1]);
	BOOST_CHECK_EQUAL(source[2], storage[2]);
}

BOOST_AUTO_TEST_CASE(CqRefVec3_interop_test)
{
	// Test general interoperation between the two Cq*Vec3 types.

	CqVec3 v1(1,1,0);
	CqVec3 v2(0,1,0);
	TqFloat store3[3] = {0,0,1};
	CqRefVec3 v3(store3);

	TqFloat resStore[3] = {0,0,0};
	CqRefVec3 v(resStore);
	v = (v1 - 2*v3) % (v2 - CqRefVec3(store3));

	BOOST_CHECK_EQUAL(resStore[0], 1);
	BOOST_CHECK_EQUAL(resStore[1], 1);
	BOOST_CHECK_EQUAL(resStore[2], 1);
}

BOOST_AUTO_TEST_SUITE_END()
