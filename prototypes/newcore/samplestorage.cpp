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
#include "arrayview.h"
#include "util.h"


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

