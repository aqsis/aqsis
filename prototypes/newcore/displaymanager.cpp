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

#include "displaymanager.h"

#include <algorithm>
#include <set>

#include <tiffio.h>

namespace Aqsis {

//------------------------------------------------------------------------------
// TIFF output implementation

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
    TIFFSetField(tif, TIFFTAG_SOFTWARE, "Aqsis-2.0 (aka newcore)");
    if(tiled)
    {
        TIFFSetField(tif, TIFFTAG_TILEWIDTH, tileSize.x);
        TIFFSetField(tif, TIFFTAG_TILELENGTH, tileSize.y);
    }
    else
        TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif, 0));
}

/// Simple TIFF image output interface.
class TiffDisplay : public Display
{
    public:
        /// Open TIFF file & write header
        TiffDisplay()
            : m_tif(0),
            m_tiled(false)
        { }

        /// Close file
        ~TiffDisplay()
        {
            close();
        }

        virtual bool open(const std::string& fileName, const V2i& imageSize,
                          const V2i& tileSize, const VarSpec& varSpec)
        {
            m_tiled = (tileSize.x % 16 == 0) && (tileSize.y % 16 == 0);
            if(!m_tiled)
            {
                std::cerr << "Non mutiple of 16 output image tiles, "
                             "not implemented!!\n";  // TODO
                return false;
            }
            m_tif = TIFFOpen(fileName.c_str(), "w");
            if(!m_tif)
            {
                std::cerr << "Could not open file " << fileName << "\n";
                return false;
            }
            // FIXME - Think about how channel format info is going to work -
            // don't want to generate the quantize flag in two different
            // places.
            writeHeader(m_tif, imageSize, varSpec.scalarSize(),
                        varSpec == Stdvar::z, m_tiled, tileSize);
            return true;
        }

        virtual bool close()
        {
            if(!m_tif)
                return false;
            TIFFClose(m_tif);
            m_tif = 0;
            return true;
        }

        /// Write tile data at position pos to the file.  Threadsafe.
        virtual bool writeTile(const V2i& pos, void* data)
        {
            if(!m_tif)
                return false;
            if(m_tiled)
            {
                return TIFFWriteTile(m_tif, data, pos.x, pos.y, 0, 0) != -1;
            }
            else
            {
                // TODO: Do some buffering or something
                // TIFFWriteScanline(tif, buf, lineNum);
                return false;
            }
        }

    private:
        TIFF* m_tif;
        bool m_tiled;
};


//------------------------------------------------------------------------------
// DisplayList implementation
bool DisplayList::addDisplay(const char* name, const char* type,
                             const VarSpec& outVar,
                             const Ri::ParamList& pList)
{
    DisplayInfo info;
    info.name = name;
    info.outputVar = outVar;
    if(strcmp(type, "file") == 0 || strcmp(type, "zfile") == 0)
        info.display.reset(new TiffDisplay());
    else if(strcmp(type, "__Display_instance__") == 0)
    {
        Ri::PtrArray inst = pList.findPtr("instance");
        if(!inst)
            return false;
        info.display.reset(static_cast<Display*>(inst[0]), nullDeleter);
    }
    else
        return false;
    assert(info.display);
    m_displayInfo.push_back(info);
    return true;
}

VarList DisplayList::requiredVars() const
{
    typedef std::set<VarSpec> VSet;
    VSet allVars;
    for(int i = 0; i < (int)m_displayInfo.size(); ++i)
        allVars.insert(m_displayInfo[i].outputVar);
    VarList out;
    std::copy(allVars.begin(), allVars.end(), std::back_inserter(out));
    return out;
}

//------------------------------------------------------------------------------
// DisplayManager implementation
DisplayManager::DisplayManager(const V2i& imageSize, const V2i& tileSize,
                               const OutvarSet& outVars,
                               const DisplayList& displays)
    : m_imageSize(imageSize),
    m_tileSize(tileSize),
    m_totChans(0),
    m_displayInfo()
{
    // Compute number of channels coming from renderer
    for(int i = 0; i < outVars.size(); ++i)
        m_totChans += outVars[i].scalarSize();
    // Set up displays
    for(int i = 0; i < displays.size(); ++i)
    {
        DisplayData dispInfo;
        int outIndex = outVars.find(displays[i].outputVar);
        assert(outIndex >= 0);
        dispInfo.var = outVars[outIndex];
        dispInfo.display = displays[i].display;
        dispInfo.quantize = dispInfo.var != Stdvar::z;
        if(!dispInfo.display->open(displays[i].name, m_imageSize, m_tileSize,
                                   dispInfo.var))
            continue;
        m_displayInfo.push_back(dispInfo);
    }
}

DisplayManager::~DisplayManager()
{
    // Close the displays
    for(int i = 0; i < (int)m_displayInfo.size(); ++i)
        m_displayInfo[i].display->close();
}

void DisplayManager::writeTile(V2i pos, const float* data)
{
    int nPixels = prod(m_tileSize);
    for(int idisp = 0; idisp < (int)m_displayInfo.size(); ++idisp)
    {
        DisplayData& dispInfo = m_displayInfo[idisp];
        int nChans = dispInfo.var.scalarSize();
        ConstFvecView src(data + dispInfo.var.offset, nChans, m_totChans);
        if(dispInfo.quantize)
        {
            // Quantize into temporary buffer
            uint8* tmpTile = static_cast<uint8*>(
                    tmpStorage(nChans*nPixels*sizeof(uint8)));
            quantize(src, prod(m_tileSize), tmpTile);
            LockGuard lk(m_mutex);
            dispInfo.display->writeTile(pos, tmpTile);
        }
        else
        {
            // Copy over channels directly.
            float* tmpTile = static_cast<float*>(
                    tmpStorage(nChans*nPixels*sizeof(float)));
            copy(FvecView(tmpTile, nChans), src, nPixels);
            LockGuard lk(m_mutex);
            dispInfo.display->writeTile(pos, tmpTile);
        }
    }
}

/// Get per-thread temporary storage space of size bytes
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

/// Quantize float pixels down to uint8
void DisplayManager::quantize(ConstFvecView src, int nPix, unsigned char* out)
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

} // namespace Aqsis
