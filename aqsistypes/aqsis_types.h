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

typedef	bool	TqBool;
/// Defines the 'true' value used in TqBool operations.
#define	TqTrue			true
/// Defines the 'true' value used in TqBool operations.
#define	TqFalse			false

/// Determine the largest integer value smaller than the given float.
#define FLOOR(x) ((int)(x) - ((x) < 0 && (x) != (int)(x)))
/// Determine the smallest integer value larger than the given float.
#define CEIL(x) ((int)(x) + ((x) > 0 && (x) != (int)(x)))
/// Determine the nearest integer value to the given float.
#define	ROUND(x) (((x) - FLOOR((x))) < 0.5f)?FLOOR((x)):CEIL((x))


/// Linearly interpolate between the two given values to the point t, 0>=t<=1.
#define LERP(t,x0,x1)  ((1.0-t)*(x0) + (t*x1))

/// Clamp the given value a, to be with the extents of the given range.
/// \deprecated use the template aqsis::clamp in future.
#define	CLAMP(a,min,max)	((a)<(min)?(min):((a)>(max)?(max):(a)))
/// Determine the minimum of the two values given.
#define	MIN(a,b)			(((a)<(b))?(a):(b))
/// Determine the maximum of the two values given.
#define	MAX(a,b)			(((a)<(b))?(b):(a))

/// Determine the minimum of the two colors given.
#define	CMIN(a,b)			( CqColor( MIN((a).fRed(), (b).fRed()), MIN((a).fGreen(), (b).fGreen()), MIN((a).fBlue(), (b).fBlue()) ) )
/// Determine the maximum of the two colors given.
#define	CMAX(a,b)			( CqColor( MAX((a).fRed(), (b).fRed()), MAX((a).fGreen(), (b).fGreen()), MAX((a).fBlue(), (b).fBlue()) ) )
/// Clamp the given color a, to be with the extents of the given range.
#define	CCLAMP(a,min,max)	( CqColor( CLAMP((a).fRed(), (min).fRed(), (max).fRed()), CLAMP((a).fGreen(), (min).fGreen(), (max).fGreen()), CLAMP((a).fBlue(), (min).fBlue(), (max).fBlue()) ) )

/// Determine the minimum of the two vectors given.
#define	VMIN(a,b)			( CqVector3D( MIN((a).x(), (b).x()), MIN((a).y(), (b).y()), MIN((a).z(), (b).z()) ) )
/// Determine the maximum of the two vectors given.
#define	VMAX(a,b)			( CqVector3D( MAX((a).x(), (b).x()), MAX((a).y(), (b).y()), MAX((a).z(), (b).z()) ) )
/// Clamp the given vector a, to be with the extents of the given range.
#define	VCLAMP(a,min,max)	( CqVector3D( CLAMP( (a).x(), (min).x(), (max).x() ), CLAMP( (a).y(), (min).y(), (max).y() ), CLAMP( (a).z(), (min).z(), (max).z() ) ) )

/// Defines an approximation to PI.
#define	PI			3.14159265359f
/// Defines an approximation to PI divided by 2.
#define	PIO2		PI/2

/// Convert the given degrees value to radians.
#define RAD(a)				((a)/(180.0f/PI))
/// Convert the given radians value to degrees.
#define DEG(a)				((a)/(PI/180.0f))

/// Determine the lowest set bit in an unsigned value.
inline TqUint LOWEST_BIT( TqUint x )
{
	return ( x & ( ( ~x ) + 1 ) );
}

/// Determine if the given value is a power of two.
inline TqBool IS_POW2( TqUint x )
{
	return ( x != 0 && x == LOWEST_BIT( x ) );
}

/// Calculate the next highest power of two minus 1 from the given value.
inline TqUint CEIL_POW2_MINUS1( TqUint x )
{
	for ( TqUint i = 1; i; i <<= 1 )
		x |= x >> i;
	return ( x );
}

/// Calculate the next highest power of two from the given value.
inline TqUint CEIL_POW2( TqUint x )
{
	return ( CEIL_POW2_MINUS1( x - 1 ) + 1 );
}


namespace Aqsis {
// Eventually most of the macros above should probably be deprecated in favour
// of templated inline functions, to go below.

/// \brief clamp a value to within some min and max extents.
template<typename T> inline T clamp(T a, T min, T max)
{
	return a < min ? min : (a > max ? max : a);
}

} // namespace Aqsis



#endif	// AQSIS_TYPES_INCLUDED
