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
 * \brief Unit tests for CqImageBuffer
 * \author Chris Foster
 */

#include "imagebuffer.h"

#include <sstream>

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <tiffio.hxx>

#include "ndspy.h"
#include "aqsismath.h"

namespace PrivateTestAccess {
//class CqImageChannel : public Aqsis::CqImageChannel
//{
//	public:
//		CqImageChannel;
//		;
//}
}

// Null deleter for holding stack-allocated stuff in a boost::shared_ptr
void nullDeleter(const void*)
{ }

//------------------------------------------------------------------------------
// CqChannelInfoList test cases

// Fixture for channel list test cases.
struct ChannelInfoListFixture
{
	Aqsis::CqChannelInfoList chanList;

	ChannelInfoListFixture()
		: chanList()
	{
		chanList.addChannel(Aqsis::SqChannelInfo("a", PkDspyUnsigned16));
		chanList.addChannel(Aqsis::SqChannelInfo("b", PkDspyUnsigned8));
		chanList.addChannel(Aqsis::SqChannelInfo("g", PkDspySigned32));
		chanList.addChannel(Aqsis::SqChannelInfo("r", PkDspyFloat32));
	}
};

BOOST_AUTO_TEST_CASE(CqChannelInfoList_channelByteOffset_test)
{
	ChannelInfoListFixture f;

	BOOST_CHECK_EQUAL(f.chanList.channelByteOffset(0), TqUint(0));
	BOOST_CHECK_EQUAL(f.chanList.channelByteOffset(1), TqUint(2));
	BOOST_CHECK_EQUAL(f.chanList.channelByteOffset(3), TqUint(7));

	BOOST_CHECK_EQUAL(f.chanList.bytesPerPixel(), TqUint(11));
}

BOOST_AUTO_TEST_CASE(CqChannelInfoList_reorderChannels_test)
{
	ChannelInfoListFixture f;

	f.chanList.reorderChannels();
	f.chanList.addChannel(Aqsis::SqChannelInfo("N", PkDspyFloat32));

	BOOST_CHECK_EQUAL(f.chanList[0].name, "r");
	BOOST_CHECK_EQUAL(f.chanList[1].name, "g");
	BOOST_CHECK_EQUAL(f.chanList[2].name, "b");
	BOOST_CHECK_EQUAL(f.chanList[3].name, "a");

	BOOST_CHECK_EQUAL(f.chanList.bytesPerPixel(), TqUint(15));
}

BOOST_AUTO_TEST_CASE(CqChannelInfoList_findChannelIndex_test)
{
	ChannelInfoListFixture f;

	BOOST_CHECK_EQUAL(f.chanList.findChannelIndex("g"), TqUint(2));
	BOOST_CHECK_THROW(f.chanList.findChannelIndex("z"), Aqsis::XqInternal);
}


//------------------------------------------------------------------------------
// CqImageChannel* test cases

// Return a channel constructed from the provided raw array.
template<typename T>
boost::shared_ptr<Aqsis::CqImageChannel> channelFromArray(T* src, TqUint type,
		TqUint width, TqUint height, TqUint chansPerPixel, TqUint offset,
		TqUint rowSkip = 0)
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
			PkDspyUnsigned16, width, height, chansPerPixel, offset);

	TqUchar destData[] = {0, 0};
	boost::shared_ptr<Aqsis::CqImageChannel > destChan = channelFromArray(destData,
			PkDspyUnsigned8, width, height, chansPerPixel, offset);

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
			PkDspyUnsigned16, width, height, chansPerPixel, offset);

	TqUchar destData[] = {0, 0, 0, 0};
	TqUint destOffset = 0;
	boost::shared_ptr<Aqsis::CqImageChannel > destChan = channelFromArray(destData,
			PkDspyUnsigned8, width, height, chansPerPixel, destOffset);

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
			PkDspyUnsigned16, width, height, chansPerPixel, offset, rowSkip);

	TqUchar destData[] = {0, 0};
	TqUint destOffset = 0;
	boost::shared_ptr<Aqsis::CqImageChannel > destChan = channelFromArray(destData,
			PkDspyUnsigned8, width, height, chansPerPixel, destOffset);

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
			PkDspyUnsigned16, width, height, chansPerPixel, offset);

	TqFloat destData[] = {0.0f, 0.0f};
	boost::shared_ptr<Aqsis::CqImageChannel> destChan = channelFromArray(destData,
			PkDspyFloat32, width, height, chansPerPixel, offset);

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
			PkDspyUnsigned16, width, height, chansPerPixel, offset);

	TqUshort destData[] = {0, 0};
	boost::shared_ptr<Aqsis::CqImageChannel > destChan = channelFromArray(destData,
			PkDspyUnsigned16, width, height, chansPerPixel, offset);

	destChan->copyFrom(*srcChan);

	for(TqUint i = 0; i < width*height*chansPerPixel; ++i)
		BOOST_CHECK_EQUAL(destData[i], srcData[i]);
}

//------------------------------------------------------------------------------
// CqImageBuffer test cases

