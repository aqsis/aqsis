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

/** \file
 *
 * \brief Implementation for cached filter functor
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#include "cachedfilter.h"

#include <iostream>

namespace Aqsis
{

namespace {

/** Get the size of the lattice in the source buffer over which the filter
 * needs to be evaluated.  For even-sized images, our downsampled points
 * will lie *between* samples of the source, so we want to straddle the zero
 * point of the filter rather than include the zero point.  This leads to the
 * two different cases.
 *
 * Since we're centered around 0, if we include zero as part of the support,
 * this gives an odd-sized filter kernel.  If we straddle zero, we have an
 * even-sized filter kernel.
 */
TqInt filterSupportSize(bool includeZero, TqFloat width)
{
	if(includeZero)
		return max(2*static_cast<TqInt>(0.5*width) + 1, 3);
	else
		return max(2*static_cast<TqInt>(0.5*(width+1)), 2);
}

} // unnamed namespace


CqCachedFilter::CqCachedFilter(const SqFilterInfo& filterInfo,
		bool includeZeroX, bool includeZeroY,
		TqFloat scale)
	: m_width(filterSupportSize(includeZeroX, filterInfo.xWidth)),
	m_height(filterSupportSize(includeZeroY, filterInfo.yWidth)),
	m_topLeftX(0),
	m_topLeftY(0),
	m_weights(m_width*m_height, 0)
{
	// Compute and cache the desired filter weights on a regular grid.
	const TqFloat sOffset = (m_width-1)/2.0f;
	const TqFloat tOffset = (m_height-1)/2.0f;
	const TqFloat sWidth = filterInfo.xWidth*scale;
	const TqFloat tWidth = filterInfo.yWidth*scale;
	TqFloat totWeight = 0;
	for(TqInt j = 0; j < m_height; ++j)
	{
		TqFloat t = (j - tOffset)*scale;
		for(TqInt i = 0; i < m_width; ++i)
		{
			TqFloat s = (i - sOffset)*scale;
			TqFloat weight = filterInfo.filterFunc(s, t, sWidth, tWidth);
			m_weights[j*m_width + i] = weight;
			totWeight += weight;
		}
	}
	// Optimize filter weights
	for(std::vector<TqFloat>::iterator i = m_weights.begin(), e = m_weights.end();
			i != e; ++i)
	{
		// Normalize so that the total weight is 1.
		TqFloat weight = *i/totWeight;
		// If the weight is very small, set it to zero; this makes applying
		// the filter more efficient when zero weights are explicitly skipped
		// (see the applyFilter() function used in sampling a texture buffer).
		if(std::fabs(weight) < 1e-5)
			weight = 0;
		*i = weight;
	}
}

std::ostream& operator<<(std::ostream& out, const CqCachedFilter& filter)
{
	// print the filter kernel.
	for(TqInt j = 0; j < filter.height(); j++)
	{
		out << "[";
		for(TqInt i = 0; i < filter.width(); i++)
		{
			out << filter(i,j) << ", "; 
		}
		out << "]\n";
	}
	return out;
}

} // namespace Aqsis
