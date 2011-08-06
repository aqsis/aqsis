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
 * \brief Unit tests for CqTexFileHeader
 * \author Chris Foster
 */

#include <aqsis/tex/io/texfileheader.h>

#define BOOST_TEST_DYN_LINK
#include <boost/test/auto_unit_test.hpp>

BOOST_AUTO_TEST_SUITE(texfileheader_tests)

// tag types for header testing
AQSIS_IMAGE_ATTR_TAG(TestAttr, TqInt);
AQSIS_IMAGE_ATTR_TAG(NotPresent, TqInt);


BOOST_AUTO_TEST_CASE(CqTexFileHeader_findPtr)
{
	Aqsis::CqTexFileHeader header;

	// add and read back an attribute via the findPtr() interface
	header.set<TestAttr>(42);
	const TqInt* value = header.findPtr<TestAttr>();
	BOOST_REQUIRE(value != 0);
	BOOST_CHECK_EQUAL(*value, 42);

	BOOST_CHECK_EQUAL(header.findPtr<NotPresent>(), (TqInt*)0);
}

BOOST_AUTO_TEST_CASE(CqTexFileHeader_find)
{
	Aqsis::CqTexFileHeader header;

	// add and read back an attribute via the find() interface
	header.set<TestAttr>(42);
	BOOST_CHECK_EQUAL(header.find<TestAttr>(), 42);

	// throw when accessing an attribute which isn't present.
	BOOST_CHECK_THROW(header.find<NotPresent>(), Aqsis::XqInternal);
}

BOOST_AUTO_TEST_CASE(CqTexFileHeader_findDefault)
{
	Aqsis::CqTexFileHeader header;

	// add and read back an attribute via the find() interface with
	// default value.
	header.set<TestAttr>(42);
	BOOST_CHECK_EQUAL(header.find<TestAttr>(1), 42);

	// Return default when attribute isn't present.
	BOOST_CHECK_EQUAL(header.find<NotPresent>(42), 42);
}

BOOST_AUTO_TEST_CASE(CqTexFileHeader_defaults)
{
	Aqsis::CqTexFileHeader header;

	// Check the channels
	Aqsis::CqChannelList& channelList = header.channelList();
	BOOST_CHECK_EQUAL(channelList.numChannels(), 0);
}

BOOST_AUTO_TEST_SUITE_END()
