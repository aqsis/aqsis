#include "renderer.h"

//------------------------------------------------------------------------------
// RenderQueueImpl implementation

void RenderQueueImpl::push(const boost::shared_ptr<Geometry>& geom)
{
    m_renderer.push(geom, m_splitDepth+1);
}

void RenderQueueImpl::push(const boost::shared_ptr<Grid>& grid)
{
    m_renderer.push(grid);
}


//------------------------------------------------------------------------------
// Renderer implementation

static void writeHeader(TIFF* tif, Options& opts, int nchans, bool useFloat)
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
    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, uint32(opts.xRes));
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, uint32(opts.yRes));
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

static void quantize(const float* in, uint8* out, int len)
{
    for(int i = 0; i < len; ++i)
        out[i] = static_cast<uint8>(Imath::clamp(255*in[i], 0.0f, 255.0f));
}

// Save image to a TIFF file.
void Renderer::saveImage(const std::string& fileName)
{
    TIFF* tif = TIFFOpen(fileName.c_str(), "w");
    if(!tif)
    {
        std::cerr << "Could not open file!\n";
        return;
    }

    writeHeader(tif, m_opts, 3, false);

    // Write RGB image data
    int rowSize = 3*m_opts.xRes;
    boost::scoped_array<uint8> lineBuf(new uint8[rowSize]);
    for(int line = 0; line < m_opts.yRes; ++line)
    {
        quantize(&m_image[0] + 3*line*m_opts.xRes, lineBuf.get(), rowSize);
        TIFFWriteScanline(tif, reinterpret_cast<tdata_t>(lineBuf.get()),
                          uint32(line));
    }

    // Write Z image data
//    int rowSize = m_opts.xRes*sizeof(float);
//    boost::scoped_array<uint8> lineBuf(new uint8[rowSize]);
//    for(int line = 0; line < m_opts.yRes; ++line)
//    {
//        std::memcpy(lineBuf.get(), &m_image[0] + line*m_opts.xRes,
//                    rowSize);
//        TIFFWriteScanline(tif, reinterpret_cast<tdata_t>(lineBuf.get()),
//                          uint32(line));
//    }

    TIFFClose(tif);
}


