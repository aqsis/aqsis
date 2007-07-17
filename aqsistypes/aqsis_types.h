// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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
		\brief Declares typedefs for the basic types.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/


#ifndef	AQSIS_TYPES_INCLUDED
#define	AQSIS_TYPES_INCLUDED

typedef	char	TqChar;
typedef	unsigned char	TqUchar;
typedef	char*	TqPchar;
typedef	unsigned char*	TqPuchar;

typedef	int	TqInt;
typedef	unsigned int	TqUint;
typedef	long	TqLong;
typedef	unsigned long	TqUlong;

typedef	short	TqShort;
typedef	unsigned short	TqUshort;

typedef	float	TqFloat;
typedef	double	TqDouble;

/// Determine the largest integer value smaller than the given float.
#define FLOOR(x) ((int)(x) - ((x) < 0 && (x) != (int)(x)))
/// Determine the smallest integer value larger than the given float.
#define CEIL(x) ((int)(x) + ((x) > 0 && (x) != (int)(x)))
/// Determine the nearest integer value to the given float.
#define	ROUND(x) (((x) - FLOOR((x))) < 0.5f)?FLOOR((x)):CEIL((x))


/// Linearly interpolate between the two given values to the point t, 0>=t<=1.
#define LERP(t,x0,x1)  ((1.0-t)*(x0) + (t*x1))

/// Clamp the given value a, to be with the extents of the given range.
/// \deprecated use the template aqsis::clamp from aqsismath.h in future.
#define	CLAMP(a,min,max)	((a)<(min)?(min):((a)>(max)?(max):(a)))
/// Determine the minimum of the two values given.
#define	MIN(a,b)			(((a)<(b))?(a):(b))
/// Determine the maximum of the two values given.
#define	MAX(a,b)			(((a)<(b))?(b):(a))

/// Defines an approximation to PI.
#define	PI			3.14159265359f
/// Defines an approximation to PI divided by 2.
#define	PIO2		PI/2

/// Convert the given degrees value to radians.
#define RAD(a)				((a)/(180.0f/PI))
/// Convert the given radians value to degrees.
#define DEG(a)				((a)/(PI/180.0f))

#endif	// AQSIS_TYPES_INCLUDED