BOOST_AUTO_TEST_CASE(CqImageBuffer_test_clear)
{
	Aqsis::CqChannelInfoList ch;
	ch.addChannel(Aqsis::SqChannelInfo("r", PkDspyUnsigned16));
	ch.addChannel(Aqsis::SqChannelInfo("b", PkDspyUnsigned16));
	TqUshort data[] = {0x100, 0x0200, 0xFA00, 0xFF00};
	TqUint width = 2;
	TqUint height = 1;
	TqUint chansPerPixel = ch.numChannels();
	Aqsis::CqImageBuffer imBuf(ch, boost::shared_array<TqUchar>(
				reinterpret_cast<TqUchar*>(data), nullDeleter), width, height);
	imBuf.clearBuffer(1.0f);

	for(TqUint i = 0; i < width*height*chansPerPixel; ++i)
		BOOST_CHECK_EQUAL(data[i], TqUshort(0xFFFF));
}

BOOST_AUTO_TEST_CASE(CqImageBuffer_test_quantizeForDisplay)
{
	Aqsis::CqChannelInfoList ch;
	ch.addChannel(Aqsis::SqChannelInfo("r", PkDspyUnsigned16));
	ch.addChannel(Aqsis::SqChannelInfo("b", PkDspyUnsigned16));
	TqUshort data[] = {0x100, 0x0200, 0xFFFF/2, 0xFFFF};
	TqUint width = 2;
	TqUint height = 1;
	TqUint chansPerPixel = ch.numChannels();
	Aqsis::CqImageBuffer imBuf(ch, boost::shared_array<TqUchar>(
				reinterpret_cast<TqUchar*>(data), nullDeleter), width, height);
	
	boost::shared_ptr<Aqsis::CqImageBuffer> quantizedBuf = imBuf.quantizeForDisplay();

	BOOST_CHECK_EQUAL(quantizedBuf->bytesPerPixel(), TqUint(2));
	BOOST_CHECK_EQUAL(quantizedBuf->channelsInfo()[0].type, TqUint(PkDspyUnsigned8));
	BOOST_CHECK_EQUAL(quantizedBuf->channelsInfo()[1].type, TqUint(PkDspyUnsigned8));

	TqUchar* quantizedData = quantizedBuf->rawData().get();
	TqUchar expectedData[] = {0x1, 0x02, 0xFF/2, 0xFF};

	for(TqUint i = 0; i < width*height*chansPerPixel; ++i)
		BOOST_CHECK_EQUAL(quantizedData[i], expectedData[i]);
}

BOOST_AUTO_TEST_CASE(CqImageBuffer_test_copyFromSubregion)
{
	Aqsis::CqChannelInfoList ch;
	ch.addChannel(Aqsis::SqChannelInfo("r", PkDspyUnsigned16));
	TqUshort srcData[] = {1, 2,
	                      3, 4};
	TqUint width = 2;
	TqUint height = 2;
	TqUint chansPerPixel = ch.numChannels();
	Aqsis::CqImageBuffer srcBuf(ch, boost::shared_array<TqUchar>(
				reinterpret_cast<TqUchar*>(srcData), nullDeleter), width, height);
	
	TqUshort destData[] = {0,0,0,
	                       0,0,0};
	TqUint destWidth = 3;
	TqUint destHeight = 2;
	Aqsis::CqImageBuffer destBuf(ch, boost::shared_array<TqUchar>(
				reinterpret_cast<TqUchar*>(destData), nullDeleter), destWidth, destHeight);

	destBuf.copyFrom(srcBuf, 0, 0);

	TqUshort expectedData[] = {1, 2, 0,
							   3, 4, 0};

	for(TqUint i = 0; i < destWidth*destHeight*chansPerPixel; ++i)
		BOOST_CHECK_EQUAL(destData[i], expectedData[i]);
}

BOOST_AUTO_TEST_CASE(CqImageBuffer_test_read_write_tiff)
{
	Aqsis::CqChannelInfoList ch;
	ch.addChannel(Aqsis::SqChannelInfo("r", PkDspyUnsigned16));
	TqUshort srcData[] = {1, 2,
	                      3, 4};
	TqUint width = 2;
	TqUint height = 2;
	TqUint chansPerPixel = ch.numChannels();
	Aqsis::CqImageBuffer srcBuf(ch, boost::shared_array<TqUchar>(
				reinterpret_cast<TqUchar*>(srcData), nullDeleter), width, height);

	// Write the image buffer to a tiff stream.
	std::ostringstream outStream;
	TIFF* outTif = TIFFStreamOpen("test_write", &outStream);
	srcBuf.saveToTiff(outTif);
	TIFFClose(outTif);

	std::istringstream inStream(outStream.str());
	TIFF* inTif = TIFFStreamOpen("test_read", &inStream);
	boost::shared_ptr<Aqsis::CqImageBuffer> destBuf = Aqsis::CqImageBuffer::loadFromTiff(inTif);

	BOOST_CHECK_EQUAL(width, destBuf->width());
	BOOST_CHECK_EQUAL(height, destBuf->height());

	TIFFClose(inTif);

	TqUshort* destData = reinterpret_cast<TqUshort*>(srcBuf.rawData().get());
	for(TqUint i = 0; i < width*height*chansPerPixel; ++i)
		BOOST_CHECK_EQUAL(destData[i], srcData[i]);
}

