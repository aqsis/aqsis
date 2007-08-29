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

#ifndef AQSIS_GENERATE_TIFF_EXAMPLES

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
// Test cases for CqTiffFileHandle

BOOST_AUTO_TEST_CASE(CqTiffFileHandle_test_constructor)
{
	BOOST_CHECK_THROW(CqTiffFileHandle("blah_nonexistant_test.tif", "r"),
			XqEnvironment);
	BOOST_MESSAGE("Unit test note: Expect a libtiff error about blah_nonexistant_test.tif above ^^");

	// just create.  Should not throw.
	TiffFileHandleFixture f(stripTiffString);
}


//------------------------------------------------------------------------------
// Test cases for CqTiffDirHandle

BOOST_AUTO_TEST_CASE(CqTiffDirHandle_test_dirindex)
{
	TiffFileHandleFixture f(stripTiffString);
	{
		CqTiffDirHandle dirHandle(f.fileHandlePtr, 1);

		BOOST_CHECK(dirHandle.tiffPtr() != 0);

		BOOST_CHECK_EQUAL(dirHandle.dirIndex(), 1);
	}
	{
		BOOST_CHECK_THROW(CqTiffDirHandle(f.fileHandlePtr, 2), XqTiffError);
	}
}


BOOST_AUTO_TEST_CASE(CqTiffDirHandle_test_tifftag_stuff)
{
	TiffFileHandleFixture f(stripTiffString);

	CqTiffDirHandle dirHandle(f.fileHandlePtr, 0);

	BOOST_CHECK_EQUAL(dirHandle.tiffTagValue<uint32>(TIFFTAG_IMAGEWIDTH), uint32(6));
	BOOST_CHECK_EQUAL(dirHandle.tiffTagValue<uint32>(TIFFTAG_IMAGEWIDTH, 0), uint32(6));
	BOOST_CHECK_EQUAL(dirHandle.tiffTagValue<uint32>(TIFFTAG_IMAGELENGTH), uint32(4));
	BOOST_CHECK_EQUAL(dirHandle.tiffTagValue<uint32>(TIFFTAG_ROWSPERSTRIP), uint32(2));

	// There is no ARTIST TAG in the created tiffs - hence an exception
	BOOST_CHECK_THROW(dirHandle.tiffTagValue<char*>(TIFFTAG_ARTIST), XqTiffError);

	char* artistDefault = "blah";
	BOOST_CHECK_EQUAL(dirHandle.tiffTagValue<char*>(TIFFTAG_ARTIST, artistDefault), artistDefault);

	BOOST_CHECK(dirHandle.checkTagValue<uint32>(TIFFTAG_IMAGEWIDTH, 6));
	BOOST_CHECK(dirHandle.checkTagValue<char*>(TIFFTAG_ARTIST, artistDefault));
	BOOST_CHECK(!dirHandle.checkTagValue<char*>(TIFFTAG_ARTIST, artistDefault, false));
}


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


#else // ifndef AQSIS_GENERATE_TIFF_EXAMPLES

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// The following autogenerates some test files using only libtiff.  The results
// have been included above as binary blobs for ease of unit testing.
//------------------------------------------------------------------------------

#include <tiffio.h>
#include <tiffio.hxx>
#include <sstream>
#include <fstream>
#include <iostream>

