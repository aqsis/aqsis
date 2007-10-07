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
 * \brief Tests for the tiff file wrappers
 * \author Chris Foster
 */

#include <algorithm>

#include "tifffile.h"

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

using namespace Aqsis;

//------------------------------------------------------------------------------
// Declare fixtures which wrap up some binary tiff data.
//
// The following binary data was generated via libtiff and a python script at
// the bottom of this file.
//


// stripped tiff data:


// tiled tiff data:

const unsigned char stripTiffData[] =
{
73,73,42,0,38,0,0,0,120,156,251,207,128,14,254,99,136,0,0,53,238,1,255,120,156,
99,96,248,207,128,14,208,69,0,49,242,1,255,13,0,0,1,3,0,1,0,0,0,6,0,0,0,1,1,
3,0,1,0,0,0,4,0,0,0,2,1,3,0,3,0,0,0,200,0,0,0,3,1,3,0,1,0,0,0,178,128,0,0,6,
1,3,0,1,0,0,0,2,0,0,0,17,1,4,0,2,0,0,0,206,0,0,0,18,1,3,0,1,0,0,0,1,0,0,0,21,
1,3,0,1,0,0,0,3,0,0,0,22,1,3,0,1,0,0,0,2,0,0,0,23,1,4,0,2,0,0,0,214,0,0,0,28,
1,3,0,1,0,0,0,1,0,0,0,61,1,3,0,1,0,0,0,2,0,0,0,83,1,3,0,3,0,0,0,222,0,0,0,0,
1,0,0,8,0,8,0,8,0,8,0,0,0,23,0,0,0,15,0,0,0,15,0,0,0,1,0,1,0,1,0,120,156,251,
255,255,63,3,42,248,143,33,2,0,155,136,5,251,120,156,99,96,32,12,0,0,36,0,1,
13,0,0,1,3,0,1,0,0,0,6,0,0,0,1,1,3,0,1,0,0,0,4,0,0,0,2,1,3,0,3,0,0,0,162,1,0,
0,3,1,3,0,1,0,0,0,178,128,0,0,6,1,3,0,1,0,0,0,2,0,0,0,17,1,4,0,2,0,0,0,168,1,
0,0,18,1,3,0,1,0,0,0,1,0,0,0,21,1,3,0,1,0,0,0,3,0,0,0,22,1,3,0,1,0,0,0,2,0,0,
0,23,1,4,0,2,0,0,0,176,1,0,0,28,1,3,0,1,0,0,0,1,0,0,0,61,1,3,0,1,0,0,0,2,0,0,
0,83,1,3,0,3,0,0,0,184,1,0,0,0,0,0,0,8,0,8,0,8,0,228,0,0,0,245,0,0,0,17,0,0,
0,11,0,0,0,1,0,1,0,1,0
};
const std::string stripTiffString(stripTiffData, stripTiffData+sizeof(stripTiffData));

