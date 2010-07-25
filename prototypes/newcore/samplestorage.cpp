// Aqsis
// Copyright (C) 1997 - 2010, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "samplestorage.h"

#include "samplegen.h"
#include "arrayview.h"
#include "util.h"

#include <fstream>

//static void loadTileSet(const std::string& fname,
//                        std::vector<SampleStorage::TimeLens>& tuv,
//                        std::vector<int>& tiles, int& tileWidth)
//{
//    std::ifstream in(fname.c_str());
//    assert(in);
//    int h=0, w=0;
//    in >> h >> w;
//    assert(h == w);
//    tuv.resize(w*w);
//    for(int i = 0, iend = tuv.size(); i < iend; ++i)
//        in >> tuv[i].time >> tuv[i].lens.x >> tuv[i].lens.y;
//    tiles.resize(81*w*w);
//    for(int i = 0, iend = tiles.size(); i < iend; ++i)
//        in >> tiles[i];
//    tileWidth = w;
//}

//static void saveTileSet(const std::string& fname,
//                        std::vector<SampleStorage::TimeLens>& tuv,
//                        std::vector<int>& tiles, int& tileWidth)
//{
//    std::ofstream out(fname.c_str());
//    assert(out);
//    out << tileWidth << " " << tileWidth << "\n";
//    for(int i = 0, iend = tuv.size(); i < iend; ++i)
//        out << tuv[i].time << " " << tuv[i].lens.x << " " << tuv[i].lens.y << " ";
//    out << "\n";
//    int nsamps = tileWidth*tileWidth;
//    for(int i = 0, iend = tiles.size(); i < iend; ++i)
//    {
//        out << tiles[i] << " ";
//        if((i+1) % nsamps == 0)
//            out << "\n";
//    }
//}

class SpatialHash
{
    private:
        std::vector<int> m_permTable;
        int m_nColors;
    public:
        SpatialHash(int nColors, int tableSize = 0)
            : m_permTable(tableSize),
            m_nColors(nColors)
        {
            if(tableSize == 0)
            {
                // A particular permutation table for definiteness.  Having a
                // fixed table is really useful when debugging.
                int tableInit[] = {
                 41,  52, 248, 166, 162,  92,  95,  11,   3,  94, 219, 198, 170,
                165,  80, 231,  75,  19,  20, 240, 169, 105,  15,  51,  45,  84,
                 85, 157, 232,  23,  97,  14, 174,   0,  82,  42, 145,  69, 111,
                178, 158, 138,  17,  70, 238, 137, 179,  64, 172,  61,  53, 177,
                 91,  73, 234,  18, 148, 223,  68, 187, 123,   7,   8,  12,   2,
                 27, 133,  66, 194, 160, 189, 142, 253,  81,  87, 212, 193, 102,
                210, 154,  28,  21,  24, 101, 192, 249, 168,  71, 233, 237,  98,
                 79, 119, 152, 183, 116, 211, 217,  29, 184, 220,  54,  31, 163,
                134, 120,  78, 245, 195, 204,  56, 140, 180,  99, 112,  57, 146,
                228, 201, 106, 197, 115, 250,  47, 122,  40, 203, 132,  33,  89,
                225, 196,  83,  58, 182,  34, 218, 205,  50,  10, 206,  88,  49,
                  1, 109, 254,  67, 144, 150, 230, 235, 213, 181, 242, 208, 246,
                 86, 236, 190,  13, 188,  59, 209,  48,  74, 214, 175, 215,  77,
                114,  35, 252, 227, 118,  72, 167, 153, 129, 121, 164, 244, 243,
                226, 125,  44,  62, 239, 136, 151, 173, 171,   5, 110,  96, 216,
                222, 113, 229, 161, 155, 221, 124, 103,   6, 255, 108, 126, 147,
                 32,  93, 135, 200, 100,  43, 251, 141, 130, 117,  39,  60, 185,
                149,  38,  26, 143,  55,  36, 127, 128,   4,  16, 207, 139, 159,
                 22, 191,  25, 156, 131,  46,  30, 224, 176,  65,  90, 202, 186,
                104, 107,   9, 247,  63, 199,  37, 241,  76
                };
                m_permTable.assign(tableInit, tableInit+256);
            }
            else
            {
                for(int i = 0; i < tableSize; ++i)
                    m_permTable[i] = i;
                std::random_shuffle(m_permTable.begin(), m_permTable.end());
            }
        }
        int operator()(int x, int y) const
        {
            assert(x >= 0 && y >= 0);
            const int N = m_permTable.size();
            return m_permTable[(m_permTable[x%N] + y)%N] % m_nColors;
        }
};

