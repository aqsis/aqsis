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

#ifndef	AQSIS_SYSTEM_WIN32
#define BOOST_TEST_DYN_LINK
#endif //AQSIS_SYSTEM_WIN32

#include "popen.h"

#ifndef	AQSIS_SYSTEM_WIN32
#	include <time.h> // for nanosleep()
#endif

#include <boost/test/auto_unit_test.hpp>


// Suspend execution for the given number of milliseconds
void milliSleep(TqInt milliSecs)
{
#	ifdef AQSIS_SYSTEM_WIN32
	Sleep(milliSecs);
#	else
	timespec sleepTime;
	sleepTime.tv_sec = milliSecs/1000;
	sleepTime.tv_nsec = 1000000*(milliSecs % 1000);
	nanosleep(&sleepTime, 0);
#	endif // AQSIS_SYSTEM_WIN32
}

// Read a line from the input stream and return as a string.
std::string readLine(std::istream& in)
{
	std::string s;
	std::getline(in, s);
	return s;
}

BOOST_AUTO_TEST_CASE(CqPopenDevice_test)
{
	const char* argvInit[] = {
		"./pipethrough",
		"argument1",
		"arg2"
	};
	std::vector<std::string> argv(argvInit, argvInit + sizeof(argvInit)/sizeof(char*));

	Aqsis::CqPopenDevice pipeDev(argv[0], argv);

	// Allow time for the child process to start
	milliSleep(10);

	const int bufLen = 256;
	char buf[bufLen];

	{
		std::streamsize n = pipeDev.read(buf, bufLen);
		buf[n] = 0; // make sure buf is safely null-terminated.
		std::istringstream pipeOutput(buf);

		BOOST_CHECK_EQUAL(argv[0], readLine(pipeOutput));
		BOOST_CHECK_EQUAL(argv[1], readLine(pipeOutput));
		BOOST_CHECK_EQUAL(argv[2], readLine(pipeOutput));
		BOOST_CHECK_EQUAL("end-of-args", readLine(pipeOutput));
		BOOST_CHECK_EQUAL(pipeOutput.get(), EOF);
	}

	{
		pipeDev.write("a\nb1\nc23\n", 9);
		// Allow time for the child process to echo back the input.
		milliSleep(10);

		std::streamsize n = pipeDev.read(buf, bufLen);
		buf[n] = 0; // make sure buf is safely null-terminated.
		std::istringstream pipeOutput(buf);

		// The script just echos back the input lines
		BOOST_CHECK_EQUAL("a", readLine(pipeOutput));
		BOOST_CHECK_EQUAL("b1", readLine(pipeOutput));
		BOOST_CHECK_EQUAL("c23", readLine(pipeOutput));
		BOOST_CHECK_EQUAL(pipeOutput.get(), EOF);
	}
}

