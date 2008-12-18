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
 * \brief Declares the CqVector3D class which encapsulates a 3D vector/point/normal.
 * \author Paul C. Gregory (pgregory@aqsis.org)
 */

#ifndef VECTOR3D_H_INCLUDED
#define VECTOR3D_H_INCLUDED

#include "aqsis.h"

#include <iostream>

#include "aqsismath.h"
#include "vector2d.h"

namespace Aqsis {

//-----------------------------------------------------------------------

class CqVector4D;
class CqColor;

//----------------------------------------------------------------------
/** \class CqVector3D
 * \brief Define class structure for 3D vector.
 */

class COMMON_SHARE CqVector3D
{
	public:
		/// Construct the zero vector.
		CqVector3D();
		CqVector3D(const CqVector2D &From);
		CqVector3D(const CqColor &From);
		CqVector3D(TqFloat x, TqFloat y, TqFloat z);
		CqVector3D(TqFloat f);
		CqVector3D(const CqVector4D &From);
		CqVector3D(const TqFloat Array[3]);

		/** Get the x component.
		 */
		TqFloat x() const;
		/// Get a reference to the x-componenet.
		TqFloat& x();
		/** Set the x component.
		 *
		 * \deprecated.  Use the x() method returning a non-const reference.
		 */
		void x(TqFloat x);
		/** Get the y component.
		 */
		TqFloat y() const;
		/// Get a reference to the y-componenet.
		TqFloat& y();
		/** Set the y component.
		 *
		 * \deprecated.  Use the y() method returning a non-const reference.
		 */
		void y(TqFloat y);
		/** Get the z component.
		 */
		TqFloat z() const;
		/// Get a reference to the z-componenet.
		TqFloat& z();
		/** Set the z component.
		 *
		 * \deprecated.  Use the z() method returning a non-const reference.
		 */
		void z(TqFloat z);

		/** Array based component access.
		 * \param i Integer component index, 0-2.
		 * \return Appropriate component, or z if index is invalid.
		 */
		TqFloat& operator[] (TqInt i);

		/** Array based component access.
		 * \param i Integer component index, 0-2.
		 * \return Appropriate component, or z if index is invalid.
		 */
		const TqFloat& operator[] (TqInt i) const;

		/** Get the length squared.
		 */
		TqFloat Magnitude2() const;
		/** Get the length.
		 */
		TqFloat Magnitude() const;
		CqVector3D& Unit();

		CqVector3D& operator= (const CqVector4D &From);
		CqVector3D& operator= (const CqColor &From);
		/** Addition assignment operator.
		 */
		CqVector3D& operator+=(const CqVector3D &From);
		/** Component wise addition assignment operator.
		 */
		CqVector3D& operator+=(const TqFloat f);
		/** Subtraction assignment operator.
		 */
		CqVector3D& operator-=(const CqVector3D &From);
		/** Component wise subtraction assignment operator.
		 */
		CqVector3D& operator-=(const TqFloat f);
		/// Cross product assignment operator
		CqVector3D& operator%=(const CqVector3D &From);
		/** Component wise scale assignment operator.
		 */
		CqVector3D& operator*=(const TqFloat Scale);
		/** Scale assignment operator.
		 */
		CqVector3D& operator*=(const CqVector3D &Scale);
		/** Inverse scale assignment operator.
		 */
		CqVector3D& operator/=(const CqVector3D &Scale);
		/** Component wise inverse scale assignment operator.
		 */
		CqVector3D& operator/=(const TqFloat Scale);

		/** Component wise equality operator.
		 */
		bool operator==(const CqVector3D &Cmp) const;
		/** Component wise inequality operator.
		 */
		bool operator!=(const CqVector3D &Cmp) const;
		/** Component wise greater than or equal to operator.
		 */
		bool operator>=(const CqVector3D &Cmp) const;
		/** Component wise less than or equal to operator.
		 */
		bool operator<=(const CqVector3D &Cmp) const;
		/** Component wise greater than operator.
		 */
		bool operator>(const CqVector3D &Cmp) const;
		/** Component wise less than operator.
		 */
		bool operator<(const CqVector3D &Cmp) const;