SampleStorage::SampleStorage(const OutvarSet& outVars, const Options& opts)
    : m_opts(opts),
    m_fragSize(0),
    m_xSampRes(0),
    m_ySampRes(0),
    m_samples(),
    m_defaultFragment(),
    m_fragments(),
    m_filterResults(),
    m_doFilter(true),
    m_useSeparable(false),
    m_filter()
{
    // Initialize cached filter coefficients
    cacheFilter(m_filter, opts, m_useSeparable, m_filtExpand, m_filtWidth);

    m_xSampRes = opts.xRes*opts.superSamp.x + 2*m_filtExpand.x;
    m_ySampRes = opts.yRes*opts.superSamp.y + 2*m_filtExpand.y;

    // Initialize default fragment
    fillDefault(m_defaultFragment, outVars);
    m_fragSize = m_defaultFragment.size();

    m_doFilter = opts.doFilter && m_filtWidth != Imath::V2i(1,1);
    // Allocate filter result array when necessary
    if(m_doFilter)
        m_filterResults.resize(m_xSampRes*m_fragSize);

    // Initialize sample array.  Non-jittered for now.
    int nsamples = m_xSampRes*m_ySampRes;
    m_samples.resize(nsamples);
    Vec2 invRes = Vec2(1.0f/opts.superSamp.x, 1.0f/opts.superSamp.y);
    for(int j = 0; j < m_ySampRes; ++j)
    {
        for(int i = 0; i < m_xSampRes; ++i)
        {
            Vec2 pos = invRes*(  Vec2(i, j) - Vec2(m_filtExpand)
                               + Vec2(0.5f) );
            m_samples[j*m_xSampRes + i] = Sample(pos);
        }
    }

    bool hasMotion = opts.shutterMin != opts.shutterMax;
    bool hasDof = opts.fstop != FLT_MAX;
    float timeStratQuality = 0;
    if(hasMotion && hasDof)
        timeStratQuality = 0.5;
    else if(hasMotion)
        timeStratQuality = 1;
    else if(hasDof)
        timeStratQuality = 0;
    // TODO: Avoid running makeTileSet entirely if hasMotion is false.
    int tileWidth = 13;
    std::vector<int> tiles;
    std::vector<float> tuv;
    canonicalTimeLensSamps(tuv, tileWidth*tileWidth);
    makeTileSet(tiles, tileWidth, tuv, timeStratQuality);
    m_extraDims.resize(tileWidth*tileWidth);
    for(int i = 0, iend=m_extraDims.size(); i < iend; ++i)
    {
        m_extraDims[i].time = tuv[3*i];
        m_extraDims[i].lens = Vec2(tuv[3*i+1], tuv[3*i+2]);
    }
//    saveTileSet("samples_cpp.txt", m_extraDims, tiles, tileWidth);

//    loadTileSet("samples.txt", m_extraDims, tiles, tileWidth);

    const int ncol = 3;
    SpatialHash hash(ncol);
    // Initialize interleaved sampling info
    m_tileSize = Imath::V2i(tileWidth,tileWidth);
    m_nTiles = Imath::V2i(m_xSampRes-1, m_ySampRes-1)/m_tileSize + Imath::V2i(1);
    // Make shuffled tile indices
    int sampsPerTile = m_tileSize.x*m_tileSize.y;
    m_tileShuffleIndices.resize(sampsPerTile*m_nTiles.x*m_nTiles.y, -1);
    for(int ty = 0, shuffStart = 0; ty < m_nTiles.y; ++ty)
    {
        for(int tx = 0; tx < m_nTiles.x; ++tx, shuffStart += sampsPerTile)
        {
            // Compute current tile using spatial hash function.
            int c1 = hash(tx, ty);
            int c2 = hash(tx+1, ty);
            int c3 = hash(tx, ty+1);
            int c4 = hash(tx+1, ty+1);
            const int* inTile = &tiles[(((c4*ncol + c3)*ncol + c2)*ncol + c1) *
                                       sampsPerTile];
            // The suffle indices are the inverse of the mapping provided by
            // inTile.
            int* outTile = &m_tileShuffleIndices[shuffStart];
            int k = 0;
            const int tyend = std::min((ty+1)*m_tileSize.y, m_ySampRes);
            const int txend = std::min((tx+1)*m_tileSize.x, m_xSampRes);
            for(int j = ty*m_tileSize.y; j < tyend; ++j)
                for(int i = tx*m_tileSize.x; i < txend; ++i, ++k)
                    outTile[inTile[k]] = j*m_xSampRes + i;
        }
    }

    // Rescale times
    for(int i = 0; i < sampsPerTile; ++i)
        m_extraDims[i].time = opts.shutterMin +
                (opts.shutterMax-opts.shutterMin)*m_extraDims[i].time;

    // Initialize fragment array using default fragment.
    m_fragments.resize(m_fragSize*nsamples);
    copy(FvecView(&m_fragments[0], m_fragSize),
         ConstFvecView(&m_defaultFragment[0], m_fragSize, 0),
         nsamples);
}


