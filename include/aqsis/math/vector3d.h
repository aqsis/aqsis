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
 *
 * \brief 3D vectors classes.
 *
 * \author Paul C. Gregory (pgregory@aqsis.org)
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef VECTOR3D_H_INCLUDED
#define VECTOR3D_H_INCLUDED

#include <aqsis/aqsis.h>

#include <iostream>

#include <aqsis/math/math.h>
#include <aqsis/math/vecfwd.h>
#include <aqsis/math/vectorstorage.h>

namespace Aqsis {

//----------------------------------------------------------------------
/** \brief A 3D vector class
 *
 * CqBasicVec3 is templated on a storage type, StoreT, which allows us to have
 * more than one storage mechanism for the underlying vector components.
 *
 * Two storage mechanisms of interest are implemented in CqVec3Data and
 * CqVecRefData.  CqVec3Data stores the components by value, while CqVecRefData
 * stores only a pointer to the first element of the data.  The by-value
 * version should be used in most circumstances; the by-reference version
 * should be used when it's desirable to treat an array of floats as a vector
 * or array of vectors.
 */
template<typename StoreT>
class CqBasicVec3
{
	public:
		/// Construct the zero vector.
		CqBasicVec3();
		/// Construct with given x,y,z componenets.
		CqBasicVec3(TqFloat x, TqFloat y, TqFloat z);
		/// Construct with x = y = z = f
		explicit CqBasicVec3(TqFloat f);
		/// Get components from the first three elements of a float array
		explicit CqBasicVec3(const TqFloat* array);
		/** Get components from the first three elements of a float array
		 * Detailed semantics depend on the storage policy.
		 */
		explicit CqBasicVec3(TqFloat* array);
		/// Copy from another 3D vector.
		CqBasicVec3(const CqBasicVec3& rhs);
		/// Copy from another 3D vector.
		template<typename T>
		CqBasicVec3(const CqBasicVec3<T>& rhs);

		/// \name Assignment operators
		//@{
		CqBasicVec3& operator=(const CqBasicVec3& rhs);
		template<typename T>
		CqBasicVec3& operator=(const CqBasicVec3<T>& rhs);
		//@}

		/// \name Vector component access
		//@{
		TqFloat x() const;
		TqFloat y() const;
		TqFloat z() const;
		//@}

		/// \name Assignable vector component access
		//@{
		TqFloat& x();
		TqFloat& y();
		TqFloat& z();
		//@}

		//@{
		/** Vector component setters
		 * \deprecated.  Use the methods returning component references
		 */
		void x(TqFloat x);
		void y(TqFloat y);
		void z(TqFloat z);
		//@}

		/// \name Index-based component access 
		//@{
		/** Indices outside the valid range 0-2 will cause an assertion in
		 * debug mode.
		 */
		TqFloat& operator[] (TqInt i);
		TqFloat operator[] (TqInt i) const;
		//@}

		/// Get the length squared.
		TqFloat Magnitude2() const;
		/// Get the length.
		TqFloat Magnitude() const;
		/// Normalize and return the vector
		CqBasicVec3& Unit();

		/// \name Op-assign with 3D vectors as the rhs
		//@{
		/** Opassign functions with another 3D vector type on the right hand
		 * side causes the associated operation to be carried out componentwise.
		 */
		template<typename T> CqBasicVec3& operator+=(const CqBasicVec3<T> &rhs);
		template<typename T> CqBasicVec3& operator-=(const CqBasicVec3<T> &rhs);
		template<typename T> CqBasicVec3& operator*=(const CqBasicVec3<T> &scale);
		template<typename T> CqBasicVec3& operator/=(const CqBasicVec3<T> &scale);
		//@}
		/// Set this equal to the cross product (*this) x rhs
		template<typename T> CqBasicVec3& operator%=(const CqBasicVec3<T> &rhs);

		/// \name Op-assign with floats as the rhs.
		//@{
		/** Opassign functions with floats on the right hand side cause the
		 * associated operation to be carried out on each component.
		 */
		CqBasicVec3& operator+=(const TqFloat f);
		CqBasicVec3& operator-=(const TqFloat f);
		CqBasicVec3& operator*=(const TqFloat scale);
		CqBasicVec3& operator/=(const TqFloat scale);
		//@}

