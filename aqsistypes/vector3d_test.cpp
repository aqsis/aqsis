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

//------------------------------------------------------------------------------
// Unit tests

#include "vector3d.h"

#define BOOST_TEST_MODULE CqVector3D_Tests
#define BOOST_AUTO_TEST_MAIN

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

using Aqsis::CqVector3D;

BOOST_AUTO_TEST_CASE(vector3d_min)
{
	const CqVector3D vMin(0.3,0.4,0.5);
	const CqVector3D vMax(0.4,0.5,0.6);
	BOOST_CHECK_EQUAL(Aqsis::min(vMin, vMax), vMin);
	BOOST_CHECK_EQUAL(Aqsis::min(vMax, vMin), vMin);
	BOOST_CHECK_EQUAL(Aqsis::min(CqVector3D(1,0.5,0), CqVector3D(0.2,0.5,0.8)), CqVector3D(0.2,0.5,0));
}

BOOST_AUTO_TEST_CASE(vector3d_max)
{
	const CqVector3D vMin(0.3,0.4,0.5);
	const CqVector3D vMax(0.4,0.5,0.6);
	BOOST_CHECK_EQUAL(Aqsis::max(vMin, vMax), vMax);
	BOOST_CHECK_EQUAL(Aqsis::max(vMax, vMin), vMax);
	BOOST_CHECK_EQUAL(Aqsis::max(CqVector3D(1,0.5,0), CqVector3D(0.2,0.5,0.8)), CqVector3D(1,0.5,0.8));
}

BOOST_AUTO_TEST_CASE(vector3d_clamp)
{
	const CqVector3D vMin(0.3,0.4,0.5);
	const CqVector3D vMax(0.4,0.5,0.6);
	BOOST_CHECK_EQUAL(Aqsis::clamp(CqVector3D(0,0,0), vMin, vMax), vMin);
	BOOST_CHECK_EQUAL(Aqsis::clamp(CqVector3D(1,1,1), vMin, vMax), vMax);
	const CqVector3D vMiddle = (vMin+vMax)/2;
	BOOST_CHECK_EQUAL(Aqsis::clamp(vMiddle, vMin, vMax), vMiddle);
	BOOST_CHECK_EQUAL(Aqsis::clamp(CqVector3D(0.9, 0.45, 0.1), vMin, vMax), CqVector3D(0.4, 0.45, 0.5));
}

BOOST_AUTO_TEST_CASE(vector3d_lerp)
{
	const CqVector3D vMin(0.1, 0.2, 0.3);
	const CqVector3D vMax(0.1, 0.05, 1);
	BOOST_CHECK_EQUAL(Aqsis::lerp(0.2f, vMin, vMax), 0.8f*vMin + 0.2f*vMax);
	BOOST_CHECK_EQUAL(Aqsis::lerp(0.0f, vMin, vMax), vMin);
	BOOST_CHECK_EQUAL(Aqsis::lerp(1.0f, vMin, vMax), vMax);
}

