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
 * \brief Unit tests for CqTiffDirHandle
 *
 * \author Chris Foster
 */

#include "tiffdirhandle.h"
#include "tifffile_test.h"

#include <boost/test/auto_unit_test.hpp>

//------------------------------------------------------------------------------
// Test cases for CqTiffFileHandle

BOOST_AUTO_TEST_CASE(CqTiffFileHandle_test_constructor)
{
	BOOST_CHECK_THROW(Aqsis::CqTiffFileHandle("nonexistant_file.tif", "r"),
			Aqsis::XqInternal);
	BOOST_MESSAGE("Unit test note: Expect a libtiff error about nonexistant_file.tif above ^^");

	// just create.  Should not throw.
	TiffFileHandleFixture f(stripTiffString);
}


//------------------------------------------------------------------------------
// Test cases for CqTiffDirHandle

BOOST_AUTO_TEST_CASE(CqTiffDirHandle_dirindex_test)
{
	TiffFileHandleFixture f(stripTiffString);
	{
		// Check that we can grab a handle to the second directory
		Aqsis::CqTiffDirHandle dirHandle(f.fileHandlePtr, 1);

		BOOST_CHECK(dirHandle.tiffPtr() != 0);

		BOOST_CHECK_EQUAL(dirHandle.dirIndex(), 1);
	}
	{
		// The test tiff only has two directories - accessing the third should
		// throw.
		BOOST_CHECK_THROW(Aqsis::CqTiffDirHandle(f.fileHandlePtr, 2), Aqsis::XqInternal);
	}
}


BOOST_AUTO_TEST_CASE(CqTiffDirHandle_tiffTagValue_test)
{
	TiffFileHandleFixture f(stripTiffString);

	Aqsis::CqTiffDirHandle dirHandle(f.fileHandlePtr, 0);

	BOOST_CHECK_EQUAL(dirHandle.tiffTagValue<uint32>(TIFFTAG_IMAGEWIDTH), uint32(6));
	BOOST_CHECK_EQUAL(dirHandle.tiffTagValue<uint32>(TIFFTAG_IMAGEWIDTH, 0), uint32(6));
	BOOST_CHECK_EQUAL(dirHandle.tiffTagValue<uint32>(TIFFTAG_IMAGELENGTH), uint32(4));
	BOOST_CHECK_EQUAL(dirHandle.tiffTagValue<uint32>(TIFFTAG_ROWSPERSTRIP), uint32(2));

	// There is no ARTIST TAG in the created tiffs - hence an exception
	BOOST_CHECK_THROW(dirHandle.tiffTagValue<char*>(TIFFTAG_ARTIST), Aqsis::XqInternal);

	char* artistDefault = "blah";
	BOOST_CHECK_EQUAL(dirHandle.tiffTagValue<char*>(TIFFTAG_ARTIST, artistDefault), artistDefault);
}

BOOST_AUTO_TEST_CASE(CqTiffDirHandle_fillHeader_test)
{
	TiffFileHandleFixture f(stripTiffString);

	Aqsis::CqTiffDirHandle dirHandle(f.fileHandlePtr);

	// extract header information from the TIFF file.
	Aqsis::CqTexFileHeader header;
	dirHandle.fillHeader(header);

	// Check that the header has been initialized properly...
	BOOST_CHECK_EQUAL(header.findAttribute<TqInt>("width"), 6);
	BOOST_CHECK_EQUAL(header.findAttribute<TqInt>("height"), 4);

	// Check the channels
	Aqsis::CqChannelList& channelList = header.findAttribute<Aqsis::CqChannelList>("channelList");
	BOOST_REQUIRE_EQUAL(channelList.numChannels(), 3);
	BOOST_CHECK_EQUAL(channelList[0].name, "r");
	BOOST_CHECK_EQUAL(channelList[0].type, Aqsis::Channel_Unsigned8);
	BOOST_CHECK_EQUAL(channelList[1].name, "g");
	BOOST_CHECK_EQUAL(channelList[2].name, "b");

	// Check informational strings
	BOOST_CHECK_EQUAL(header.findAttribute<std::string>("description"),
			"Strip-allocated tiff for unit tests");

	BOOST_CHECK(!header.findAttribute<bool>("isTiled"));
}

