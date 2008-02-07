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
 * \brief Sampling quadrilateral struct definition
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef SAMPLEQUAD_H_INCLUDED
#define SAMPLEQUAD_H_INCLUDED

#include <cmath>

#include "vector2d.h"
#include "vector3d.h"
#include "aqsismath.h"

namespace Aqsis {

/** \brief 2D quadrilateral over which to sample a texture
 *
 * The vertices of the quad have the ordering such that v4 corresponds to the
 * diagonally opposite corner of the quad from v1.  This is the same way that
 * texture coordinates are interpreted in the RiTextureCoordinates() interface
 * call.  As a picture:
 *
 * \verbatim
 *
 *   v1---v2
 *   |     |
 *   |     |
 *   v3---v4
 *
 * \endverbatim
 */
template<typename VecT>
struct SqSampleQuad
{
	VecT v1;
	VecT v2;
	VecT v3;
	VecT v4;

	/// Trivial constructor
	SqSampleQuad(const VecT& v1, const VecT& v2, const VecT& v3, const VecT& v4);

	/// Get the center point of the quadrilateral by averaging the vertices.
	VecT center() const;
};

/// Sample quad for indexing textures with 2D coordinates (for plain texture maps)
typedef SqSampleQuad<CqVector2D> Tq2DSampleQuad;
/// Sample quad for indexing textures with 3D coordinates (for shadows & env maps)
typedef SqSampleQuad<CqVector3D> Tq3DSampleQuad;

/** Remap a sample quad by translation to lie in the box [0,1] x [0,1]
 *
 * Specifically, after remapping,
 *   (s, t) = (min(s1,s2,s3,s4), min(t1,t2,t3,t4))
 * is guarenteed to lie in the box [0,1]x[0,1]
 */
void remapPeriodic(Tq2DSampleQuad& quad, bool xPeriodic, bool yPeriodic);

//==============================================================================
// Implementation details
//==============================================================================

// SqSampleQuad implementation
template<typename VecT>
inline SqSampleQuad<VecT>::SqSampleQuad(const VecT& v1, const VecT& v2,
		const VecT& v3, const VecT& v4)
	: v1(v1),
	v2(v2),
	v3(v3),
	v4(v4)
{ }

template<typename VecT>
inline VecT SqSampleQuad<VecT>::center() const
{
	return 0.25*(v1+v2+v3+v4);
}


//------------------------------------------------------------------------------
// Implementation of functions
inline void remapPeriodic(Tq2DSampleQuad& quad, bool xPeriodic, bool yPeriodic)
{
	if(xPeriodic || yPeriodic)
	{
		TqFloat x = xPeriodic
			? min(min(min(quad.v1.x(), quad.v2.x()), quad.v3.x()), quad.v4.x())
			: 0;
		TqFloat y = yPeriodic
			? min(min(min(quad.v1.y(), quad.v2.y()), quad.v3.y()), quad.v4.y())
			: 0;
		if(x < 0 || y < 0 || x >= 1 || y >= 1)
		{
			CqVector2D v(std::floor(x),std::floor(y));
			quad.v1 -= v;
			quad.v2 -= v;
			quad.v3 -= v;
			quad.v4 -= v;
		}
	}
}

} // namespace Aqsis

#endif // SAMPLEQUAD_H_INCLUDED
