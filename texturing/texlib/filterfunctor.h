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

#ifndef FILTERFUNCTOR_H_INCLUDED
#define FILTERFUNCTOR_H_INCLUDED

#include "aqsis.h"

#include <cmath>

#include "aqsismath.h"

namespace Aqsis
{

//------------------------------------------------------------------------------
/** \brief A functor to encapsulate traditional RI filter functions
 */

class CqFunctionFilter
{
	public:
		/** \brief Wrap a filter function in a functor
		 *
		 * \param func - filter function to wrap
		 * \param width - width of filter support
		 * \param height - height of filter support
		 * \param separable - use true if the filter is separable in x and y.
		 */
		inline CqFunctionFilter(TqFloat (*func)(TqFloat, TqFloat, TqFloat, TqFloat),
				TqFloat width, TqFloat height, bool separable = false);

		inline bool isSeparable();

		inline TqFloat operator()(TqFloat x, TqFloat y) const;
	private:
		TqFloat (*m_func)(TqFloat, TqFloat, TqFloat, TqFloat);
		TqFloat m_width;
		TqFloat m_height;
		bool m_separable;
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
class CqBoxFilter
{
	public:
		inline CqBoxFilter(TqFloat width, TqFloat height);

		/// Box filters are separable
		inline static bool isSeparable() { return true; }

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
		inline CqSincFilter(TqFloat width, TqFloat height);

		/// Sinc is a separable filter.
		inline static bool isSeparable() { return true; }

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
		/// Construct a gaussian filter with the given width and height.
		inline CqGaussianFilter(TqFloat width, TqFloat height);

		/// Gaussians are separable filters.
		inline static bool isSeparable() { return true; }

		inline TqFloat operator()(TqFloat x, TqFloat y) const;
	private:
		TqFloat m_invWidth;
		TqFloat m_invHeight;
};


//==============================================================================
// Implementation details
//==============================================================================

// CqFunctionFilter implementation
inline CqFunctionFilter::CqFunctionFilter(TqFloat (*func)(TqFloat, TqFloat, TqFloat, TqFloat),
		TqFloat width, TqFloat height, bool separable)
	: m_func(func),
	m_width(width),
	m_height(height),
	m_separable(separable)
{ }

inline bool CqFunctionFilter::isSeparable()
{
	return m_separable;
}

inline TqFloat CqFunctionFilter::operator()(TqFloat x, TqFloat y) const
{
	return m_func(x, y, m_width, m_height);
}


//------------------------------------------------------------------------------
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

} // namespace Aqsis

#endif // FILTERFUNCTOR_H_INCLUDED
