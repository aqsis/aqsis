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

#ifndef SAMPLESTORAGE_H_INCLUDED
#define SAMPLESTORAGE_H_INCLUDED

#include <vector>

#include "renderer.h"  // For OutvarSet.
#include "sample.h"


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
            Imath::V2i bndMin = ifloor(vec2_cast(bound.min)
                    *storage.m_opts.superSamp) + storage.m_filtExpand;
            Imath::V2i bndMax = ifloor(vec2_cast(bound.max)
                    *storage.m_opts.superSamp) + storage.m_filtExpand;
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

#endif // SAMPLESTORAGE_H_INCLUDED
