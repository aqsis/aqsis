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
 * \brief Unit tests for image buffers
 * \author Chris Foster
 */

#include "mixedimagebuffer.h"

#include <sstream>

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include "aqsismath.h"
#include "smartptr.h"

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
}

BOOST_AUTO_TEST_CASE(CqChannelList_findChannelIndex_test)
{
	ChannelInfoListFixture f;

	BOOST_CHECK_EQUAL(f.chanList.findChannelIndex("g"), 2);
	BOOST_CHECK_THROW(f.chanList.findChannelIndex("z"), Aqsis::XqInternal);
}


//------------------------------------------------------------------------------
// CqMixedImageBuffer test cases

BOOST_AUTO_TEST_CASE(CqMixedImageBuffer_test_clear)
{
	Aqsis::CqChannelList ch;
	ch.addChannel(Aqsis::SqChannelInfo("r", Aqsis::Channel_Unsigned16));
	ch.addChannel(Aqsis::SqChannelInfo("b", Aqsis::Channel_Unsigned16));
	TqUshort data[] = {0x100, 0x0200, 0xFA00, 0xFF00};
	TqInt width = 2;
	TqInt height = 1;
	TqInt chansPerPixel = ch.numChannels();
	Aqsis::CqMixedImageBuffer imBuf(ch, boost::shared_array<TqUchar>(
				reinterpret_cast<TqUchar*>(data), Aqsis::nullDeleter), width, height);
	imBuf.clearBuffer(1.0f);

	for(TqInt i = 0; i < width*height*chansPerPixel; ++i)
		BOOST_CHECK_EQUAL(data[i], TqUshort(0xFFFF));
}

BOOST_AUTO_TEST_CASE(CqMixedImageBuffer_test_copyFromSubregion)
{
	Aqsis::CqChannelList ch;
	ch.addChannel(Aqsis::SqChannelInfo("r", Aqsis::Channel_Unsigned16));
	TqUshort srcData[] = {1, 2,
	                      3, 4};
	TqInt width = 2;
	TqInt height = 2;
	TqInt chansPerPixel = ch.numChannels();
	Aqsis::CqMixedImageBuffer srcBuf(ch, boost::shared_array<TqUchar>(
				reinterpret_cast<TqUchar*>(srcData), Aqsis::nullDeleter), width, height);
	
	TqUshort destData[] = {0,0,0,
	                       0,0,0};
	TqInt destWidth = 3;
	TqInt destHeight = 2;
	Aqsis::CqMixedImageBuffer destBuf(ch, boost::shared_array<TqUchar>(
				reinterpret_cast<TqUchar*>(destData), Aqsis::nullDeleter), destWidth, destHeight);

	destBuf.copyFrom(srcBuf, 0, 0);

	TqUshort expectedData[] = {1, 2, 0,
							   3, 4, 0};

	for(TqInt i = 0; i < destWidth*destHeight*chansPerPixel; ++i)
		BOOST_CHECK_EQUAL(destData[i], expectedData[i]);
}

BOOST_AUTO_TEST_CASE(CqMixedImageBuffer_resize_test)
{
	Aqsis::CqMixedImageBuffer buf;

	BOOST_CHECK_EQUAL(buf.channelList().numChannels(), 0);
	BOOST_CHECK_EQUAL(buf.width(), 0);
	BOOST_CHECK_EQUAL(buf.height(), 0);

	// Now resize the channel
	Aqsis::CqChannelList ch;
	ch.addChannel(Aqsis::SqChannelInfo("r", Aqsis::Channel_Unsigned16));
	buf.resize(10, 20, ch);
	
	BOOST_CHECK_EQUAL(buf.channelList().numChannels(), 1);
	BOOST_CHECK_EQUAL(buf.width(), 10);
	BOOST_CHECK_EQUAL(buf.height(), 20);
	BOOST_CHECK(buf.rawData() != 0);
}