const unsigned char tileTiffData[] =
{
73,73,42,0,24,0,0,0,120,156,99,96,24,5,163,96,228,2,0,3,0,0,1,0,14,0,0,1,3,0,
1,0,0,0,4,0,0,0,1,1,3,0,1,0,0,0,5,0,0,0,2,1,3,0,3,0,0,0,198,0,0,0,3,1,3,0,1,
0,0,0,178,128,0,0,6,1,3,0,1,0,0,0,2,0,0,0,18,1,3,0,1,0,0,0,1,0,0,0,21,1,3,0,
1,0,0,0,3,0,0,0,28,1,3,0,1,0,0,0,1,0,0,0,61,1,3,0,1,0,0,0,2,0,0,0,66,1,3,0,1,
0,0,0,16,0,0,0,67,1,3,0,1,0,0,0,16,0,0,0,68,1,4,0,1,0,0,0,8,0,0,0,69,1,4,0,1,
0,0,0,15,0,0,0,83,1,3,0,3,0,0,0,204,0,0,0,226,0,0,0,8,0,8,0,8,0,1,0,1,0,1,0,
120,156,99,96,24,5,163,96,228,2,0,3,0,0,1,0,14,0,0,1,3,0,1,0,0,0,2,0,0,0,1,1,
3,0,1,0,0,0,3,0,0,0,2,1,3,0,3,0,0,0,144,1,0,0,3,1,3,0,1,0,0,0,178,128,0,0,6,
1,3,0,1,0,0,0,2,0,0,0,18,1,3,0,1,0,0,0,1,0,0,0,21,1,3,0,1,0,0,0,3,0,0,0,28,1,
3,0,1,0,0,0,1,0,0,0,61,1,3,0,1,0,0,0,2,0,0,0,66,1,3,0,1,0,0,0,16,0,0,0,67,1,
3,0,1,0,0,0,16,0,0,0,68,1,4,0,1,0,0,0,210,0,0,0,69,1,4,0,1,0,0,0,15,0,0,0,83,
1,3,0,3,0,0,0,150,1,0,0,172,1,0,0,8,0,8,0,8,0,1,0,1,0,1,0,120,156,99,96,24,5,
163,96,228,2,0,3,0,0,1,0,14,0,0,1,3,0,1,0,0,0,1,0,0,0,1,1,3,0,1,0,0,0,2,0,0,
0,2,1,3,0,3,0,0,0,90,2,0,0,3,1,3,0,1,0,0,0,178,128,0,0,6,1,3,0,1,0,0,0,2,0,0,
0,18,1,3,0,1,0,0,0,1,0,0,0,21,1,3,0,1,0,0,0,3,0,0,0,28,1,3,0,1,0,0,0,1,0,0,0,
61,1,3,0,1,0,0,0,2,0,0,0,66,1,3,0,1,0,0,0,16,0,0,0,67,1,3,0,1,0,0,0,16,0,0,0,
68,1,4,0,1,0,0,0,156,1,0,0,69,1,4,0,1,0,0,0,15,0,0,0,83,1,3,0,3,0,0,0,96,2,0,
0,118,2,0,0,8,0,8,0,8,0,1,0,1,0,1,0,120,156,99,96,24,5,163,96,228,2,0,3,0,0,
1,0,14,0,0,1,3,0,1,0,0,0,1,0,0,0,1,1,3,0,1,0,0,0,1,0,0,0,2,1,3,0,3,0,0,0,36,
3,0,0,3,1,3,0,1,0,0,0,178,128,0,0,6,1,3,0,1,0,0,0,2,0,0,0,18,1,3,0,1,0,0,0,1,
0,0,0,21,1,3,0,1,0,0,0,3,0,0,0,28,1,3,0,1,0,0,0,1,0,0,0,61,1,3,0,1,0,0,0,2,0,
0,0,66,1,3,0,1,0,0,0,16,0,0,0,67,1,3,0,1,0,0,0,16,0,0,0,68,1,4,0,1,0,0,0,102,
2,0,0,69,1,4,0,1,0,0,0,15,0,0,0,83,1,3,0,3,0,0,0,42,3,0,0,0,0,0,0,8,0,8,0,8,
0,1,0,1,0,1,0
};
const std::string tileTiffString(tileTiffData, tileTiffData+sizeof(tileTiffData));

// Null deleter for holding stack-allocated stuff in a boost::shared_ptr
void nullDeleter(const void*)
{ }

// Fixture which includes a CqTiffFileHandle allocated using a std::string as
// the "file" buffer
struct TiffFileHandleFixture
{
	std::istringstream tiffStream;
	CqTiffFileHandle fileHandle;
	boost::shared_ptr<CqTiffFileHandle> fileHandlePtr;
	
	TiffFileHandleFixture(const std::string& data)
		: tiffStream(data),
		fileHandle(tiffStream),
		fileHandlePtr(&fileHandle, nullDeleter)
	{ }
};

