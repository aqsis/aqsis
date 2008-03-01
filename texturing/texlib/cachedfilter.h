// Aqsis
// Copyright (C) 1997 - 2007, Paul C. Gregory
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

/** \file
 *
 * \brief Define several filter functors
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef CACHEDFILTER_H_INCLUDED
#define CACHEDFILTER_H_INCLUDED

#include "aqsis.h"

#include <iosfwd>
#include <vector>

#include "aqsismath.h"
#include "filtersupport.h"

namespace Aqsis
{

//------------------------------------------------------------------------------
/** \brief Cached filter weights for resampling an image.
 *
 * When resampling images for such purposes as mipmapping, the positions of
 * sample points are fixed relative to each output pixel.  This means it's
 * possible to cache the filter weights once and use them for each pixel.  This
 * is good, because many filter functions are rather costly to compute.
 */
class CqCachedFilter
{
	public:
		/** \brief Create a zeroed filter on a regular grid.
		 *
		 * The filter is cached on a regular lattice centered about (0,0).  If
		 * the filter contains an odd number of points in the x or y
		 * directions, the central filter value will be evaluated at (0,0).
		 * Otherwise the central filter values will straddle the origin.
		 *
		 * \param width - filter width in the source image
		 * \param height - filter height in the source image
		 * \param scale - Scale factor which will produce the new image
		 *                dimensions from the old ones.  For example, scale
		 *                should be 1/2 when downsampling by a factor of 2
		 *                during mipmapping.
		 */
		template<typename FilterFuncT>
		inline CqCachedFilter(const FilterFuncT& filter, TqFloat width, TqFloat height,
				bool evenNumberX, bool evenNumberY, TqFloat scale);

		/** \brief Get the cached filter weight.
		 *
		 * \param x - x-coordinate in the support [support().startX, support().endX-1]
		 * \param y - y-coordinate in the support [support().startY, support().endY-1]
		 */
		inline TqFloat operator()(TqInt x, TqInt y) const;

		/// Cached filters are normalized on construction; return true.
		inline static bool isNormalized() { return true; }

		/** \brief Get the number of points in the x-direction for the discrete
		 * filter kernel.
		 *
		 * Note that this is different from the floating point width provided
		 * to the constructor.
		 */
		inline TqInt width() const;
		/** \brief Get the number of points in the y-direction for the discrete
		 * filter kernel
		 *
		 * Note that this is different from the floating point height provided
		 * to the constructor.
		 */
		inline TqInt height() const;

		/** \brief Get the support for the filter in the source image.
		 *
		 * The support is the (rectangular) region over which the filter has
		 * nonzero coefficients.
		 */
		inline SqFilterSupport support() const;
		/// Set the top left point in the filter support
		inline void setSupportTopLeft(TqInt x, TqInt y);
	private:
		/** \brief Cache the given filter functor at the lattice points.
		 * \param filter - filter functor to cache.
		 */
		template<typename FilterFuncT>
		void cacheFilter(const FilterFuncT& filter, TqFloat scale);

		static TqInt filterSupportSize(bool includeZero, TqFloat width);
		TqInt m_width; ///< number of points in horizontal lattice directon
		TqInt m_height; ///< number of points in vertical lattice directon
		TqInt m_topLeftX; ///< top left x-position in filter support
		TqInt m_topLeftY; ///< top left y-position in filter support
		std::vector<TqFloat> m_weights; ///< cached weights.
};

/// Stream insertion operator for printing a filter kernel
std::ostream& operator<<(std::ostream& out, const CqCachedFilter& filter);



//==============================================================================
// Implementation details
//==============================================================================
// CqCachedFilter

template<typename FilterFuncT>
inline CqCachedFilter::CqCachedFilter(const FilterFuncT& filter,
		TqFloat width, TqFloat height, bool includeZeroX, bool includeZeroY,
		TqFloat scale)
	: m_width(filterSupportSize(includeZeroX, width)),
	m_height(filterSupportSize(includeZeroY, height)),
	m_topLeftX(0),
	m_topLeftY(0),
	m_weights(m_width*m_height, 0)
{
	cacheFilter(filter, scale);
}

inline TqFloat CqCachedFilter::operator()(TqInt x, TqInt y) const
{
	return m_weights[(y-m_topLeftY)*m_width + (x-m_topLeftX)];
}

inline TqInt CqCachedFilter::width() const
{
	return m_width;
}

inline TqInt CqCachedFilter::height() const
{
	return m_height;
}

inline SqFilterSupport CqCachedFilter::support() const
{
	return SqFilterSupport(m_topLeftX, m_topLeftX+m_width,
			m_topLeftY, m_topLeftY+m_height);
}

inline void CqCachedFilter::setSupportTopLeft(TqInt x, TqInt y)
{
	m_topLeftX = x;
	m_topLeftY = y;
}

inline TqInt CqCachedFilter::filterSupportSize(bool includeZero,
		TqFloat width)
{
	// Get the size of the lattice in the source buffer over which the filter
	// needs to be evaluated.  For even-sized images, our downsampled points
	// will lie *between* samples of the source, so we want to straddle the
	// zero point of the filter rather than include the zero point.  This leads
	// to the two different cases.

	// Since we're centered around 0, if we include zero as part of the
	// support, this gives an odd-sized filter kernel.  If we straddle zero, we
	// have an even-sized filter kernel.
	if(includeZero)
		return max(2*static_cast<TqInt>(0.5*width) + 1, 3);
	else
		return max(2*static_cast<TqInt>(0.5*(width+1)), 2);
}

template<typename FilterFuncT>
void CqCachedFilter::cacheFilter(const FilterFuncT& filter, TqFloat scale)
{
	// Compute and cache the desired filter weights on a regular grid.
	TqFloat sOffset = (m_width-1)/2.0f;
	TqFloat tOffset = (m_height-1)/2.0f;
	TqFloat totWeight = 0;
	for(TqInt j = 0; j < m_height; ++j)
	{
		TqFloat t = (j - tOffset)*scale;
		for(TqInt i = 0; i < m_width; ++i)
		{
			TqFloat s = (i - sOffset)*scale;
			TqFloat weight = filter(s, t);
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


} // namespace Aqsis

#endif // CACHEDFILTER_H_INCLUDED