		/// Determine a componentwise minimum for two vectors
		friend CqVector3D min(const CqVector3D& a, const CqVector3D& b);
		/// Determine a componentwise maximum for two vectors
		friend CqVector3D max(const CqVector3D& a, const CqVector3D& b);
		/// Clamp the components of a vector to between two given vectors.
		friend CqVector3D clamp(const CqVector3D& v, const CqVector3D& min, const CqVector3D& max);
		/** \brief Linearly interpolate between two vectors
		 *
		 * \param t - interpolation parameter
		 * \param c0 - color corresponding to t = 0
		 * \param c1 - color corresponding to t = 1
		 */
		friend CqVector3D lerp(const TqFloat t, const CqVector3D& v0, const CqVector3D& v1);

		friend CqVector3D operator+(const TqFloat f, const CqVector3D& v);
		friend CqVector3D operator+(const CqVector3D& v, const TqFloat f);
		friend CqVector3D operator-(const TqFloat f, const CqVector3D& v);
		friend CqVector3D operator-(const CqVector3D& v, const TqFloat f);
		friend CqVector3D operator*(const TqFloat f, const CqVector3D& v);
		friend CqVector3D operator*(const CqVector3D& v, const TqFloat f);
		friend CqVector3D operator/(const TqFloat f, const CqVector3D& v);
		friend CqVector3D operator/(const CqVector3D& v, const TqFloat f);

		friend CqVector3D operator+(const CqVector3D& a, const CqVector3D& b);
		friend CqVector3D operator-(const CqVector3D& a, const CqVector3D& b);
		friend CqVector3D operator/(const CqVector3D& a, const CqVector3D& b);
		friend CqVector3D operator-(const CqVector3D& v);

		/// Dot product
		friend TqFloat operator*(const CqVector3D& a, const CqVector3D& b);
		/// Cross product
		COMMON_SHARE friend CqVector3D operator%(const CqVector3D& a, const CqVector3D& b);

		// Stream insertion
		COMMON_SHARE friend std::ostream &operator<<(std::ostream &Stream, const CqVector3D &Vector);

