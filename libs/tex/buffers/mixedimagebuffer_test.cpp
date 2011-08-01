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
 * \brief Unit tests for image buffers
 * \author Chris Foster
 */

#include <aqsis/tex/buffers/mixedimagebuffer.h>

#include <sstream>

#define BOOST_TEST_DYN_LINK
#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <aqsis/math/math.h>
#include <aqsis/util/smartptr.h>

//------------------------------------------------------------------------------
// CqMixedImageBuffer test cases

BOOST_AUTO_TEST_SUITE(mixedimagebuffer_tests)

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

BOOST_AUTO_TEST_SUITE_END()
