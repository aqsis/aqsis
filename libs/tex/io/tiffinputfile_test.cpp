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
 * \brief Unit tests for scanline-oriented pixel access for TIFF input.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 *
 */

#include "tiffinputfile.h"

#define BOOST_TEST_DYN_LINK
#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <sstream>

#include <aqsis/tex/buffers/texturebuffer.h>
#include "tifffile_test.h"

BOOST_AUTO_TEST_SUITE(tiffinputfile_tests)

// note: header stuff is tested elsewhere, in tiffdirhandle_test.

BOOST_AUTO_TEST_CASE(CqTiffInputFile_readPixels_test)
{
	std::istringstream inStream(stripTiffString);
	Aqsis::CqTiffInputFile inFile(inStream);

	// Read the whole tiff into a buffer
	Aqsis::CqTextureBuffer<TqUint8> buffer;
	inFile.readPixels(buffer);

	// Check that the buffer is the right size.
	BOOST_CHECK_EQUAL(buffer.width(), 6);
	BOOST_CHECK_EQUAL(buffer.height(), 4);
	BOOST_CHECK_EQUAL(buffer.numChannels(), 3);

	// Check that the buffer contains the right data.
	// part of the fist strip (should be red)
	BOOST_CHECK_CLOSE(buffer(0,0)[0], 1.0f, 1e-5);
	BOOST_CHECK_CLOSE(buffer(0,0)[1], 0.0f, 1e-5);
	BOOST_CHECK_CLOSE(buffer(0,0)[2], 0.0f, 1e-5);
	// part of the second strip (should be blue)
	BOOST_CHECK_CLOSE(buffer(0,2)[0], 0.0f, 1e-5);
	BOOST_CHECK_CLOSE(buffer(0,2)[1], 0.0f, 1e-5);
	BOOST_CHECK_CLOSE(buffer(0,2)[2], 1.0f, 1e-5);
}

// Check that the second sub-image of the tiff data from stripTiffString
// contains the expected data.
BOOST_AUTO_TEST_CASE(CqTiffInputFile_readPixels_secondSubImage_test)
{
	std::istringstream inStream(stripTiffString);
	Aqsis::CqTiffInputFile inFile(inStream);
	inFile.setImageIndex(1);

	// Read the whole tiff into a buffer
	Aqsis::CqTextureBuffer<TqUint8> buffer;
	inFile.readPixels(buffer);

	// Check that the buffer is the right size.
	BOOST_CHECK_EQUAL(buffer.width(), 6);
	BOOST_CHECK_EQUAL(buffer.height(), 4);
	BOOST_CHECK_EQUAL(buffer.numChannels(), 3);

	// part of the fist strip (should be white)
	BOOST_CHECK_CLOSE(buffer(0,0)[0], 1.0f, 1e-5);
	BOOST_CHECK_CLOSE(buffer(0,0)[1], 1.0f, 1e-5);
	BOOST_CHECK_CLOSE(buffer(0,0)[2], 1.0f, 1e-5);
	// part of the second strip (should be black)
	BOOST_CHECK_CLOSE(buffer(0,2)[0], 0.0f, 1e-5);
	BOOST_CHECK_CLOSE(buffer(0,2)[1], 0.0f, 1e-5);
	BOOST_CHECK_CLOSE(buffer(0,2)[2], 0.0f, 1e-5);
}

BOOST_AUTO_TEST_CASE(CqTiffInputFile_imageIndex_test)
{
	std::istringstream inStream(stripTiffString);
	Aqsis::CqTiffInputFile inFile(inStream);

	BOOST_CHECK_EQUAL(inFile.imageIndex(), 0);
	inFile.setImageIndex(1);
	BOOST_CHECK_EQUAL(inFile.imageIndex(), 1);

	BOOST_CHECK_EQUAL(inFile.numSubImages(), 2);
}

BOOST_AUTO_TEST_SUITE_END()
