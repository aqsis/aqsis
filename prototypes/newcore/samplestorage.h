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

#include "util.h"
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
        // Stuff describing size of sample area
        const Options& m_opts; ///< options structure
        int m_fragSize;        ///< length of single fragment storage
        int m_xSampRes;        ///< number of samples in x-direction
        int m_ySampRes;        ///< number of samples in y-direction

        // Stuff describing samples
        std::vector<Sample> m_samples;        ///< sample positions

        // Stuff describing fragments
        std::vector<float> m_defaultFragment; ///< Default fragment channels
        std::vector<float> m_fragments;       ///< array of fragments

        // Temporary array for filtering results
        mutable std::vector<float> m_filterResults;

        // Cached filter info
        bool m_doFilter;         ///< perform filtering ?
        Imath::V2i m_filtExpand; ///< num samples to expand by for filter.
        Imath::V2i m_disWidth;   ///< discrete filter width
        std::vector<float> m_filter;

        /// Fill an array with the default no-hit fragment sample values
        static void fillDefault(std::vector<float>& defaultFrag,
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

        /// Compute the discrete filter size in sample widths
        ///
        /// The floating point filter radius implies a discrete filter size
        static void filterSize(float radius, int sampsPerPix, int& size,
                               int& offset)
        {
            // Separate cases for even & odd numbers of samples per pixel.
            if(sampsPerPix%2 == 0)
            {
                int discreteRadius = floor(radius*sampsPerPix + 0.5);
                size = 2*discreteRadius;
                offset = discreteRadius - sampsPerPix/2;
            }
            else
            {
                int discreteRadius = floor(radius*sampsPerPix);
                size = 2*discreteRadius + 1;
                offset = discreteRadius - sampsPerPix/2;
            }
        }

        static float pixelFilter(float x, float y, float xwidth, float ywidth)
        {
            // Use a gaussian filter for now...
            x /= xwidth;
            y /= ywidth;
            return std::exp(-8*(x*x + y*y));
        }

        // Cache filter coefficients
        //
        // \param offset - Offset between top left sample in pixel & top-left
        //                 sample in the filter region.
        // \param disWidth - Discrete filter width in number of supersamples
        static void cacheFilter(std::vector<float>& filter, const Options& opts,
                                Imath::V2i& offset, Imath::V2i& disWidth)
        {
            filterSize(opts.filterWidth.x/2, opts.superSamp.x, disWidth.x, offset.x);
            filterSize(opts.filterWidth.y/2, opts.superSamp.x, disWidth.y, offset.y);
            // Compute filter
            filter.resize(disWidth.x*disWidth.y);
            float* f = &filter[0];
            float totWeight = 0;
            for(int j = 0; j < disWidth.y; ++j)
            {
                float y = (j-(disWidth.y-1)/2.0f)/opts.superSamp.y;
                for(int i = 0; i < disWidth.x; ++i, ++f)
                {
                    float x = (i-(disWidth.x-1)/2.0f)/opts.superSamp.x;
                    float w = pixelFilter(x, y, opts.filterWidth.x,
                                                opts.filterWidth.y);
                    *f = w;
                    totWeight += w;
                }
            }
            // Normalize total weight to 1.
            float renorm = 1/totWeight;
            for(int i = 0, iend = disWidth.y*disWidth.x; i < iend; ++i)
                filter[i] *= renorm;
        }
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
            Imath::V2i bndMin = floor(vec2_cast(bound.min)
                    *storage.m_opts.superSamp) + storage.m_filtExpand;
            Imath::V2i bndMax = floor(vec2_cast(bound.max)
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
