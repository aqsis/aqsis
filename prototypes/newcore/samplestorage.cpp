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
    m_filter()
{
    // Initialize cached filter coefficients
    cacheFilter(m_filter, opts, m_filtExpand, m_disWidth);

    m_xSampRes = opts.xRes*opts.superSamp.x + 2*m_filtExpand.x;
    m_ySampRes = opts.yRes*opts.superSamp.y + 2*m_filtExpand.y;

    // Initialize default fragment
    fillDefault(m_defaultFragment, outVars);
    m_fragSize = m_defaultFragment.size();

    m_doFilter = opts.doFilter && m_disWidth != Imath::V2i(1,1);
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
    const int xSuperSamp = m_opts.superSamp.x;
    const int ySuperSamp = m_opts.superSamp.y;
    if(m_doFilter)
    {
        // Execute filter.
        const int rowStride = m_fragSize*(m_xSampRes - m_disWidth.x);
        for(int col = 0; col < m_opts.xRes; ++col)
        {
            // Zero channels of the current output
            float* out = &m_filterResults[col*m_fragSize];
            std::memset(out, 0, m_fragSize*sizeof(float));
            // Iterate over samples in the current filter region.
            // Non-seperable filter only for now.
            const float* src
                = &m_fragments[ (m_xSampRes*line*ySuperSamp
                                    + col*xSuperSamp) * m_fragSize ];
            const float* filt = &m_filter[0];
            for(int k = 0; k < m_disWidth.y; ++k)
            {
                for(int j = 0; j < m_disWidth.x; ++j, ++filt)
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
    else
    {
        return &m_fragments[0] + line*m_xSampRes*m_fragSize;
    }
}


Imath::V2i SampleStorage::outputSize()
{
    if(m_doFilter)
        return Imath::V2i(m_opts.xRes, m_opts.yRes);
    else
        return Imath::V2i(m_xSampRes, m_ySampRes);
}
