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
class CqSincFilter
{
	public:
		/// Sinc is a seperable filter.
		static const bool isSeperable = true;

		inline CqSincFilter(TqFloat width, TqFloat height);

		inline TqFloat operator()(TqFloat x, TqFloat y);
	private:
		inline TqFloat windowedSinc(TqFloat x, invWidthOn2);
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

		inline TqFloat operator()(TqFloat x, TqFloat y);
	private:
		TqFloat m_invWidth;
		TqFloat m_invHeight;
};

//------------------------------------------------------------------------------
/** \brief A Cache for filter weights on a regular grid.
 *
 * When resampling images for such purposes as mipmapping, the positions of
 * sample points are fixed relative to each output pixel.  This means it's
 * possible to cache the filter weights once and use them for each pixel.  This
 * is good, because many filter functions are rather costly to compute.
 */
class CqCachedFilter
{
	public:
		/** \brief Compute and cache the given filter on a regular grid.
		 *
		 * In the x-direction, the filter is cached on a regular lattice with
		 * width+1 points, step size "stepSize" and centered about 0.  That is,
		 *
		 * [-stepSize*width/2.0, -stepSize*(width-1)/2.0, ..., stepSize*width/2.0]
		 *
		 * The lattice in the y-direction is similar.
		 *
		 * \param filter - filter functor to cache.
		 * \param width - width (number of points) of cached grid
		 * \param height - height (number of points) of cached grid
		 * \param stepSize - Step size for the cached lattice.  For
		 *                   downsampling by a factor of 2 in mipmapping, this
		 *                   should be 1/2 for example.
		 */
		template<typename FilterFuncT>
		CqCachedFilter(const FilterFuncT& filter, TqInt width, TqInt height,
				TqInt stepSize);
		/** \brief Get the cached filter weight.
		 *
		 * \param x - x-coordinate in the range [0, width+1]
		 * \param y - y-coordinate in the range [0, height+1]
		 */
		inline TqFloat operator()(TqInt x, TqInt y);
	private:
		TqInt m_width; ///< number of points in horizontal lattice directon
		TqInt m_height; ///< number of points in vertical lattice directon
		std::vector<TqFloat> m_weights; ///< cached weights.
};


//==============================================================================
// Implementation details
//==============================================================================

//--------------------------------------------------
// CqSincFilter implementation
inline CqSincFilter::CqSincFilter(TqFloat width, TqFloat height)
	: m_invWidthOn2(0.5/width),
	m_invHeightOn2(0.5/height)
{ }

inline TqFloat CqSincFilter::operator()(TqFloat x, TqFloat y)
{
	return windowedSinc(x, m_invWidthOn2) * windowedSinc(y, m_invHeightOn2);
}

inline TqFloat CqSincFilter::windowedSinc(TqFloat x, invWidthOn2)
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

//--------------------------------------------------
// CqGaussianFilter
inline CqGaussianFilter::CqGaussianFilter(TqFloat width, TqFloat height)
	: m_invWidth(1/width),
	m_invHeight(1/height)
{ }

inline TqFloat CqGaussianFilter::operator()(TqFloat x, TqFloat y)
{
	x *= m_invWidth;
	y *= m_invHeigth;
	return exp(-8*(x*x + y*y));
}

//--------------------------------------------------
// CqCachedFilter

template<typename FilterFuncT>
CqCachedFilter::CqCachedFilter(const FilterFuncT& filter, TqInt width,
		TqInt height, TqInt stepSize);
	: m_width(width+1), // Need width+1 to include *both* endpoints in the lattice
	                    // from -width/2 to width/2
	m_height(height+1),
	m_weights(m_width*m_height, 0)
{
	// Compute and cache the desired filter weights on a regular grid.
	TqFloat sOffset = width/2.0f;
	TqFloat tOffset = height/2.0f;
	TqFloat totWeight = 0;
	for(TqInt j = 0; j < m_height; ++j)
	{
		TqFloat t = (j - tOffset)*stepSize;
		for(TqInt i = 0; i < m_width; ++i)
		{
			TqFloat s = (i - sOffset)*stepSize;
			TqFloat w = filter(s, t);
			m_weights[j*width + i] = w;
			totWeight += w;
		}
	}
	// Normalize so that the total weight is 1.
	for(std::vector<TqFloat>::iterator i = m_weight.begin(), e = m_weights.end();
			i != e; ++i)
		*i /= totWeight;
}

inline TqFloat CqCachedFilter::operator()(TqInt x, TqInt y)
{
	return m_weights[y*m_width + x];
}

} // namespace Aqsis

#endif // FILTERS_H_INCLUDED
