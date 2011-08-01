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
 * \brief Unit tests for image channel stuff
 * \author Chris Foster
 */
#include <aqsis/tex/buffers/imagechannel.h>

#define BOOST_TEST_DYN_LINK
#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

BOOST_AUTO_TEST_SUITE(imagechannel_tests)

//------------------------------------------------------------------------------
// CqImageChannel* test cases

// Return a channel constructed from the provided raw array.
template<typename T>
boost::shared_ptr<Aqsis::CqImageChannel> channelFromArray(T* src,
		Aqsis::EqChannelType type, TqInt width, TqInt height, TqInt
		chansPerPixel, TqInt offset, TqInt rowSkip = 0)
{
	return boost::shared_ptr<Aqsis::CqImageChannelTyped<T> > (
			new Aqsis::CqImageChannelTyped<T>(
				Aqsis::SqChannelInfo("r", type),
				reinterpret_cast<TqUint8*>(src + offset),
				width, height, chansPerPixel*sizeof(T), rowSkip ) );
}

BOOST_AUTO_TEST_CASE(CqImageChannel_test_copyFrom_simpleTest)
{
	TqUint16 srcData[] = {0x100, 0x200};
	TqInt width = 2;
	TqInt height = 1;
	TqInt chansPerPixel = 1;
	TqInt offset = 0;
	boost::shared_ptr<Aqsis::CqImageChannel > srcChan = channelFromArray(srcData,
			Aqsis::Channel_Unsigned16, width, height, chansPerPixel, offset);

	TqUint8 destData[] = {0, 0};
	boost::shared_ptr<Aqsis::CqImageChannel > destChan = channelFromArray(destData,
			Aqsis::Channel_Unsigned8, width, height, chansPerPixel, offset);

	destChan->copyFrom(*srcChan);

	TqUint8 expectedData[] = {1, 2};
	for(TqInt i = 0; i < width*height*chansPerPixel; ++i)
		BOOST_CHECK_EQUAL(destData[i], expectedData[i]);
}


BOOST_AUTO_TEST_CASE(CqImageChannel_test_copyFrom_offset_and_stride)
{
	TqUint16 srcData[] = {0x100, 0x200, 0x300, 0x400};
	TqInt width = 2;
	TqInt height = 1;
	TqInt chansPerPixel = 2;
	TqInt offset = 1;
	boost::shared_ptr<Aqsis::CqImageChannel > srcChan = channelFromArray(srcData,
			Aqsis::Channel_Unsigned16, width, height, chansPerPixel, offset);

	TqUint8 destData[] = {0, 0, 0, 0};
	TqInt destOffset = 0;
	boost::shared_ptr<Aqsis::CqImageChannel > destChan = channelFromArray(destData,
			Aqsis::Channel_Unsigned8, width, height, chansPerPixel, destOffset);

	destChan->copyFrom(*srcChan);

	TqUint8 expectedData[] = {2, 0, 4, 0};
	for(TqInt i = 0; i < width*height*chansPerPixel; ++i)
		BOOST_CHECK_EQUAL(destData[i], expectedData[i]);
}


BOOST_AUTO_TEST_CASE(CqImageChannel_test_copyFrom_rowSkip)
{
	TqUint16 srcData[] = {0x100, 0x200,
		                  0x300, 0x400};
	TqInt realWidth = 2;
	TqInt width = 1;
	TqInt height = 2;
	TqInt chansPerPixel = 1;
	TqInt offset = 0;
	TqInt rowSkip = realWidth - width;
	boost::shared_ptr<Aqsis::CqImageChannel > srcChan = channelFromArray(srcData,
			Aqsis::Channel_Unsigned16, width, height, chansPerPixel, offset, rowSkip);

	TqUint8 destData[] = {0, 0};
	TqInt destOffset = 0;
	boost::shared_ptr<Aqsis::CqImageChannel > destChan = channelFromArray(destData,
			Aqsis::Channel_Unsigned8, width, height, chansPerPixel, destOffset);

	destChan->copyFrom(*srcChan);

	TqUint8 expectedData[] = {1, 3};
	for(TqInt i = 0; i < width*height*chansPerPixel; ++i)
		BOOST_CHECK_EQUAL(destData[i], expectedData[i]);
}

BOOST_AUTO_TEST_CASE(CqImageChannel_test_copyFrom_floatOut)
{
	TqUint16 uint16Max = std::numeric_limits<TqUint16>::max();
	TqUint16 srcData[] = {uint16Max/2, uint16Max};
	TqInt width = 2;
	TqInt height = 1;
	TqInt chansPerPixel = 1;
	TqInt offset = 0;
	boost::shared_ptr<Aqsis::CqImageChannel> srcChan = channelFromArray(srcData,
			Aqsis::Channel_Unsigned16, width, height, chansPerPixel, offset);

	TqFloat destData[] = {0.0f, 0.0f};
	boost::shared_ptr<Aqsis::CqImageChannel> destChan = channelFromArray(destData,
			Aqsis::Channel_Float32, width, height, chansPerPixel, offset);

	destChan->copyFrom(*srcChan);

	TqFloat expectedData[] = {0.5f, 1.0f};
	for(TqInt i = 0; i < width*height*chansPerPixel; ++i)
		BOOST_CHECK_CLOSE(destData[i], expectedData[i], 1.1*100.0f/uint16Max);
}

BOOST_AUTO_TEST_CASE(CqImageChannel_test_copyFromSameType)
{
	TqUint16 srcData[] = {0x100, 0x200};
	TqInt width = 2;
	TqInt height = 1;
	TqInt chansPerPixel = 1;
	TqInt offset = 0;
	boost::shared_ptr<Aqsis::CqImageChannel > srcChan = channelFromArray(srcData,
			Aqsis::Channel_Unsigned16, width, height, chansPerPixel, offset);

	TqUint16 destData[] = {0, 0};
	boost::shared_ptr<Aqsis::CqImageChannel > destChan = channelFromArray(destData,
			Aqsis::Channel_Unsigned16, width, height, chansPerPixel, offset);

	destChan->copyFrom(*srcChan);

	for(TqInt i = 0; i < width*height*chansPerPixel; ++i)
		BOOST_CHECK_EQUAL(destData[i], srcData[i]);
}

BOOST_AUTO_TEST_SUITE_END()
