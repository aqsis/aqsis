#include <cfloat>
#include <cmath>
#include <complex>
#include <vector>

#include <tiffio.h>

#include <boost/math/special_functions/sinc.hpp>

typedef std::complex<double> dcomplex;

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

void makeFilter(std::vector<float>& filter, int filterWidth, int superSamp)
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

//    for(int j = 0; j < fw; ++j)
//    {
//        for(int i = 0; i < fw; ++i)
//            std::cout << filter[fw*j + i] << " ";
//        std::cout << "\n";
//    }
}

//------------------------------------------------------------------------------
int mandelEscapetime(dcomplex c, const int maxIter)
{
    int i = 0;
    dcomplex z = 0;
    while(imag(z)*imag(z) + real(z)*real(z) < 4 && i < maxIter)
    {
        z = z*z + c;
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
    int t = mandelEscapetime(dcomplex(x,y), maxIter);
    colorMap(rgb, t, maxIter);
}


//------------------------------------------------------------------------------

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
void writeHeader(TIFF* tif, int width, int height)
{
    // Write TIFF header
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
}


int main()
{
    const int width = 800;
    const int height = width;
    const int maxIter = 1000;

    const int superSamp = 4;

    double scale = 0.01;
    double aspect = double(width)/height;
    double xmult = scale/superSamp * aspect * 2.0/width;
    double xoff =  -0.79 + scale * aspect * -1.0;
    double ymult = -scale/superSamp * 2.0/height;
    double yoff =  0.15 + scale * 1.0;


    const int filterWidth = 3;
    std::vector<float> filter;
    makeFilter(filter, filterWidth, superSamp);

    TIFF* tif = TIFFOpen("mandel.tif", "w");
    writeHeader(tif, width, height);

    int fullwidth = superSamp*(width + filterWidth-1);
    std::vector<float> colVals(3*fullwidth*fullwidth, -1);
    renderTile(&colVals[0], fullwidth, xoff, yoff, xmult, ymult, maxIter);

    std::vector<char> colFilt(width*3, 0);

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
                    float* c = &colVals[3*(fullwidth*(superSamp*iy+j) + superSamp*ix+i)];
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

//    for(int j = 0; j < fullwidth; ++j)
//    {
//        for(int i = 0; i < fullwidth; ++i)
//        {
//            int idx = 3*(fullwidth*j + i);
//            colFilt[3*i+0] = uint8(colVals[idx + 0]*255);
//            colFilt[3*i+1] = uint8(colVals[idx + 1]*255);
//            colFilt[3*i+2] = uint8(colVals[idx + 2]*255);
//        }
//        TIFFWriteScanline(tif, &colFilt[0], j);
//    }

    TIFFClose(tif);

    return 0;
}

// Link with -ltiff
