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
 * \brief Unit tests for image channel stuff
 * \author Chris Foster
 */
#include <aqsis/tex/buffers/imagechannel.h>

#ifndef	AQSIS_SYSTEM_WIN32
#define BOOST_TEST_DYN_LINK
#endif //AQSIS_SYSTEM_WIN32

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>


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

