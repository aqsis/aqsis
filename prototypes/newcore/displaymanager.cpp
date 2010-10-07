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


/// Write TIFF header
static void writeHeader(TIFF* tif, const V2i& imageSize,
                        int nchans, bool useFloat,
                        bool tiled, const V2i& tileSize)
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
    if(tiled)
    {
        TIFFSetField(tif, TIFFTAG_TILEWIDTH, tileSize.x);
        TIFFSetField(tif, TIFFTAG_TILELENGTH, tileSize.y);
    }
    else
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


//------------------------------------------------------------------------------
/// Simple TIFF image output interface.
class DisplayManager::ImageFile
{
    public:
        /// Open TIFF file & write header
        ImageFile(const std::string& fileName, const V2i& imageSize,
                  const V2i& tileSize, const VarSpec& varSpec)
            : m_tif(0),
            m_tiled(tileSize.x % 16 == 0  &&  tileSize.y % 16 == 0),
            m_doQuantize(varSpec != Stdvar::z)
        {
            m_tif = TIFFOpen(fileName.c_str(), "w");
            if(!m_tif)
            {
                std::cerr << "Could not open file " << fileName << "\n";
                return;
            }
            if(!m_tiled)
                std::cerr << "Non mutiple of 16 output image tiles, "
                             "not implemented!!\n";  // TODO
            writeHeader(m_tif, imageSize, varSpec.scalarSize(), !m_doQuantize,
                        m_tiled, tileSize);
        }

        /// Close file
        ~ImageFile()
        {
            if(!m_tif)
                return;
            TIFFClose(m_tif);
        }

        /// Write tile data at position pos to the file.  Threadsafe.
        void writeTile(const V2i& pos, void* data)
        {
            if(!m_tif)
                return;
            LockGuard lock(m_tifMutex);
            if(m_tiled)
            {
                TIFFWriteTile(m_tif, data, pos.x, pos.y, 0, 0);
            }
            else
            {
                // TODO: Do some buffering or something
                // TIFFWriteScanline(tif, buf, lineNum);
            }
        }

        /// Return true if we want quantized channels as input to writeTile()
        bool doQuantize() const
        {
            return m_doQuantize;
        }

    private:
        TIFF* m_tif;
        Mutex m_tifMutex;
        bool m_tiled;
        bool m_doQuantize;
};


//------------------------------------------------------------------------------
DisplayManager::DisplayManager(const V2i& imageSize, const V2i& tileSize,
                               const OutvarSet& outVars)
    : m_imageSize(imageSize),
    m_tileSize(tileSize),
    m_outVars(outVars),
    m_totChans(0)
{
    for(int i = 0; i < m_outVars.size(); ++i)
        m_totChans += m_outVars[i].scalarSize();
    for(int i = 0; i < m_outVars.size(); ++i)
    {
        std::string fileName = "test_";
        fileName += m_outVars[i].name.c_str();
        fileName += ".tif";
        m_files.push_back(boost::shared_ptr<ImageFile>(
            new ImageFile(fileName, imageSize, tileSize, m_outVars[i])
        ));
    }
}

void DisplayManager::writeTile(V2i pos, const float* data)
{
    int nPixels = prod(m_tileSize);
    for(int ifile = 0; ifile < m_outVars.size(); ++ifile)
    {
        int nChans = m_outVars[ifile].scalarSize();
        ConstFvecView src(data + m_outVars[ifile].offset, nChans, m_totChans);
        ImageFile& file = *m_files[ifile];
        if(file.doQuantize())
        {
            // Quantize into temporary buffer
            uint8* tmpTile = static_cast<uint8*>(
                    tmpStorage(nChans*nPixels*sizeof(uint8)));
            quantize(src, prod(m_tileSize), tmpTile);
            file.writeTile(pos, tmpTile);
        }
        else
        {
            // Copy over channels directly.
            float* tmpTile = static_cast<float*>(
                    tmpStorage(nChans*nPixels*sizeof(float)));
            copy(FvecView(tmpTile, nChans), src, nPixels);
            file.writeTile(pos, tmpTile);
        }
    }
}

void DisplayManager::closeFiles()
{
    m_files.clear();
}

/// Get temporary storage of size bytes
void* DisplayManager::tmpStorage(size_t size)
{
    std::vector<char>* store = m_tileTmpStorage.get();
    if(!store)
    {
        store = new std::vector<char>(size);
        m_tileTmpStorage.reset(store);
    }
    if(store->size() < size)
        store->resize(size);
    return &(*store)[0];
}
