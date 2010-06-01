#include <cassert>
#include <cfloat>
#include <cmath>
#include <vector>

#include <tiffio.h>

#include <boost/math/special_functions/sinc.hpp>

// A define to disable inlining so that selected functions show up in the
// profile.
#define noinline __attribute__((noinline))

// Compute ceil(real(n)/d) using integers for positive n and d.
template<typename T>
inline T ceildiv(T n, T d)
{
    return (n-1)/d + 1;
}

template<typename T>
inline T clamp(T x, T low, T high)
{
    return (x < low) ? low : ((x > high) ? high : x);
}


float windowedsinc(float x, float width)
{
    float xscale = x*M_PI;
    float window = 0;
    if(std::abs(x) < width)
        window = boost::math::sinc_pi(xscale/width);
    return boost::math::sinc_pi(xscale)*window;
}

/// Cache filter coefficients
void cacheFilter(std::vector<float>& filter, int filterWidth, int superSamp)
{
    int fw = filterWidth*superSamp;
    filter.resize(fw*fw, FLT_MAX);
    // compute filter coefficients
    for(int j = 0; j < fw; ++j)
    {
        float y = (j+0.5)/superSamp - filterWidth/2.0;
        for(int i = 0; i < fw; ++i)
        {
            float x = (i+0.5)/superSamp - filterWidth/2.0;
            filter[j*fw + i] = windowedsinc(x, filterWidth) *
                                        windowedsinc(y, filterWidth);
        }
    }
    // normalize
    float sum = 0;
    for(int i = 0, iend=filter.size(); i < iend; ++i)
        sum += filter[i];
    float renorm = 1/sum;
    for(int i = 0, iend=filter.size(); i < iend; ++i)
        filter[i] *= renorm;

    // Debug; print filter coeffs
//    for(int j = 0; j < fw; ++j)
//    {
//        for(int i = 0; i < fw; ++i)
//            std::cout << filter[fw*j + i] << " ";
//        std::cout << "\n";
//    }
}

//------------------------------------------------------------------------------
// Fractal stuff.
int mandelEscapetime(double cx, double cy, const int maxIter)
{
    int i = 0;
    double x = 0;
    double y = 0;
    while(x*x + y*y < 4 && i < maxIter)
    {
        double xtmp = x*x - y*y + cx;
        y = 2*x*y + cy;
        x = xtmp;
        ++i;
    }
    return i;
}


void colorMap(float* rgb, int i, int maxIter)
{
    if(i == maxIter)
        rgb[0] = rgb[1] = rgb[2] = 0;
    else
    {
        rgb[0] = (i%2048)/2047.0;
        rgb[1] = 0;
        rgb[2] = clamp(i, 0, 255)/255.0;
    }
}

void mandelColor(float* rgb, double x, double y, int maxIter)
{
    int t = mandelEscapetime(x, y, maxIter);
    colorMap(rgb, t, maxIter);
}

noinline
void renderTile(float* rgb, int w, double x0, double y0, double dx, double dy,
                int maxIter)
{
    for(int j = 0; j < w; ++j)
    {
        double y = y0 + j*dy;
        for(int i = 0; i < w; ++i)
        {
            double x = x0 + i*dx;
            mandelColor(&rgb[3*(w*j + i)], x, y, maxIter);
        }
    }
}


//------------------------------------------------------------------------------

