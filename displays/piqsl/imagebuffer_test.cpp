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

#define private protected
#include "imagebuffer.h"
#undef private

#include "ndspy.h"

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

namespace PrivateAccess {

class CqImageBuffer : public Aqsis::CqImageBuffer
{
	public:
		Aqsis::CqImageBuffer::quantize8bit;
		Aqsis::CqImageBuffer::quantize8bitChannelStrided;
		Aqsis::CqImageBuffer::quantizeForDisplay;
		Aqsis::CqImageBuffer::bytesPerPixel;
};

}


BOOST_AUTO_TEST_CASE(CqImageBuffer_test_quantize8bit)
{
	using namespace PrivateAccess;
	BOOST_CHECK_EQUAL(CqImageBuffer::quantize8bit(TqUchar(123)), 123);
	BOOST_CHECK_EQUAL(CqImageBuffer::quantize8bit(TqUshort(123 << (8*sizeof(TqUchar)))), 123);
	BOOST_CHECK_EQUAL(CqImageBuffer::quantize8bit(TqShort(0)), 128);
	BOOST_CHECK_EQUAL(CqImageBuffer::quantize8bit(TqUint(0xFFFFFFFF)), 0xFF);
	BOOST_CHECK_EQUAL(CqImageBuffer::quantize8bit(TqUint(0x8F123456)), 0x8F);
	BOOST_CHECK_EQUAL(CqImageBuffer::quantize8bit(TqInt(0)), 128);
	BOOST_CHECK_EQUAL(CqImageBuffer::quantize8bit(TqFloat(0.5)), 127);
	BOOST_CHECK_EQUAL(CqImageBuffer::quantize8bit(TqFloat(1.0)), 255);
	BOOST_CHECK_EQUAL(CqImageBuffer::quantize8bit(TqFloat(0.0)), 0);
}

BOOST_AUTO_TEST_CASE(CqImageBuffer_test_quantize8bitChannelStrided)
{
	using namespace PrivateAccess;
	TqUshort src[] = {0x100,0x200,0x300,0x400,0x500,0x600};
	TqUchar dest[] = {0,0,0,0,0,0};
	TqUchar expected[] =  {1,0,3,0,5,0};
	CqImageBuffer::quantize8bitChannelStrided<TqUshort>(
			reinterpret_cast<TqUchar*>(src), reinterpret_cast<TqUchar*>(dest),
			2*sizeof(TqUshort), 2*sizeof(TqUchar), 3);
	for(int i = 0; i < 6; ++i)
		BOOST_CHECK_EQUAL(dest[i], expected[i]);
}

BOOST_AUTO_TEST_CASE(CqImageBuffer_test_bytesPerPixel)
{
	using namespace PrivateAccess;
	Aqsis::TqChannelList channels;
	channels.push_back(Aqsis::CqImageChannel("r", PkDspyFloat32));
	channels.push_back(Aqsis::CqImageChannel("g", PkDspySigned8));
	channels.push_back(Aqsis::CqImageChannel("b", PkDspyUnsigned16));
	BOOST_CHECK_EQUAL(Aqsis::CqImageBuffer::bytesPerPixel(channels), TqUint(4+1+2));
}

BOOST_AUTO_TEST_CASE(CqImageBuffer_test_quantizeForDisplay)
{
	using namespace PrivateAccess;
	TqUshort src[] = {0x100,0x200,0x300,0x400,0x500,0x600};
	TqUchar dest[] = {0,0,0,0,0,0};
	TqUchar expected[] =  {1,2,3,4,5,6};
	Aqsis::TqChannelList channels;
	channels.push_back(Aqsis::CqImageChannel("r", PkDspyUnsigned16));
	channels.push_back(Aqsis::CqImageChannel("g", PkDspyUnsigned16));
	channels.push_back(Aqsis::CqImageChannel("b", PkDspyUnsigned16));
	CqImageBuffer::quantizeForDisplay(reinterpret_cast<TqUchar*>(src),
			reinterpret_cast<TqUchar*>(dest), channels, 2, 1);
	for(int i = 0; i < 6; ++i)
		BOOST_CHECK_EQUAL(dest[i], expected[i]);
}

