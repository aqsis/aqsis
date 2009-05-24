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

#ifndef	AQSIS_SYSTEM_WIN32
#define BOOST_TEST_DYN_LINK
#endif //AQSIS_SYSTEM_WIN32

#include <sstream>

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

	const char* artistDefault = "blah";
	BOOST_CHECK_EQUAL(dirHandle.tiffTagValue<const char*>(TIFFTAG_ARTIST, artistDefault), artistDefault);
}

BOOST_AUTO_TEST_CASE(CqTiffDirHandle_fillHeader_test)
{
	TiffFileHandleFixture f(stripTiffString);

	Aqsis::CqTiffDirHandle dirHandle(f.fileHandlePtr);

	// extract header information from the TIFF file.
	Aqsis::CqTexFileHeader header;
	dirHandle.fillHeader(header);

	// Check that the header has been initialized properly...
	BOOST_CHECK_EQUAL(header.width(), 6);
	BOOST_CHECK_EQUAL(header.height(), 4);

	// Check the channels
	Aqsis::CqChannelList& channelList = header.channelList();
	BOOST_REQUIRE_EQUAL(channelList.numChannels(), 3);
	BOOST_CHECK_EQUAL(channelList[0].name, "r");
	BOOST_CHECK_EQUAL(channelList[0].type, Aqsis::Channel_Unsigned8);
	BOOST_CHECK_EQUAL(channelList[1].name, "g");
	BOOST_CHECK_EQUAL(channelList[2].name, "b");

	// Check informational strings
	BOOST_CHECK_EQUAL(header.find<Aqsis::Attr::Description>(),
			"Strip-allocated tiff for unit tests");
}

BOOST_AUTO_TEST_CASE(CqTiffDirHandle_write_read_header_test)
{
	std::ostringstream out;
	{
		// Create a tiff with some given header data.
		Aqsis::CqTiffFileHandle tiffHandle(out);
		boost::shared_ptr<Aqsis::CqTiffFileHandle>
			tiffHandlePtr(&tiffHandle, Aqsis::nullDeleter);
		Aqsis::CqTiffDirHandle dirHandle(tiffHandlePtr);

		Aqsis::CqTexFileHeader header;
		header.setWidth(11);
		header.setHeight(1);
		Aqsis::CqChannelList& channels = header.channelList();
		channels.addChannel(Aqsis::SqChannelInfo("y", Aqsis::Channel_Signed8));
		header.set<Aqsis::Attr::DisplayWindow>( Aqsis::SqImageRegion(1,2,3,4) );

		dirHandle.writeHeader(header);
		TqUint8 rubbishPixels[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
		TIFFWriteScanline(dirHandle.tiffPtr(), rubbishPixels, 0);
	}

	{
		// Read the tiff back in again.
		std::istringstream in(out.str());
		Aqsis::CqTiffFileHandle tiffHandle(in);
		boost::shared_ptr<Aqsis::CqTiffFileHandle>
			tiffHandlePtr(&tiffHandle, Aqsis::nullDeleter);
		Aqsis::CqTiffDirHandle dirHandle(tiffHandlePtr);

		Aqsis::CqTexFileHeader header;
		dirHandle.fillHeader(header);

		// check dimensions
		BOOST_CHECK_EQUAL(header.width(), 11);
		BOOST_CHECK_EQUAL(header.height(), 1);
		// check channels
		Aqsis::CqChannelList& channels = header.channelList();
		// Note that we can't store actual channel names in tiff, but a single
		// channel is deduced to be an intensity channel under normal circumstances.
		BOOST_CHECK_EQUAL(channels[0].name, "y");
		BOOST_CHECK_EQUAL(channels[0].type, Aqsis::Channel_Signed8);
		// check display window
		Aqsis::SqImageRegion& displayWindow = header.find<Aqsis::Attr::DisplayWindow>();
		BOOST_CHECK_EQUAL(displayWindow.width, 1);
		BOOST_CHECK_EQUAL(displayWindow.height, 2);
		BOOST_CHECK_EQUAL(displayWindow.topLeftX, 3);
		BOOST_CHECK_EQUAL(displayWindow.topLeftY, 4);
	}
}

