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
 * \brief Unit tests for scanline-oriented pixel access for TIFF input.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 *
 */

#include "tiffinputfile.h"

#ifndef	AQSIS_SYSTEM_WIN32
#define BOOST_TEST_DYN_LINK
#endif //AQSIS_SYSTEM_WIN32

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <sstream>

#include <aqsis/tex/buffers/texturebuffer.h>
#include "tifffile_test.h"

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

