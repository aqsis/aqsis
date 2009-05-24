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
 * \brief Unit tests for sampling quad operations.
 * \author Chris Foster
 */
#include <aqsis/tex/filtering/samplequad.h>

#ifndef	AQSIS_SYSTEM_WIN32
#define BOOST_TEST_DYN_LINK
#endif //AQSIS_SYSTEM_WIN32

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>


//------------------------------------------------------------------------------
// CqTextureSampleOptions unit tests

BOOST_AUTO_TEST_CASE(SqSampleQuad_scaleWidth_test)
{
	Aqsis::SqSampleQuad quad(
			Aqsis::CqVector2D(0,0),
			Aqsis::CqVector2D(0,1), 
			Aqsis::CqVector2D(1,0),
			Aqsis::CqVector2D(1,1));
	quad.scaleWidth(3, 0.1);

	BOOST_CHECK_CLOSE(quad.v1.x(), -1.0f, 0.01f);
	BOOST_CHECK_CLOSE(quad.v1.y(), 0.45f, 0.01f);

	BOOST_CHECK_CLOSE(quad.v2.x(), -1.0f, 0.01f);
	BOOST_CHECK_CLOSE(quad.v2.y(), 0.55f, 0.01f);

	BOOST_CHECK_CLOSE(quad.v3.x(), 2.0f, 0.01f);
	BOOST_CHECK_CLOSE(quad.v3.y(), 0.45f, 0.01f);

	BOOST_CHECK_CLOSE(quad.v4.x(), 2.0f, 0.01f);
	BOOST_CHECK_CLOSE(quad.v4.y(), 0.55f, 0.01f);
}

