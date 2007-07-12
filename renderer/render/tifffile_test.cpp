/** \file
 *
 * \brief Tests for tifffile.h
 * \author Chris Foster
 */

#include <algorithm>

#include <tiffio.h>

#include "tifffile.h"

#define BOOST_TEST_MODULE tiff_wrapper
#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

//------------------------------------------------------------------------------
// The following autogenerates some test files using only libtiff.

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

// Write a 4x6 test tiff consisting of two directories
void writeStripTiff(const char* fileName)
{
	uint32 height = 4;
	uint32 width = 6;
	uint32 rowsPerStrip = height/2;
	uint16 samplesPerPixel = 3;
	uint16 bitsPerSample = 8;

	TIFF* outFile = TIFFOpen(fileName, "w");

	setTiffFields(outFile, width, height, samplesPerPixel, bitsPerSample);
	TIFFSetField(outFile, TIFFTAG_ROWSPERSTRIP, rowsPerStrip);

	tsize_t bufSize = TIFFStripSize(outFile);
	char* buf = reinterpret_cast<char*>(_TIFFmalloc(bufSize));

	// first directory is a black strip and a white strip
	_TIFFmemset(buf, 0x00, bufSize);
	TIFFWriteEncodedStrip(outFile, 0, buf, bufSize);
	_TIFFmemset(buf, 0xFF, bufSize);
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

// Write a tiled test tiff intended to have the correct sizes for a mipmap
void writeMipmapTiff(const char* fileName)
{
	uint32 height = 5;
	uint32 width = 4;
	uint16 samplesPerPixel = 3;
	uint16 bitsPerSample = 8;
	uint32 tileWidth = 16;
	uint32 tileHeight = 16;

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

const char* stripFileName = "rgb_strip_test.tif";
const char* mipmapFileName = "mipmap_test.tif";

void writeTestTiffFiles()
{
	writeStripTiff(stripFileName);
	writeMipmapTiff(mipmapFileName);
}

//------------------------------------------------------------------------------
// Test cases


// Test the basic libtiff wrapping with CqTiffDirHandle / CqTiffFileHandle
BOOST_AUTO_TEST_CASE(CqTiffDirHandleTest)
{
	writeTestTiffFiles();

	BOOST_CHECK_THROW(Aqsis::CqTiffFileHandle("nonexistant_test.tif", "r"), Aqsis::XqEnvironment);

	boost::shared_ptr<Aqsis::CqTiffFileHandle>fileHandle (new Aqsis::CqTiffFileHandle(stripFileName, "r"));

	{
		Aqsis::CqTiffDirHandle dirHandle(fileHandle, 0);

		BOOST_CHECK(dirHandle.tiffPtr() != 0);

		BOOST_CHECK_EQUAL(dirHandle.dirIndex(), 0);

		BOOST_CHECK_EQUAL(dirHandle.tiffTagValue<uint32>(TIFFTAG_IMAGEWIDTH), 6);
		BOOST_CHECK_EQUAL(dirHandle.tiffTagValue<uint32>(TIFFTAG_IMAGEWIDTH, 0), 6);
		BOOST_CHECK_EQUAL(dirHandle.tiffTagValue<uint32>(TIFFTAG_IMAGELENGTH), 4);
		BOOST_CHECK_EQUAL(dirHandle.tiffTagValue<uint32>(TIFFTAG_ROWSPERSTRIP), 2);

		BOOST_CHECK_THROW(dirHandle.tiffTagValue<char*>(TIFFTAG_ARTIST), Aqsis::XqTiffError);

		// The following are currently causing a compile error... I don't know why.
		char* artistDefault = "blah";
		BOOST_CHECK_EQUAL(dirHandle.tiffTagValue<char*>(TIFFTAG_ARTIST, artistDefault), artistDefault);

		BOOST_CHECK(dirHandle.checkTagValue<char*>(TIFFTAG_ARTIST, artistDefault));
		BOOST_CHECK(!dirHandle.checkTagValue<char*>(TIFFTAG_ARTIST, artistDefault, false));
		BOOST_CHECK(dirHandle.checkTagValue<uint32>(TIFFTAG_IMAGEWIDTH, 6));
	}
	{
		Aqsis::CqTiffDirHandle dirHandle(fileHandle, 1);

		BOOST_CHECK_EQUAL(dirHandle.dirIndex(), 1);
	}
	{
		BOOST_CHECK_THROW(Aqsis::CqTiffDirHandle(fileHandle, 2), Aqsis::XqTiffError);
	}
}

// Test the high-level tiff input class
BOOST_AUTO_TEST_CASE(CqTiffInputFileTest)
{
	writeTestTiffFiles();
	Aqsis::CqTiffInputFile stripFile(stripFileName);
	BOOST_CHECK_EQUAL(stripFile.tileWidth(), 6);
	BOOST_CHECK_EQUAL(stripFile.tileHeight(), 2);
}

