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
 * \brief Unit tests for CqTexFileHeader
 * \author Chris Foster
 */

#include <aqsis/tex/io/texfileheader.h>

#ifndef	AQSIS_SYSTEM_WIN32
#define BOOST_TEST_DYN_LINK
#endif //AQSIS_SYSTEM_WIN32

#include <boost/test/auto_unit_test.hpp>

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

