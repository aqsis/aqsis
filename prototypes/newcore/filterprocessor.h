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

#ifndef AQSIS_FILTERPROCESSOR_H_INCLUDED
#define AQSIS_FILTERPROCESSOR_H_INCLUDED

#include <boost/unordered_map.hpp>
#include <boost/scoped_array.hpp>

#include "arrayview.h"
#include "displaymanager.h"
#include "refcount.h"
#include "thread.h"
#include "util.h"

namespace Aqsis {

//------------------------------------------------------------------------------
/// Storage for fragments generated during sampling.
///
/// A "fragment" is here taken to be the array of surface data generated when
/// a sample point hits a micropolygon.  For instance, if we're rendering
/// colour, opacity and depth, a fragment is a short 7-element array of
/// floats, something like
///
///   [ R G B OR OG OB Z ]
///
/// (Note that the ordering of the channels in the fragment isn't fixed and is
/// defined by the output variable set, but ordering is irrelevant to the
/// FramentTile class anyway.)
class FragmentTile : public RefCounted
{
    public:
        /// Create an empty fragment tile.
        ///
        /// size - size of tile in samples
        /// sampleOffset - position of top left sample
        /// defaultFrag  - initialize all fragments with these samples
        /// fragSize     - number of samples in each fragment
        FragmentTile(const V2i& size, const V2i& sampleOffset,
                     const float* defaultFrag, int fragSize)
            : m_size(size),
            m_sampleOffset(sampleOffset),
            m_fragments(),
            m_defaultFrag(defaultFrag),
            m_fragSize(fragSize)
        { }

        /// Get fragment relative to (0,0) in upper-left of tile.
        float* fragment(int x, int y)
        {
            return fragment(m_size.x*y + x);
        }
        /// Get fragment at index, assuming row-major fragment storage.
        float* fragment(int index)
        {
            if(!m_fragments)
            {
                // Deferred allocation for memory efficiency.
                m_fragments.reset(new float[prod(m_size)*m_fragSize]);
                copy(FvecView(m_fragments.get(), m_fragSize),
                    ConstFvecView(m_defaultFrag, m_fragSize, 0), prod(m_size));
            }
            return &m_fragments[m_fragSize*(index)];
        }
        const float* fragment(int x, int y) const
        {
            assert(m_fragments);
            return &m_fragments[m_fragSize*(m_size.x*y + x)];
        }

        /// Get size of tile in samples
        V2i size() const { return m_size; }

        /// Return true if the tile contains any samples
        ///
        /// (If the tile has no samples, filtering is trivial.)
        bool hasSamples() { return m_fragments; }

        /// Get default fragment
        const float* defaultFrag() { return m_defaultFrag; }
        /// Number of floats in a fragment
        int fragSize() const { return m_fragSize; }

        /// Get position of top-left of tile in sraster coordinates.
        V2i sampleOffset() const { return m_sampleOffset; }

    private:
        V2i m_size;
        V2i m_sampleOffset;
        boost::scoped_array<float> m_fragments;
        const float* m_defaultFrag;
        int m_fragSize;
};

typedef boost::intrusive_ptr<FragmentTile> FragmentTilePtr;


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

        /// Get cached filter coefficient for a non-separable filter
        float operator()(int x, int y) const
        {
            assert(!m_isSeparable);
            return m_weights[m_size.x*y + x];
        }
        /// Get cached x filter coefficient for a separable filter
        float xweight1d(int x) const
        {
            assert(m_isSeparable);
            return m_weights[x];
        }
        /// Get cached y filter coefficient for a separable filter
        float yweight1d(int y) const
        {
            assert(m_isSeparable);
            return m_weights[m_size.x + y];
        }

        /// Return true if filter is separable.
        bool isSeparable() const { return m_isSeparable; }

    private:
        static void filterSize(float radius, int sampsPerPix,
                               int& size, int& offset);
        static void cacheFilterSeparable(std::vector<float>& cache,
                                         const Filter& filterFunc,
                                         const V2i& superSamp,
                                         V2i& filtSize);
        static void cacheFilterNonSeparable(std::vector<float>& cache,
                                            const Filter& filterFunc,
                                            const V2i& superSamp,
                                            V2i& filtSize);
        static void normalizeFilter(float* weights, int nWeights);

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
                        const Box2i& outTileRange,
                        const CachedFilter& cachedFilter,
                        const V2i& filterStride);

        /// Insert a finished tile of fragments at the given position
        ///
        /// position - location of the sample tile, in "tile coordinates" from
        ///            the top left of the image.
        /// tile - holder for sample data.
        ///
        /// This function is threadsafe.
        void insert(V2i position, const FragmentTilePtr& tile);

    private:
        /// 2x2 block of sample tiles, as needed for generating an output tile
        struct FilterBlock
        {
            FragmentTilePtr tiles[2][2];

            bool readyForFilter() const { return tiles[0][0] && tiles[0][1]
                                              && tiles[1][0] && tiles[1][1]; }
            bool hasSamples() const
            {
                return tiles[0][0]->hasSamples() || tiles[0][1]->hasSamples()
                    || tiles[1][0]->hasSamples() || tiles[1][1]->hasSamples();
            }
        };

        typedef boost::unordered_map<V2i, FilterBlock> FilterBlockMap;

        void filter(std::vector<float>& output,
                    const FilterBlock& block) const;
        void filterNonSeparable(std::vector<float>& output,
                                const FilterBlock& block) const;
        void filterSeparable(std::vector<float>& output,
                             const FilterBlock& block) const;

        DisplayManager& m_displayManager; ///< Filtered tiles go here.
        FilterBlockMap m_waitingTiles; ///< Storage for waiting sampled tiles
        Mutex m_waitingTilesMutex;     ///< Protection for m_waitingTiles
        Box2i m_outTileRange;   ///< Generate output tiles in here

        /// Output tile working space
        ThreadSpecificPtr<std::vector<float> >::type m_outTileWorkspace;
        V2i m_filterStride; ///< Stride in samples between adjacent pixels
        const CachedFilter& m_filter; ///< cached filter coeffs
};


} // namespace Aqsis

#endif // AQSIS_FILTERPROCESSOR_H_INCLUDED
