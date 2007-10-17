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

#include "texfileheader.h"

#include <boost/test/auto_unit_test.hpp>

BOOST_AUTO_TEST_CASE(CqTexFileHeader_findAttributePtr)
{
	Aqsis::CqTexFileHeader header;

	// add and read back an attribute via the findAttributePtr() interface
	header.setAttribute<TqInt>("asdf", 42);
	const TqInt* value = header.findAttributePtr<TqInt>("asdf");
	BOOST_REQUIRE(value != 0);
	BOOST_CHECK_EQUAL(*value, 42);

	BOOST_CHECK_EQUAL(header.findAttributePtr<TqFloat>("asdf"), (TqFloat*)0);

	BOOST_CHECK_EQUAL(header.findAttributePtr<TqInt>("not_present"), (TqInt*)0);
}

BOOST_AUTO_TEST_CASE(CqTexFileHeader_findAttribute)
{
	Aqsis::CqTexFileHeader header;

	// add and read back an attribute via the findAttribute() interface
	header.setAttribute<TqInt>("asdf", 42);
	BOOST_CHECK_EQUAL(header.findAttribute<TqInt>("asdf"), 42);
	// throw when accessing an attribute by the wrong type.
	BOOST_CHECK_THROW(header.findAttribute<TqFloat>("asdf"), Aqsis::XqInternal);

	// throw when accessing an attribute which isn't present.
	BOOST_CHECK_THROW(header.findAttribute<TqInt>("not_present"), Aqsis::XqInternal);
}

BOOST_AUTO_TEST_CASE(CqTexFileHeader_findAttributeDefault)
{
	Aqsis::CqTexFileHeader header;

	// add and read back an attribute via the findAttribute() interface with
	// default value.
	header.setAttribute<TqInt>("asdf", 42);
	BOOST_CHECK_EQUAL(header.findAttribute<TqInt>("asdf", 1), 42);
	BOOST_CHECK_EQUAL(header.findAttribute<TqFloat>("asdf", 0.0f), 0.0f);

	// throw when accessing an attribute which isn't present.
	BOOST_CHECK_EQUAL(header.findAttribute<TqInt>("not_present", 42), 42);
}

BOOST_AUTO_TEST_CASE(CqTexFileHeader_defaults)
{
	Aqsis::CqTexFileHeader header;

	// Set/check the width
	header.setAttribute<TqInt>("width", 42);
	BOOST_CHECK_EQUAL(header.findAttribute<TqInt>("width"), 42);
	BOOST_CHECK_EQUAL(header.width(), 42);
	// should throw, since we shouldn't be able to change the type of "width"
	// from int to float, & "width" is a default attribute.
	BOOST_CHECK_THROW(header.setAttribute<TqFloat>("width", 1.0f), Aqsis::XqInternal);

	// Set/check the height
	header.setAttribute<TqInt>("height", 142);
	BOOST_CHECK_EQUAL(header.findAttribute<TqInt>("height"), 142);
	BOOST_CHECK_EQUAL(header.height(), 142);

	// Check the channels
	Aqsis::CqChannelList& channelList = header.channelList();
	BOOST_CHECK_EQUAL(channelList.numChannels(), 0);
}

