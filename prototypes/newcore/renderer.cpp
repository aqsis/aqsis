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

// Save image to a TIFF file.
void Renderer::saveImage(const std::string& fileName)
{
    TIFF* tif = TIFFOpen(fileName.c_str(), "w");
    if(!tif)
    {
        std::cerr << "Could not open file!\n";
        return;
    }

    // Write header
    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, uint32(m_opts.xRes));
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, uint32(m_opts.yRes));
    TIFFSetField(tif, TIFFTAG_ORIENTATION, uint16(ORIENTATION_TOPLEFT));
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, uint16(PLANARCONFIG_CONTIG));
    TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, uint16(RESUNIT_NONE));
    TIFFSetField(tif, TIFFTAG_XRESOLUTION, 1.0f);
    TIFFSetField(tif, TIFFTAG_YRESOLUTION, 1.0f);
    TIFFSetField(tif, TIFFTAG_COMPRESSION, uint16(COMPRESSION_LZW));
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, uint16(1));
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, uint16(8*sizeof(float)));
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, uint16(PHOTOMETRIC_MINISBLACK));
    TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, uint16(SAMPLEFORMAT_IEEEFP));
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif, 0));

    // Write image data
    int rowSize = m_opts.xRes*sizeof(float);
    boost::scoped_array<uint8> lineBuf(new uint8[rowSize]);
    for(int line = 0; line < m_opts.yRes; ++line)
    {
        std::memcpy(lineBuf.get(), &m_image[0] + line*m_opts.xRes,
                    rowSize);
        TIFFWriteScanline(tif, reinterpret_cast<tdata_t>(lineBuf.get()),
                          uint32(line));
    }

    TIFFClose(tif);
}