noinline
void filterAndQuantizeTile(uint8* rgbOut, const float* rgbTiles[2][2],
                           int tileWidth, int superSamp, const float* filter,
                           int filterWidth)
{
    const int fw = filterWidth*superSamp;
    const int widthOn2 = tileWidth/2;
    const int inTileWidth = superSamp*tileWidth; // Input tile size before filtering
    assert(tileWidth % 2 == 0);
    assert(filterWidth < widthOn2*superSamp);
    // Iterate over output tile
    for(int iy = 0; iy < tileWidth; ++iy)
        for(int ix = 0; ix < tileWidth; ++ix)
        {
            // Filter pixel.  The filter support can lie across any or all of
            // the 2x2 block of input tiles.
            float col[3] = {0,0,0};
            for(int j = 0; j < fw; ++j)
                for(int i = 0; i < fw; ++i)
                {
                    // location x,y of the current pixel in the 2x2 block
                    int x = superSamp*(ix + widthOn2) + i;
                    int y = superSamp*(iy + widthOn2) + j;
                    int tx = x >= inTileWidth;
                    int ty = y >= inTileWidth;
                    const float* c = rgbTiles[ty][tx] +
                           3*( inTileWidth*(y - ty*inTileWidth) +
                                           (x - tx*inTileWidth) );
                    float w = filter[fw*j + i];
                    col[0] += w*c[0];
                    col[1] += w*c[1];
                    col[2] += w*c[2];
                }
//            int xb = superSamp*(ix + widthOn2);
//            int xe = xb + fw;
//            int yb = superSamp*(iy + widthOn2);
//            int ye = yb + fw;
//            if(    (xb < inTileWidth && xe >= inTileWidth)
//                || (yb < inTileWidth && ye >= inTileWidth) )
//            {
//                col[0] = 0;
//            }
//            else
//            {
//                const float* rgb = rgbTiles[yb >= inTileWidth][xb >= inTileWidth];
//                if(xb >= inTileWidth)
//                    xb -= inTileWidth;
//                if(yb >= inTileWidth)
//                    yb -= inTileWidth;
//                for(int j = 0, y = yb; j < fw; ++j, ++y)
//                {
//                    for(int i = 0, x = xb; i < fw; ++i, ++x)
//                    {
//                        const float* c = rgb + 3*(inTileWidth*y + x);
//                        float w = filter[fw*j + i];
//                        col[0] += w*c[0];
//                        col[1] += w*c[1];
//                        col[2] += w*c[2];
//                    }
//                }
//            }
            // Quantize & store in output Tile
            int idx = 3*(tileWidth*iy + ix);
            rgbOut[idx+0] = uint8(clamp(255*col[0], 0.0f, 255.0f));
            rgbOut[idx+1] = uint8(clamp(255*col[1], 0.0f, 255.0f));
            rgbOut[idx+2] = uint8(clamp(255*col[2], 0.0f, 255.0f));
        }
}

//------------------------------------------------------------------------------
/// Write required TIFF header data
void writeHeader(TIFF* tif, int width, int height, int tileWidth = -1)
{
    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, uint32(width));
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, uint32(height));
    TIFFSetField(tif, TIFFTAG_ORIENTATION, uint16(ORIENTATION_TOPLEFT));
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, uint16(PLANARCONFIG_CONTIG));
    TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, uint16(RESUNIT_NONE));
    TIFFSetField(tif, TIFFTAG_XRESOLUTION, 1.0f);
    TIFFSetField(tif, TIFFTAG_YRESOLUTION, 1.0f);
    TIFFSetField(tif, TIFFTAG_COMPRESSION, uint16(COMPRESSION_LZW));
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, uint16(3));
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, uint16(8));
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif, 0));

    if(tileWidth > 0)
    {
        TIFFSetField(tif, TIFFTAG_TILEWIDTH, uint32(tileWidth));
        TIFFSetField(tif, TIFFTAG_TILELENGTH, uint32(tileWidth));
    }
}


//------------------------------------------------------------------------------

/// Simple sequential method.
///
/// Render a big buffer of samples, then filter those samples.
noinline
void renderImageSimple(TIFF* tif, int width, int superSamp, const float* filter,
                       int filterWidth, double xoff, double xmult, double yoff,
                       double ymult, int maxIter)
{
    writeHeader(tif, width, width);
    int fullWidth = superSamp*(width + filterWidth-1);
    std::vector<float> colVals(3*fullWidth*fullWidth, -1);
    renderTile(&colVals[0], fullWidth, xoff, yoff, xmult, ymult, maxIter);

    std::vector<uint8> colFilt(width*3, 0);

    const int fw = filterWidth*superSamp;
    for(int iy = 0; iy < width; ++iy)
    {
        for(int ix = 0; ix < width; ++ix)
        {
            // Filter pixel
            float col[3] = {0,0,0};
            for(int j = 0; j < fw; ++j)
                for(int i = 0; i < fw; ++i)
                {
                    float* c = &colVals[3*(fullWidth*(superSamp*iy+j) + superSamp*ix+i)];
                    float w = filter[fw*j + i];
                    col[0] += w*c[0];
                    col[1] += w*c[1];
                    col[2] += w*c[2];
                }
            // Quantize & store in scanline
            colFilt[3*ix+0] = uint8(clamp(255*col[0], 0.0f, 255.0f));
            colFilt[3*ix+1] = uint8(clamp(255*col[1], 0.0f, 255.0f));
            colFilt[3*ix+2] = uint8(clamp(255*col[2], 0.0f, 255.0f));
        }
        // Save result
        TIFFWriteScanline(tif, &colFilt[0], iy);
    }
}


