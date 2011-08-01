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

#include "arrayview.h"
#include "filterprocessor.h"

#include <boost/functional/hash.hpp>

namespace Imath // [namespace needed to allow ADL to find hash_value :-( ]
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

namespace Aqsis {

//------------------------------------------------------------------------------
// Cached filter implementation

CachedFilter::CachedFilter(const Filter& filterFunc, const V2i& superSamp)
    : m_offset(0),
    m_size(0),
    m_isSeparable(filterFunc.isSeparable()),
    m_weights()
{
    filterSize(filterFunc.width().x/2, superSamp.x, m_size.x, m_offset.x);
    filterSize(filterFunc.width().y/2, superSamp.y, m_size.y, m_offset.y);
    if(m_isSeparable)
        cacheFilterSeparable(m_weights, filterFunc, superSamp, m_size);
    else
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

/// Cache filter coefficients for a separable filter.
///
/// \see cacheFilterNonSeparable
void CachedFilter::cacheFilterSeparable(std::vector<float>& cache,
                                         const Filter& filterFunc,
                                         const V2i& superSamp,
                                         V2i& filtSize)
{
    cache.resize(filtSize.x + filtSize.y);
    // Compute filter coefficients along x-direction.
    float* fx = cbegin(cache);
    for(int i = 0; i < filtSize.x; ++i)
    {
        float x = (i-(filtSize.x-1)/2.0f)/superSamp.x;
        fx[i] = filterFunc(x, 0);
    }
    normalizeFilter(fx, filtSize.x);
    // Compute filter coefficients along y-direction.
    float* fy = &cache[filtSize.x];
    for(int i = 0; i < filtSize.y; ++i)
    {
        float y = (i-(filtSize.y-1)/2.0f)/superSamp.y;
        fy[i] = filterFunc(0, y);
    }
    normalizeFilter(fy, filtSize.y);
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
                                           V2i& filtSize)
{
    cache.resize(filtSize.x*filtSize.y);
    float* f = cbegin(cache);
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
    normalizeFilter(cbegin(cache), cache.size());
}

/// Normalize filter weights so that they add to one.
///
/// weights - filter weights
/// nWeights - number of weights
void CachedFilter::normalizeFilter(float* weights, int nWeights)
{
    float totWeight = 0;
    for(int i = 0; i < nWeights; ++i)
        totWeight += weights[i];
    assert(totWeight != 0);
    float renorm = 1/totWeight;
    for(int i = 0; i < nWeights; ++i)
        weights[i] *= renorm;
}


//------------------------------------------------------------------------------
// FilterProcessor implementation

FilterProcessor::FilterProcessor(DisplayManager& displayManager,
                                 const Box2i& outTileRange,
                                 const CachedFilter& cachedFilter,
                                 const V2i& filterStride)
    : m_displayManager(displayManager),
    m_waitingTiles(2*(outTileRange.max.x - outTileRange.min.x)),
    m_outTileRange(outTileRange),
    m_outTileWorkspace(),
    m_filterStride(filterStride),
    m_filter(cachedFilter)
{ }

void FilterProcessor::insert(V2i position, const FragmentTilePtr& tile)
{
    // Get working space for filter.
    if(!m_outTileWorkspace.get())
        m_outTileWorkspace.reset(new std::vector<float>());
    std::vector<float>& outTileWorkspace = *m_outTileWorkspace;
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
        V2i p = position + V2i(i-1,j-1);
        // Ignore filtering tiles outside image boundaries
        if(p.x < m_outTileRange.min.x || p.y < m_outTileRange.min.y ||
           p.x >= m_outTileRange.max.x || p.y >= m_outTileRange.max.y)
            continue;
        FilterBlock blockToFilter;
        {
            // Look up the filter block in the map, and add the current tile.
            LockGuard lock(m_waitingTilesMutex);
            FilterBlockMap::iterator blocki = m_waitingTiles.find(p);
            if(blocki == m_waitingTiles.end())
            {
                // Create new block if it didn't exist yet
                std::pair<FilterBlockMap::iterator, bool> insRes =
                    m_waitingTiles.insert(std::make_pair(p, FilterBlock()));
                assert(insRes.second);
                blocki = insRes.first;
            }
            blocki->second.tiles[1-j][1-i] = tile;
            if(blocki->second.readyForFilter())
            {
                // If the block is complete, grab a copy and delete from map
                blockToFilter = blocki->second;
                m_waitingTiles.erase(blocki);
            }
        }
        if(blockToFilter.readyForFilter())
        {
            // Filter, quantize & save result.
            filter(outTileWorkspace, blockToFilter);
            const V2i outTileSize = blockToFilter.tiles[0][0]->size()/m_filterStride;
            m_displayManager.writeTile(p*outTileSize, cbegin(outTileWorkspace));
        }
    }
}

/// Filter a 2x2 block of sampled tiles
void FilterProcessor::filter(std::vector<float>& output,
                             const FilterBlock& block) const
{
    // Input tile size before filtering
    const V2i tileSize = block.tiles[0][0]->size();
    const int fragSize = block.tiles[0][0]->fragSize();
    const float* defaultFrag = block.tiles[0][0]->defaultFrag();
    const V2i outSize = tileSize/m_filterStride;
    output.resize(fragSize*prod(outSize));
    // We distinguish four types of filtering, increasing in work from smallest
    // to greatest:
    if(!block.hasSamples())
    {
        // (1) No filtering
        //
        // If there's no samples, we don't need to filter, we only need to
        // assign the default fragment to every pixel.
        copy(FvecView(cbegin(output), fragSize),
             ConstFvecView(defaultFrag, fragSize, 0), prod(outSize));
    }
    else if(m_filter.size() == V2i(1) /*todo: or filtering turned off*/)
    {
        // (2) Trivial filtering
        //
        // Filter support is a single point, so no filtering is necessary;
        // just copy the samples over in four blocks.
        float* defaultRowStore = 0;
        int rowLen = outSize.x*fragSize;
        int halfRowLen = outSize.x/2*fragSize;
        for(int j = 0; j < 2; ++j)
        for(int i = 0; i < 2; ++i)
        {
            FvecView dest = FvecView(cbegin(output) + i*halfRowLen,
                                     halfRowLen, rowLen) + j*outSize.y/2;
            if(block.tiles[j][i]->hasSamples())
            {
                ConstFvecView src = ConstFvecView(
                        block.tiles[j][i]->fragment(0,0) + (1-i)*halfRowLen,
                        halfRowLen, rowLen) + (1-j)*outSize.y/2;
                copy(dest, src, outSize.y/2);
            }
            else
            {
                // No source samples to copy!  Fill the tile quater in with
                // default fragments instead.
                if(!defaultRowStore)
                {
                    // Construct a row of duplicate pixels from defaultFrag.
                    defaultRowStore = FALLOCA(halfRowLen);
                    copy(FvecView(defaultRowStore, fragSize),
                         ConstFvecView(defaultFrag, fragSize, 0),
                         outSize.x/2);
                }
                copy(dest, ConstFvecView(defaultRowStore, halfRowLen, 0),
                     outSize.y/2);
            }
        }
    }
    else if(m_filter.isSeparable())
    {
        // (3) Separable filtering
        filterSeparable(output, block);
    }
    else
    {
        // (4) Non-separable filtering
        filterNonSeparable(output, block);
    }
}


/// Compute a nonseparable filter.
///
/// A nonseparable filter is the most general case, and costs O(N^2) per pixel,
/// where N is the filter width in samples.
void FilterProcessor::filterNonSeparable(std::vector<float>& output,
                                         const FilterBlock& block) const
{
    const V2i tileSize = block.tiles[0][0]->size();
    const int fragSize = block.tiles[0][0]->fragSize();
    const float* defaultFrag = block.tiles[0][0]->defaultFrag();
    const V2i outSize = tileSize/m_filterStride;
    output.assign(fragSize*prod(outSize), 0);
    const V2i offset = tileSize/2 - m_filter.offset();
    // Loop over every pixel in the output
    for(int iy = 0; iy < outSize.y; ++iy)
    for(int ix = 0; ix < outSize.x; ++ix)
    {
        // Filter pixel at (ix,iy).  The filter support can lie across any
        // or all of the 2x2 block of input tiles.
        float* out = cbegin(output) + (outSize.x*iy + ix)*fragSize;
        for(int j = 0; j < m_filter.size().y; ++j)
        for(int i = 0; i < m_filter.size().x; ++i)
        {
            // x,y is location of the current sample with respect to top
            // left of top left tile in the 2x2 block.
            int x = m_filterStride.x*ix + offset.x + i;
            int y = m_filterStride.y*iy + offset.y + j;
            int tx = x >= tileSize.x;
            int ty = y >= tileSize.y;
            const float* c = defaultFrag;
            if(block.tiles[ty][tx]->hasSamples())
                c = block.tiles[ty][tx]->fragment(x - tx*tileSize.x,
                                                  y - ty*tileSize.y);
            float w = m_filter(i,j);
            for(int k = 0; k < fragSize; ++k)
                out[k] += w*c[k];
        }
    }
}


/// Compute a separable filter.
///
/// This isn't as general as the nonseparable case, but it only costs O(N) per
/// pixel where N is the filter width in samples.  This is a great saving over
/// the nonseparable case when the filter widths are large.
///
/// For nonseparable filtering to be valid, the 2D filter function f(x,y) must
/// have the form
///
///   f(x,y) = f1(x) * f2(y)
///
/// for some 1D functions f1 and f2.  Luckily, a bunch of the more popular
/// filters have this property!
void FilterProcessor::filterSeparable(std::vector<float>& output,
                                      const FilterBlock& block) const
{
    const V2i tileSize = block.tiles[0][0]->size();
    const int fragSize = block.tiles[0][0]->fragSize();
    const float* defaultFrag = block.tiles[0][0]->defaultFrag();
    const V2i outSize = tileSize/m_filterStride;
    output.assign(fragSize*prod(outSize), 0);
    // The details of how the separable filter works are very similar to
    // the nonseparable one (compare the code), but the filtering steps in
    // the two directions are separated.  This results in an algorithm
    // which is O(N) rather than O(N^2) where N is the width of the
    // filter.
    int rowLength = 2*m_filter.offset().x + tileSize.x;
    float* partial = FALLOCA(rowLength*fragSize);
    const V2i offset = tileSize/2 - m_filter.offset();
    // Loop over every row in the output
    for(int iy = 0; iy < outSize.y; ++iy)
    {
        // Compute the filter results for the row.
        //
        // First, filter in the y-direction, and save the results into the
        // partial buffer.  The partial buffer is allocated to be just
        // long enough (rowLength) to include the full filter support of
        // the pixels at the ends of the row.
        std::memset(partial, 0, rowLength*fragSize*sizeof(float));
        for(int col = 0; col < rowLength; ++col)
        {
            float* out = partial + fragSize*col;
            for(int j = 0; j < m_filter.size().y; ++j)
            {
                int x = offset.x + col;
                int y = m_filterStride.y*iy + offset.y + j;
                int tx = x >= tileSize.x;
                int ty = y >= tileSize.y;
                const float* c = defaultFrag;
                if(block.tiles[ty][tx]->hasSamples())
                    c = block.tiles[ty][tx]->fragment(x - tx*tileSize.x,
                                                      y - ty*tileSize.y);
                float w = m_filter.yweight1d(j);
                for(int k = 0; k < fragSize; ++k)
                    out[k] += w*c[k];
            }
        }
        // Next, filter the partial buffer in x to get the output pixels.
        for(int ix = 0; ix < outSize.x; ++ix)
        {
            float* out = cbegin(output) + (outSize.x*iy + ix)*fragSize;
            const float* c = partial + m_filterStride.x*fragSize*ix;
            for(int i = 0; i < m_filter.size().x; ++i)
            {
                float w = m_filter.xweight1d(i);
                for(int k = 0; k < fragSize; ++k)
                    out[k] += w*c[k];
                c += fragSize;
            }
        }
    }
}

} // namespace Aqsis