const float* SampleStorage::outputScanline(int line) const
{
    if(m_doFilter)
    {
        if(m_useSeparable)
            return filterSeparable(line);
        else
            return filterNonSeparable(line);
    }
    else
        return &m_fragments[0] + line*m_xSampRes*m_fragSize;
}


Imath::V2i SampleStorage::outputSize()
{
    if(m_doFilter)
        return Imath::V2i(m_opts.xRes, m_opts.yRes);
    else
        return Imath::V2i(m_xSampRes, m_ySampRes);
}


//------------------------------------------------------------------------------
// Private SampleStorage methods

/// Fill an array with the default no-hit fragment sample values
void SampleStorage::fillDefault(std::vector<float>& defaultFrag,
                                const OutvarSet& outVars)
{
    int nchans = 0;
    for(int i = 0, iend = outVars.size(); i < iend; ++i)
        nchans += outVars[i].scalarSize();
    // Set up default values for samples.
    defaultFrag.assign(nchans, 0.0f);
    // Fill in default depth if relevant
    int zIdx = outVars.find(StdOutInd::z);
    if(zIdx != OutvarSet::npos)
    {
        int zOffset = outVars[zIdx].offset;
        defaultFrag[zOffset] = FLT_MAX;
    }
}


/// Execute separable filter on the row using cached filter coefficients
const float* SampleStorage::filterSeparable(int row) const
{
    // Zero channels of the current output
    std::memset(&m_filterResults[0], 0, m_filterResults.size()*sizeof(float));
    // Stride between end of samples on one row & begin of samples on the next
    const int rowStride = m_fragSize*(m_xSampRes - 1);
    // Storage for partial filtering results.
    std::vector<float> partial(m_xSampRes*m_fragSize, 0);
    // First, filter in the y-direction:
    const float* regionSrc = &m_fragments[ m_xSampRes*row*m_opts.superSamp.y
                                           * m_fragSize ];
    const float* const fy = &m_filter[m_filtWidth.x];
    for(int col = 0; col < m_xSampRes; ++col)
    {
        const float* src = regionSrc + col*m_fragSize;
        float* out = &partial[col*m_fragSize];
        for(int k = 0; k < m_filtWidth.y; ++k)
        {
            float w = fy[k];
            for(int i = 0; i < m_fragSize; ++i, ++src)
                out[i] += *src * w;
            src += rowStride;
        }
    }
    // Now filter in the x-direction using the partial results from the
    // previous step.
    const float* const fx = &m_filter[0];
    for(int col = 0; col < m_opts.xRes; ++col)
    {
        const float* src = &partial[col*m_opts.superSamp.x*m_fragSize];
        float* out = &m_filterResults[col*m_fragSize];
        for(int j = 0; j < m_filtWidth.x; ++j)
        {
            float w = fx[j];
            for(int i = 0; i < m_fragSize; ++i, ++src)
                out[i] += *src * w;
        }
    }
    return &m_filterResults[0];
}


/// Execute non-separable filter on row using cached filter coefficients
const float* SampleStorage::filterNonSeparable(int row) const
{
    // Zero channels of the current output
    std::memset(&m_filterResults[0], 0, m_filterResults.size()*sizeof(float));
    const int rowStride = m_fragSize*(m_xSampRes - m_filtWidth.x);
    for(int col = 0; col < m_opts.xRes; ++col)
    {
        float* out = &m_filterResults[col*m_fragSize];
        // Iterate over samples in the current filter region.
        const float* src = &m_fragments[ (m_xSampRes*row*m_opts.superSamp.y
                                    + col*m_opts.superSamp.x) * m_fragSize ];
        const float* filt = &m_filter[0];
        for(int k = 0; k < m_filtWidth.y; ++k)
        {
            for(int j = 0; j < m_filtWidth.x; ++j, ++filt)
            {
                float w = *filt;
                for(int i = 0; i < m_fragSize; ++i, ++src)
                    out[i] += *src * w;
            }
            src += rowStride;
        }
    }
    return &m_filterResults[0];
}


