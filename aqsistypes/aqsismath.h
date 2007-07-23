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

#include "aqsis.h"

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

/** \brief Linearly interpolate between two values
 *
 * Note: It may be worth checking the efficiency of this template.
 *
 * \param t - interpolation parameter; a floating point between 0 and 1.
 * \param x0 - value corresponding to t = 0
 * \param x1 - value corresponding to t = 1
 */
template<typename T, typename V>
inline V lerp(const T t, const V x0, const V x1)
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
inline T clamp(const T x, const T min, const T max)
{
	return x < min ? min : (x > max ? max : x);
}

/// Determine the minimum of the two values given.
template<typename T>
inline T min(const T a, const T b)
{
	return (a < b) ? a : b;
}

/// Determine the maximum of the two values given.
template<typename T>
inline T max(const T a, const T b)
{
	return (a < b) ? b : a;
}

// There were originally min, max and clamp macros for colors and vectors -
// corresponding inline function have been moved to color.h and vector.h as
// more appropriate places.

/// Convert the given angle in degrees to radians.
template<typename T>
inline T rad(const T a)
{
	return a/180.0f*PI;
}

/// Convert the given angle in radians to degrees.
template<typename T>
inline T deg(const T a)
{
	return a/PI*180.0f;
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

} // namespace Aqsis

#endif // AQSISMATH_H_INCLUDED