void setTiffFields(TIFF* outFile, const uint32 width, const uint32 height,
		const uint16 samplesPerPixel, const uint16 bitsPerSample)
{
	TIFFSetField(outFile, TIFFTAG_IMAGEWIDTH, width);
	TIFFSetField(outFile, TIFFTAG_IMAGELENGTH, height);
	TIFFSetField(outFile, TIFFTAG_SAMPLESPERPIXEL, samplesPerPixel);
	TIFFSetField(outFile, TIFFTAG_BITSPERSAMPLE, bitsPerSample);
	TIFFSetField(outFile, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	TIFFSetField(outFile, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
	TIFFSetField(outFile, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(outFile, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);

	TIFFSetField(outFile, TIFFTAG_COMPRESSION, COMPRESSION_DEFLATE);
	TIFFSetField(outFile, TIFFTAG_PREDICTOR, PREDICTOR_HORIZONTAL);
}

// Set a buffer to a given color.
void setBufToColor(unsigned char* buf, const int bufSize,
		const unsigned char* color, const int numColors)
{
	for(int i = 0; i < bufSize; ++i)
		for(int j = 0; j < numColors; ++j)
			buf[i*numColors + j] = color[j];
}

// Write a 6x4 test tiff consisting of two directories
void writeStripTiff(const char* fileName)
{
	uint32 height = 4;
	uint32 width = 6;
	uint32 rowsPerStrip = height/2;
	uint16 samplesPerPixel = 3;
	uint16 bitsPerSample = 8;

	const unsigned char red[] = {0xFF, 0, 0};
	const unsigned char blue[] = {0, 0, 0xFF};

	//TIFF* outFile = TIFFStreamOpen("stream", &outStream);
	TIFF* outFile = TIFFOpen(fileName, "w");

	setTiffFields(outFile, width, height, samplesPerPixel, bitsPerSample);
	TIFFSetField(outFile, TIFFTAG_ROWSPERSTRIP, rowsPerStrip);

	tsize_t bufSize = TIFFStripSize(outFile);
	unsigned char* buf = reinterpret_cast<unsigned char*>(_TIFFmalloc(bufSize));

	// first directory is a red strip and a blue strip.
	setBufToColor(buf, bufSize/samplesPerPixel, red, samplesPerPixel);
	TIFFWriteEncodedStrip(outFile, 0, buf, bufSize);
	setBufToColor(buf, bufSize/samplesPerPixel, blue, samplesPerPixel);
	TIFFWriteEncodedStrip(outFile, 1, buf, bufSize);

	TIFFWriteDirectory(outFile);

	setTiffFields(outFile, width, height, samplesPerPixel, bitsPerSample);
	TIFFSetField(outFile, TIFFTAG_ROWSPERSTRIP, rowsPerStrip);

	// second directory is a white strip and a black strip
	_TIFFmemset(buf, 0xFF, bufSize);
	TIFFWriteEncodedStrip(outFile, 0, buf, bufSize);
	_TIFFmemset(buf, 0x00, bufSize);
	TIFFWriteEncodedStrip(outFile, 1, buf, bufSize);

	TIFFClose(outFile);
	_TIFFfree(buf);
	//TIFFSetField(outFile, TIFFTAG_TILEWIDTH, tileWidth);
	//TIFFSetField(outFile, TIFFTAG_TILELENGTH, tileHeight);
}

// Write a 4x5 tiled test tiff intended to have the correct sizes for a mipmap
void writeTiledTiff(const char* fileName)
{
	uint32 height = 5;
	uint32 width = 4;
	uint16 samplesPerPixel = 3;
	uint16 bitsPerSample = 8;
	uint32 tileWidth = 16;
	uint32 tileHeight = 16;

	//TIFF* outFile = TIFFStreamOpen("stream", &outStream);
	TIFF* outFile = TIFFOpen(fileName, "w");

	setTiffFields(outFile, width, height, samplesPerPixel, bitsPerSample);
	TIFFSetField(outFile, TIFFTAG_TILEWIDTH, tileWidth);
	TIFFSetField(outFile, TIFFTAG_TILELENGTH, tileHeight);

	tsize_t bufSize = TIFFTileSize(outFile);
	char* buf = reinterpret_cast<char*>(_TIFFmalloc(bufSize));
	_TIFFmemset(buf, 0x00, bufSize);

	// first directory
	TIFFWriteEncodedTile(outFile, 0, buf, bufSize);
	TIFFWriteDirectory(outFile);

	// rest of the directories
	while(width > 1 || height > 1)
	{
		width = std::max<uint32>((width+1)/2, 1);
		height = std::max<uint32>((height+1)/2, 1);
		setTiffFields(outFile, width, height, samplesPerPixel, bitsPerSample);
		TIFFSetField(outFile, TIFFTAG_TILEWIDTH, tileWidth);
		TIFFSetField(outFile, TIFFTAG_TILELENGTH, tileHeight);

		TIFFWriteEncodedTile(outFile, 0, buf, bufSize);
		TIFFWriteDirectory(outFile);
	}

	TIFFClose(outFile);
	_TIFFfree(buf);
}

void outputStreamAsNumArray(std::istream& inStream, const int lineLen)
{
	std::ostringstream oss;

	while(inStream)
	{
		std::istream::int_type c = inStream.get();
		if(c != EOF)
		{
			oss << static_cast<int>(static_cast<unsigned char>(c)) << ",";
			if(oss.tellp() > lineLen)
			{
				std::cout << oss.str();
				std::cout << "\n";
				oss.str("");
			}
		}
	}
	std::cout << oss.str();
	std::cout << "\n";
}

int main()
{
	const int lineLen = 75;
	// We've gotta do this messy thing - writing out to a file, and then
	// reading back in again.  This is because libtiff seems to have a bug in
	// the way it outputs to std::ostream (so we can't just use a stringstream).

	{
		const char* stripFileName = "stripped.tif";
		writeStripTiff(stripFileName);
		std::ifstream inFile(stripFileName);
		std::cout << "// stripped tiff data:\n";
		outputStreamAsNumArray(inFile, lineLen);
	}

	std::cout << "\n\n";

	{
		const char* tileFileName = "tiled.tif";
		writeTiledTiff(tileFileName);
		std::ifstream inFile(tileFileName);
		std::cout << "// tiled tiff data:\n";
		outputStreamAsNumArray(inFile, lineLen);
	}

	return 0;
}

#endif // ifndef AQSIS_GENERATE_TIFF_EXAMPLES
