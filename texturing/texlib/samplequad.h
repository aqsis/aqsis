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

class CqMatrix;
struct Sq3DSampleQuad;

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
struct SqSampleQuad
{
	CqVector2D v1;
	CqVector2D v2;
	CqVector2D v3;
	CqVector2D v4;

	/// Trivial constructor
	SqSampleQuad(const CqVector2D& v1, const CqVector2D& v2,
			const CqVector2D& v3, const CqVector2D& v4);

	/** \brief Copy the x and y coordinates from the given 3D sample quad.
	 *
	 * \param quad - 3D source coordinates
	 */
	SqSampleQuad(const Sq3DSampleQuad& srcQuad);

	/** Remap a sample quad by translation to lie in the box [0,1] x [0,1]
	 *
	 * Specifically, after remapping,
	 *   (s, t) = (min(s1,s2,s3,s4), min(t1,t2,t3,t4))
	 * is guarenteed to lie in the box [0,1]x[0,1]
	 *
	 * \param xPeriodic - if true, remap in the x direction.
	 * \param yPeriodic - if true, remap in the y direction.
	 */
	void remapPeriodic(bool xPeriodic, bool yPeriodic);

	/// Get the center point of the quadrilateral by averaging the vertices.
	CqVector2D center() const;
};


/** \brief 3D quadrilateral over which to sample a texture
 *
 * Some texture types such as shadow maps and environment maps need to be
 * sampled over a 3D quadrilateral rather than a 2D one.  For shadow maps, this
 * is because the third coordinate is needed to represent the depth away from
 * the light source of the surface being shadowed.
 *
 * \see SqSampleQuad for more detail.
 */
struct Sq3DSampleQuad
{
	CqVector3D v1;
	CqVector3D v2;
	CqVector3D v3;
	CqVector3D v4;

	/// Default constructor - set all corner vectors to 0.
	Sq3DSampleQuad();
	/// Trivial constructor
	Sq3DSampleQuad(const CqVector3D& v1, const CqVector3D& v2,
			const CqVector3D& v3, const CqVector3D& v4);

	/** \brief Multiply all the vectors in the quad by the specified matrix.
	 *
	 * That is perform the transformation
	 *
	 *   v1 = mat*v1;
	 *
	 * etc.
	 */
	void transform(const CqMatrix& mat);
	/// Get the center point of the quadrilateral by averaging the vertices.
	CqVector3D center() const;
};



//==============================================================================
// Implementation details
//==============================================================================

// SqSampleQuad implementation
inline SqSampleQuad::SqSampleQuad(const CqVector2D& v1, const CqVector2D& v2,
		const CqVector2D& v3, const CqVector2D& v4)
	: v1(v1),
	v2(v2),
	v3(v3),
	v4(v4)
{ }

inline SqSampleQuad::SqSampleQuad(const Sq3DSampleQuad& srcQuad)
	: v1(srcQuad.v1.x(), srcQuad.v1.y()),
	v2(srcQuad.v2.x(), srcQuad.v2.y()),
	v3(srcQuad.v3.x(), srcQuad.v3.y()),
	v4(srcQuad.v4.x(), srcQuad.v4.y())
{ }

inline void SqSampleQuad::remapPeriodic(bool xPeriodic, bool yPeriodic)
{
	if(xPeriodic || yPeriodic)
	{
		TqFloat x = xPeriodic ? min(min(min(v1.x(), v2.x()), v3.x()), v4.x()) : 0;
		TqFloat y = yPeriodic ? min(min(min(v1.y(), v2.y()), v3.y()), v4.y()) : 0;
		if(x < 0 || y < 0 || x >= 1 || y >= 1)
		{
			CqVector2D v(std::floor(x),std::floor(y));
			v1 -= v;
			v2 -= v;
			v3 -= v;
			v4 -= v;
		}
	}
}

inline CqVector2D SqSampleQuad::center() const
{
	return 0.25*(v1+v2+v3+v4);
}

//------------------------------------------------------------------------------
// Sq3DSampleQuad implementation
inline Sq3DSampleQuad::Sq3DSampleQuad()
	: v1(0,0,0),
	v2(0,0,0),
	v3(0,0,0),
	v4(0,0,0)
{ }

inline Sq3DSampleQuad::Sq3DSampleQuad(const CqVector3D& v1, const CqVector3D& v2,
		const CqVector3D& v3, const CqVector3D& v4)
	: v1(v1),
	v2(v2),
	v3(v3),
	v4(v4)
{ }

inline CqVector3D Sq3DSampleQuad::center() const
{
	return 0.25*(v1+v2+v3+v4);
}

//------------------------------------------------------------------------------

} // namespace Aqsis

#endif // SAMPLEQUAD_H_INCLUDED
