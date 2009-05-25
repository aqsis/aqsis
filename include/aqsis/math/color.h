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
 * \brief Declares the CqColor class for handling generic 3 element colors.
 * \author Paul C. Gregory (pgregory@aqsis.org)
 */

#ifndef COLOR_H_INCLUDED
#define COLOR_H_INCLUDED

#include <aqsis/aqsis.h>

#include <iostream>

#include <aqsis/math/math.h>
#include <aqsis/math/vecfwd.h>
#include <aqsis/math/vectorstorage.h>

namespace Aqsis {

//-----------------------------------------------------------------------
/** \brief Class to store and manipulate three component color information.
 */
template<typename StoreT>
class CqBasicColor
{
	public:
		/// Construct a black color (r,g,b = 0)
		CqBasicColor();
		/// Construct a color with the given red,green,blue components.
		CqBasicColor(TqFloat r, TqFloat g, TqFloat b);
		/// Greyscale constructor (r,g,b = f)
		explicit CqBasicColor(TqFloat f);
		/// Copy (r,g,b) color componenets from an array
		explicit CqBasicColor(const TqFloat* array);
		/** \brief Copy value from an array or assign storage
		 * (semantics depends on the storage policy)
		 */
		explicit CqBasicColor(TqFloat* array);
		/// Copy from another color
		CqBasicColor(const CqBasicColor& rhs);
		/// Copy from another color
		template<typename T>
		CqBasicColor(const CqBasicColor<T>& rhs);

		/// \name Assignment operators
		//@{
		CqBasicColor& operator=(const CqBasicColor& rhs);
		template<typename T>
		CqBasicColor& operator=(const CqBasicColor<T>& rhs);
		//@}

		//@{
		/// Const component access
		TqFloat r() const;
		TqFloat g() const;
		TqFloat b() const;
		//@}

		//@{
		/// Assignable component access
		TqFloat& r();
		TqFloat& g();
		TqFloat& b();
		//@}

		//@{
		/** Color component setter methods.
		 * \deprecated Use the methods returning component references.
		 */
		void r(TqFloat r);
		void g(TqFloat g);
		void b(TqFloat b);
		//@}

		/// \name Index-based component access 
		//@{
		/** Indices outside the valid range 0-2 will cause an assertion in
		 * debug mode.
		 */
		TqFloat& operator[] (TqInt i);
		TqFloat operator[] (TqInt i) const;
		//@}

		/// \name Op-assign with colors as the rhs
		//@{
		/** Opassign functions with another color type on the right hand side
		 * causes the associated operation to be carried out componentwise.
		 */
		template<typename T> CqBasicColor& operator+=(const CqBasicColor<T> &rhs);
		template<typename T> CqBasicColor& operator-=(const CqBasicColor<T> &rhs);
		template<typename T> CqBasicColor& operator*=(const CqBasicColor<T> &scale);
		template<typename T> CqBasicColor& operator/=(const CqBasicColor<T> &scale);
		//@}

		/// \name Op-assign with floats as the rhs.
		//@{
		/** Opassign functions with floats on the right hand side cause the
		 * associated operation to be carried out on each component.
		 */
		CqBasicColor& operator+=(const TqFloat f);
		CqBasicColor& operator-=(const TqFloat f);
		CqBasicColor& operator*=(const TqFloat scale);
		CqBasicColor& operator/=(const TqFloat scale);
		//@}

		/// \name Comparison functions
		//@{
		/**
		 * Comparison operators return true when the associated comparison is
		 * true for all componenets at once, except for != which returns !(c1 == c2)
		 */
		template<typename T> bool operator==(const CqBasicColor<T> &rhs) const;
		template<typename T> bool operator!=(const CqBasicColor<T> &rhs) const;
		template<typename T> bool operator>=(const CqBasicColor<T> &rhs) const;
		template<typename T> bool operator<=(const CqBasicColor<T> &rhs) const;
		template<typename T> bool operator>(const CqBasicColor<T> &rhs) const;
		template<typename T> bool operator<(const CqBasicColor<T> &rhs) const;
		//@}

