// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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
		\author Paul C. Gregory (pgregory@aqsis.com)
*/


#ifndef	AQSIS_TYPES_INCLUDED
#define	AQSIS_TYPES_INCLUDED

typedef	char			TqChar;
typedef	unsigned char	TqUchar;
typedef	char*			TqPchar;
typedef	unsigned char*	TqPuchar;

typedef	int				TqInt;
typedef	unsigned int	TqUint;
typedef	long			TqLong;
typedef	unsigned long	TqUlong;

typedef	short			TqShort;
typedef	unsigned short	TqUshort;

typedef	float			TqFloat;
typedef	double			TqDouble;

typedef	bool			TqBool;
#define	TqTrue			true
#define	TqFalse			false

#define FLOOR(x) ((int)(x) - ((x) < 0 && (x) != (int)(x)))
#define CEIL(x) ((int)(x) + ((x) > 0 && (x) != (int)(x)))

#define LERP(t,x0,x1)  ((x0) + (t)*((x1)-(x0)))

#define	CLAMP(a,min,max)	((a)<(min)?(min):((a)>(max)?(max):(a)))
#define	MIN(a,b)			(((a)<(b))?(a):(b))
#define	MAX(a,b)			(((a)<(b))?(b):(a))
#define RAD(a)				((a)/(180.0f/RI_PI))
#define DEG(a)				(((a)/180.0f)*RI_PI)

#endif	// AQSIS_TYPES_INCLUDED
