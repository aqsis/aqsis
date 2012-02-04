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
 * \brief Unit tests for functions to get magic numbers and match them against
 * the possible file types.
 *
 * \author Chris Foster
 */
#include "magicnumber.h"

#define BOOST_TEST_DYN_LINK
#include <boost/test/auto_unit_test.hpp>
#include <sstream>

#include "tifffile_test.h"

BOOST_AUTO_TEST_SUITE(magicnumber_tests)

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

/* This is a dump of a png image which is 7 pixels wide and 6 pixels height.
 * The image consists of three color components (RGB) and _no_ alpha channel.
 *
 * It consists of three horizontal (pure) colored stripes each 2 pixels height:
 * The first two rows are red, the second two are green and the last two rows are
 * blue.
 */
const unsigned char pngHeadData[85] = {
    0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0x00,0x00,0x00,0x0d,0x49,0x48,0x44,0x52
    ,0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x06,0x08,0x02,0x00,0x00,0x00,0x80,0x6c,0x13
    ,0x21,0x00,0x00,0x00,0x1c,0x49,0x44,0x41,0x54,0x78,0x9c,0x62,0xf8,0xcf,0xc0,0x80
    ,0x89,0xb0,0x08,0x81,0x44,0x71,0x08,0x53,0xaa,0x18,0x00,0x00,0x00,0xff,0xff,0x03
    ,0x00,0xca,0x46,0x29,0xd7,0x18,0x59,0x90,0xcd,0x00,0x00,0x00,0x00,0x49,0x45,0x4e
    ,0x44,0xae,0x42,0x60,0x82
}; /* end of rgb76_pngData */

BOOST_AUTO_TEST_CASE(pngMagicNumber_test)
{
	std::string pngStr(pngHeadData, pngHeadData + sizeof(pngHeadData));
	std::istringstream inStream(pngStr);

	// Check that what should be a tiff magic number actually checks out as
	// one.
	BOOST_CHECK(Aqsis::guessFileType(inStream) == Aqsis::ImageFile_Png);
}

BOOST_AUTO_TEST_SUITE_END()