	private:
		StoreT m_data;
};

// Non-member operators & functions for colors

/// \name Float/color operators
//@{
/** The float is promoted to a greyscale color with equal components after
 * which operations are performed componentwise.
 */
template<typename T> CqColor operator+(const TqFloat f, const CqBasicColor<T>& c);
template<typename T> CqColor operator-(const TqFloat f, const CqBasicColor<T>& c);
template<typename T> CqColor operator*(const TqFloat f, const CqBasicColor<T>& c);
template<typename T> CqColor operator/(const TqFloat f, const CqBasicColor<T>& c);
template<typename T> CqColor operator+(const CqBasicColor<T>& c, const TqFloat f);
template<typename T> CqColor operator-(const CqBasicColor<T>& c, const TqFloat f);
template<typename T> CqColor operator*(const CqBasicColor<T>& c, const TqFloat f);
template<typename T> CqColor operator/(const CqBasicColor<T>& c, const TqFloat f);
//@}

/// \name color/color arithmetic
//@{
/// Componentwise operations on the two colors produce a third.
template<typename T1, typename T2>
CqColor operator+(const CqBasicColor<T1>& a, const CqBasicColor<T2>& b);
template<typename T1, typename T2>
CqColor operator-(const CqBasicColor<T1>& a, const CqBasicColor<T2>& b);
template<typename T1, typename T2>
CqColor operator*(const CqBasicColor<T1>& a, const CqBasicColor<T2>& b);
template<typename T1, typename T2>
CqColor operator/(const CqBasicColor<T1>& a, const CqBasicColor<T2>& b);
template<typename T1, typename T2>
CqColor min(const CqBasicColor<T1>& a, const CqBasicColor<T2>& b);
template<typename T1, typename T2>
CqColor max(const CqBasicColor<T1>& a, const CqBasicColor<T2>& b);
template<typename T1, typename T2, typename T3>
CqColor clamp(const CqBasicColor<T1>& c, const CqBasicColor<T2>& min, const CqBasicColor<T3>& max);
//@}

/// Negation
template<typename T>
CqColor operator-(const CqBasicColor<T>& c);
/** \brief Linearly interpolate between two colors
 *
 * \param t - interpolation parameter
 * \param c0 - color corresponding to t = 0
 * \param c1 - color corresponding to t = 1
 */
template<typename T1, typename T2>
CqColor lerp(const TqFloat t, const CqBasicColor<T1>& c0, const CqBasicColor<T2>& c1);

// Stream insertion
template<typename T>
std::ostream& operator<<(std::ostream &out, const CqBasicColor<T> &c);

/** \brief Determine whether two colours are equal to within some tolerance.
 *
 * This performs elementwise comparisons of the componenets using the float
 * version of isClose().  The colours are close whenever all thier componenets
 * are.
 *
 * \param c1, c2 - colours to compare
 * \param tol - tolerance for the comparison.
 */
template<typename T1, typename T2>
inline bool isClose(const CqBasicColor<T1>& c1, const CqBasicColor<T2>& c2,
		TqFloat tol = 10*std::numeric_limits<TqFloat>::epsilon());


//----------------------------------------
/// \name Color-space conversion functions.  These are restricted to 
//@{
AQSIS_MATH_SHARE CqColor rgbtohsv(const CqColor& col);
AQSIS_MATH_SHARE CqColor rgbtohsl(const CqColor& col);
AQSIS_MATH_SHARE CqColor rgbtoXYZ(const CqColor& col);
AQSIS_MATH_SHARE CqColor rgbtoxyY(const CqColor& col);
AQSIS_MATH_SHARE CqColor rgbtoYIQ(const CqColor& col);
AQSIS_MATH_SHARE CqColor hsvtorgb(const CqColor& col);
AQSIS_MATH_SHARE CqColor hsltorgb(const CqColor& col);
AQSIS_MATH_SHARE CqColor XYZtorgb(const CqColor& col);
AQSIS_MATH_SHARE CqColor xyYtorgb(const CqColor& col);
AQSIS_MATH_SHARE CqColor YIQtorgb(const CqColor& col);
//@}


//----------------------------------------
/// Static white color
AQSIS_MATH_SHARE extern const CqColor gColWhite;
/// Static black color
AQSIS_MATH_SHARE extern const CqColor gColBlack;



//==============================================================================
// Implementation details.
//==============================================================================
// CqBasicColor implementation

//----------------------------------------
// Constructors
template<typename StoreT>
inline CqBasicColor<StoreT>::CqBasicColor()
	: m_data(0,0,0)
{ }

template<typename StoreT>
inline CqBasicColor<StoreT>::CqBasicColor(TqFloat r, TqFloat g, TqFloat b)
	: m_data(r,g,b)
{ }

template<typename StoreT>
inline CqBasicColor<StoreT>::CqBasicColor(TqFloat f)
	: m_data(f,f,f)
{ }

template<typename StoreT>
inline CqBasicColor<StoreT>::CqBasicColor(const TqFloat* array)
	: m_data(array)
{ }

template<typename StoreT>
inline CqBasicColor<StoreT>::CqBasicColor(TqFloat* array)
	: m_data(array)
{ }

template<typename StoreT>
inline CqBasicColor<StoreT>::CqBasicColor(const CqBasicColor<StoreT>& rhs)
	: m_data(rhs.m_data)
{ }

template<typename StoreT>
template<typename T>
inline CqBasicColor<StoreT>::CqBasicColor(const CqBasicColor<T>& rhs)
	: m_data(rhs.r(), rhs.g(), rhs.b())
{ }

//----------------------------------------
// Assignment operators

// Need to override the default assignment operator; the templated version
// below doesn't seem to do so.
template<typename StoreT>
inline CqBasicColor<StoreT>& CqBasicColor<StoreT>::operator=(const CqBasicColor<StoreT>& rhs)
{
	r() = rhs.r();
	g() = rhs.g();
	b() = rhs.b();
	return *this;
}

template<typename StoreT>
template<typename T>
inline CqBasicColor<StoreT>& CqBasicColor<StoreT>::operator=(const CqBasicColor<T>& rhs)
{
	r() = rhs.r();
	g() = rhs.g();
	b() = rhs.b();
	return *this;
}

//----------------------------------------
// Component access
template<typename StoreT>
inline TqFloat CqBasicColor<StoreT>::r() const { return m_data[0]; }
template<typename StoreT>
inline TqFloat CqBasicColor<StoreT>::g() const { return m_data[1]; }
template<typename StoreT>
inline TqFloat CqBasicColor<StoreT>::b() const { return m_data[2]; }

template<typename StoreT>
inline TqFloat& CqBasicColor<StoreT>::r() { return m_data[0]; }
template<typename StoreT>
inline TqFloat& CqBasicColor<StoreT>::g() { return m_data[1]; }
template<typename StoreT>
inline TqFloat& CqBasicColor<StoreT>::b() { return m_data[2]; }

template<typename StoreT>
inline void CqBasicColor<StoreT>::r(TqFloat r) { m_data[0] = r; }
template<typename StoreT>
inline void CqBasicColor<StoreT>::g(TqFloat g) { m_data[1] = g; }
template<typename StoreT>
inline void CqBasicColor<StoreT>::b(TqFloat b) { m_data[2] = b; }

template<typename StoreT>
inline TqFloat& CqBasicColor<StoreT>::operator[](TqInt i)
{
	assert(i >= 0 && i <= 2);
	return m_data[i];
}
template<typename StoreT>
inline TqFloat CqBasicColor<StoreT>::operator[](TqInt i) const
{
	assert(i >= 0 && i <= 2);
	return m_data[i];
}

//----------------------------------------
// opassign operators with colors as the rhs.
template<typename StoreT>
template<typename T>
inline CqBasicColor<StoreT>& CqBasicColor<StoreT>::operator+=(const CqBasicColor<T> &col)
{
	r() += col.r();
	g() += col.g();
	b() += col.b();
	return *this;
}

template<typename StoreT>
template<typename T>
inline CqBasicColor<StoreT>& CqBasicColor<StoreT>::operator-=(const CqBasicColor<T> &col)
{
	r() -= col.r();
	g() -= col.g();
	b() -= col.b();
	return *this;
}

template<typename StoreT>
template<typename T>
inline CqBasicColor<StoreT>& CqBasicColor<StoreT>::operator*=(const CqBasicColor<T> &scale)
{
	r() *= scale.r();
	g() *= scale.g();
	b() *= scale.b();
	return *this;
}

template<typename StoreT>
template<typename T>
inline CqBasicColor<StoreT>& CqBasicColor<StoreT>::operator/=(const CqBasicColor<T> &scale)
{
	r() /= scale.r();
	g() /= scale.g();
	b() /= scale.b();
	return *this;
}

//----------------------------------------
// opassign operators with floats as the rhs
template<typename StoreT>
inline CqBasicColor<StoreT>& CqBasicColor<StoreT>::operator+=(const TqFloat f)
{
	r() += f;
	g() += f;
	b() += f;
	return *this;
}

template<typename StoreT>
inline CqBasicColor<StoreT>& CqBasicColor<StoreT>::operator-=(const TqFloat f)
{
	r() -= f;
	g() -= f;
	b() -= f;
	return *this;
}

template<typename StoreT>
inline CqBasicColor<StoreT>& CqBasicColor<StoreT>::operator*=(const TqFloat scale)
{
	r() *= scale;
	g() *= scale;
	b() *= scale;
	return *this;
}

template<typename StoreT>
inline CqBasicColor<StoreT>& CqBasicColor<StoreT>::operator/=(const TqFloat scale)
{
	assert(scale != 0);
	(*this) *= (1/scale);
	return *this;
}

//----------------------------------------
// comparison operators
template<typename StoreT>
template<typename T>
inline bool CqBasicColor<StoreT>::operator==(const CqBasicColor<T> &col) const
{
	return (r() == col.r()) && (g() == col.g()) && (b() == col.b());
}
template<typename StoreT>
template<typename T>
inline bool CqBasicColor<StoreT>::operator!=(const CqBasicColor<T> &col) const
{
	return (r() != col.r()) || (g() != col.g()) || (b() != col.b());
}
template<typename StoreT>
template<typename T>
inline bool CqBasicColor<StoreT>::operator>=(const CqBasicColor<T> &col) const
{
	return (r() >= col.r()) && (g() >= col.g()) && (b() >= col.b());
}
template<typename StoreT>
template<typename T>
inline bool CqBasicColor<StoreT>::operator<=(const CqBasicColor<T> &col) const
{
	return (r() <= col.r()) && (g() <= col.g()) && (b() <= col.b());
}
template<typename StoreT>
template<typename T>
inline bool CqBasicColor<StoreT>::operator>(const CqBasicColor<T> &col) const
{
	return (r() > col.r()) && (g() > col.g()) && (b() > col.b());
}
template<typename StoreT>
template<typename T>
inline bool CqBasicColor<StoreT>::operator<(const CqBasicColor<T> &col) const
{
	return (r() < col.r()) && (g() < col.g()) && (b() < col.b());
}

//------------------------------------------------------------------------------
// helper functions

//----------------------------------------
// float/color operators
template<typename T>
inline CqColor operator+(const TqFloat f, const CqBasicColor<T>& col)
{
	return CqColor(f + col.r(), f + col.g(), f + col.b());
}
template<typename T>
inline CqColor operator-(const TqFloat f, const CqBasicColor<T>& col)
{
	return CqColor(f - col.r(), f - col.g(), f - col.b());
}
template<typename T>
inline CqColor operator*(const TqFloat f, const CqBasicColor<T>& col)
{
	return CqColor(f * col.r(), f * col.g(), f * col.b());
}
template<typename T>
inline CqColor operator/(const TqFloat f, const CqBasicColor<T>& col)
{
	return CqColor(f / col.r(), f / col.g(), f / col.b());
}

template<typename T>
inline CqColor operator+(const CqBasicColor<T>& col, const TqFloat f)
{
	return CqColor(col) += f;
}
template<typename T>
inline CqColor operator-(const CqBasicColor<T>& col, const TqFloat f)
{
	return CqColor(col) -= f;
}
template<typename T>
inline CqColor operator*(const CqBasicColor<T>& col, const TqFloat f)
{
	return CqColor(col) *= f;
}
template<typename T>
inline CqColor operator/(const CqBasicColor<T>& col, const TqFloat f)
{
	return CqColor(col) /= f;
}

//----------------------------------------
// color/color arithmetic

template<typename T1, typename T2>
inline CqColor operator+(const CqBasicColor<T1>& a, const CqBasicColor<T2>& b)
{
	return CqColor(a) += b;
}

template<typename T1, typename T2>
inline CqColor operator-(const CqBasicColor<T1>& a, const CqBasicColor<T2>& b)
{
	return CqColor(a) -= b;
}

template<typename T1, typename T2>
inline CqColor operator*(const CqBasicColor<T1>& a, const CqBasicColor<T2>& b)
{
	return CqColor(a) *= b;
}

template<typename T1, typename T2>
inline CqColor operator/(const CqBasicColor<T1>& a, const CqBasicColor<T2>& b)
{
	return CqColor(a) /= b;
}

template<typename T1, typename T2>
inline CqColor min(const CqBasicColor<T1>& a, const CqBasicColor<T2>& b)
{
	return CqColor(min(a.r(), b.r()), min(a.g(), b.g()), min(a.b(), b.b()));
}

template<typename T1, typename T2>
inline CqColor max(const CqBasicColor<T1>& a, const CqBasicColor<T2>& b)
{
	return CqColor(max(a.r(), b.r()), max(a.g(), b.g()), max(a.b(), b.b()));
}

template<typename T1, typename T2, typename T3>
inline CqColor clamp(const CqBasicColor<T1>& col, const CqBasicColor<T2>& min,
		const CqBasicColor<T3>& max)
{
	return CqColor(clamp(col.r(), min.r(), max.r()),
			clamp(col.g(), min.g(), max.g()),
			clamp(col.b(), min.b(), max.b()));
}

//----------------------------------------
// misc color ops
template<typename T>
inline CqColor operator-(const CqBasicColor<T>& col)
{
	return CqColor(-col.r(), -col.g(), -col.b());
}

template<typename T1, typename T2>
inline CqColor lerp(TqFloat t, const CqBasicColor<T1>& c0, const CqBasicColor<T2>& c1)
{
	return CqColor((1-t)*c0.r() + t*c1.r(),
			      (1-t)*c0.g() + t*c1.g(),
			      (1-t)*c0.b() + t*c1.b());
}

template<typename T>
inline std::ostream& operator<<(std::ostream &out, const CqBasicColor<T> &col)
{
	out << col.r() << "," << col.g() << "," << col.b();
	return out;
}

template<typename T1, typename T2>
inline bool isClose(const CqBasicColor<T1>& c1, const CqBasicColor<T2>& c2, TqFloat tol)
{
	return isClose(c1.r(), c2.r(), tol)
		&& isClose(c1.g(), c2.g(), tol)
		&& isClose(c1.b(), c2.b(), tol);
}


} // namespace Aqsis

#endif
