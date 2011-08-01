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
 * \brief Unit tests for channel info lists
 *
 * \author Chris Foster
 */

#include <aqsis/tex/buffers/channellist.h>

#define BOOST_TEST_DYN_LINK
#include <boost/test/auto_unit_test.hpp>

BOOST_AUTO_TEST_SUITE(channellist_tests)

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

BOOST_AUTO_TEST_SUITE_END()
