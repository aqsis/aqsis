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
 * \brief Minimal 2D matrix class.
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef MATRIX2D_H_INCLUDED
#define MATRIX2D_H_INCLUDED

namespace Aqsis
{

/** \brief A minimal 2D matrix class.
 *
 * This class considers vectors to be column vectors, and that vectors multipy
 * matrices on the right hand side.  This is the usual form taught in linear
 * algebra courses, *however*, it is the opposite from CqMatrix, which assumes
 * that vectors are row vectors.
 */
struct SqMatrix2D
{
	/** \brief Matrix components:
	 *
	 * The matrix looks like
	 *
	 *   [a b]
	 *   [c d]
	 */
	/// \todo Consider whether the components should be public or not.
	TqFloat a;
	TqFloat b;
	TqFloat c;
	TqFloat d;

	/// Construct a multiple of the identity.
	inline SqMatrix2D(TqFloat diag);
	/// Construct a diagonal matrix.
	inline SqMatrix2D(TqFloat a, TqFloat d);
	/// Construct a general matrix.
	inline SqMatrix2D(TqFloat a, TqFloat b, TqFloat c, TqFloat d);

	/// \name Addition operators
	//@{
	inline SqMatrix2D operator+(const SqMatrix2D& rhs) const;
	inline SqMatrix2D operator+(TqFloat f) const;
	friend inline SqMatrix2D operator+(TqFloat f, const SqMatrix2D& mat);

	inline SqMatrix2D& operator+=(const SqMatrix2D& rhs);
	//@}

	/// \name Multiplication operators
	//@{
	inline SqMatrix2D operator*(const SqMatrix2D& rhs) const;
	inline SqMatrix2D operator*(TqFloat mult) const;
	friend inline SqMatrix2D operator*(TqFloat mult, const SqMatrix2D& mat);
	//@}

	/// \name Properties and transformations of the matrix.
	//@{
	/// Return the inverse
	inline SqMatrix2D inv() const;
	/// Return the determinant.
	inline TqFloat det() const;
	/// Return the matrix transpose
	inline SqMatrix2D transpose() const;
	//@}
};


//==============================================================================
// Implentation details
//==============================================================================
// SqMatrix2D implementation

// Constructors
inline SqMatrix2D::SqMatrix2D(TqFloat diag)
	: a(diag), b(0), c(0), d(diag)
{ }
inline SqMatrix2D::SqMatrix2D(TqFloat a, TqFloat d)
	: a(a), b(0), c(0), d(d)
{ }
inline SqMatrix2D::SqMatrix2D(TqFloat a, TqFloat b, TqFloat c, TqFloat d)
	: a(a), b(b), c(c), d(d)
{ }

// Addition
inline SqMatrix2D SqMatrix2D::operator+(const SqMatrix2D& rhs) const
{
	return SqMatrix2D(a+rhs.a, b+rhs.b, c+rhs.c, d+rhs.d);
}
inline SqMatrix2D SqMatrix2D::operator+(TqFloat f) const
{
	return SqMatrix2D(a+f, b+f, c+f, d+f);
}
inline SqMatrix2D operator+(TqFloat f, const SqMatrix2D& mat)
{
	return mat+f;
}
inline SqMatrix2D& SqMatrix2D::operator+=(const SqMatrix2D& rhs)
{
	a += rhs.a;
	b += rhs.b;
	c += rhs.c;
	d += rhs.d;
	return *this;
}

// Multiplication
inline SqMatrix2D SqMatrix2D::operator*(const SqMatrix2D& rhs) const
{
	return SqMatrix2D(
			a*rhs.a + b*rhs.c, a*rhs.b + b*rhs.d,
			c*rhs.a + d*rhs.c, c*rhs.b + d*rhs.d
			);
}
inline SqMatrix2D SqMatrix2D::operator*(TqFloat mult) const
{
	return SqMatrix2D(a*mult, b*mult, c*mult, d*mult);
}
inline SqMatrix2D operator*(TqFloat mult, const SqMatrix2D& mat)
{
	return mat*mult;
}

// Inverse, determinant, etc.
inline SqMatrix2D SqMatrix2D::inv() const
{
	// There's a simple formula for the inverse of a 2D matrix.  We use this here.
	TqFloat D = det();
	return SqMatrix2D(d/D, -b/D, -c/D, a/D);
}
inline TqFloat SqMatrix2D::det() const
{
	return a*d - b*c;
}
inline SqMatrix2D SqMatrix2D::transpose() const
{
	return SqMatrix2D(a, c, b, d);
}


} // namespace Aqsis

#endif // MATRIX2D_H_INCLUDED
