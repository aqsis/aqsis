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

#include "displaymanager.h"

#include <tiffio.h>

#include "arrayview.h"


/*
template<typename TileSinkT>
class TileResizer
{
    public:
        TileResize(TileSinkT& tileSink, const V2i& inSize, const V2i& outSize)
            : m_tileSink(tileSink),
            m_inSize(inSize),
            m_outSize(outSize)
        { }

        void writeTile(const V2i& pos, const float* data)
        {
        }

    private:
        TileSinkT& m_tileSink;
};
*/


/// Write TIFF header
static void writeHeader(TIFF* tif, const Imath::V2i& imageSize,
                        int nchans, bool useFloat)
{
    uint16 bitsPerSample = 8;
    uint16 photometric = PHOTOMETRIC_RGB;
    uint16 sampleFormat = SAMPLEFORMAT_UINT;
    if(useFloat)
    {
        bitsPerSample = 8*sizeof(float);
        sampleFormat = SAMPLEFORMAT_IEEEFP;
    }
    if(nchans == 1)
        photometric = PHOTOMETRIC_MINISBLACK;
    // Write TIFF header
    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, uint32(imageSize.x));
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, uint32(imageSize.y));
    TIFFSetField(tif, TIFFTAG_ORIENTATION, uint16(ORIENTATION_TOPLEFT));
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, uint16(PLANARCONFIG_CONTIG));
    TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, uint16(RESUNIT_NONE));
    TIFFSetField(tif, TIFFTAG_XRESOLUTION, 1.0f);
    TIFFSetField(tif, TIFFTAG_YRESOLUTION, 1.0f);
    TIFFSetField(tif, TIFFTAG_COMPRESSION, uint16(COMPRESSION_LZW));
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, uint16(nchans));
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bitsPerSample);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, photometric);
    TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, sampleFormat);
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif, 0));
}

/// Quantize float pixels down to uint8
static void quantize(ConstFvecView src, int nPix, uint8* out)
{
    int nSamps = src.elSize();
    for(int j = 0; j < nPix; ++j)
    {
        const float* in = src[j];
        for(int i = 0; i < nSamps; ++i)
            out[i] = static_cast<uint8>(clamp(255*in[i], 0.0f, 255.0f));
        out += nSamps;
    }
}


//--------------------------------------------------
DisplayManager::DisplayManager(const V2i& imageSize, const V2i& tileSize,
                               const OutvarSet& outVars)
    : m_imageSize(imageSize),
    m_tileSize(tileSize),
    m_outVars(outVars),
    m_totChans(0),
    m_imageData()
{
    for(int i = 0; i < m_outVars.size(); ++i)
        m_totChans += m_outVars[i].scalarSize();
    m_imageData.resize(m_totChans * m_imageSize.x * m_imageSize.y);
}

void DisplayManager::writeTile(V2i pos, const float* data)
{
    int tileRowSize = m_totChans*m_tileSize.x;
    // Clamp output tile size to extent of image.
    V2i outTileEnd = min((pos+V2i(1))*m_tileSize, m_imageSize);
    V2i outTileSize = outTileEnd - pos*m_tileSize;
    int outTileRowSize = m_totChans*outTileSize.x;
    // Copy data
    ConstFvecView srcRows(data, outTileRowSize, tileRowSize);
    FvecView destRows = FvecView(&m_imageData[0] + tileRowSize*pos.x,
                                 outTileRowSize, m_totChans*m_imageSize.x);
    copy(destRows + m_tileSize.y*pos.y, srcRows, outTileSize.y);
}

void DisplayManager::writeFiles()
{
    for(int i = 0, nFiles = m_outVars.size(); i < nFiles; ++i)
    {
        int nChans = m_outVars[i].scalarSize();

        std::string fileName = "test_";
        fileName += m_outVars[i].name.c_str();
        fileName += ".tif";

        TIFF* tif = TIFFOpen(fileName.c_str(), "w");
        if(!tif)
        {
            std::cerr << "Could not open file " << fileName << "\n";
            continue;
        }

        // Don't quatize if we've got depth data.
        bool doQuantize = m_outVars[i] != Stdvar::z;
        writeHeader(tif, m_imageSize, nChans, !doQuantize);

        // Write image data
        int rowSize = nChans*m_imageSize.x*(doQuantize ? 1 : sizeof(float));
        boost::scoped_array<uint8> lineBuf(new uint8[rowSize]);
        FvecView src(&m_imageData[0] + m_outVars[i].offset, nChans, m_totChans);
        for(int line = 0; line < m_imageSize.y; ++line)
        {
            if(doQuantize)
                quantize(src, m_imageSize.x, lineBuf.get());
            else
                copy(FvecView(reinterpret_cast<float*>(lineBuf.get()), nChans),
                     src, m_imageSize.x);
            TIFFWriteScanline(tif, lineBuf.get(), uint32(line));
            src += m_imageSize.x;
        }

        TIFFClose(tif);
    }
}
