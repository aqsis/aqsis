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

#include <aqsis/aqsis.h>

#include <iostream>

#include <aqsis/math/math.h>

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
	/** \brief Get the eigenvalues of the matrix
	 *
	 * If the eigenvalues are complex rather than real, trigger an assert in
	 * debug mode.  In release mode, return the real part of the eigenvalues.
	 *
	 * The eigenvalues are guarenteed to be ordered such that that l1 >= l2.
	 *
	 * \param l1 - first eigenvalue (output variable)
	 * \param l2 - second eigenvalue (output variable)
	 */
	inline void eigenvalues(TqFloat& l1, TqFloat& l2) const;
	/** \brief Attempt to get a matrix which orthogonally diagonalizes this matrix.
	 *
	 * That is, find a matrix R such that if A is the matrix to diagonalize,
	 *
	 *   A = R * D * R^T
	 *
	 * where R is an orthogonal matrix, and D is the diagonal matrix, diag(l1,l2).
	 *
	 * \param l1 - first eigenvalue
	 * \param l2 - the second eigenvalue.
	 *
	 * The somewhat nasty behaviour of feeding eigenvalues are back into this
	 * function is used for efficiency, since we may want to compute the
	 * eigenvalues seperately, but at the same time avoid computing them again
	 * when computing the diagonalizing matrix.
	 *
	 * The behaviour of this function is undefined if the matrix is not symmetric.
	 */
	inline SqMatrix2D orthogDiagonalize(TqFloat l1, TqFloat l2) const;
	//@}
};

/// Stream insertion operator
inline std::ostream& operator<<(std::ostream& out, const SqMatrix2D& mat);

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
	return SqMatrix2D(a+f, b, c, d+f);
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
	assert(D != 0);
	if(D != 0)
		return SqMatrix2D(d/D, -b/D, -c/D, a/D);
	else
		return SqMatrix2D(1);
}
inline TqFloat SqMatrix2D::det() const
{
	return a*d - b*c;
}
inline SqMatrix2D SqMatrix2D::transpose() const
{
	return SqMatrix2D(a, c, b, d);
}

inline void SqMatrix2D::eigenvalues(TqFloat& l1, TqFloat& l2) const
{
	// Special-case formula for eigenvalues of a 2D matrix.  It simply boils
	// down to solving the quadratic equation
	//
	// l^2 - Tr(A)*l + det(A) = 0
	//
	// for l.
	TqFloat firstTerm = (a+d)*0.5;
	TqFloat secondTerm = (a-d)*(a-d) + 4*c*b;
	assert(secondTerm >= -std::numeric_limits<TqFloat>::epsilon());
	// For robustness, set secondTerm = 0 if it's negative.  This will get the
	// real part of the result.
	if(secondTerm < 0)
		secondTerm = 0;
	secondTerm = std::sqrt(secondTerm)*0.5;
	l1 = firstTerm + secondTerm;
	l2 = firstTerm - secondTerm;
}

inline SqMatrix2D SqMatrix2D::orthogDiagonalize(TqFloat l1, TqFloat l2) const
{
	// As usual, we construct the matrix from the two orthonormal eigenvectors.
	// These eigenvectors only exist if the matrix is symmetric, so assert
	// symmetry:
	assert( std::fabs((b - c)) <= 1e-5*std::fabs(c) ||
			std::fabs((b - c)) <= 1e-5*std::fabs(b) );
	if(l1 == l2)
	{
		// Special (easy) case for degenerate eigenvalues
		return SqMatrix2D(1, 0,
						  0, 1);
	}
	// Calculate first eigenvector.  [u1, u2] and [v1,v2] are two alternatives
	// for the eigenvector - we take the one with the larger length for
	// maximum numerical stability.
	TqFloat u1 = b;
	TqFloat u2 = l1-a;
	TqFloat uLen2 = u1*u1 + u2*u2;
	TqFloat v1 = l1-d;
	TqFloat v2 = c;
	TqFloat vLen2 = v1*v1 + v2*v2;
	if(vLen2 > uLen2)
	{
		u1 = v1;
		u2 = v2;
		uLen2 = vLen2;
	}
	TqFloat invLenU = 1/std::sqrt(uLen2);
	u1 *= invLenU;
	u2 *= invLenU;
	return SqMatrix2D(u1, -u2,
					  u2, u1);
}

inline std::ostream& operator<<(std::ostream& out, const SqMatrix2D& mat)
{
	out << "[" << mat.a << ", " << mat.b << ", "
		<< mat.c << ", " << mat.d << "]";
	return out;
}

} // namespace Aqsis

#endif // MATRIX2D_H_INCLUDED
