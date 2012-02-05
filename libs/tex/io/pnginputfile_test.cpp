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
 * \brief Unit tests for scanline-oriented pixel access for PNG input.
 *
 * \author Peter Dusel  hdusel _at_ tangerine-soft.de
 *
 */

#include "pnginputfile.h"

#define BOOST_TEST_DYN_LINK
#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <boost/filesystem.hpp>
#include <fstream>

#include <aqsis/tex/buffers/texturebuffer.h>
#include <aqsis/util/tinyformat.h>

/* This is a dump of a png image which is 7 pixels wide and 6 pixels height.
 * The image consists of three color components (RGB) and _no_ alpha channel.
 *
 * It consists of three horizontal (pure) colored stripes each 2 pixels height:
 * The first two rows are red, the second two are green and the last two rows are
 * blue.
 */
const unsigned char rgb76_pngData[85] = {
     0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0x00,0x00,0x00,0x0d,0x49,0x48,0x44,0x52
    ,0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x06,0x08,0x02,0x00,0x00,0x00,0x80,0x6c,0x13
    ,0x21,0x00,0x00,0x00,0x1c,0x49,0x44,0x41,0x54,0x78,0x9c,0x62,0xf8,0xcf,0xc0,0x80
    ,0x89,0xb0,0x08,0x81,0x44,0x71,0x08,0x53,0xaa,0x18,0x00,0x00,0x00,0xff,0xff,0x03
    ,0x00,0xca,0x46,0x29,0xd7,0x18,0x59,0x90,0xcd,0x00,0x00,0x00,0x00,0x49,0x45,0x4e
    ,0x44,0xae,0x42,0x60,0x82
}; /* end of rgb76_pngData */

/* This is a dump of a png image which is 7 pixels wide and 8 pixels height.
 * The image consists of four color components (RGBA) thus it contains an alpha
 * channel.
 *
 * It consists of four horizontal (pure) colored stripes each 2 pixels height:
 * The first two rows are red, the second two are green and the last two rows are
 * blue. These first three stripes are opaque which means that their alpha channel
 * is 1.0. The last two rows of this image is pure transparent which means that
 * its alpha channel value is 0.0.
 */
const unsigned char rgba78_pngData[96] = {
     0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0x00,0x00,0x00,0x0d,0x49,0x48,0x44,0x52
    ,0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x08,0x08,0x06,0x00,0x00,0x00,0x35,0x04,0xe5
    ,0x06,0x00,0x00,0x00,0x27,0x49,0x44,0x41,0x54,0x78,0x9c,0x62,0xf8,0xcf,0xc0,0xf0
    ,0x1f,0x17,0x66,0xc0,0x2b,0x09,0x96,0xc7,0x05,0xf1,0x4b,0xe2,0x36,0x95,0x80,0xe4
    ,0x7f,0x3c,0x18,0xaf,0x24,0x00,0x00,0x00,0xff,0xff,0x03,0x00,0x10,0xb3,0x7d,0x83
    ,0x66,0x0e,0x6a,0x67,0x00,0x00,0x00,0x00,0x49,0x45,0x4e,0x44,0xae,0x42,0x60,0x82
}; /* end of rgba78_pngData */

/* This is a dump of a png image which is 7 pixels wide and 8 pixels height.
 * The image consists of four color components (RGBA) thus it contains an alpha
 * channel.
 *
 * It consists of four horizontal (pure) colored stripes each 2 pixels height:
 * The first two rows are red, the second two are green and the last two rows are
 * blue.
 *
 * These first three stripes are 70 % opaque (30 % transparent) which means that their
 * alpha channel is 0.7
 * The last two rows of this image is pure transparent which means that its alpha channel
 * value is 0.0.
 */
