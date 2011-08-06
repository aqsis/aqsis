// Aqsis
// Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the software's owners nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// (This is the New BSD license)

/** \file
 *
 * \brief Sampling quadrilateral struct definition
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef SAMPLEQUAD_H_INCLUDED
#define SAMPLEQUAD_H_INCLUDED

#include <aqsis/math/math.h>
#include <aqsis/math/matrix.h>
#include <aqsis/math/vector2d.h>
#include <aqsis/math/vector3d.h>
#include <aqsis/math/vectorcast.h>

namespace Aqsis {

class CqMatrix;
struct Sq3DSampleQuad;

//------------------------------------------------------------------------------
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
	/// v1 to v4 are vectors defining the vertices of the quadrilateral.
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

	/** \brief Scale the sample quad about its center point.
	 *
	 * The width parameters effects the quad in a simple way:  All the
	 * vertices are contracted toward or exapanded away from the quad
	 * center point by multiplying by the width in the appropriate
	 * direction.
	 *
	 * \param xWidth - amount to expand sample quad in the x direction
	 * \param yWidth - amount to expand sample quad in the y direction
	 */
	void scaleWidth(TqFloat xWidth, TqFloat yWidth);

	/// Get the center point of the quadrilateral by averaging the vertices.
	CqVector2D center() const;
};


//------------------------------------------------------------------------------
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
	/// v1 to v4 are vectors defining the vertices of the quadrilateral.
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

	/** \brief Assign the first two coordinates of a 2D sample quad to this quad.
	 *
	 * Leaves other coordinates unchanged.
	 */
	void copy2DCoords(const SqSampleQuad& toCopy);
};


//------------------------------------------------------------------------------
/** \brief 2D parallelogram over which to sample a texture
 *
 * The parallelogram is defined by its center point along with vectors along
 * its sides.  As a picture:
 *
 * \verbatim
 *
 *         _-------
 *         /|    /
 *     s2 /  c  /
 *       /     /
 *      .----->
 *        s1
 *
 * \endverbatim
 *
 * This is a special case of SqSampleQuad, holding less information, but more
 * specifically adapted to some types of filtering such as EWA.
 */
struct SqSamplePllgram
{
	/// center point for the sample
	CqVector2D c;
	/// first side of parallelogram
	CqVector2D s1;
	/// second side of parallelogram
	CqVector2D s2;

	/// Trivial constructor
	SqSamplePllgram(const CqVector2D& c, const CqVector2D& s1, const CqVector2D s2);
	/// Convert from a sample quad to a sample parallelogram
	explicit SqSamplePllgram(const SqSampleQuad& quad);

	/** \brief Remap the parallelogram to lie in the box [0,1]x[0,1]
	 *
	 * The remapping occurs by translating the center by an integer lattice
	 * point (n,m) such that c lies inside [0,1]x[0,1].
	 *
	 * \param xPeriodic - if true, remap in the x direction.
	 * \param yPeriodic - if true, remap in the y direction.
	 */
	void remapPeriodic(bool xPeriodic, bool yPeriodic);

	/** \brief Scale the parallelogram about its center point.
	 *
	 * \param xWidth - amount to expand sample quad in the x direction
	 * \param yWidth - amount to expand sample quad in the y direction
	 */
	void scaleWidth(TqFloat xWidth, TqFloat yWidth);
};


//------------------------------------------------------------------------------
/** \brief 3D parallelogram over which to sample a texture
 *
 * \see SqSamplePllgram
 */
struct Sq3DSamplePllgram
{
	/// center point for the sample
	CqVector3D c;
	/// first side of parallelogram
	CqVector3D s1;
	/// second side of parallelogram
	CqVector3D s2;

