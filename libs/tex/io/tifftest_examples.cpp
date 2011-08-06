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
 * \brief Code to create tiff testing examples using only libtiff.
 *
 * This example generating code requires only libtiff, and can be compiled with
 * something like:
 *
 * g++ tifftest_examples.cpp -ltiff
 *
 * \author Chris Foster
 */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

#include <tiffio.h>

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
	TIFFSetField(outFile, TIFFTAG_IMAGEDESCRIPTION, "Strip-allocated tiff for unit tests");

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
	// the way it outputs to std::ostream (so we can't just use a stringstream
	// with libtiffxx :-/ ).

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