/// Tiled sequential method.
///
/// Render samples in tiled chunks.  Filtering is interleaved (in time) with
/// sampling, and happens whenever sufficient adjacent tiles have been
/// generated.
///
/// For simplicity full tiles of samples are always generated, though in
/// practise that means there's some sampling wastage at the edges of the
/// image, which means this function will be slightly slower than
/// renderImageSimple().
noinline
void renderImageTiled(TIFF* tif, int width, int superSamp, const float* filter,
                      int filterWidth, double xoff, double xmult, double yoff,
                      double ymult, int maxIter)
{
    const int tileWidth = 16;
    writeHeader(tif, width, width, tileWidth);
    int ntiles = ceildiv(width, tileWidth) + 1;

    int inTileWidth = superSamp*tileWidth;
    int tileSize = inTileWidth*inTileWidth*3;

    // Storage pool for rendered tiles
    int poolSize = ntiles+2;
    std::vector<float> tileStorage(poolSize*tileSize);
    std::vector<float*> tilePool(poolSize);
    for(int i = 0; i < poolSize; ++i)
        tilePool[i] = &tileStorage[i*tileSize];

    // Render tiles left to right, top to bottom.
    int poolPos = 0;
    std::vector<uint8> colFilt(3*tileWidth*tileWidth, 0);
    for(int ty = 0; ty < ntiles; ++ty)
    {
        for(int tx = 0; tx < ntiles; ++tx, ++poolPos)
        {
            float* stor = tilePool[poolPos % poolSize];
            renderTile(stor, inTileWidth, xoff + xmult*inTileWidth*tx,
                       yoff + ymult*inTileWidth*ty, xmult, ymult, maxIter);
            if(ty > 0 && tx > 0)
            {
                // When four adjacent tiles are rendered, filter the interior
                // region, equal in size to one tile.
                const float* toFilter[2][2] = {
                    {tilePool[(poolPos+1) % poolSize], tilePool[(poolPos+2) % poolSize]},
                    {tilePool[(poolPos-1) % poolSize], stor}
                };
                filterAndQuantizeTile(&colFilt[0], toFilter, tileWidth, superSamp,
                                      &filter[0], filterWidth);
                TIFFWriteTile(tif, &colFilt[0], (tx-1)*tileWidth,
                              (ty-1)*tileWidth, 0, 0);
            }
        }
    }
}


int main()
{
    const int width = 800;
    const int maxIter = 2000;

    const int superSamp = 2;

    double scale = 0.01;
    double xmult = scale/superSamp * 2.0/width;
    double xoff =  -0.79 + scale * -1.0;
    double ymult = -scale/superSamp * 2.0/width;
    double yoff =  0.15 + scale * 1.0;

    const int filterWidth = 3;
    std::vector<float> filter;
    cacheFilter(filter, filterWidth, superSamp);

    TIFF* tif = TIFFOpen("mandel.tif", "w");

//    renderImageSimple(tif, width, superSamp, &filter[0], filterWidth,
//                      xoff, xmult, yoff, ymult, maxIter);
    renderImageTiled(tif, width, superSamp, &filter[0], filterWidth,
                     xoff, xmult, yoff, ymult, maxIter);

    TIFFClose(tif);

    return 0;
}

// Link with -ltiff
