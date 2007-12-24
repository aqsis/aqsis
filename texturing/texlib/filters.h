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

#ifndef FILTERS_H_INCLUDED
#define FILTERS_H_INCLUDED

#include "aqsis.h"

#include <cmath>
#include <vector>
#include <iosfwd>

#include "filtersupport.h"
#include "aqsismath.h"

namespace Aqsis
{

//------------------------------------------------------------------------------
/** \brief A sinc filter functional.
 *
 * The width/height of this filter determine only the behaviour of the
 * windowing function (which is chosen to be the centeral lobe of a cosine).
 *
 * The wavelength of oscillations for the sinc function is taken to be 2.0
 * (meaning that the distance between consecutive zeros of the function is
 * 1.0).  This choice of frequency has the consequence that for large width and
 * height the filter represents an ideal lowpass filter, which keeps only
 * frequencies representable on a regular grid with spacing 1.
 */
class CqBoxFilter
{
	public:
		/// Box is a seperable filter.
		static const bool isSeperable = true;

		inline CqBoxFilter(TqFloat width, TqFloat height);

		inline TqFloat operator()(TqFloat x, TqFloat y) const;
	private:
		TqFloat m_widthOn2;
		TqFloat m_heightOn2;
};

//------------------------------------------------------------------------------
/** \brief A sinc filter functional.
 *
 * The width/height of this filter determine only the behaviour of the
 * windowing function (which is chosen to be the centeral lobe of a cosine).
 *
 * The wavelength of oscillations for the sinc function is taken to be 2.0
 * (meaning that the distance between consecutive zeros of the function is
 * 1.0).  This choice of frequency has the consequence that for large width and
 * height the filter represents an ideal lowpass filter, which keeps only
 * frequencies representable on a regular grid with spacing 1.
 */
class CqSincFilter
{
	public:
		/// Sinc is a seperable filter.
		static const bool isSeperable = true;

		inline CqSincFilter(TqFloat width, TqFloat height);

		inline TqFloat operator()(TqFloat x, TqFloat y) const;
	private:
		inline TqFloat windowedSinc(TqFloat x, TqFloat invWidthOn2) const;
		TqFloat m_invWidthOn2;
		TqFloat m_invHeightOn2;
};

//------------------------------------------------------------------------------
/** \brief Gaussian filter functor
 *
 * A tight gaussian filter, falling to exp(-8) ~= 0.0003 at the given width or
 * height.  This is the same as given in renderman specification, though it
 * seems very small.
 */
class CqGaussianFilter
{
	public:
		/// Gaussians are seperable filters.
		static const bool isSeperable = true;

		/// Construct a gaussian filter with the given width and height.
		inline CqGaussianFilter(TqFloat width, TqFloat height);

		inline TqFloat operator()(TqFloat x, TqFloat y) const;
	private:
		TqFloat m_invWidth;
		TqFloat m_invHeight;
};

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
		 * In the x-direction, the filter is cached on a regular lattice
		 * \todo finish docs
		 *
		 * \param width - filter width in the source image
		 * \param height - filter height in the source image
		 * \param scale - Scale factor which will produce the new image
		 *                dimensions from the old ones.  For example, scale
		 *                should be 1/2 when downsampling by a factor of 2
		 *                during mipmapping.
		 */
		template<typename FilterFuncT>
		CqCachedFilter(const FilterFuncT& filter, TqFloat width, TqFloat height,
				bool evenNumberX, bool evenNumberY, TqFloat scale);

		/** \brief Get the cached filter weight.
		 *
		 * \param x - x-coordinate in the support [support().startX, support().endX-1]
		 * \param y - y-coordinate in the support [support().startY, support().endY-1]
		 */
		inline TqFloat operator()(TqInt x, TqInt y) const;

		/** \brief Get the number of points in the x-direction for the discrete
		 * filter kernel.
		 *
		 * Note that this is different from the floating point width provided
		 * to the constructor.
		 */
		TqInt width() const;
		/** \brief Get the number of points in the y-direction for the discrete
		 * filter kernel
		 *
		 * Note that this is different from the floating point height provided
		 * to the constructor.
		 */
		TqInt height() const;

		/** \brief Get the support for the filter in the source image.
		 *
		 * The support is the (rectangular) region over which the filter has
		 * nonzero coefficients.
		 */
		SqFilterSupport support() const;
		/// \brief Set the top left point in the filter support
		void setSupportTopLeft(TqInt x, TqInt y);
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

// CqBoxFilter implementation
inline CqBoxFilter::CqBoxFilter(TqFloat width, TqFloat height)
	: m_widthOn2(width/2), m_heightOn2(height/2)
{ }

inline TqFloat CqBoxFilter::operator()(TqFloat x, TqFloat y) const
{
	if(std::fabs(x) <= m_widthOn2 && std::fabs(y) <= m_heightOn2)
		return 1;
	return 0;
}

//------------------------------------------------------------------------------
// CqSincFilter implementation
inline CqSincFilter::CqSincFilter(TqFloat width, TqFloat height)
	: m_invWidthOn2(0.5/width),
	m_invHeightOn2(0.5/height)
{ }

inline TqFloat CqSincFilter::operator()(TqFloat x, TqFloat y) const
{
	return windowedSinc(x, m_invWidthOn2) * windowedSinc(y, m_invHeightOn2);
}

inline TqFloat CqSincFilter::windowedSinc(TqFloat x, TqFloat invWidthOn2) const
{
	x *= M_PI;
	if(x != 0)
	{
		// Use a -PI to PI cosine window.
		/// \todo: Investigate if this is really best for mipmap downsampling.
		return cos(x*invWidthOn2) * sin(x)/x;
	}
	else
		return 1.0;
}

//------------------------------------------------------------------------------
// CqGaussianFilter
inline CqGaussianFilter::CqGaussianFilter(TqFloat width, TqFloat height)
	: m_invWidth(1/width),
	m_invHeight(1/height)
{ }

inline TqFloat CqGaussianFilter::operator()(TqFloat x, TqFloat y) const
{
	x *= m_invWidth;
	y *= m_invHeight;
	return exp(-8*(x*x + y*y));
}

//------------------------------------------------------------------------------
// CqCachedFilter

template<typename FilterFuncT>
CqCachedFilter::CqCachedFilter(const FilterFuncT& filter,
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

TqInt CqCachedFilter::width() const
{
	return m_width;
}

TqInt CqCachedFilter::height() const
{
	return m_height;
}

inline SqFilterSupport CqCachedFilter::support() const
{
	return SqFilterSupport(m_topLeftX, m_topLeftY,
			m_topLeftX+m_width, m_topLeftY+m_height);
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
		// (see CqTextureBuffer<T>::applyFilter, for eg).
		if(std::fabs(weight) < 1e-5)
			weight = 0;
		*i = weight;
	}
}


} // namespace Aqsis

#endif // FILTERS_H_INCLUDED
