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
 * \brief Declare some commonly used math functions designed for numeric types.
 * \author Chris Foster
 */

#ifndef AQSISMATH_H_INCLUDED
#define AQSISMATH_H_INCLUDED

#include <aqsis/aqsis.h>
#include <cmath>
#include <limits>

#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/arithmetic_traits.hpp>

namespace Aqsis {
// These inline functions are intended to eventually replace all the old macros
// which reside in aqsis_types.h

/** \brief Determine the largest integer smaller than the given floating point value
 *
 * \note If you really want a floating point value in the end, it's better to
 * use std::floor from <cmath>.  This avoids overflow issues with the maximum
 * TqLong value.
 */
template<typename T>
inline TqLong lfloor(const T x)
{
	return static_cast<TqLong>(x) - (x < 0 && x != static_cast<TqLong>(x));
}

/** \brief Return the smallest integer larger than the given floating point value
 *
 * \note If you really want a floating point value in the end, it's better to
 * use std::ceil from <cmath>.  This avoids overflow issues with the maximum
 * TqLong value.
 */
template<typename T>
inline TqLong lceil(const T x)
{
	return static_cast<TqLong>(x) + (x > 0 && x != static_cast<TqLong>(x));
}

/** \brief Return the nearest integer value to the given float.
 */
template<typename T>
inline TqLong lround(const T x)
{
	return lfloor(x - 0.5) + 1;
}

/** \brief Round the given floating point number to the nearest integer
 *
 * Something like function this is part of the C99 standard.
 */
template<typename T>
inline T round(const T x)
{
	return std::floor(x - 0.5) + 1;
}

/** \brief Linearly interpolate between two values
 *
 * \param t - interpolation parameter; a floating point between 0 and 1.
 * \param x0 - value corresponding to t = 0
 * \param x1 - value corresponding to t = 1
 */
inline TqFloat lerp(TqFloat t, const TqFloat x0, const TqFloat x1)
{
	return (1-t)*x0 + t*x1;
}

/** \brief clamp a value to between some min and max extents.
 *
 * \param x - value to clamp
 * \param min - minimum value for x
 * \param max - maximum value for x
 */
template<typename T>
inline typename boost::enable_if<boost::is_arithmetic<T>, T>::type
clamp(const T x, const T min, const T max)
{
	return x < min ? min : (x > max ? max : x);
}

/** \brief Determine the minimum of the two values given.
 *
 * Note that there are specific versions of min() for other types like CqColor.
 */
template<typename T>
inline typename boost::enable_if<boost::is_arithmetic<T>, T>::type
min(const T a, const T b)
{
	return (a < b) ? a : b;
}

/** \brief Determine the maximum of the two values given.
 *
 * Note that there are specific versions of max() for other types like CqColor.
 */
template<typename T>
inline typename boost::enable_if<boost::is_arithmetic<T>, T>::type
max(const T a, const T b)
{
	return (a < b) ? b : a;
}

/// Convert the given angle in degrees to radians.
inline TqFloat degToRad(const TqFloat a)
{
	return a/180.0*M_PI;
}

/// Convert the given angle in radians to degrees.
inline TqFloat radToDeg(const TqFloat a)
{
	return a/M_PI*180.0;
}

/** \brief Calculate the next highest power of two above the given value
 */
inline TqUint ceilPow2(const TqUint x)
{
	TqUint y = x - 1;
	for (TqUint i = 1; i; i <<= 1)
		y |= y >> i;
	// y is now the next highest power of two minus 1.
	return y + 1;
}

/** \brief Calculate the base-2 logarithm of a number.
 *
 * In principle we could use log2() from math.h here.  However it's part of the
 * C99 standard rather than C89, and won't be available on all platforms.
 */
inline TqFloat log2(TqFloat x)
{
	// log2(x) = log(x)/log(2) ~= 1.4426950408889633 * log(x)
	return 1.4426950408889633 * std::log(x);
}

/** \brief Determine whether two numbers are equal to within a tolerance.
 *
 * Compare two numbers for closeness - we want the difference to be less than
 * some desired fraction of the maximum of the two values.  By default the
 * tolerance is close to the smallest representable TqFloat.
 *
 * This function is overloaded for several other aqsis types (eg
 * vectors/colours) for ease of use in templates.
 *
 * \param x1, x2 - numbers to compare
 * \param tol - tolerance for the comparison.
 */
inline bool isClose(TqFloat x1, TqFloat x2,
		TqFloat tol = 10*std::numeric_limits<TqFloat>::epsilon())
{
	// The relative efficiency of using of std::fabs() here vs multiplication
	// appears to depend on architecture.  on amd64 multiplication seems to be
	// faster, while on pentium4 it seems better to use std::fabs()...
	TqFloat d = std::fabs(x1-x2);
	return d <= tol*std::fabs(x1) || d <= tol*std::fabs(x2);
}


/** \brief Return a number with only the lowest bit of the input set.
 *
 * Example:
 * Input:  001101000
 * Output: 000001000
 */
//inline TqUint lowestBit(const TqUint x)
//{
//	return x & ((~x) + 1);
//}

/// Return true if the number is a power of two.
//inline bool isPow2(const TqUint x)
//{
//	return x == lowestBit(x) && x != 0;
//}

/** \brief Calculate the next highest power of two minus 1.
 */
//inline TqUint ceilPow2Minus1(TqUint x)
//{
//	for (TqUint i = 1; i; i <<= 1)
//		x |= x >> i;
//	return x;
//}

} // namespace Aqsis

#endif // AQSISMATH_H_INCLUDED
