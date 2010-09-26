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

#ifndef FILTERPROCESSOR_H_INCLUDED
#define FILTERPROCESSOR_H_INCLUDED

#include <boost/unordered_map.hpp>

#include "arrayview.h"
#include "refcount.h"
#include "sample.h"
#include "displaymanager.h"
#include "util.h"

//------------------------------------------------------------------------------
/// Sample position and fragment storage
class SampleTile : public RefCounted
{
    public:
        /// Create an empty sample tile
        ///
        /// pos  - discrete tile coordinates
        /// size - size of tile in samples
        /// sampleOffset - position of top left sample
        /// defaultFrag  - initialize all fragments with these samples
        /// fragSize     - number of samples in defaultFrag
        SampleTile(const V2i& pos, const V2i& size, const V2i& sampleOffset,
                   const float* defaultFrag, int fragSize)
            : m_size(size),
            m_position(pos),
            m_sampleOffset(sampleOffset),
            m_samples(prod(m_size)),
            m_fragments(m_samples.size()*fragSize),
            m_fragSize(fragSize)
        {
            for(int j = 0; j < m_size.y; ++j)
            for(int i = 0; i < m_size.x; ++i)
            {
                m_samples[m_size.x*j + i] = Sample(
                    Vec2(i,j) + Vec2(m_sampleOffset) + Vec2(0.5f));
            }
            copy(FvecView(&m_fragments[0], fragSize),
                 ConstFvecView(defaultFrag, fragSize, 0), prod(m_size));
        }


        /// Get fragment relative to (0,0) in upper-left of tile.
        float* fragment(int x, int y)
        {
            return &m_fragments[m_fragSize*(m_size.x*y + x)];
        }
        const float* fragment(int x, int y) const
        {
            return &m_fragments[m_fragSize*(m_size.x*y + x)];
        }
        /// Get sample position relative to (0,0) in upper-left of tile.
        Sample& sample(int x, int y)
        {
            return m_samples[m_size.x*y + x];
        }

        /// Get position of the tile in tile coordinates
        V2i position() const { return m_position; }

        /// Get size of tile in samples
        V2i size() const { return m_size; }

        /// NUmber of floats in a fragment
        int fragSize() const { return m_fragSize; }

        /// Get position of top-left of tile in sraster coordinates.
        V2i sampleOffset() const { return m_sampleOffset; }

    private:
        V2i m_size;
        V2i m_position;
        V2i m_sampleOffset;
        std::vector<Sample> m_samples;
        std::vector<float> m_fragments;
        int m_fragSize;
};

typedef boost::intrusive_ptr<SampleTile> SampleTilePtr;


//------------------------------------------------------------------------------
/// Storage for cached filter coefficients.
///
/// Filter coefficients can be quite expensive to recompute many times,
/// especially because they often use special functions in the definition of
/// the filter kernel.  This class caches a given filter at a fixed set of
/// points for efficiency.  The points chosen are on an evenly spaced grid of
/// sample positions centred at (0,0).
///
/// Here's a 1D example with two samples per pixel and filter width = 2:
///
///              filter support width 2.0
///         <---------------X--------------->
///
///                        radius = 1.0 pixels
///                         <--------------->
/// |               |               |               |
/// |   s       s   |   s       s   |   s       s   |
/// |               |               |               |
///
///             <------->
///          offset=1 samples
///
///             <----------------------->
///                   size=4 samples
///
/// | = pixel "boundaries"
/// s = sample position (without jittering)
/// X = filter centre
///
/// For filters of width 1, offset is always 0.
///
class CachedFilter
{
    public:
        /// Cache a given filter using the provided sampling resolution.
        CachedFilter(const Filter& filterFunc, const V2i& superSamp);

        /// Get sample offset to top left of filter
        const V2i& offset() const { return m_offset; }
        /// Get filter size in samples
        const V2i& size()   const { return m_size; }

        /// Get cached filter coefficient
        float operator()(int x, int y) const { return m_weights[m_size.x*y + x]; }
        float xweight1d(int x) const { return m_weights[x]; }
        float yweight1d(int y) const { return m_weights[m_size.x + y]; }

        /// Return true if filter is separable.
        bool isSeparable() const { return m_isSeparable; }

    private:
        static void filterSize(float radius, int sampsPerPix,
                               int& size, int& offset);

        static void cacheFilterNonSeparable(std::vector<float>& cache,
                                            const Filter& filterFunc,
                                            const V2i& superSamp,
                                            Imath::V2i& filtSize);

        V2i m_offset; ///< Sample offset for top left of filter
        V2i m_size;   ///< Size of the filter support
        bool m_isSeparable; ///< True if filter is separable
        std::vector<float> m_weights; ///< Cached filter weights
};


//------------------------------------------------------------------------------
/// Pixel filter execution class
///
/// This class collectes tiles of image samples and filters them to produce
/// tiles of output pixels.  In general, the filter region for each pixel
/// covers several adjacent pixels worth of samples so we need several
/// adjacent input tiles to produce an output tile of the same size.
///
/// To simplify things, we choose to make the output tiling to be the "dual
/// tessellation" of the input tiling as in the diagram below.  (That is, the
/// centres of the input tiles are the corners of the output tiles.)  As a
/// result, each output tile is the same size (in raster coordinates) as each
/// input tile, and we always need a 2x2 block of input tiles to produce each
/// output tile.
///
/// Diagram of filtering geometry:
///
///  A--------------------B--------------------+
///  |                    |                    |
///  |      .   .   .   . | .   .   .   .      |
///  |                    |                    |
///  |      .  P.....................Q  .      |
///  |         .          |          .         |
///  |      .  .          |          .  .      |
///  |         .          |          .         |
///  D---------.----------C----------.---------+
///  |         .          |          .         |
///  |      .  .          |          .  .      |
///  |         .          |          .         |
///  |      .  S.....................R  .      |
///  |                    |                    |
///  |      .   .   .   . | .   .   .   .      |
///  |                    |                    |
///  +--------------------+--------------------+
///
///  ----- ABCD is an tile of input samples
///  ..... PQRS is an tile of output filtered pixels
///  .   . is the filter region expanded by the filter radius
class FilterProcessor
{
    public:
        /// Create filter processor
        ///
        /// displayManager - sink for completed output tiles of pixels
        /// outTileRange - range of valid output tiles
        /// cachedFilter - cached filter coefficients
        /// filterStride - number of samples between pixels
        FilterProcessor(DisplayManager& displayManager,
                        const Imath::Box2i& outTileRange,
                        const CachedFilter& cachedFilter,
                        const V2i& filterStride);

        /// Insert a finished sample tile
        void insert(const SampleTilePtr& tile);

    private:
        /// 2x2 block of sample tiles, as needed for an output tile.
        struct FilterBlock
        {
            SampleTilePtr tiles[2][2];

            bool readyForFilter() const { return tiles[0][0] && tiles[0][1]
                                              && tiles[1][0] && tiles[1][1]; }
        };

        typedef boost::unordered_map<V2i, FilterBlock> FilterBlockMap;

        void filter(std::vector<float>& output, const FilterBlock& block);


        DisplayManager& m_displayManager; ///< Filtered tiles go here.
        FilterBlockMap m_waitingTiles; ///< Storage for waiting sampled tiles
        Imath::Box2i m_outTileRange;   ///< Generate output tiles in here

        std::vector<float> m_outTileWorkspace; ///< Output tile working space
        V2i m_filterStride; ///< Stride in samples between adjacent pixels
        const CachedFilter& m_filter; ///< cached filter coeffs
};


#endif // FILTERPROCESSOR_H_INCLUDED
