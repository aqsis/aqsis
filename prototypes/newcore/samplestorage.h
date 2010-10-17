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

#ifndef AQSIS_SAMPLESTORAGE_H_INCLUDED
#define AQSIS_SAMPLESTORAGE_H_INCLUDED

#include <vector>

#include "renderer.h"  // For OutvarSet.
#include "sample.h"

namespace Aqsis {


/// Storage for samples positions and output fragments
class SampleStorage
{
    public:
        SampleStorage(const OutvarSet& outVars, const Options& opts);

        /// Get a scanline of the output image, filtering if necessary.
        const float* outputScanline(int line) const;
        /// Get the size of the output image.
        Imath::V2i outputSize();

        /// Get number of floats required to store a fragment
        int fragmentSize() const { return m_defaultFragment.size(); }
        /// Get array containing the default fragment values
        const float* defaultFragment() const { return &m_defaultFragment[0]; }

        class Iterator;

        /// Start iterating over samples inside a given bound
        Iterator begin(const Box& bound);


    private:
        static void fillDefault(std::vector<float>& defaultFrag,
                                const OutvarSet& outVars);

        const float* filterNonSeparable(int row) const;
        const float* filterSeparable(int row) const;

        static void filterSize(float radius, int sampsPerPix, int& size,
                               int& offset);
        static void cacheFilter(std::vector<float>& filter, const Options& opts,
                                bool& useSeparable, Imath::V2i& offset,
                                Imath::V2i& filtWidth);
        static void cacheFilterSeparable(std::vector<float>& filter,
                                         const Options& opts,
                                         Imath::V2i& filtWidth);
        static void cacheFilterNonSeparable(std::vector<float>& filter,
                                            const Options& opts,
                                            Imath::V2i& filtWidth);

    // TODO: Clean up so that member data can be private again!
    public:
        // Stuff describing size of sample area
        const Options& m_opts; ///< options structure
        int m_fragSize;        ///< length of single fragment storage
        int m_xSampRes;        ///< number of samples in x-direction
        int m_ySampRes;        ///< number of samples in y-direction

        // Stuff describing samples
        std::vector<Sample> m_samples;        ///< sample positions

        /// Time and Lens positions
        struct TimeLens
        {
            float time;
            Vec2 lens;
        };

        // Info for interleaved sampling
        Imath::V2i m_tileSize;  ///< tile size for interleaved sampling
        Imath::V2i m_nTiles;    ///< number of tiles
        std::vector<int> m_tileShuffleIndices; ///< Indices into m_extraDims
        std::vector<TimeLens> m_extraDims;     ///< times & lens positions

        // Stuff describing fragments
        std::vector<float> m_defaultFragment; ///< Default fragment channels
        std::vector<float> m_fragments;       ///< array of fragments

        // Temporary array for filtering results
        mutable std::vector<float> m_filterResults;

        // Cached filter info
        bool m_doFilter;         ///< perform filtering ?
        bool m_useSeparable;     ///< use separable filtering optimization ?
        Imath::V2i m_filtExpand; ///< num samples to expand by for filter.
        Imath::V2i m_filtWidth;  ///< width of filter _in subpixels_
        std::vector<float> m_filter;
};


/// Iterator over a rectangular region of samples.
///
/// Note that the implementation here is quite performance critical,
/// since it's inside one of the inner sampling loops.
///
class SampleStorage::Iterator
{
    private:
        int m_startx;
        int m_endx;
        int m_endy;
        int m_x;
        int m_y;

        int m_rowStride;   ///< stride between end of one row & start of next
        int m_fragSize;    ///< number of floats in a fragment
        Sample* m_sample;  ///< current sample data
        float* m_fragment; ///< current fragment data

    public:
        Iterator(const Box& bound, SampleStorage& storage)
        {
            // Bounding box for relevant samples, clamped to image extent.
            Imath::V2i bndMin = ifloor(vec2_cast(bound.min));
            Imath::V2i bndMax = ifloor(vec2_cast(bound.max));
            m_startx = clamp(bndMin.x, 0, storage.m_xSampRes);
            m_endx = clamp(bndMax.x+1, 0, storage.m_xSampRes);
            int starty = clamp(bndMin.y, 0, storage.m_ySampRes);
            m_endy = clamp(bndMax.y+1, 0, storage.m_ySampRes);

            m_x = m_startx;
            m_y = starty;
            // ensure !valid() if bound is empty in x-direction
            if(m_startx >= m_endx)
                m_y = m_endy;

            if(valid())
            {
                m_rowStride = storage.m_xSampRes - (m_endx - m_startx);
                m_fragSize = storage.m_fragSize;
                int idx = m_y*storage.m_xSampRes + m_x;
                m_sample = &storage.m_samples[idx];
                m_fragment = &storage.m_fragments[m_fragSize*idx];
            }
        }

        /// Advance to next sample
        Iterator& operator++()
        {
            ++m_x;
            ++m_sample;
            m_fragment += m_fragSize;
            if(m_x == m_endx)
            {
                // Advance to next row.
                m_x = m_startx;
                ++m_y;
                m_sample += m_rowStride;
                m_fragment += m_rowStride*m_fragSize;
            }
            return *this;
        }

        /// Determine whether current sample is in the bound
        bool valid() const { return m_y < m_endy; }

        /// Get current sample data
        Sample& sample() const { return *m_sample; }
        /// Get current fragment storage
        float* fragment() const { return m_fragment; }
};


inline SampleStorage::Iterator SampleStorage::begin(const Box& bound)
{
    return Iterator(bound, *this);
}


} // namespace Aqsis
#endif // AQSIS_SAMPLESTORAGE_H_INCLUDED
