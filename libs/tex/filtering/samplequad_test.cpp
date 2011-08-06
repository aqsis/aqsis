// Aqsis
// Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the software's owners nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// (This is the New BSD license)

/** \file
 *
 * \brief Unit tests for sampling quad operations.
 * \author Chris Foster
 */
#include <aqsis/tex/filtering/samplequad.h>

#define BOOST_TEST_DYN_LINK
#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

BOOST_AUTO_TEST_SUITE(samplequad_tests)

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

BOOST_AUTO_TEST_SUITE_END()
