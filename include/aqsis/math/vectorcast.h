// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
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
 * \brief Conversion functions between vector types.
 *
 * We cannot easily have constructors for mutual type conversion since this
 * would introduce interdependencies between the various vector header files.
 * Usually the dependencies can be worked around by forward declaration, but
 * not when some of the vector types are templates which need to be implemented
 * in the header file (ugh).
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef VECTORCAST_H_INCLUDED
#define VECTORCAST_H_INCLUDED

#include <aqsis/math/color.h>
#include <aqsis/math/vector2d.h>
#include <aqsis/math/vector3d.h>
#include <aqsis/math/vector4d.h>

namespace Aqsis {


//@{
/** \brief Convert between vector and vector-like types.
 *
 * Conversion from a smaller vector to a larger one causes the smaller vector
 * components to be duplicated across the first components of the larger, with
 * the remaining components filled with zeros.
 *
 * Conversion from a larger to a smaller vector causes the last components of
 * the larger vector to be truncated.
 *
 * Homogenous vectors are treated as vectors with dimension one less, and last
 * component 1.0.
 *
 * Colors are treated as 3-vectors with the component correspondences x=r, y=g, z=b.
 */
template<typename T> T vectorCast(const CqVector2D& v);

template<typename T> T vectorCast(const CqVector3D& v);

template<typename T> T vectorCast(const CqVector4D& v);

template<typename T> T vectorCast(const CqColor& c);
//@}


//==============================================================================
// Implementation details
//==============================================================================

//----------------------------------------
// casting from CqVector2D
template<typename T>
inline T vectorCast(const CqVector2D& v)
{
	return T(v);
}

template<>
inline CqVector3D vectorCast(const CqVector2D& v)
{
	return CqVector3D(v.x(), v.y(), 0);
}


//----------------------------------------
// casting from CqVector3D
template<typename T>
inline T vectorCast(const CqVector3D& v)
{
	return T(v);
}

template<>
inline CqColor vectorCast(const CqVector3D& v)
{
	return CqColor(v.x(), v.y(), v.z());
}

template<>
inline CqVector2D vectorCast(const CqVector3D& v)
{
	return CqVector2D(v.x(), v.y());
}

template<>
inline CqVector4D vectorCast(const CqVector3D& v)
{
	return CqVector4D(v.x(), v.y(), v.z());
}


//----------------------------------------
// casting from CqVector4D
template<typename T>
inline T vectorCast(const CqVector4D& v)
{
	return T(v);
}

template<>
inline CqVector3D vectorCast(const CqVector4D& v)
{
	if(v.h() != 1)
	{
		TqFloat hInv = 1/v.h();
		return CqVector3D(hInv*v.x(), hInv*v.y(), hInv*v.z());
	}
	else
	{
		return CqVector3D(v.x(), v.y(), v.z());
	}
}

template<>
inline CqVector2D vectorCast(const CqVector4D& v)
{
	if(v.h() != 1)
	{
		TqFloat hInv = 1/v.h();
		return CqVector2D(hInv*v.x(), hInv*v.y());
	}
	else
	{
		return CqVector2D(v.x(), v.y());
	}
}

//----------------------------------------
// Casting from CqColor
template<typename T>
inline T vectorCast(const CqColor& c)
{
	return T(c);
}

template<>
inline CqVector3D vectorCast(const CqColor& c)
{
	return CqVector3D(c.r(), c.g(), c.b());
}


} // namespace Aqsis

#endif // VECTORCAST_H_INCLUDED
