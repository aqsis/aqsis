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
 * \brief Unit tests for texture sample options
 * \author Chris Foster
 */

#include "texturesampleoptions.h"
#include "samplequad.h"

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>


//------------------------------------------------------------------------------
// CqTextureSampleOptions unit tests

BOOST_AUTO_TEST_CASE(CqTextureSampleOptions_adjustSampleQuad_test)
{
	Aqsis::SqSampleQuad quad(
			Aqsis::CqVector2D(0,0),
			Aqsis::CqVector2D(0,1), 
			Aqsis::CqVector2D(1,0),
			Aqsis::CqVector2D(1,1));
	Aqsis::CqTextureSampleOptions opts;
	opts.setSWidth(3);
	opts.setTWidth(0.1);
	opts.adjustSampleQuad(quad);
	BOOST_CHECK_CLOSE(quad.v1.x(), -1.0f, 0.01f);
	BOOST_CHECK_CLOSE(quad.v1.y(), 0.45f, 0.01f);

	BOOST_CHECK_CLOSE(quad.v2.x(), -1.0f, 0.01f);
	BOOST_CHECK_CLOSE(quad.v2.y(), 0.55f, 0.01f);

	BOOST_CHECK_CLOSE(quad.v3.x(), 2.0f, 0.01f);
	BOOST_CHECK_CLOSE(quad.v3.y(), 0.45f, 0.01f);

	BOOST_CHECK_CLOSE(quad.v4.x(), 2.0f, 0.01f);
	BOOST_CHECK_CLOSE(quad.v4.y(), 0.55f, 0.01f);
}