const unsigned char rgba78_png_70_percent_opaqueData[284] = {
     0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0x00,0x00,0x00,0x0d,0x49,0x48,0x44,0x52
    ,0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x08,0x08,0x06,0x00,0x00,0x00,0x35,0x04,0xe5
    ,0x06,0x00,0x00,0x00,0x09,0x70,0x48,0x59,0x73,0x00,0x00,0x0b,0x13,0x00,0x00,0x0b
    ,0x13,0x01,0x00,0x9a,0x9c,0x18,0x00,0x00,0x00,0x97,0x74,0x45,0x58,0x74,0x43,0x6f
    ,0x6d,0x6d,0x65,0x6e,0x74,0x00,0x63,0x48,0x52,0x4d,0x20,0x63,0x68,0x75,0x6e,0x6b
    ,0x6c,0x65,0x6e,0x20,0x33,0x32,0x20,0x69,0x67,0x6e,0x6f,0x72,0x65,0x64,0x3a,0x0d
    ,0x41,0x53,0x43,0x49,0x49,0x3a,0x20,0x2e,0x2e,0x7a,0x25,0x2e,0x2e,0xc3,0x84,0xc3
    ,0x89,0x2e,0x2e,0xcb,0x98,0xcb,0x87,0x2e,0x2e,0xc3,0x84,0xc3,0x88,0x2e,0x2e,0x75
    ,0x30,0x2e,0x2e,0xc3,0x8d,0x60,0x2e,0x2e,0x3a,0xc3,0xb2,0x2e,0x2e,0x2e,0x6f,0x0d
    ,0x48,0x45,0x58,0x3a,0x20,0x30,0x30,0x30,0x30,0x37,0x41,0x32,0x35,0x30,0x30,0x30
    ,0x30,0x38,0x30,0x38,0x33,0x30,0x30,0x30,0x30,0x46,0x39,0x46,0x46,0x30,0x30,0x30
    ,0x30,0x38,0x30,0x45,0x39,0x30,0x30,0x30,0x30,0x37,0x35,0x33,0x30,0x30,0x30,0x30
    ,0x30,0x45,0x41,0x36,0x30,0x30,0x30,0x30,0x30,0x33,0x41,0x39,0x38,0x30,0x30,0x30
    ,0x30,0x31,0x37,0x36,0x46,0xa3,0xf6,0x8d,0x7b,0x00,0x00,0x00,0x2b,0x49,0x44,0x41
    ,0x54,0x78,0x9c,0x7c,0xc9,0x31,0x01,0x00,0x00,0x0c,0xc2,0xb0,0xfa,0xb7,0x83,0xc0
    ,0x6e,0x06,0xe0,0xc8,0x15,0x84,0x34,0xcc,0xfc,0x4d,0xb5,0x13,0xd3,0xad,0x54,0xab
    ,0x99,0x07,0x00,0x00,0xff,0xff,0x03,0x00,0x13,0x06,0x71,0x0b,0x1a,0x23,0x85,0x8c
    ,0x00,0x00,0x00,0x00,0x49,0x45,0x4e,0x44,0xae,0x42,0x60,0x82
}; /* end of rgba78_png_70_percent_opaqueData */

BOOST_AUTO_TEST_SUITE(pnginputfile_tests)

/* This is a helper class which will temporarily create a file which contains some
 * arbitrary binary data.
 *
 * The file will be delete in the dtor.
 */
class CTempBinDataAsFile
{
    std::string m_fileName;

public:
    CTempBinDataAsFile(const void* inData, size_t inDataSize)
    {
        // Horrible but hopefully portable alternative to mktemp.  Could use
        // tmpnam() if it wasn't so horrible, or boost::filesystem::unique_file
        // if it was supported in v2 :(
        for(int i = 0;; ++i)
        {
            m_fileName = tfm::format("aqsis_tmpfile_%05d", i);
            if(!boost::filesystem::exists(m_fileName))
            {
                std::ofstream file(m_fileName.c_str(),
                                   std::ios::out | std::ios::binary);
                BOOST_REQUIRE(file);
                file.write(reinterpret_cast<const char*>(inData), inDataSize);
                break;
            }
        }
    }

    ~CTempBinDataAsFile()
    {
        boost::filesystem::remove(m_fileName);
    }

    const char* getFileName() const
    {
        return m_fileName.c_str();
    }
};

/** This test utilizes the PNG image in order to load and parse a given PNG
 *  image which is has the size 7 x 6 pixels and contains of the three
 *  color components RGB.
 *
 * The image consists of three horizontal colord stripes each 2 pixels height.
 * The first two pixels rows are _pure_ red, the second two are green and the
 * last two rows are blue colored.
 */
