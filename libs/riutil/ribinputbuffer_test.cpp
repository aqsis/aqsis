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
 * \brief Unit tests for RIB input buffer.
 * \author Chris Foster  [chris42f (at) gmail (dot) com]
 */

#include "ribinputbuffer.h"

#define BOOST_TEST_DYN_LINK

#include <stdio.h>
#include <sstream>

#include <boost/test/auto_unit_test.hpp>

using namespace Aqsis;

BOOST_AUTO_TEST_SUITE(rib_input_buffer_tests)

BOOST_AUTO_TEST_CASE(RibInputBuffer_sourcepos_test)
{
	std::istringstream in("some rib\ncharacters\r\nhere");
	RibInputBuffer inBuf(in);

	for(int i = 0; i < 7; ++i)
		inBuf.get();
	BOOST_CHECK_EQUAL(inBuf.get(), 'b');
	SourcePos pos = inBuf.pos();
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

BOOST_AUTO_TEST_CASE(RibInputBuffer_gzip_test)
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
	RibInputBuffer inBuf(in);

	std::istream::int_type c = 0;
	std::string extractedStr;
	while((c = inBuf.get()) != RibInputBuffer::eof)
		extractedStr += c;

	BOOST_CHECK_EQUAL(extractedStr, "some rib\ncharacters\nhere\n");
}

BOOST_AUTO_TEST_CASE(RibInputBuffer_bufwrap_test)
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
	RibInputBuffer inBuf(in);

	std::istream::int_type c = 0;
	std::string extractedStr;
	while((c = inBuf.get()) != RibInputBuffer::eof)
		extractedStr += c;

	BOOST_CHECK_EQUAL(extractedStr, inStr);
}

BOOST_AUTO_TEST_SUITE_END()
