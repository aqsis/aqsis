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
 * \brief Unit tests for CqPopenDevice
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#include <aqsis/util/popen.h>

#include <iostream>
#include <sstream>
#include <cstring> // for std::memset()

#define BOOST_TEST_DYN_LINK
#include <boost/test/auto_unit_test.hpp>

#include <aqsis/util/exception.h>

BOOST_AUTO_TEST_SUITE(popen_tests)
using namespace Aqsis;

//------------------------------------------------------------------------------
// Simple linear buffer for testing purposes.  This like a very crippled
// version of a std::streambuf, and should be a simple approximation of the
// buffering which goes on inside boost::streambuf<CqPopenDevice>
//
// Warning: when m_bufSize is reached something bad will occur!
class LineBuffer
{
	private:
		CqPopenDevice& m_device;
		static const int m_bufSize = 1024;
		char m_buf[m_bufSize];
		char* m_pos;
		char* m_end;

		// Get a character from the device.
		char getChar()
		{
			if(m_pos == m_end)
			{
				// Buffer in some more characters.
				std::streamsize nRead = 0;
				while(nRead == 0)
					nRead = m_device.read(m_end, m_buf+m_bufSize - m_end);
				if(nRead == -1)
					return '\t';
				m_end += nRead;
			}
			return *(m_pos++);
		}
	public:
		LineBuffer(CqPopenDevice& device)
			: m_device(device),
			m_pos(m_buf),
			m_end(m_buf)
		{
			std::memset(m_buf, 0, m_bufSize);
		}

		// Get a line from the device, terminated with '\t' to avoid problems
		// with windows end-of-line translation.
		std::string getLine()
		{
			std::string result;
			char c = getChar();
			while(c != '\t')
			{
				result += c;
				c = getChar();
			}
			return result;
		}
};

//------------------------------------------------------------------------------
// Test cases

BOOST_AUTO_TEST_CASE(CqPopenDevice_test)
{
	const char* argvInit[] = {
		"./pipethrough",
		"argument1",
		"arg2"
	};
	std::vector<std::string> argv(argvInit, argvInit + sizeof(argvInit)/sizeof(char*));

	CqPopenDevice pipeDev(argv[0], argv);

	LineBuffer pipe(pipeDev);

	BOOST_CHECK_EQUAL(argv[0], pipe.getLine());
	BOOST_CHECK_EQUAL(argv[1], pipe.getLine());
	BOOST_CHECK_EQUAL(argv[2], pipe.getLine());
	BOOST_CHECK_EQUAL("end-of-args", pipe.getLine());

	pipeDev.write("a\tb1\tc23\t", 9);
	// The pipethrough executable just echos back the input lines
	BOOST_CHECK_EQUAL("a", pipe.getLine());
	BOOST_CHECK_EQUAL("b1", pipe.getLine());
	BOOST_CHECK_EQUAL("c23", pipe.getLine());
}


BOOST_AUTO_TEST_CASE(CqPopenDevice_earlyexit_test)
{
	const char* argvInit[] = {
		"./pipethrough",
		"-earlyexit"
	};
	std::vector<std::string> argv(argvInit, argvInit + sizeof(argvInit)/sizeof(char*));

	CqPopenDevice pipeDev(argv[0], argv);
	LineBuffer pipe(pipeDev);

	BOOST_CHECK_EQUAL(argv[0], pipe.getLine());
	BOOST_CHECK_EQUAL(argv[1], pipe.getLine());
	BOOST_CHECK_EQUAL("end-of-args", pipe.getLine());

	// The child process should have exited at this point, so writing should fail.
	// TODO: This test has a race condition, since the child process may not
	// have exited yet.  Need to figure out how to fix it...
//	BOOST_CHECK_THROW(pipeDev.write("a\tb1\tc23\t", 9),
//			std::ios_base::failure);

	// Check that the device thinks that it's reached EOF.
	char buf[2];
	BOOST_CHECK_EQUAL(pipeDev.read(buf, 2), -1);
}


BOOST_AUTO_TEST_CASE(CqPopenDevice_notfound_test)
{
	// Check that trying to open a nonexistant executable throws an error.
	const char* argvInit[] = {
		"some_nonexistant_executable"
	};
	std::vector<std::string> argv(argvInit, argvInit + sizeof(argvInit)/sizeof(char*));

	BOOST_CHECK_THROW(
		CqPopenDevice pipeDev(argv[0], argv),
		XqEnvironment
	);
}

BOOST_AUTO_TEST_SUITE_END()