BOOST_AUTO_TEST_CASE(CqPngInputFile_readRGBPixels_test)
{
    CTempBinDataAsFile tmpFile(rgb76_pngData, sizeof(rgb76_pngData));
	Aqsis::CqPngInputFile inFile(tmpFile.getFileName());

	// Read the whole tiff into a buffer
	Aqsis::CqTextureBuffer<TqUint8> buffer;
	inFile.readPixels(buffer);

	// Check that the buffer is the right size.
	BOOST_CHECK_EQUAL(buffer.width(), 7);
	BOOST_CHECK_EQUAL(buffer.height(), 6);
	BOOST_CHECK_EQUAL(buffer.numChannels(), 3);

    int x;

	// Check that the buffer contains the right data.
	// part of the first two stripes (should be red)
	for (x=0; x != buffer.width(); ++x)
    {
        // 1st row
        BOOST_CHECK_CLOSE(buffer(x,0)[0], 1.0f, 1e-5);
        BOOST_CHECK_CLOSE(buffer(x,0)[1], 0.0f, 1e-5);
        BOOST_CHECK_CLOSE(buffer(x,0)[2], 0.0f, 1e-5);

        // 2nd row
        BOOST_CHECK_CLOSE(buffer(x,1)[0], 1.0f, 1e-5);
        BOOST_CHECK_CLOSE(buffer(x,1)[1], 0.0f, 1e-5);
        BOOST_CHECK_CLOSE(buffer(x,1)[2], 0.0f, 1e-5);
    }

	// Check that the buffer contains the right data.
	// part of the second two stripes (should be green)
	for (x=0; x != buffer.width(); ++x)
    {
        // 1st row
        BOOST_CHECK_CLOSE(buffer(x,2)[0], 0.0f, 1e-5);
        BOOST_CHECK_CLOSE(buffer(x,2)[1], 1.0f, 1e-5);
        BOOST_CHECK_CLOSE(buffer(x,2)[2], 0.0f, 1e-5);

        // 2nd row
        BOOST_CHECK_CLOSE(buffer(x,3)[0], 0.0f, 1e-5);
        BOOST_CHECK_CLOSE(buffer(x,3)[1], 1.0f, 1e-5);
        BOOST_CHECK_CLOSE(buffer(x,3)[2], 0.0f, 1e-5);
    }

	// Check that the buffer contains the right data.
	// part of the third two stripes (should be blue)
	for (x=0; x != buffer.width(); ++x)
    {
        // 1st row
        BOOST_CHECK_CLOSE(buffer(x,4)[0], 0.0f, 1e-5);
        BOOST_CHECK_CLOSE(buffer(x,4)[1], 0.0f, 1e-5);
        BOOST_CHECK_CLOSE(buffer(x,4)[2], 1.0f, 1e-5);

        // 2nd row
        BOOST_CHECK_CLOSE(buffer(x,5)[0], 0.0f, 1e-5);
        BOOST_CHECK_CLOSE(buffer(x,5)[1], 0.0f, 1e-5);
        BOOST_CHECK_CLOSE(buffer(x,5)[2], 1.0f, 1e-5);
    }

}

/** This test utilizes the PNG image in order to load and parse a given PNG
 *  image which is has the size 7 x 8 pixels and contains of the four
 *  color components RGBA.
 *
 * The image consists of four horizontal colored stripes each 2 pixels height.
 * The first two pixels rows are _pure_ red, the second two are green the
 * third two rows are blue colored and the last two rows are pure transparent.
 *
 * The first three colored stripes are opaque which means that their
 * alpha value is 1.0. The last two stripes are transparent which means that
 * the alpha values are expected to be 0.0.
 */