	private:
		TqFloat m_x; ///< X component.
		TqFloat m_y; ///< Y component.
		TqFloat m_z; ///< Z component.
};


//------------------------------------------------------------------------------
/** \brief Determine whether two vectors are equal to within some tolerance
 *
 * The closeness criterion for vectors is based on the euclidian norm - ie, the
 * usual distance function between two vectors.  v1 and v2 are "close" if
 *
 * length(v1 - v2) < tol*max(length(v1), length(v2));
 *
 * \param v1, v2 - vectors to compare
 * \param tolerance for comparison
 */
bool isClose(const CqVector3D& v1, const CqVector3D& v2,
		TqFloat tol = 10*std::numeric_limits<TqFloat>::epsilon());



//==============================================================================
// Implementation details.
//==============================================================================
// CqVector3D implementation

inline CqVector3D::CqVector3D()
	: m_x(0.0f),
	m_y(0.0f),
	m_z(0.0f)
{}
inline CqVector3D::CqVector3D(const CqVector2D &From)
	: m_x(From.x()),
	m_y(From.y()),
	m_z(0)
{}
inline CqVector3D::CqVector3D(TqFloat x, TqFloat y, TqFloat z)
	: m_x(x),
	m_y(y),
	m_z(z)
{}
inline CqVector3D::CqVector3D(TqFloat f)
	: m_x(f),
	m_y(f),
	m_z(f)
{}
inline CqVector3D::CqVector3D(const TqFloat Array[3])
	: m_x(Array[0]),
	m_y(Array[1]),
	m_z(Array[2])
{}

inline TqFloat CqVector3D::x() const
{
	return m_x;
}
inline TqFloat& CqVector3D::x()
{
	return m_x;
}
inline void CqVector3D::x(TqFloat x)
{
	m_x = x;
}
inline TqFloat CqVector3D::y() const
{
	return m_y;
}
inline TqFloat& CqVector3D::y()
{
	return m_y;
}
inline void CqVector3D::y(TqFloat y)
{
	m_y = y;
}
inline TqFloat CqVector3D::z() const
{
	return m_z;
}
inline TqFloat& CqVector3D::z()
{
	return m_z;
}
inline void CqVector3D::z(TqFloat z)
{
	m_z = z;
}

inline TqFloat& CqVector3D::operator[](TqInt i)
{
	switch (i)
	{
		case 0:
			return m_x;
			break;
		case 1:
			return m_y;
			break;
		case 2:
			return m_z;
			break;
		default:
			break;
	}
	return m_z;
}
inline const TqFloat& CqVector3D::operator[](TqInt i) const
{
	switch (i)
	{
		case 0:
			return m_x;
			break;
		case 1:
			return m_y;
			break;
		case 2:
			return m_z;
			break;
		default:
			break;
	}
	return m_z;
}

inline TqFloat CqVector3D::Magnitude2() const
{
	return (m_x * m_x) + (m_y * m_y) + (m_z * m_z);
}
inline TqFloat CqVector3D::Magnitude() const
{
	return std::sqrt(Magnitude2());
}
inline CqVector3D& CqVector3D::Unit()
{
	TqFloat Mag = Magnitude();
	if(Mag > 0.0f)
	{
		m_x /= Mag;
		m_y /= Mag;
		m_z /= Mag;
	}
	return *this;
}

inline CqVector3D& CqVector3D::operator+=(const CqVector3D &From)
{
	m_x += From.m_x;
	m_y += From.m_y;
	m_z += From.m_z;
	return *this;
}
inline CqVector3D& CqVector3D::operator+=(const TqFloat f)
{
	m_x += f;
	m_y += f;
	m_z += f;
	return *this;
}
inline CqVector3D& CqVector3D::operator-=(const CqVector3D &From)
{
	m_x -= From.m_x;
	m_y -= From.m_y;
	m_z -= From.m_z;
	return *this;
}
inline CqVector3D& CqVector3D::operator-=(const TqFloat f)
{
	m_x -= f;
	m_y -= f;
	m_z -= f;
	return *this;
}
inline CqVector3D& CqVector3D::operator%=(const CqVector3D &From)
{
	CqVector3D	vecTemp( *this );

	m_x = ( vecTemp.m_y * From.m_z ) - ( vecTemp.m_z * From.m_y );
	m_y = ( vecTemp.m_z * From.m_x ) - ( vecTemp.m_x * From.m_z );
	m_z = ( vecTemp.m_x * From.m_y ) - ( vecTemp.m_y * From.m_x );

	return ( *this );
}
inline CqVector3D& CqVector3D::operator*=(const TqFloat Scale)
{
	m_x *= Scale;
	m_y *= Scale;
	m_z *= Scale;
	return *this;
}
inline CqVector3D& CqVector3D::operator*=(const CqVector3D &Scale)
{
	m_x *= Scale.m_x;
	m_y *= Scale.m_y;
	m_z *= Scale.m_z;
	return *this;
}
inline CqVector3D& CqVector3D::operator/=(const CqVector3D &Scale)
{
	m_x /= Scale.m_x;
	m_y /= Scale.m_y;
	m_z /= Scale.m_z;
	return *this;
}
inline CqVector3D& CqVector3D::operator/=(const TqFloat Scale)
{
	m_x /= Scale;
	m_y /= Scale;
	m_z /= Scale;
	return *this;
}

inline bool CqVector3D::operator==(const CqVector3D &Cmp) const
{
	return (m_x == Cmp.m_x) && (m_y == Cmp.m_y) && (m_z == Cmp.m_z);
}
inline bool CqVector3D::operator!=(const CqVector3D &Cmp) const
{
	return (m_x != Cmp.m_x) || (m_y != Cmp.m_y) || (m_z != Cmp.m_z);
}
inline bool CqVector3D::operator>=(const CqVector3D &Cmp) const
{
	return (m_x >= Cmp.m_x) && (m_y >= Cmp.m_y) && (m_z >= Cmp.m_z);
}
inline bool CqVector3D::operator<=(const CqVector3D &Cmp) const
{
	return (m_x <= Cmp.m_x) && (m_y <= Cmp.m_y) && (m_z <= Cmp.m_z);
}
inline bool CqVector3D::operator>(const CqVector3D &Cmp) const
{
	return (m_x > Cmp.m_x) && (m_y > Cmp.m_y) && (m_z > Cmp.m_z);
}
inline bool CqVector3D::operator<(const CqVector3D &Cmp) const
{
	return (m_x < Cmp.m_x) && (m_y < Cmp.m_y) && (m_z < Cmp.m_z);
}

//------------------------------------------------------------------------------
// friend functions

inline CqVector3D min(const CqVector3D& a, const CqVector3D& b)
{
	return CqVector3D(min(a.m_x, b.m_x), min(a.m_y, b.m_y), min(a.m_z, b.m_z));
}

inline CqVector3D max(const CqVector3D& a, const CqVector3D& b)
{
	return CqVector3D(max(a.m_x, b.m_x), max(a.m_y, b.m_y), max(a.m_z, b.m_z));
}

inline CqVector3D clamp(const CqVector3D& v, const CqVector3D& min, const CqVector3D& max)
{
	return CqVector3D(clamp(v.m_x, min.m_x, max.m_x),
			clamp(v.m_y, min.m_y, max.m_y),
			clamp(v.m_z, min.m_z, max.m_z));
}

inline CqVector3D lerp(TqFloat t, const CqVector3D& v0, const CqVector3D& v1)
{
	return CqVector3D((1-t)*v0.m_x + t*v1.m_x,
			       (1-t)*v0.m_y + t*v1.m_y,
			       (1-t)*v0.m_z + t*v1.m_z);
}

inline CqVector3D operator+(const TqFloat f, const CqVector3D& v)
{
	return CqVector3D(f + v.x(), f + v.y(), f + v.z());
}
inline CqVector3D operator+(const CqVector3D& v, const TqFloat f)
{
	CqVector3D r(v);
	return r += f;
}
inline CqVector3D operator-(const TqFloat f, const CqVector3D& v)
{
	return CqVector3D(f -v.x(), f - v.y(), f - v.z());
}
inline CqVector3D operator-(const CqVector3D& v, const TqFloat f)
{
	CqVector3D r(v);
	return r -= f;
}
inline CqVector3D operator*(const TqFloat f, const CqVector3D& v)
{
	return CqVector3D(f * v.x(), f * v.y(), f * v.z());
}
inline CqVector3D operator*(const CqVector3D& v, const TqFloat f)
{
	CqVector3D r(v);
	return r *= f;
}
inline CqVector3D operator/(const TqFloat f, const CqVector3D& v)
{
	return CqVector3D(f / v.x(), f / v.y(), f / v.z());
}
inline CqVector3D operator/(const CqVector3D& v, const TqFloat f)
{
	CqVector3D r(v);
	return r /= f;
}

inline CqVector3D operator+(const CqVector3D& a, const CqVector3D& b)
{
	CqVector3D r(a);
	return r += b;
}
inline CqVector3D operator-(const CqVector3D& a, const CqVector3D& b)
{
	CqVector3D r(a);
	return r -= b;
}
inline CqVector3D operator/(const CqVector3D& a, const CqVector3D& b)
{
	CqVector3D r(a);
	return r /= b;
}
inline CqVector3D operator-(const CqVector3D& v)
{
	return CqVector3D(-v.m_x, -v.m_y, -v.m_z);
}

inline TqFloat operator*(const CqVector3D& a, const CqVector3D& b)
{
	return a.m_x * b.m_x + a.m_y * b.m_y + a.m_z * b.m_z;
}
inline CqVector3D operator%(const CqVector3D &a, const CqVector3D &b)
{
	return CqVector3D(
		(a.m_y * b.m_z) - (a.m_z * b.m_y),
	    (a.m_z * b.m_x) - (a.m_x * b.m_z),
	    (a.m_x * b.m_y) - (a.m_y * b.m_x)
	);
}


//-----------------------------------------------------------------------
// free functions

inline bool isClose(const CqVector3D& v1, const CqVector3D& v2, TqFloat tol)
{
	TqFloat diff2 = (v1 - v2).Magnitude2();
	TqFloat tol2 = tol*tol;
	return diff2 <= tol2*v1.Magnitude2() || diff2 <= tol2*v2.Magnitude2();
}

//-----------------------------------------------------------------------

} // namespace Aqsis

#endif // VECTOR3D_H_INCLUDED
