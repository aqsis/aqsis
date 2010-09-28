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

#include "filterprocessor.h"

#include <boost/functional/hash.hpp>


//------------------------------------------------------------------------------
// Cached filter implementation

CachedFilter::CachedFilter(const Filter& filterFunc, const V2i& superSamp)
    : m_offset(0),
    m_size(0),
    m_isSeparable(false), // TODO
    m_weights()
{
    filterSize(filterFunc.width().x/2, superSamp.x, m_size.x, m_offset.x);
    filterSize(filterFunc.width().y/2, superSamp.y, m_size.y, m_offset.y);
    cacheFilterNonSeparable(m_weights, filterFunc, superSamp, m_size);
}

/// Compute the discrete filter size in sample coordinates
///
/// radius - filter radius in pixel widths
/// sampsPerPix - number of samples inside pixel
/// size   - discrete filter size (output)
/// offset - number of samples between start of filter support and the first
///          sample in the pixel holding the filter centre.
///
void CachedFilter::filterSize(float radius, int sampsPerPix,
                              int& size, int& offset)
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

/// Cache filter coefficients for a nonseparable filter and normalize.
///
/// cache - computed filter coefficients go here
/// filterFunc - filter functor
/// superSamp - supersampling resolution
/// filtSize - size of the cache in samples
void CachedFilter::cacheFilterNonSeparable(std::vector<float>& cache,
                                           const Filter& filterFunc,
                                           const V2i& superSamp,
                                           Imath::V2i& filtSize)
{
    cache.resize(filtSize.x*filtSize.y);
    float* f = &cache[0];
    float totWeight = 0;
    for(int j = 0; j < filtSize.y; ++j)
    {
        float y = (j-(filtSize.y-1)/2.0f)/superSamp.y;
        for(int i = 0; i < filtSize.x; ++i, ++f)
        {
            float x = (i-(filtSize.x-1)/2.0f)/superSamp.x;
            float w = filterFunc(x, y);
            *f = w;
            totWeight += w;
        }
    }
    // Normalize total weight to 1.
    float renorm = 1/totWeight;
    for(int i = 0, iend = filtSize.y*filtSize.x; i < iend; ++i)
        cache[i] *= renorm;
#if 0
    // Debug: Dump filter coefficients
    f = &cache[0];
    for(int j = 0; j < filtSize.y; ++j)
    {
        for(int i = 0; i < filtSize.x; ++i, ++f)
            std::cout << *f << ",  ";
        std::cout << "\n";
    }
#endif
}


namespace Imath // namespace needed to allow ADL to find hash_value :-(
{
/// Hash function for 2D points, as in boost docs.
std::size_t hash_value(V2i const& p)
{
    std::size_t seed = 0;
    boost::hash_combine(seed, p.x);
    boost::hash_combine(seed, p.y);
    return seed;
}
}


FilterProcessor::FilterProcessor(DisplayManager& displayManager,
                                 const Imath::Box2i& outTileRange,
                                 const CachedFilter& cachedFilter,
                                 const V2i& filterStride)
    : m_displayManager(displayManager),
    m_waitingTiles(2*(outTileRange.max.x - outTileRange.min.x)),
    m_outTileRange(outTileRange),
    m_outTileWorkspace(),
    m_filterStride(filterStride),
    m_filter(cachedFilter)
{ }

void FilterProcessor::insert(const FragmentTilePtr& tile)
{
    // The sample tile with coordinates (tx,ty) overlaps the four
    // filtering tiles with coordinates:
    //
    //   (tx-1, ty-1)   (tx,ty-1)
    //   (tx-1, ty)     (tx,ty)
    //
    // we insert the sample tile into each of these filtering tiles,
    // ignoring filtering tile coordinates which lie outside the image
    // boundaries.
    for(int j = 0; j < 2; ++j)
    for(int i = 0; i < 2; ++i)
    {
        // p is position of the output tile
        V2i p = tile->position() + V2i(i-1,j-1);
        // Ignore filtering tiles outside image boundaries
        if(p.x < m_outTileRange.min.x || p.y < m_outTileRange.min.y ||
           p.x >= m_outTileRange.max.x || p.y >= m_outTileRange.max.y)
            continue;
        FilterBlockMap::iterator blocki = m_waitingTiles.find(p);
        if(blocki == m_waitingTiles.end())
        {
            // Create new block if it didn't exist yet
            std::pair<FilterBlockMap::iterator, bool> insRes =
                m_waitingTiles.insert(std::make_pair(p, FilterBlock()));
            assert(insRes.second);
            blocki = insRes.first;
        }
        FilterBlock& block = blocki->second;
        block.tiles[1-j][1-i] = tile;
        if(block.readyForFilter())
        {
            // Filter, quantize & save result.
            filter(m_outTileWorkspace, block);
            const V2i outTileSize = block.tiles[0][0]->size()/m_filterStride;
            m_displayManager.writeTile(p*outTileSize, &m_outTileWorkspace[0]);
            m_waitingTiles.erase(blocki);
        }
    }
}

/// Filter a 2x2 block of sampled tiles
void FilterProcessor::filter(std::vector<float>& output,
                             const FilterBlock& block)
{
    // Input tile size before filtering
    const V2i tileSize = block.tiles[0][0]->size();
    const int nChans = block.tiles[0][0]->fragSize();
    output.resize(nChans*prod(tileSize));
    std::memset(&output[0], '\0', sizeof(float)*nChans*prod(tileSize));
    const V2i offset = tileSize/2 - m_filter.offset();
    const V2i outWidth = tileSize/m_filterStride;
    // Loop over every pixel in the output
    for(int iy = 0; iy < outWidth.y; ++iy)
    for(int ix = 0; ix < outWidth.x; ++ix)
    {
        // Filter pixel at (ix,iy).  The filter support can lie across any or
        // all of the 2x2 block of input tiles.
        float* samps = &output[0] + (outWidth.x*iy + ix)*nChans;
        for(int j = 0; j < m_filter.size().y; ++j)
        for(int i = 0; i < m_filter.size().x; ++i)
        {
            // x,y is location of the current sample with respect to top left
            // of top left tile in the 2x2 block.
            int x = m_filterStride.x*ix + offset.x + i;
            int y = m_filterStride.y*iy + offset.y + j;
            int tx = x >= tileSize.x;
            int ty = y >= tileSize.y;
            const float* c = block.tiles[ty][tx]->fragment(x - tx*tileSize.x,
                                                           y - ty*tileSize.y);
            float w = m_filter(i,j);
            for(int k = 0; k < nChans; ++k)
                samps[k] += w*c[k];
        }
    }
}