BOOST_AUTO_TEST_CASE(CqPngInputFile_readRGBAPixels_test)
{
    CTempBinDataAsFile tmpFile(rgba78_pngData, sizeof(rgba78_pngData));
	Aqsis::CqPngInputFile inFile(tmpFile.getFileName());

	// Read the whole tiff into a buffer
	Aqsis::CqTextureBuffer<TqUint8> buffer;
	inFile.readPixels(buffer);

	// Check that the buffer is the right size.
	BOOST_CHECK_EQUAL(buffer.width(), 7);
	BOOST_CHECK_EQUAL(buffer.height(), 8);
	BOOST_CHECK_EQUAL(buffer.numChannels(), 4);

    int x;

	// Check that the buffer contains the right data.
	// part of the first two stripes (should be red)
	for (x=0; x != buffer.width(); ++x)
    {
        // 1st row
        BOOST_CHECK_CLOSE(buffer(x,0)[0], 1.0f, 1e-5); // red
        BOOST_CHECK_CLOSE(buffer(x,0)[1], 0.0f, 1e-5); // green
        BOOST_CHECK_CLOSE(buffer(x,0)[2], 0.0f, 1e-5); // blue
        BOOST_CHECK_CLOSE(buffer(x,0)[3], 1.0f, 1e-5); // alpha

        // 2nd row
        BOOST_CHECK_CLOSE(buffer(x,1)[0], 1.0f, 1e-5); // red
        BOOST_CHECK_CLOSE(buffer(x,1)[1], 0.0f, 1e-5); // green
        BOOST_CHECK_CLOSE(buffer(x,1)[2], 0.0f, 1e-5); // blue
        BOOST_CHECK_CLOSE(buffer(x,1)[3], 1.0f, 1e-5); // alpha
    }

	// Check that the buffer contains the right data.
	// part of the second two stripes (should be green)
	for (x=0; x != buffer.width(); ++x)
    {
        // 1st row
        BOOST_CHECK_CLOSE(buffer(x,2)[0], 0.0f, 1e-5); // red
        BOOST_CHECK_CLOSE(buffer(x,2)[1], 1.0f, 1e-5); // green
        BOOST_CHECK_CLOSE(buffer(x,2)[2], 0.0f, 1e-5); // blue
        BOOST_CHECK_CLOSE(buffer(x,2)[3], 1.0f, 1e-5); // alpha

        // 2nd row
        BOOST_CHECK_CLOSE(buffer(x,3)[0], 0.0f, 1e-5); // red
        BOOST_CHECK_CLOSE(buffer(x,3)[1], 1.0f, 1e-5); // green
        BOOST_CHECK_CLOSE(buffer(x,3)[2], 0.0f, 1e-5); // blue
        BOOST_CHECK_CLOSE(buffer(x,3)[3], 1.0f, 1e-5); // alpha
    }

	// Check that the buffer contains the right data.
	// part of the third two stripes (should be blue)
	for (x=0; x != buffer.width(); ++x)
    {
        // 1st row
        BOOST_CHECK_CLOSE(buffer(x,4)[0], 0.0f, 1e-5); // red
        BOOST_CHECK_CLOSE(buffer(x,4)[1], 0.0f, 1e-5); // green
        BOOST_CHECK_CLOSE(buffer(x,4)[2], 1.0f, 1e-5); // blue
        BOOST_CHECK_CLOSE(buffer(x,4)[3], 1.0f, 1e-5); // alpha

        // 2nd row
        BOOST_CHECK_CLOSE(buffer(x,5)[0], 0.0f, 1e-5); // red
        BOOST_CHECK_CLOSE(buffer(x,5)[1], 0.0f, 1e-5); // green
        BOOST_CHECK_CLOSE(buffer(x,5)[2], 1.0f, 1e-5); // blue
        BOOST_CHECK_CLOSE(buffer(x,5)[3], 1.0f, 1e-5); // alpha
    }

	// Check that the buffer contains the right data.
	// part of the forth two stripes (should be transparent (alpha = 0))
    // Note that the actual color values will not be considered here!
	for (x=0; x != buffer.width(); ++x)
    {
        // 1st row
        BOOST_CHECK_CLOSE(buffer(x,6)[3], 0.0f, 1e-5); // alpha

        // 2nd row
        BOOST_CHECK_CLOSE(buffer(x,7)[3], 0.0f, 1e-5); // alpha
    }

}

/** This test is supposed to test if a PNG image will be loaded that the resulting image
 * buffer contains color components with a premultiplied alpha value.
 * This is a requirement of Aqsis because it relies on premultiplied alpha.
 *
 * The issue here is that PNG does *not* generate color components with premultiplied alpha
 * by definition. However the PNG image loader (pnginputfile.cpp) does because it performs a
 * multiplication of each color component with the pixels alpha value if a alpha component
 * exists.
 *
 * In order to check if this works this test utilizes the PNG image in order to load and
 * parse a given PNG image which is has the size 7 x 8 pixels and contains of the four
 * color components RGBA.
 *
 * The image consists of four horizontal colored stripes each 2 pixels height.
 * The first two pixels rows are _pure_ red, the second two are green the
 * third two rows are blue colored and the last two rows are pure transparent.
 *
 * The first three colored stripes are opaque with a value of 70% which
 * means that their alpha value is 0.7.
 *
 * The last two stripes are transparent which means that
 * the alpha values are expected to be 0.0.
 */
