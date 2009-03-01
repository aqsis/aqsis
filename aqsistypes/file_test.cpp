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
 * \brief Unit tests for file utilities.
 *
 * \author Chris Foster [ chris42f (at) g mail (dot) com ]
 */

#include "file.h"

#ifndef	AQSIS_SYSTEM_WIN32
#define BOOST_TEST_DYN_LINK
#endif //AQSIS_SYSTEM_WIN32
#include <boost/test/auto_unit_test.hpp>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include "exception.h"

using namespace Aqsis;
namespace fs = boost::filesystem;


//------------------------------------------------------------------------------
// File search tests

// Make a file, including any parent directories as necessary.
void touch(const fs::path& filePath)
{
	fs::create_directories(filePath.branch_path());
	fs::ofstream file(filePath);
	file << "created by findFile() unit tests\n";
}


BOOST_AUTO_TEST_CASE(findFile_test)
{
	fs::path filePath = "./foo/bar/somefile.txt";
	touch(filePath);

	// Check that existing files are found.
	BOOST_CHECK_EQUAL(findFile("somefile.txt", ".:./foo/bar/"), filePath);

	// Check that non-existant files throw
	BOOST_CHECK_THROW(findFile("some_nonexistant_file.txt", ".:./foo/bar/"),
			XqInvalidFile);
	// Or that an empty path is returned.
	BOOST_CHECK_EQUAL(findFileNothrow("some_nonexistant_file.txt", ".:./foo/bar/"),
			fs::path());

	// Check that explicit relative paths are always found relative to the
	// current directory, not anywhere in the search path.
	BOOST_CHECK_THROW(findFile("./somefile.txt", ".:./foo/bar/"),
			XqInvalidFile);

	// Check that directories aren't considered.
	BOOST_CHECK_THROW(findFile("./foo", "."),
			XqInvalidFile);
}


//------------------------------------------------------------------------------
// Path tokenizer tests

BOOST_AUTO_TEST_CASE(TqPathsTokenizer_basic_test)
{
	std::string searchPath = ".:/home/foo/bar:../../asdf";
	TqPathsTokenizer paths(searchPath);
	TqPathsTokenizer::iterator i = paths.begin();
	BOOST_CHECK_EQUAL(*i, ".");
	++i;
	BOOST_CHECK_EQUAL(*i, "/home/foo/bar");
	++i;
	BOOST_CHECK_EQUAL(*i, "../../asdf");
	++i;
	BOOST_CHECK(i == paths.end());
}


BOOST_AUTO_TEST_CASE(TqPathsTokenizer_delims_test)
{
	std::string searchPath = ":::.:;/home/foo/bar;../../asdf;:;;:::";
	TqPathsTokenizer paths(searchPath);
	TqPathsTokenizer::iterator i = paths.begin();
	BOOST_CHECK_EQUAL(*i, ".");
	++i;
	BOOST_CHECK_EQUAL(*i, "/home/foo/bar");
	++i;
	BOOST_CHECK_EQUAL(*i, "../../asdf");
	++i;
	BOOST_CHECK(i == paths.end());
}


#ifdef AQSIS_SYSTEM_WIN32
BOOST_AUTO_TEST_CASE(TqPathsTokenizer_drive_letter_test)
{
	std::string searchPath = "C:\\asdf\\qwer:D:\\blah\\blah;F;E:\\";
	TqPathsTokenizer paths(searchPath);
	TqPathsTokenizer::iterator i = paths.begin();
	BOOST_CHECK_EQUAL(*i, "C:\\asdf\\qwer");
	++i;
	BOOST_CHECK_EQUAL(*i, "D:\\blah\\blah");
	++i;
	BOOST_CHECK_EQUAL(*i, "F");
	++i;
	BOOST_CHECK_EQUAL(*i, "E:\\");
	++i;
	BOOST_CHECK(i == paths.end());
}
#endif // AQSIS_SYSTEM_WIN32

