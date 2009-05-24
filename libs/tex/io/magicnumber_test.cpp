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
 * \brief Unit tests for functions to get magic numbers and match them against
 * the possible file types.
 *
 * \author Chris Foster
 */
#include "magicnumber.h"

#ifndef	AQSIS_SYSTEM_WIN32
#define BOOST_TEST_DYN_LINK
#endif //AQSIS_SYSTEM_WIN32

#include <boost/test/auto_unit_test.hpp>
#include <sstream>

#include "tifffile_test.h"

BOOST_AUTO_TEST_CASE(tiffMagicNumber_test)
{
	std::istringstream inStream(stripTiffString);

	BOOST_CHECK(Aqsis::guessFileType(inStream) == Aqsis::ImageFile_Tiff);
}


// Some data from the start of an OpenExr file.  Note: that this isn't at all a
// complete file; it's just a small part for testing purposes.
const char exrHeadData[] = {
	118,47,49,1,2,0,0,0,99,97,112,68,97,116,101,0,115,116,114,105,110,103,0,19,0,
	0,0,50,48,48,50,58,48,54,58,50,51,32,49,53,58,48,48,58,48,48,99,104,97,110,110,
	101,108,115,0,99,104,108,105,115,116,0,73,0,0,0,65,0,1,0,0,0,0,0,0,0,1,0,0,0,
	1,0,0,0,66,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,71,0,1,0,0,0,0,0,0,0,1,0,0,0,1
};

BOOST_AUTO_TEST_CASE(exrMagicNumber_test)
{
	std::string exrStr(exrHeadData, exrHeadData + sizeof(exrHeadData));
	std::istringstream inStream(exrStr);

	BOOST_CHECK(Aqsis::guessFileType(inStream) == Aqsis::ImageFile_Exr);
}

// Some data from the start of an aqsis bake file.  (It's an ASCII format)
const char bakeHeadData[] =
	"Aqsis bake file\n"
	"3\n"
	"0.326785 0.207725 0.986308 0.726669 0.534031\n"
	"0.327523 0.20563 0.985446 0.726986 0.534352\n";

BOOST_AUTO_TEST_CASE(bakeMagicNumber_test)
{
	std::string bakeStr(bakeHeadData, bakeHeadData + sizeof(bakeHeadData));
	std::istringstream inStream(bakeStr);

	// Check that what should be a tiff magic number actually checks out as
	// one.
	BOOST_CHECK(Aqsis::guessFileType(inStream) == Aqsis::ImageFile_AqsisBake);
}