BOOST_AUTO_TEST_CASE(CqPngInputFile_readRGBAPixels_with_70_percent_opaque_test)
{
    CTempBinDataAsFile tmpFile(rgba78_png_70_percent_opaqueData, sizeof(rgba78_png_70_percent_opaqueData));
	Aqsis::CqPngInputFile inFile(tmpFile.getFileName());

	// Read the whole tiff into a buffer
	Aqsis::CqTextureBuffer<TqUint8> buffer;
	inFile.readPixels(buffer);

	// Check that the buffer is the right size.
	BOOST_CHECK_EQUAL(buffer.width(), 7);
	BOOST_CHECK_EQUAL(buffer.height(), 8);
	BOOST_CHECK_EQUAL(buffer.numChannels(), 4);

    int x;

	// Check that the buffer contains the right data.
	// part of the first two stripes (should be red)
	for (x=0; x != buffer.width(); ++x)
    {
        // 1st row
        BOOST_CHECK_CLOSE(buffer(x,0)[0], 0.7f, 0.3f); // red
        BOOST_CHECK_CLOSE(buffer(x,0)[1], 0.0f, 0.3f); // green
        BOOST_CHECK_CLOSE(buffer(x,0)[2], 0.0f, 0.3f); // blue
        BOOST_CHECK_CLOSE(buffer(x,0)[3], 0.7f, 0.3f); // alpha

        // 2nd row
        BOOST_CHECK_CLOSE(buffer(x,1)[0], 0.7f, 0.3f); // red
        BOOST_CHECK_CLOSE(buffer(x,1)[1], 0.0f, 0.3f); // green
        BOOST_CHECK_CLOSE(buffer(x,1)[2], 0.0f, 0.3f); // blue
        BOOST_CHECK_CLOSE(buffer(x,1)[3], 0.7f, 0.3f); // alpha
    }

	// Check that the buffer contains the right data.
	// part of the second two stripes (should be green)
	for (x=0; x != buffer.width(); ++x)
    {
        // 1st row
        BOOST_CHECK_CLOSE(buffer(x,2)[0], 0.0f, 0.3f); // green
        BOOST_CHECK_CLOSE(buffer(x,2)[1], 0.7f, 0.3f); // red
        BOOST_CHECK_CLOSE(buffer(x,2)[2], 0.0f, 0.3f); // blue
        BOOST_CHECK_CLOSE(buffer(x,2)[3], 0.7f, 0.3f); // alpha

        // 2nd row
        BOOST_CHECK_CLOSE(buffer(x,3)[0], 0.0f, 0.3f); // green
        BOOST_CHECK_CLOSE(buffer(x,3)[1], 0.7f, 0.3f); // red
        BOOST_CHECK_CLOSE(buffer(x,3)[2], 0.0f, 0.3f); // blue
        BOOST_CHECK_CLOSE(buffer(x,3)[3], 0.7f, 0.3f); // alpha
    }

	// Check that the buffer contains the right data.
	// part of the third two stripes (should be blue)
	for (x=0; x != buffer.width(); ++x)
    {
        // 1st row
        BOOST_CHECK_CLOSE(buffer(x,4)[0], 0.0f, 0.3f); // green
        BOOST_CHECK_CLOSE(buffer(x,4)[1], 0.0f, 0.3f); // blue
        BOOST_CHECK_CLOSE(buffer(x,4)[2], 0.7f, 0.3f); // red
        BOOST_CHECK_CLOSE(buffer(x,4)[3], 0.7f, 0.3f); // alpha

        // 2nd row
        BOOST_CHECK_CLOSE(buffer(x,5)[0], 0.0f, 0.3f); // green
        BOOST_CHECK_CLOSE(buffer(x,5)[1], 0.0f, 0.3f); // blue
        BOOST_CHECK_CLOSE(buffer(x,5)[2], 0.7f, 0.3f); // red
        BOOST_CHECK_CLOSE(buffer(x,5)[3], 0.7f, 0.3f); // alpha
    }

	// Check that the buffer contains the right data.
	// part of the forth two stripes (should be transparent (alpha = 0))
    // Note that the actual color values will not be considered here!
	for (x=0; x != buffer.width(); ++x)
    {
        // 1st row
        BOOST_CHECK_CLOSE(buffer(x,6)[3], 0.0f, 1e-5); // alpha

        // 2nd row
        BOOST_CHECK_CLOSE(buffer(x,7)[3], 0.0f, 1e-5); // alpha
    }
}

BOOST_AUTO_TEST_SUITE_END()
