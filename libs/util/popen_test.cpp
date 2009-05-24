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
 * \brief Unit tests for CqPopenDevice
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#include <aqsis/util/popen.h>

#include <iostream>
#include <sstream>
#include <cstring> // for std::memset()

#ifndef AQSIS_SYSTEM_WIN32
#	define BOOST_TEST_DYN_LINK
#endif
#include <boost/test/auto_unit_test.hpp>

#include <aqsis/util/exception.h>


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