struct TiffInputFileFixture
{
	std::istringstream tiffStream;
	CqTiffInputFile file;
	
	TiffInputFileFixture(const std::string& data)
		: tiffStream(data),
		file(tiffStream)
	{ }
};

//------------------------------------------------------------------------------
// Test cases for CqTiffInputFile
BOOST_AUTO_TEST_CASE(CqTiffInputFile_test_strip_image_sizes)
{
	TiffInputFileFixture f(stripTiffString);
	BOOST_CHECK_EQUAL(f.file.tileWidth(), TqUint(6));
	BOOST_CHECK_EQUAL(f.file.tileHeight(), TqUint(2));

	BOOST_CHECK_EQUAL(f.file.width(), TqUint(6));
	BOOST_CHECK_EQUAL(f.file.height(), TqUint(4));
}

BOOST_AUTO_TEST_CASE(CqTiffInputFile_test_tile_image_sizes)
{
	TiffInputFileFixture f(tileTiffString);

	BOOST_CHECK_EQUAL(f.file.width(), TqUint(4));
	BOOST_CHECK_EQUAL(f.file.height(), TqUint(5));

	BOOST_CHECK_EQUAL(f.file.tileWidth(), TqUint(16));
	BOOST_CHECK_EQUAL(f.file.tileHeight(), TqUint(16));
}

BOOST_AUTO_TEST_CASE(CqTiffInputFile_test_checkDirSizes)
{
	TiffInputFileFixture f(stripTiffString);

	std::vector<TqUint> widths(2,6);
	std::vector<TqUint> heights(2,4);

	BOOST_CHECK(f.file.checkDirSizes(widths, heights));
}

template<typename T>
void assertTileEqualsColor(boost::intrusive_ptr<CqTextureTile<T> > tilePtr, TqFloat* desiredCol)
{
	for(TqUint x = tilePtr->topLeftX(); x < tilePtr->topLeftX() + tilePtr->width(); ++x)
	{
		for(TqUint y = tilePtr->topLeftY(); y < tilePtr->topLeftX() + tilePtr->width(); ++y)
		{
			for(TqUint i = 0; i < tilePtr->samplesPerPixel(); ++i)
				BOOST_CHECK_EQUAL(tilePtr->value(x,y)[i], desiredCol[i]);
		}
	}
}

/*BOOST_AUTO_TEST_CASE(CqTiffInputFile_test_strip_readtile)
{
	TiffInputFileFixture f(stripTiffString);
	{
		boost::intrusive_ptr<CqTextureTile<TqUchar> > tilePtr = f.file.readTile<TqUchar>(0,0);

		TqFloat red[] = {1.0f,0.0f,0.0f};
		TqFloat blue[] = {0.0f,0.0f,1.0f};

		assertTileEqualsColor(f.file.readTile<TqUchar>(0,0), red);
		assertTileEqualsColor(f.file.readTile<TqUchar>(0,1), blue);
	}

	{
		// Check values from bottom tile.
		boost::intrusive_ptr<CqTextureTile<TqUchar> > tilePtr = f.file.readTile<TqUchar>(0,1);
		TqUint x = f.file.width()-1;
		TqUint y = f.file.height()-1;
		TqFloat tileCol[] = {1.0f,1.0f,1.0f};
		BOOST_CHECK_CLOSE(tilePtr->value(x,y)[0], 1.0f, 1e-5f);
		BOOST_CHECK_CLOSE(tilePtr->value(x,y)[1], 1.0f, 1e-5f);
		BOOST_CHECK_CLOSE(tilePtr->value(x,y)[2], 1.0f, 1e-5f);
	}

	// Try to read the tile in an invalid format.
	BOOST_CHECK_THROW(f.file.readTile<TqFloat>(0,0), XqTiffError);
}


BOOST_AUTO_TEST_CASE(CqTiffInputFile_test_readtile)
{
}*/

