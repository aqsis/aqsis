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

#include "imagechannel.h"

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>


//------------------------------------------------------------------------------
// CqImageChannel* test cases

// Return a channel constructed from the provided raw array.
template<typename T>
boost::shared_ptr<Aqsis::CqImageChannel> channelFromArray(T* src,
		Aqsis::EqChannelFormat type, TqUint width, TqUint height, TqUint
		chansPerPixel, TqUint offset, TqUint rowSkip = 0)
{
	return boost::shared_ptr<Aqsis::CqImageChannelTyped<T> > (
			new Aqsis::CqImageChannelTyped<T>(
				Aqsis::SqChannelInfo("r", type),
				reinterpret_cast<TqUchar*>(src + offset),
				width, height, chansPerPixel*sizeof(T), rowSkip ) );
}

BOOST_AUTO_TEST_CASE(CqImageChannel_test_copyFrom_simpleTest)
{
	TqUshort srcData[] = {0x100, 0x200};
	TqUint width = 2;
	TqUint height = 1;
	TqUint chansPerPixel = 1;
	TqUint offset = 0;
	boost::shared_ptr<Aqsis::CqImageChannel > srcChan = channelFromArray(srcData,
			Aqsis::Format_Unsigned16, width, height, chansPerPixel, offset);

	TqUchar destData[] = {0, 0};
	boost::shared_ptr<Aqsis::CqImageChannel > destChan = channelFromArray(destData,
			Aqsis::Format_Unsigned8, width, height, chansPerPixel, offset);

	destChan->copyFrom(*srcChan);

	TqUchar expectedData[] = {1, 2};
	for(TqUint i = 0; i < width*height*chansPerPixel; ++i)
		BOOST_CHECK_EQUAL(destData[i], expectedData[i]);
}


BOOST_AUTO_TEST_CASE(CqImageChannel_test_copyFrom_offset_and_stride)
{
	TqUshort srcData[] = {0x100, 0x200, 0x300, 0x400};
	TqUint width = 2;
	TqUint height = 1;
	TqUint chansPerPixel = 2;
	TqUint offset = 1;
	boost::shared_ptr<Aqsis::CqImageChannel > srcChan = channelFromArray(srcData,
			Aqsis::Format_Unsigned16, width, height, chansPerPixel, offset);

	TqUchar destData[] = {0, 0, 0, 0};
	TqUint destOffset = 0;
	boost::shared_ptr<Aqsis::CqImageChannel > destChan = channelFromArray(destData,
			Aqsis::Format_Unsigned8, width, height, chansPerPixel, destOffset);

	destChan->copyFrom(*srcChan);

	TqUchar expectedData[] = {2, 0, 4, 0};
	for(TqUint i = 0; i < width*height*chansPerPixel; ++i)
		BOOST_CHECK_EQUAL(destData[i], expectedData[i]);
}


BOOST_AUTO_TEST_CASE(CqImageChannel_test_copyFrom_rowSkip)
{
	TqUshort srcData[] = {0x100, 0x200,
		                  0x300, 0x400};
	TqUint realWidth = 2;
	TqUint width = 1;
	TqUint height = 2;
	TqUint chansPerPixel = 1;
	TqUint offset = 0;
	TqUint rowSkip = realWidth - width;
	boost::shared_ptr<Aqsis::CqImageChannel > srcChan = channelFromArray(srcData,
			Aqsis::Format_Unsigned16, width, height, chansPerPixel, offset, rowSkip);

	TqUchar destData[] = {0, 0};
	TqUint destOffset = 0;
	boost::shared_ptr<Aqsis::CqImageChannel > destChan = channelFromArray(destData,
			Aqsis::Format_Unsigned8, width, height, chansPerPixel, destOffset);

	destChan->copyFrom(*srcChan);

	TqUchar expectedData[] = {1, 3};
	for(TqUint i = 0; i < width*height*chansPerPixel; ++i)
		BOOST_CHECK_EQUAL(destData[i], expectedData[i]);
}

BOOST_AUTO_TEST_CASE(CqImageChannel_test_copyFrom_floatOut)
{
	TqUshort uShortMax = std::numeric_limits<TqUshort>::max();
	TqUshort srcData[] = {uShortMax/2, uShortMax};
	TqUint width = 2;
	TqUint height = 1;
	TqUint chansPerPixel = 1;
	TqUint offset = 0;
	boost::shared_ptr<Aqsis::CqImageChannel> srcChan = channelFromArray(srcData,
			Aqsis::Format_Unsigned16, width, height, chansPerPixel, offset);

	TqFloat destData[] = {0.0f, 0.0f};
	boost::shared_ptr<Aqsis::CqImageChannel> destChan = channelFromArray(destData,
			Aqsis::Format_Float32, width, height, chansPerPixel, offset);

	destChan->copyFrom(*srcChan);

	TqFloat expectedData[] = {0.5f, 1.0f};
	for(TqUint i = 0; i < width*height*chansPerPixel; ++i)
		BOOST_CHECK_CLOSE(destData[i], expectedData[i], 100.0f/uShortMax);
}

BOOST_AUTO_TEST_CASE(CqImageChannel_test_copyFromSameType)
{
	TqUshort srcData[] = {0x100, 0x200};
	TqUint width = 2;
	TqUint height = 1;
	TqUint chansPerPixel = 1;
	TqUint offset = 0;
	boost::shared_ptr<Aqsis::CqImageChannel > srcChan = channelFromArray(srcData,
			Aqsis::Format_Unsigned16, width, height, chansPerPixel, offset);

	TqUshort destData[] = {0, 0};
	boost::shared_ptr<Aqsis::CqImageChannel > destChan = channelFromArray(destData,
			Aqsis::Format_Unsigned16, width, height, chansPerPixel, offset);

	destChan->copyFrom(*srcChan);

	for(TqUint i = 0; i < width*height*chansPerPixel; ++i)
		BOOST_CHECK_EQUAL(destData[i], srcData[i]);
}

