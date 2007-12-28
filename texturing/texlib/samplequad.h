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
struct SqSampleQuad
{
	CqVector2D v1;
	CqVector2D v2;
	CqVector2D v3;
	CqVector2D v4;

	/// Construct from explicit texture coordinates
	inline SqSampleQuad(TqFloat s1, TqFloat t1, TqFloat s2, TqFloat t2,
			TqFloat s3, TqFloat t3, TqFloat s4, TqFloat t4);
	inline SqSampleQuad(CqVector2D v1, CqVector2D v2, CqVector2D v3, CqVector2D v4);

	/// Get the center point of the quadrilateral by averaging the vertices.
	inline CqVector2D center() const;

	/** Remap the sample quad by translation to lie in the box [0,1] x [0,1]
	 *
	 * Specifically, after remapping,
	 *   (s, t) = (min(s1,s2,s3,s4), min(t1,t2,t3,t4))
	 * is guarenteed to lie in the box [0,1]x[0,1]
	 */
	inline void remapPeriodic(bool xPeriodic, bool yPeriodic);
};


//==============================================================================
// Implementation details
//==============================================================================

inline SqSampleQuad::SqSampleQuad(TqFloat s1, TqFloat t1, TqFloat s2, TqFloat t2,
		TqFloat s3, TqFloat t3, TqFloat s4, TqFloat t4)
	: v1(s1, t1),
	v2(s2, t2),
	v3(s3, t3),
	v4(s4, t4)
{ }

inline SqSampleQuad::SqSampleQuad(CqVector2D v1, CqVector2D v2,
		CqVector2D v3, CqVector2D v4)
	: v1(v1),
	v2(v2),
	v3(v3),
	v4(v4)
{ }

inline CqVector2D SqSampleQuad::center() const
{
	return 0.25*(v1+v2+v3+v4);
}

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

} // namespace Aqsis

#endif // SAMPLEQUAD_H_INCLUDED