		/// \name Comparison functions
		//@{
		/**
		 * Comparison operators return true when the associated comparison is
		 * true for all componenets at once.
		 */
		template<typename T> bool operator==(const CqBasicVec3<T> &rhs) const;
		/// return true when any of the components of rhs differ from this.
		template<typename T> bool operator!=(const CqBasicVec3<T> &rhs) const;
		template<typename T> bool operator>=(const CqBasicVec3<T> &rhs) const;
		template<typename T> bool operator<=(const CqBasicVec3<T> &rhs) const;
		template<typename T> bool operator>(const CqBasicVec3<T> &rhs) const;
		template<typename T> bool operator<(const CqBasicVec3<T> &rhs) const;
		//@}

	private:
		StoreT m_data;
};

// Non-member operators & functions for 3D vectors

/// \name Float/vector operators
//@{
/** The float is promoted to a vector with equal components after which
 * operations are performed componentwise.
 */
template<typename T> CqVec3 operator+(const TqFloat f, const CqBasicVec3<T>& v);
template<typename T> CqVec3 operator-(const TqFloat f, const CqBasicVec3<T>& v);
template<typename T> CqVec3 operator*(const TqFloat f, const CqBasicVec3<T>& v);
template<typename T> CqVec3 operator/(const TqFloat f, const CqBasicVec3<T>& v);
template<typename T> CqVec3 operator+(const CqBasicVec3<T>& v, const TqFloat f);
template<typename T> CqVec3 operator-(const CqBasicVec3<T>& v, const TqFloat f);
template<typename T> CqVec3 operator*(const CqBasicVec3<T>& v, const TqFloat f);
template<typename T> CqVec3 operator/(const CqBasicVec3<T>& v, const TqFloat f);
//@}

/// \name vector/vector arithmetic
//@{
/// Componentwise operations on the two vectors produce a third.
template<typename T1, typename T2>
CqVec3 operator+(const CqBasicVec3<T1>& a, const CqBasicVec3<T2>& b);
template<typename T1, typename T2>
CqVec3 operator-(const CqBasicVec3<T1>& a, const CqBasicVec3<T2>& b);
template<typename T1, typename T2>
CqVec3 operator/(const CqBasicVec3<T1>& a, const CqBasicVec3<T2>& b);
template<typename T1, typename T2>
CqVec3 min(const CqBasicVec3<T1>& a, const CqBasicVec3<T2>& b);
template<typename T1, typename T2>
CqVec3 max(const CqBasicVec3<T1>& a, const CqBasicVec3<T2>& b);
template<typename T1, typename T2, typename T3>
CqVec3 clamp(const CqBasicVec3<T1>& v, const CqBasicVec3<T2>& min, const CqBasicVec3<T3>& max);
//@}

/// Vector negation
template<typename T>
CqVec3 operator-(const CqBasicVec3<T>& v);
/// Dot product
template<typename T1, typename T2>
TqFloat operator*(const CqBasicVec3<T1>& a, const CqBasicVec3<T2>& b);
/// Cross product
template<typename T1, typename T2>
CqVec3 operator%(const CqBasicVec3<T1>& a, const CqBasicVec3<T2>& b);
/** \brief Linearly interpolate between two vectors
 *
 * \param t - interpolation parameter
 * \param v0 - vector corresponding to t = 0
 * \param v1 - vector corresponding to t = 1
 */
template<typename T1, typename T2>
CqVec3 lerp(const TqFloat t, const CqBasicVec3<T1>& v0, const CqBasicVec3<T2>& v1);

// Stream insertion
template<typename T>
std::ostream& operator<<(std::ostream &out, const CqBasicVec3<T> &v);

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
template<typename T1, typename T2>
bool isClose(const CqBasicVec3<T1>& v1, const CqBasicVec3<T2>& v2,
		TqFloat tol = 10*std::numeric_limits<TqFloat>::epsilon());



//==============================================================================
// Implementation details.
//==============================================================================
// CqBasicVec3 implementation

//----------------------------------------
// Constructors
template<typename StoreT>
inline CqBasicVec3<StoreT>::CqBasicVec3()
	: m_data(0,0,0)
{ }

template<typename StoreT>
inline CqBasicVec3<StoreT>::CqBasicVec3(TqFloat x, TqFloat y, TqFloat z)
	: m_data(x,y,z)
{ }

template<typename StoreT>
inline CqBasicVec3<StoreT>::CqBasicVec3(TqFloat f)
	: m_data(f,f,f)
{ }

template<typename StoreT>
inline CqBasicVec3<StoreT>::CqBasicVec3(const TqFloat* array)
	: m_data(array)
{ }

template<typename StoreT>
inline CqBasicVec3<StoreT>::CqBasicVec3(TqFloat* array)
	: m_data(array)
{ }

template<typename StoreT>
inline CqBasicVec3<StoreT>::CqBasicVec3(const CqBasicVec3<StoreT>& rhs)
	: m_data(rhs.m_data)
{ }

template<typename StoreT>
template<typename T>
inline CqBasicVec3<StoreT>::CqBasicVec3(const CqBasicVec3<T>& rhs)
	: m_data(rhs.x(), rhs.y(), rhs.z())
{ }

//----------------------------------------
// Assignment operators

// Need to override the default assignment operator; the templated version
// below doesn't seem to do so.
template<typename StoreT>
inline CqBasicVec3<StoreT>& CqBasicVec3<StoreT>::operator=(const CqBasicVec3<StoreT>& rhs)
{
	x() = rhs.x();
	y() = rhs.y();
	z() = rhs.z();
	return *this;
}

template<typename StoreT>
template<typename T>
inline CqBasicVec3<StoreT>& CqBasicVec3<StoreT>::operator=(const CqBasicVec3<T>& rhs)
{
	x() = rhs.x();
	y() = rhs.y();
	z() = rhs.z();
	return *this;
}

//----------------------------------------
// Component access
template<typename StoreT>
inline TqFloat CqBasicVec3<StoreT>::x() const { return m_data[0]; }
template<typename StoreT>
inline TqFloat CqBasicVec3<StoreT>::y() const { return m_data[1]; }
template<typename StoreT>
inline TqFloat CqBasicVec3<StoreT>::z() const { return m_data[2]; }

template<typename StoreT>
inline TqFloat& CqBasicVec3<StoreT>::x() { return m_data[0]; }
template<typename StoreT>
inline TqFloat& CqBasicVec3<StoreT>::y() { return m_data[1]; }
template<typename StoreT>
inline TqFloat& CqBasicVec3<StoreT>::z() { return m_data[2]; }

template<typename StoreT>
inline void CqBasicVec3<StoreT>::x(TqFloat x) { m_data[0] = x; }
template<typename StoreT>
inline void CqBasicVec3<StoreT>::y(TqFloat y) { m_data[1] = y; }
template<typename StoreT>
inline void CqBasicVec3<StoreT>::z(TqFloat z) { m_data[2] = z; }

template<typename StoreT>
inline TqFloat& CqBasicVec3<StoreT>::operator[](TqInt i)
{
	assert(i >= 0 && i <= 2);
	return m_data[i];
}
template<typename StoreT>
inline TqFloat CqBasicVec3<StoreT>::operator[](TqInt i) const
{
	assert(i >= 0 && i <= 2);
	return m_data[i];
}


//----------------------------------------
// length etc
template<typename StoreT>
inline TqFloat CqBasicVec3<StoreT>::Magnitude2() const
{
	return x()*x() + y()*y() + z()*z();
}
template<typename StoreT>
inline TqFloat CqBasicVec3<StoreT>::Magnitude() const
{
	return std::sqrt(Magnitude2());
}
template<typename StoreT>
inline CqBasicVec3<StoreT>& CqBasicVec3<StoreT>::Unit()
{
	TqFloat length = Magnitude();
	if(length != 0)
	{
		x() /= length;
		y() /= length;
		z() /= length;
	}
	return *this;
}

//----------------------------------------
// opassign operators with 3D vectors as the rhs.
template<typename StoreT>
template<typename T>
inline CqBasicVec3<StoreT>& CqBasicVec3<StoreT>::operator+=(const CqBasicVec3<T> &vec)
{
	x() += vec.x();
	y() += vec.y();
	z() += vec.z();
	return *this;
}

template<typename StoreT>
template<typename T>
inline CqBasicVec3<StoreT>& CqBasicVec3<StoreT>::operator-=(const CqBasicVec3<T> &vec)
{
	x() -= vec.x();
	y() -= vec.y();
	z() -= vec.z();
	return *this;
}

template<typename StoreT>
template<typename T>
inline CqBasicVec3<StoreT>& CqBasicVec3<StoreT>::operator*=(const CqBasicVec3<T> &scale)
{
	x() *= scale.x();
	y() *= scale.y();
	z() *= scale.z();
	return *this;
}

template<typename StoreT>
template<typename T>
inline CqBasicVec3<StoreT>& CqBasicVec3<StoreT>::operator/=(const CqBasicVec3<T> &scale)
{
	x() /= scale.x();
	y() /= scale.y();
	z() /= scale.z();
	return *this;
}

template<typename StoreT>
template<typename T>
inline CqBasicVec3<StoreT>& CqBasicVec3<StoreT>::operator%=(const CqBasicVec3<T> &vec)
{
	const CqVec3 tmp(*this);
	x() = tmp.y()*vec.z() - tmp.z()*vec.y();
	y() = tmp.z()*vec.x() - tmp.x()*vec.z();
	z() = tmp.x()*vec.y() - tmp.y()*vec.x();
	return ( *this );
}

//----------------------------------------
// opassign operators with floats as the rhs
template<typename StoreT>
inline CqBasicVec3<StoreT>& CqBasicVec3<StoreT>::operator+=(const TqFloat f)
{
	x() += f;
	y() += f;
	z() += f;
	return *this;
}

template<typename StoreT>
inline CqBasicVec3<StoreT>& CqBasicVec3<StoreT>::operator-=(const TqFloat f)
{
	x() -= f;
	y() -= f;
	z() -= f;
	return *this;
}

template<typename StoreT>
inline CqBasicVec3<StoreT>& CqBasicVec3<StoreT>::operator*=(const TqFloat scale)
{
	x() *= scale;
	y() *= scale;
	z() *= scale;
	return *this;
}

template<typename StoreT>
inline CqBasicVec3<StoreT>& CqBasicVec3<StoreT>::operator/=(const TqFloat scale)
{
	assert(scale != 0);
	(*this) *= (1/scale);
	return *this;
}

//----------------------------------------
// comparison operators
template<typename StoreT>
template<typename T>
inline bool CqBasicVec3<StoreT>::operator==(const CqBasicVec3<T> &v) const
{
	return (x() == v.x()) && (y() == v.y()) && (z() == v.z());
}
template<typename StoreT>
template<typename T>
inline bool CqBasicVec3<StoreT>::operator!=(const CqBasicVec3<T> &v) const
{
	return (x() != v.x()) || (y() != v.y()) || (z() != v.z());
}
template<typename StoreT>
template<typename T>
inline bool CqBasicVec3<StoreT>::operator>=(const CqBasicVec3<T> &v) const
{
	return (x() >= v.x()) && (y() >= v.y()) && (z() >= v.z());
}
template<typename StoreT>
template<typename T>
inline bool CqBasicVec3<StoreT>::operator<=(const CqBasicVec3<T> &v) const
{
	return (x() <= v.x()) && (y() <= v.y()) && (z() <= v.z());
}
template<typename StoreT>
template<typename T>
inline bool CqBasicVec3<StoreT>::operator>(const CqBasicVec3<T> &v) const
{
	return (x() > v.x()) && (y() > v.y()) && (z() > v.z());
}
template<typename StoreT>
template<typename T>
inline bool CqBasicVec3<StoreT>::operator<(const CqBasicVec3<T> &v) const
{
	return (x() < v.x()) && (y() < v.y()) && (z() < v.z());
}

//------------------------------------------------------------------------------
// helper functions

//----------------------------------------
// float/vector operators
template<typename T>
inline CqVec3 operator+(const TqFloat f, const CqBasicVec3<T>& v)
{
	return CqVec3(f + v.x(), f + v.y(), f + v.z());
}
template<typename T>
inline CqVec3 operator-(const TqFloat f, const CqBasicVec3<T>& v)
{
	return CqVec3(f - v.x(), f - v.y(), f - v.z());
}
template<typename T>
inline CqVec3 operator*(const TqFloat f, const CqBasicVec3<T>& v)
{
	return CqVec3(f * v.x(), f * v.y(), f * v.z());
}
template<typename T>
inline CqVec3 operator/(const TqFloat f, const CqBasicVec3<T>& v)
{
	return CqVec3(f / v.x(), f / v.y(), f / v.z());
}

template<typename T>
inline CqVec3 operator+(const CqBasicVec3<T>& v, const TqFloat f)
{
	return CqVec3(v) += f;
}
template<typename T>
inline CqVec3 operator-(const CqBasicVec3<T>& v, const TqFloat f)
{
	return CqVec3(v) -= f;
}
template<typename T>
inline CqVec3 operator*(const CqBasicVec3<T>& v, const TqFloat f)
{
	return CqVec3(v) *= f;
}
template<typename T>
inline CqVec3 operator/(const CqBasicVec3<T>& v, const TqFloat f)
{
	return CqVec3(v) /= f;
}


//----------------------------------------
// vector/vector arithmetic

template<typename T1, typename T2>
inline CqVec3 operator+(const CqBasicVec3<T1>& a, const CqBasicVec3<T2>& b)
{
	return CqVec3(a) += b;
}

template<typename T1, typename T2>
inline CqVec3 operator-(const CqBasicVec3<T1>& a, const CqBasicVec3<T2>& b)
{
	return CqVec3(a) -= b;
}

template<typename T1, typename T2>
inline CqVec3 operator/(const CqBasicVec3<T1>& a, const CqBasicVec3<T2>& b)
{
	return CqVec3(a) /= b;
}

template<typename T1, typename T2>
inline CqVec3 min(const CqBasicVec3<T1>& a, const CqBasicVec3<T2>& b)
{
	return CqVec3(min(a.x(), b.x()), min(a.y(), b.y()), min(a.z(), b.z()));
}

template<typename T1, typename T2>
inline CqVec3 max(const CqBasicVec3<T1>& a, const CqBasicVec3<T2>& b)
{
	return CqVec3(max(a.x(), b.x()), max(a.y(), b.y()), max(a.z(), b.z()));
}

template<typename T1, typename T2, typename T3>
inline CqVec3 clamp(const CqBasicVec3<T1>& v, const CqBasicVec3<T2>& min,
		const CqBasicVec3<T3>& max)
{
	return CqVec3(clamp(v.x(), min.x(), max.x()),
			clamp(v.y(), min.y(), max.y()),
			clamp(v.z(), min.z(), max.z()));
}

//----------------------------------------
// misc vector ops
template<typename T>
inline CqVec3 operator-(const CqBasicVec3<T>& v)
{
	return CqVec3(-v.x(), -v.y(), -v.z());
}

template<typename T1, typename T2>
inline CqVec3 lerp(TqFloat t, const CqBasicVec3<T1>& v0, const CqBasicVec3<T2>& v1)
{
	return CqVec3((1-t)*v0.x() + t*v1.x(),
			      (1-t)*v0.y() + t*v1.y(),
			      (1-t)*v0.z() + t*v1.z());
}

template<typename T1, typename T2>
inline TqFloat operator*(const CqBasicVec3<T1>& a, const CqBasicVec3<T2>& b)
{
	return a.x()*b.x() + a.y()*b.y() + a.z()*b.z();
}

template<typename T1, typename T2>
inline CqVec3 operator%(const CqBasicVec3<T1> &a, const CqBasicVec3<T2> &b)
{
	return CqVec3(
		a.y()*b.z() - a.z()*b.y(),
	    a.z()*b.x() - a.x()*b.z(),
	    a.x()*b.y() - a.y()*b.x()
	);
}

template<typename T>
inline std::ostream& operator<<(std::ostream &out, const CqBasicVec3<T> &v)
{
	out << v.x() << "," << v.y() << "," << v.z();
	return out;
}

template<typename T1, typename T2>
inline bool isClose(const CqBasicVec3<T1>& v1, const CqBasicVec3<T2>& v2, TqFloat tol)
{
	TqFloat diff2 = (v1 - v2).Magnitude2();
	TqFloat tol2 = tol*tol;
	return diff2 <= tol2*v1.Magnitude2() || diff2 <= tol2*v2.Magnitude2();
}


//-----------------------------------------------------------------------

} // namespace Aqsis

#endif // VECTOR3D_H_INCLUDED