/// Compute the discrete filter size in sample widths
///
/// The floating point filter radius implies a discrete filter size
void SampleStorage::filterSize(float radius, int sampsPerPix, int& size,
                               int& offset)
{
    // Separate cases for even & odd numbers of samples per pixel.
    if(sampsPerPix%2 == 0)
    {
        int discreteRadius = ifloor(radius*sampsPerPix + 0.5);
        size = 2*discreteRadius;
        offset = discreteRadius - sampsPerPix/2;
    }
    else
    {
        int discreteRadius = ifloor(radius*sampsPerPix);
        size = 2*discreteRadius + 1;
        offset = discreteRadius - sampsPerPix/2;
    }
}

// Cache filter coefficients for efficiency
//
// We compute the value of the filter at subpixel centres, and don't worry
// about the small jittering of position.  In practise the noise arising from
// the underlying image tends to be much larger than any resulting errors.
//
// \param filter - cache for filter coeffs
// \param opts - renderer options
// \param useSeparable - use separable filter? (output)
// \param offset - Offset between top left sample in pixel & top-left
//                 sample in the filter region (output)
// \param filtWidth - Discrete filter width in number of subpixels (output)
void SampleStorage::cacheFilter(std::vector<float>& filter, const Options& opts,
                                bool& useSeparable, Imath::V2i& offset,
                                Imath::V2i& filtWidth)
{
    filterSize(opts.pixelFilter->width().x/2, opts.superSamp.x,
               filtWidth.x, offset.x);
    filterSize(opts.pixelFilter->width().y/2, opts.superSamp.x,
               filtWidth.y, offset.y);
    if(opts.doFilter)
    {
        useSeparable = opts.pixelFilter->isSeparable();
        if(useSeparable)
            cacheFilterSeparable(filter, opts, filtWidth);
        else
            cacheFilterNonSeparable(filter, opts, filtWidth);
    }
}

/// Cache filter coefficients for a separable filter.
void SampleStorage::cacheFilterSeparable(std::vector<float>& filter,
                                         const Options& opts,
                                         Imath::V2i& filtWidth)
{
    filter.resize(filtWidth.x + filtWidth.y);
    float* const fx = &filter[0];
    float* const fy = &filter[filtWidth.x];
    // Compute filter coefficients along x-direction.
    for(int i = 0; i < filtWidth.x; ++i)
    {
        float x = (i-(filtWidth.x-1)/2.0f)/opts.superSamp.x;
        fx[i] = (*opts.pixelFilter)(x, 0);
    }
    for(int i = 0; i < filtWidth.y; ++i)
    {
        float y = (i-(filtWidth.y-1)/2.0f)/opts.superSamp.y;
        fy[i] = (*opts.pixelFilter)(0, y);
    }
    // Normalize.  First compute total filter weight
    float totWeight = 0;
    for(int j = 0; j < filtWidth.y; ++j)
        for(int i = 0; i < filtWidth.x; ++i)
            totWeight += fx[i]*fy[j];
    // then renormalize both x and y weights
    float renorm = 1.0f/std::sqrt(totWeight);
    for(int i = 0, iend = filter.size(); i < iend; ++i)
        filter[i] *= renorm;
#if 0
    // Debug: Dump filter coefficients
    for(int j = 0; j < filtWidth.y; ++j)
    {
        for(int i = 0; i < filtWidth.x; ++i)
            std::cout << fx[i]*fy[j] << ",  ";
        std::cout << "\n";
    }
#endif
}

/// Cache filter coefficients for a nonseparable filter
void SampleStorage::cacheFilterNonSeparable(std::vector<float>& filter,
                                      const Options& opts,
                                      Imath::V2i& filtWidth)
{
    filter.resize(filtWidth.x*filtWidth.y);
    float* f = &filter[0];
    float totWeight = 0;
    for(int j = 0; j < filtWidth.y; ++j)
    {
        float y = (j-(filtWidth.y-1)/2.0f)/opts.superSamp.y;
        for(int i = 0; i < filtWidth.x; ++i, ++f)
        {
            float x = (i-(filtWidth.x-1)/2.0f)/opts.superSamp.x;
            float w = (*opts.pixelFilter)(x, y);
            *f = w;
            totWeight += w;
        }
    }
    // Normalize total weight to 1.
    float renorm = 1/totWeight;
    for(int i = 0, iend = filtWidth.y*filtWidth.x; i < iend; ++i)
        filter[i] *= renorm;
#if 0
    // Debug: Dump filter coefficients
    f = &filter[0];
    for(int j = 0; j < filtWidth.y; ++j)
    {
        for(int i = 0; i < filtWidth.x; ++i, ++f)
            std::cout << *f << ",  ";
        std::cout << "\n";
    }
#endif
}

