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
 * \brief Unit tests for RIB input buffer.
 * \author Chris Foster  [chris42f (at) gmail (dot) com]
 */

#include "ribinputbuffer.h"

#ifndef	AQSIS_SYSTEM_WIN32
#define BOOST_TEST_DYN_LINK
#endif //AQSIS_SYSTEM_WIN32

#include <sstream>

#include <boost/test/auto_unit_test.hpp>

using namespace Aqsis;

BOOST_AUTO_TEST_CASE(CqRibInputBuffer_sourcepos_test)
{
	std::istringstream in("some rib\ncharacters\r\nhere");
	CqRibInputBuffer inBuf(in);

	for(int i = 0; i < 7; ++i)
		inBuf.get();
	BOOST_CHECK_EQUAL(inBuf.get(), 'b');
	SqSourcePos pos = inBuf.pos();
	BOOST_CHECK_EQUAL(pos.line, 1);
	BOOST_CHECK_EQUAL(pos.col, 8);

	inBuf.get();
	BOOST_CHECK_EQUAL(inBuf.get(), 'c');
	pos = inBuf.pos();
	BOOST_CHECK_EQUAL(pos.line, 2);
	BOOST_CHECK_EQUAL(pos.col, 1);

	for(int i = 0; i < 9; ++i)
		inBuf.get();
	BOOST_CHECK_EQUAL(inBuf.get(), '\r');
	BOOST_CHECK_EQUAL(inBuf.get(), '\n');
	pos = inBuf.pos();
	BOOST_CHECK_EQUAL(pos.line, 3);
	BOOST_CHECK_EQUAL(pos.col, 0);
}

BOOST_AUTO_TEST_CASE(CqRibInputBuffer_gzip_test)
{
	// Test automatic detection and ungzip'ping of an input stream.
	//
	// The zippedChars below were generated with the command:
	//
	// echo -e -n 'some rib\ncharacters\nhere\n' | gzip | hexdump -C | sed -e 's/^[^ ]* *//' -e 's/|.*$//' -e 's/\</0x/g'
	const unsigned char zippedChars[] = {
		0x1f, 0x8b, 0x08, 0x00, 0x32, 0xd4, 0xc3, 0x48, 0x00, 0x03, 0x2b, 0xce, 0xcf, 0x4d, 0x55, 0x28, 
		0xca, 0x4c, 0xe2, 0x4a, 0xce, 0x48, 0x2c, 0x4a, 0x4c, 0x2e, 0x49, 0x2d, 0x2a, 0xe6, 0xca, 0x48, 
		0x2d, 0x4a, 0xe5, 0x02, 0x00, 0x0f, 0xc5, 0xdd, 0x8b, 0x19, 0x00, 0x00, 0x00
	};
	std::string zipStr(zippedChars, zippedChars + sizeof(zippedChars));
	std::istringstream in(zipStr);
	CqRibInputBuffer inBuf(in);

	std::istream::int_type c = 0;
	std::string extractedStr;
	while((c = inBuf.get()) != EOF)
		extractedStr += c;

	BOOST_CHECK_EQUAL(extractedStr, "some rib\ncharacters\nhere\n");
}

BOOST_AUTO_TEST_CASE(CqRibInputBuffer_bufwrap_test)
{
	// Test that buffer wrapping works correctly.
	std::string inStr(
		// 5*64 chars, which is enough to cause the buffer of 256 chars to wrap around.
		"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijkl"
		"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijkl"
		"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijkl"
		"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijkl"
		"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijkl"
	);
	std::istringstream in(inStr);
	CqRibInputBuffer inBuf(in);

	std::istream::int_type c = 0;
	std::string extractedStr;
	while((c = inBuf.get()) != EOF)
		extractedStr += c;

	BOOST_CHECK_EQUAL(extractedStr, inStr);
}
