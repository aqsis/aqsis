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
 * \brief Unit tests for channel info lists
 *
 * \author Chris Foster
 */

#include <aqsis/tex/buffers/channellist.h>

#ifndef	AQSIS_SYSTEM_WIN32
#define BOOST_TEST_DYN_LINK
#endif //AQSIS_SYSTEM_WIN32

#include <boost/test/auto_unit_test.hpp>

//------------------------------------------------------------------------------
// CqChannelList test cases

// Fixture for channel list test cases.
struct ChannelInfoListFixture
{
	Aqsis::CqChannelList chanList;

	ChannelInfoListFixture()
		: chanList()
	{
		chanList.addChannel(Aqsis::SqChannelInfo("a", Aqsis::Channel_Unsigned16));
		chanList.addChannel(Aqsis::SqChannelInfo("b", Aqsis::Channel_Unsigned8));
		chanList.addChannel(Aqsis::SqChannelInfo("g", Aqsis::Channel_Signed32));
		chanList.addChannel(Aqsis::SqChannelInfo("r", Aqsis::Channel_Float32));
	}
};

BOOST_AUTO_TEST_CASE(CqChannelList_channelByteOffset_test)
{
	ChannelInfoListFixture f;

	BOOST_CHECK_EQUAL(f.chanList.channelByteOffset(0), 0);
	BOOST_CHECK_EQUAL(f.chanList.channelByteOffset(1), 2);
	BOOST_CHECK_EQUAL(f.chanList.channelByteOffset(3), 7);

	BOOST_CHECK_EQUAL(f.chanList.bytesPerPixel(), 11);
}

BOOST_AUTO_TEST_CASE(CqChannelList_sharedChannelType_test)
{
	Aqsis::CqChannelList chanList;
	// Check that the shared type comes out correctly
	chanList.addChannel(Aqsis::SqChannelInfo("r", Aqsis::Channel_Unsigned8));
	BOOST_CHECK_EQUAL(chanList.sharedChannelType(), Aqsis::Channel_Unsigned8);
	chanList.addChannel(Aqsis::SqChannelInfo("g", Aqsis::Channel_Unsigned8));
	BOOST_CHECK_EQUAL(chanList.sharedChannelType(), Aqsis::Channel_Unsigned8);
	// There's no shared channel type anymore since the "b" channel has an
	// incompatible type.
	chanList.addChannel(Aqsis::SqChannelInfo("b", Aqsis::Channel_Unsigned16));
	BOOST_CHECK_EQUAL(chanList.sharedChannelType(), Aqsis::Channel_TypeUnknown);
}

BOOST_AUTO_TEST_CASE(CqChannelList_reorderChannels_test)
{
	ChannelInfoListFixture f;

	f.chanList.reorderChannels();
	f.chanList.addChannel(Aqsis::SqChannelInfo("N", Aqsis::Channel_Float32));

	BOOST_CHECK_EQUAL(f.chanList[0].name, "r");
	BOOST_CHECK_EQUAL(f.chanList[1].name, "g");
	BOOST_CHECK_EQUAL(f.chanList[2].name, "b");
	BOOST_CHECK_EQUAL(f.chanList[3].name, "a");

	BOOST_CHECK_EQUAL(f.chanList.bytesPerPixel(), 15);

	// Check that the reorder also works for less than 4 elements
	Aqsis::CqChannelList chanList2;
	chanList2.addChannel(Aqsis::SqChannelInfo("N", Aqsis::Channel_Float32));
	chanList2.addChannel(Aqsis::SqChannelInfo("r", Aqsis::Channel_Unsigned8));
	chanList2.reorderChannels();
	BOOST_CHECK_EQUAL(chanList2[0].name, "r");
	BOOST_CHECK_EQUAL(chanList2[1].name, "N");
}

BOOST_AUTO_TEST_CASE(CqChannelList_findChannelIndex_test)
{
	ChannelInfoListFixture f;

	BOOST_CHECK_EQUAL(f.chanList.findChannelIndex("g"), 2);
	BOOST_CHECK_THROW(f.chanList.findChannelIndex("z"), Aqsis::XqInternal);
}

BOOST_AUTO_TEST_CASE(CqChannelList_addUnnamedChannels)
{
	Aqsis::CqChannelList channelList;

	channelList.addUnnamedChannels(Aqsis::Channel_Unsigned8, 5);
	BOOST_CHECK_EQUAL(channelList[0].name, "?01");
	BOOST_CHECK_EQUAL(channelList[0].type, Aqsis::Channel_Unsigned8);
	BOOST_CHECK_EQUAL(channelList[4].name, "?05");
}