	/// Trivial constructor
	Sq3DSamplePllgram(const CqVector3D& c, const CqVector3D& s1, const CqVector3D s2);
	/// Convert from a sample quad to a sample parallelogram
	explicit Sq3DSamplePllgram(const Sq3DSampleQuad& quad);
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
	: v1(vectorCast<CqVector2D>(srcQuad.v1)),
	v2(vectorCast<CqVector2D>(srcQuad.v2)),
	v3(vectorCast<CqVector2D>(srcQuad.v3)),
	v4(vectorCast<CqVector2D>(srcQuad.v4))
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

inline void Sq3DSampleQuad::copy2DCoords(const SqSampleQuad& toCopy)
{
	v1.x(toCopy.v1.x());
	v1.y(toCopy.v1.y());
	v2.x(toCopy.v2.x());
	v2.y(toCopy.v2.y());
	v3.x(toCopy.v3.x());
	v3.y(toCopy.v3.y());
	v4.x(toCopy.v4.x());
	v4.y(toCopy.v4.y());
}

inline void SqSampleQuad::scaleWidth(TqFloat xWidth, TqFloat yWidth)
{
	if(xWidth != 1 || yWidth != 1)
	{
		CqVector2D c = center();
		TqFloat cxWeighted = (1 - xWidth)*c.x();
		TqFloat cyWeighted = (1 - yWidth)*c.y();
		// Expand v1...v4 away from the quad center by multiplying the x
		// and y components of the vectors which point from the quad center to
		// the vertices by the x and y widths respectively.
		v1.x(xWidth*v1.x() + cxWeighted);
		v1.y(yWidth*v1.y() + cyWeighted);
		v2.x(xWidth*v2.x() + cxWeighted);
		v2.y(yWidth*v2.y() + cyWeighted);
		v3.x(xWidth*v3.x() + cxWeighted);
		v3.y(yWidth*v3.y() + cyWeighted);
		v4.x(xWidth*v4.x() + cxWeighted);
		v4.y(yWidth*v4.y() + cyWeighted);
	}
}


inline void Sq3DSampleQuad::transform(const CqMatrix& mat)
{
	v1 = mat*v1;
	v2 = mat*v2;
	v3 = mat*v3;
	v4 = mat*v4;
}

//------------------------------------------------------------------------------
// SqSamplePllgram implementation
inline SqSamplePllgram::SqSamplePllgram(const CqVector2D& c, const CqVector2D& s1,
		const CqVector2D s2)
	: c(c),
	s1(s1),
	s2(s2)
{ }

inline SqSamplePllgram::SqSamplePllgram(const SqSampleQuad& quad)
	: c(quad.center()),
	s1(0.5*(quad.v2 - quad.v1 + quad.v4 - quad.v3)),
	s2(0.5*(quad.v1 - quad.v3 + quad.v2 - quad.v4))
{ }

inline void SqSamplePllgram::remapPeriodic(bool xPeriodic, bool yPeriodic)
{
	if(xPeriodic || yPeriodic)
	{
		if(c.x() < 0 || c.y() < 0 || c.x() >= 1 || c.y() >= 1)
		{
			CqVector2D v(std::floor(c.x()),std::floor(c.y()));
			c -= v;
		}
	}
}

inline void SqSamplePllgram::scaleWidth(TqFloat xWidth, TqFloat yWidth)
{
	if(xWidth != 1 || yWidth != 1)
	{
		s1.x(s1.x()*xWidth);
		s1.y(s1.y()*yWidth);
		s2.x(s1.x()*xWidth);
		s2.y(s1.y()*yWidth);
	}
}


//------------------------------------------------------------------------------
// Sq3DSamplePllgram implementation
inline Sq3DSamplePllgram::Sq3DSamplePllgram(const CqVector3D& c,
		const CqVector3D& s1, const CqVector3D s2)
	: c(c),
	s1(s1),
	s2(s2)
{ }

inline Sq3DSamplePllgram::Sq3DSamplePllgram(const Sq3DSampleQuad& quad)
	: c(quad.center()),
	s1(0.5*(quad.v2 - quad.v1 + quad.v4 - quad.v3)),
	s2(0.5*(quad.v1 - quad.v3 + quad.v2 - quad.v4))
{ }


//------------------------------------------------------------------------------

} // namespace Aqsis

#endif // SAMPLEQUAD_H_INCLUDED
