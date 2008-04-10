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

#include "aqsismath.h"

namespace Aqsis
{

//------------------------------------------------------------------------------
/** \brief Interface representing a filter function object.
 *
 * The renderman API defines a type "RtFilterFunc" which is used for filtering
 * images during downsampling, as well as geometry antialiasing etc.  These
 * functions can have several attributes such as separability which cannot be
 * stored together with the function pointer.  The IqFilterFunctor interface
 * ties all such attributes together, as well as allowing caching of
 * precomputed data to make evaluations of the filter more efficient.
 *
 * Filter functors are assumed to have their support centred on the origin.
 */
class IqFilterFunctor
{
	public:
		virtual ~IqFilterFunctor() {};

		/** \brief Determine whether the filter is separable or not.
		 *
		 * To be separable means that the filter function, f(x,y) can be
		 * written as the product of two other functions:  f(x,y) = h(x)*g(y)
		 * for some functions h() and g().  Convolution of an image with a
		 * 2D separable filters is equivilant to two successive 1D
		 * convolutions - a worthwhile optimization for large filter widths.
		 */
		virtual bool isSeparable() { return false; }

		/** \brief Evaluate the filter amplitude at the point (x,y)
		 *
		 * \param x
		 * \param y - (x,y) is the point to evaluate the filter amplitude at.
		 *
		 * \return 
		 */
		virtual TqFloat operator()(TqFloat x, TqFloat y) const = 0;
};

//------------------------------------------------------------------------------
/** \brief A filter functor to encapsulate traditional RI filter functions
 *
 * It is possible to pass arbitrary filter function handles through the
 * renderman interface using the C API.  This functor encapsulates such an
 * "RtFilterFunc" function pointer inside an interface conforming to the
 * FilterFunctorConcept.
 */
class CqFunctionFilter : public IqFilterFunctor
{
	public:
		/** \brief Wrap a filter function in a functor
		 *
		 * \param func - filter function to wrap
		 * \param width - width of filter support
		 * \param height - height of filter support
		 * \param separable - use true if the filter is separable in x and y.
		 */
		CqFunctionFilter(TqFloat (*func)(TqFloat, TqFloat, TqFloat, TqFloat),
				TqFloat width, TqFloat height, bool separable = false);

		virtual bool isSeparable();

		virtual TqFloat operator()(TqFloat x, TqFloat y) const;
	private:
		TqFloat (*m_func)(TqFloat, TqFloat, TqFloat, TqFloat);
		TqFloat m_width;
		TqFloat m_height;
		bool m_separable;
};

//------------------------------------------------------------------------------
/** \brief A box filter.
 */
class CqBoxFilter : public IqFilterFunctor
{
	public:
		CqBoxFilter(TqFloat width, TqFloat height);

		/// Box filters are separable
		virtual static bool isSeparable() { return true; }

		virtual TqFloat operator()(TqFloat x, TqFloat y) const;
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
class CqSincFilter : public IqFilterFunctor
{
	public:
		CqSincFilter(TqFloat width, TqFloat height);

		/// Sinc is a separable filter.
		virtual static bool isSeparable() { return true; }

		virtual TqFloat operator()(TqFloat x, TqFloat y) const;
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
class CqGaussianFilter : public IqFilterFunctor
{
	public:
		/// Construct a gaussian filter with the given width and height.
		CqGaussianFilter(TqFloat width, TqFloat height);

		/// Gaussians are separable filters.
		virtual static bool isSeparable() { return true; }

		virtual TqFloat operator()(TqFloat x, TqFloat y) const;
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
