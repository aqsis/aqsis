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

#include <aqsis/tex/buffers/mixedimagebuffer.h>

#ifndef	AQSIS_SYSTEM_WIN32
#define BOOST_TEST_DYN_LINK
#endif //AQSIS_SYSTEM_WIN32

#include <sstream>

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <aqsis/math/math.h>
#include <aqsis/util/smartptr.h>

//------------------------------------------------------------------------------
// CqMixedImageBuffer test cases

BOOST_AUTO_TEST_CASE(CqMixedImageBuffer_test_clear)
{
	Aqsis::CqChannelList ch;
	ch.addChannel(Aqsis::SqChannelInfo("r", Aqsis::Channel_Unsigned16));
	ch.addChannel(Aqsis::SqChannelInfo("b", Aqsis::Channel_Unsigned16));
	TqUint16 data[] = {0x100, 0x0200, 0xFA00, 0xFF00};
	TqInt width = 2;
	TqInt height = 1;
	TqInt chansPerPixel = ch.numChannels();
	Aqsis::CqMixedImageBuffer imBuf(ch, boost::shared_array<TqUint8>(
				reinterpret_cast<TqUint8*>(data), Aqsis::nullDeleter), width, height);
	imBuf.clearBuffer(1.0f);

	for(TqInt i = 0; i < width*height*chansPerPixel; ++i)
		BOOST_CHECK_EQUAL(data[i], TqUint16(0xFFFF));
}

BOOST_AUTO_TEST_CASE(CqMixedImageBuffer_test_copyFromSubregion)
{
	Aqsis::CqChannelList ch;
	ch.addChannel(Aqsis::SqChannelInfo("r", Aqsis::Channel_Unsigned16));
	TqUint16 srcData[] = {1, 2,
	                      3, 4};
	TqInt width = 2;
	TqInt height = 2;
	TqInt chansPerPixel = ch.numChannels();
	Aqsis::CqMixedImageBuffer srcBuf(ch, boost::shared_array<TqUint8>(
				reinterpret_cast<TqUint8*>(srcData), Aqsis::nullDeleter), width, height);
	
	TqUint16 destData[] = {0,0,0,
	                       0,0,0};
	TqInt destWidth = 3;
	TqInt destHeight = 2;
	Aqsis::CqMixedImageBuffer destBuf(ch, boost::shared_array<TqUint8>(
				reinterpret_cast<TqUint8*>(destData), Aqsis::nullDeleter), destWidth, destHeight);

	destBuf.copyFrom(srcBuf, 0, 0);

	TqUint16 expectedData[] = {1, 2, 0,
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


BOOST_AUTO_TEST_CASE(CqMixedImageBuffer_mixed_channels_test)
{
	// Test that we can actually mix different channel types properly
	Aqsis::CqChannelList chanList;
	chanList.addChannel(Aqsis::SqChannelInfo("r", Aqsis::Channel_Unsigned8));
	chanList.addChannel(Aqsis::SqChannelInfo("g", Aqsis::Channel_Float32));
	chanList.addChannel(Aqsis::SqChannelInfo("z", Aqsis::Channel_Signed16));

	TqInt width = 10;
	TqInt height = 10;
	Aqsis::CqMixedImageBuffer buf(chanList, width, height);

	// Fill the whole buffer with the maximum value possible for each channel.
	buf.clearBuffer(1.0);

	const TqUint8* rawData = buf.rawData();

	// check pixel 0,0...
	BOOST_CHECK_EQUAL(rawData[0], 255);
	BOOST_CHECK_CLOSE(*reinterpret_cast<const TqFloat*>(rawData+chanList.channelByteOffset(1)), 1.0f, 1e-5);
	BOOST_CHECK_EQUAL(*reinterpret_cast<const TqInt16*>(rawData+chanList.channelByteOffset(2)),
			std::numeric_limits<TqInt16>::max());

	// check last pixel
	rawData += chanList.bytesPerPixel()*(width*height-1);
	BOOST_CHECK_EQUAL(rawData[0], 255);
	BOOST_CHECK_CLOSE(*reinterpret_cast<const TqFloat*>(rawData+chanList.channelByteOffset(1)), 1.0f, 1e-5);
	BOOST_CHECK_EQUAL(*reinterpret_cast<const TqInt16*>(rawData+chanList.channelByteOffset(2)),
			std::numeric_limits<TqInt16>::max());
}

