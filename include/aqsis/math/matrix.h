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


/**
 * \file
 *
 * \brief Declares the CqMatrix 4D homogenous matrix class.
 * \author Paul C. Gregory (pgregory@aqsis.org)
 */

#ifndef MATRIX_H_INCLUDED
#define MATRIX_H_INCLUDED

#include <aqsis/aqsis.h>

#include <iostream>

#include <aqsis/math/vector3d.h>
#include <aqsis/math/vector4d.h>

namespace Aqsis {

//----------------------------------------------------------------------
/** \brief 4x4 matrix class for homogenous coordinates.
 *
 * Access to matrix elements is always row,column from 0.
 *
 * \todo Document the assumptions about the order of element access in matrix
 * multiplication etc.  The convention this matrix class uses is opposite from
 * that used in university maths courses: From the conventional mathematical
 * viewpoint, multiplication of a vector by this matrix on the right is
 * actually multiplication by a row vector on the left...
 */
class AQSIS_MATH_SHARE CqMatrix
{
	public:
		//--------------------------------------------------
		// Constructors

		/** \brief Construct the identity matrix.
		 */
		CqMatrix();
		/** \brief Construct a diagonal matrix
		 *
		 * \param xs - scale factor along x.
		 * \param ys - scale factor along y.
		 * \param zs - scale factor along z.
		 */
		CqMatrix( const TqFloat xs, const TqFloat ys, const TqFloat zs );
		/** \brief Construct a translation matrix.
		 *
		 * \param trans - The vector by which to translate.
		 */
		CqMatrix( const CqVector3D& trans );
		/** \brief Rotation matrix constructor
		 *
		 * \param angle - The angle to rotate by.
		 * \param axis - The axis about which to rotate.
		 */
		CqMatrix( const TqFloat angle, const CqVector3D axis );
		/** \brief Skew matrix constructor
		 *
		 * \param angle
		 * \param dx1, - dy1, dz1
		 * \param dx2, - dy2, dz2
		 */
		CqMatrix( const TqFloat angle,
		          const TqFloat dx1, const TqFloat dy1, const TqFloat dz1,
		          const TqFloat dx2, const TqFloat dy2, const TqFloat dz2 );
		/// Copy constructor
		CqMatrix( const CqMatrix &from );
		/** \brief Construct from a list of individual matrix elements.
		 *
		 * Takes 16 floats for the elements of the matrix.
		 */
		CqMatrix( const TqFloat r1c1, const TqFloat r1c2, const TqFloat r1c3, const TqFloat r1c4,
		          const TqFloat r2c1, const TqFloat r2c2, const TqFloat r2c3, const TqFloat r2c4,
		          const TqFloat r3c1, const TqFloat r3c2, const TqFloat r3c3, const TqFloat r3c4,
		          const TqFloat r4c1, const TqFloat r4c2, const TqFloat r4c3, const TqFloat r4c4 );
		/** \brief Constructor from a 2D float array of matrix elements
		 *
		 * \param from - 2D float array to copy data from.
		 */
		CqMatrix( const TqFloat from[ 4 ][ 4 ] );
		/** \brief Constructor from a flattened array of matrix elements
		 *
		 * \param from - 1D float array to copy data from.
		 */
		CqMatrix( const TqFloat from[ 16 ] );
		/** \brief Construct a scaled 4x4 identity from a float
		 *
		 * \param f - amount to scale the identity matrix by
		 */
		CqMatrix( TqFloat f );

		//--------------------------------------------------
		/// \name Special-case identity operations
		//@{
		/// Turn this matrix into an identity.
		void Identity();
		/** \brief Mark this matrix as identity or not.
		 *
		 * \param f - boolean indicating whether or not this matrix should be
		 * considered identity irespective of its contents.
		 */
		void SetfIdentity( bool f );
		/// Return whether or not the matrix is the identity
		bool fIdentity() const;
		//@}

		//--------------------------------------------------
		/// \name Methods for concatenating transformations onto the current one.
		//@{
		/** \brief Scale matrix uniformly in all three axes.
		 * \param S - amount to scale by.
		 */
		void Scale( const TqFloat S );
		/** \brief Scale matrix in three axes.
		 * \param xs - X scale factor.
		 * \param ys - Y scale factor.
		 * \param zs - Z scale factor.
		 */
		void Scale( const TqFloat xs, const TqFloat ys, const TqFloat zs );
		/** \brief Rotate this matrix by an angle about an axis through the origin.
		 * \param angle - The angle to rotate by.
		 * \param axis - The axis about which to rotate.
		 */
		void Rotate( const TqFloat angle, const CqVector3D axis );
		/** Translates this matrix by a given vector.
		 * \param trans - The vector by which to translate.
		 */
		void Translate( const CqVector3D& trans );
		/** Translates this matrix by three axis distances.
		 * \param xt - X distance to translate.
		 * \param yt - Y distance to translate.
		 * \param zt - Z distance to translate.
		 */
		void Translate( const TqFloat xt, const TqFloat yt, const TqFloat zt );
		/** Shears this matrix's X axis according to two shear factors, yh and zh
		 * \param yh - Y shear factor.
		 * \param zh - Z shear factor.
		 */
		void ShearX( const TqFloat yh, const TqFloat zh );
		/** Shears this matrix's Y axis according to two shear factors, xh and zh
		 * \param xh - X shear factor.
		 * \param zh - Z shear factor.
		 */
		void ShearY( const TqFloat xh, const TqFloat zh );
		/** Shears this matrix's Z axis according to two shear factors, xh and yh
		 * \param xh - X shear factor.
		 * \param yh - Y shear factor.
		 */
		void ShearZ( const TqFloat xh, const TqFloat yh );
		/** Skew matrix
		 * \param angle - The angle by which to skew the transformation.
		 * \param dx1 
		 * \param dy1
		 * \param dz1 - First vector controling skew direction
		 * \param dx2
		 * \param dy2
		 * \param dz2 - Second vector controling skew direction
		 */
		void Skew( const TqFloat angle,
		           const TqFloat dx1, const TqFloat dy1, const TqFloat dz1,
		           const TqFloat dx2, const TqFloat dy2, const TqFloat dz2 );
		/** \brief Normalise the matrix, so that the homogenous part of the matrix is 1.
		 * \todo code review might be removed since not used in codebase
		 */
		void Normalise();
		//@}

		//--------------------------------------------------
		/// \name Access to matrix elements
		//@{
		/** \brief Get the element at the specified row and column index.
		 * \param row - The row index.
		 * \param col - The column index.
		 * \return Float value.
		 */
		TqFloat Element(TqInt row, TqInt col) const;
		/** \brief Set the element at the specified row and column index.
		 * \param row - The row index.
		 * \param col - The column index.
		 * \param fValue - the value to insert.
		 */
		void SetElement(TqInt row, TqInt col, TqFloat fValue);
		/** \brief Get a pointer to the row index specified.
		 * \param row - The row index.
		 * \return Pointer to array of 4 float values.
		 */
		TqFloat* operator[](TqInt row);
		/** \brief Get a read only pointer to the row index specified.
		 * \param row - The row index.
		 * \return Pointer to array of 4 float values.
		 */
		const TqFloat* operator[](TqInt row) const;
		/** \brief Get a pointer to matrix data.
		 * \return Pointer to array of 16 float values.
		 */
		TqFloat* pElements();
		/** \brief Get a read only pointer to matrix data.
		 * \return Pointer to array of 16 float values.
		 */
		const TqFloat* pElements() const;
		//@}

		//--------------------------------------------------
		/// \name Operators and functions for matrix arithmetic.
		//@{
		/** \brief Multiply two matrices together.
		 * \param from - The matrix to multiply with this matrix.
		 * \return The resultant multiplied matrix.
		 */
		CqMatrix operator*(const CqMatrix &from) const;
		/** \brief Apply scale matrix uniformly in all three axes.
		 * \param S - The amount by which to scale matrix.
		 * \return Result of scaling this matrix by S.
		 */
		CqMatrix operator*(const TqFloat S) const;
		/** \brief Multiply a vector by this matrix.
		 * \param vec - The vector to multiply.
		 * \return The result of multiplying the vector by this matrix.
		 */
		CqVector4D operator*(const CqVector4D &vec) const;
		/** \brief Premultiplies this matrix by a vector, returning v*m.
		 *
		 * This is the same as postmultiply the transpose of m by a vector: T(m)*v
		 * \param vec - The vector to multiply.
		 */
		CqVector4D PreMultiply(const CqVector4D &vec) const;
		/** \brief Multiply a 3D vector by this matrix.
		 *
		 * The meaning of this is to first construct a 4D vector corresponding
		 * to the homogeneous coordinates: [vec, 1], and multiply that by the
		 * matrix instead.  We then rescale the result such that the fourth
		 * component again has value 1.  This allows us to represent
		 * perspective transformations via matrix multiplication in homogeneous
		 * coordinates.
		 *
		 * \param vec - The vector to multiply.
		 */
		CqVector3D operator*(const CqVector3D &vec) const;
		/** \brief Add two matrices.
		 * \param from - The matrix to add.
		 * \return Result of adding from to this matrix.
		 */
		CqMatrix operator+(const CqMatrix &from) const;
		/** \brief Subtract two matrices.
		 * \param from - The matrix to subtract.
		 * \return Result of subtracting from from this matrix.
		 */
		CqMatrix operator-(const CqMatrix &from) const;

		/** \brief Return the inverse of this matrix.
		 *
		 * Uses Gauss-Jordan elimination with partial pivoting (see for
		 * example, Graphics Gems IV (p554)).
		 */
		CqMatrix Inverse() const;
		/** \brief Returns the transpose of this matrix
		 */
		CqMatrix Transpose() const;
		/** \brief Returns the determinant of this matrix
		 *
		 * The algorithm used is simple cofactor expansion.  See, for example,
		 * Graphics Gems I (p768).
		 */
		TqFloat Determinant() const;

		/** \brief assignment operator
		 * \param from - matrix to copy information from.
		 */
		CqMatrix& operator=( const CqMatrix &from );
		/** \brief Copy function.
		 * \param from - Renderman matrix to copy information from.
		 */
		CqMatrix& operator=( const TqFloat from[ 4 ][ 4 ] );
		/** \brief Copy function.
		 * \param from - Renderman matrix to copy information from.
		 */
		CqMatrix& operator=( const TqFloat from[ 16 ] );
		/** \brief Add a given matrix to this matrix.
		 * \param from - The matrix to add.
		 */
		CqMatrix& operator+=( const CqMatrix &from );
		/** \brief Subtract a given matrix from this matrix.
		 * \param from - The matrix to subtract.
		 */
		CqMatrix& operator-=( const CqMatrix &from );
		/** \brief Multiply this matrix by specified matrix.
		 *
		 * This takes into account the types of matrices, in an attempt to
		 * speed it up.
		 * \param from - The matrix to multiply with this matrix.
		 */
		CqMatrix& operator*=( const CqMatrix &from );
		/** \brief Matrix multiplication of the form a = b * a.
		 * \param from - The matrix to multiply with this matrix.
		 */
		CqMatrix& PreMultiply( const CqMatrix &from );
		/** \brief Apply scale matrix uniformly in all three axes to this matrix.
		 * \param S - The amount by which to scale this matrix.
		 * \return The result scaling this matrix by S.
		 */
		CqMatrix& operator*=( const TqFloat S );
		/** \brief Scale each element by the specified value.
		 * \param S - The amount by which to scale the matrix elements.
		 * \param a - The matrix to be scaled.
		 * \return Result of scaling this matrix by S.
		 */
		AQSIS_MATH_SHARE friend CqMatrix operator*( TqFloat S, const CqMatrix& a );
		//@}

		//--------------------------------------------------
		/// \name Other operators
		//@{
		/** \brief Compare two matrices.
		 * \param A - One matrix to be compared.
		 * \param B - Second matrix to be compared with.
		 * \return Result if matrices are equal or not.
		 *
		 * \todo code review Direct floating point comparison is rarely a good
		 * idea.  We probably need some kind of comparison tolerance in here.
		 */
		AQSIS_MATH_SHARE friend bool  operator==(const CqMatrix& A, const CqMatrix& B);
		/// Return !(A == B)
		AQSIS_MATH_SHARE friend bool  operator!=(const CqMatrix& A, const CqMatrix& B);
		/** \brief Outputs a matrix to an output stream.
		 * \param Stream - Stream to output the matrix to.
		 * \param matrix - The matrix to output.
		 * \return The new state of Stream.
		 */
		AQSIS_MATH_SHARE friend std::ostream &operator<<(std::ostream &Stream,
				const CqMatrix &matrix);
		//@}

	protected:
		/// The 4x4 array of float values.
		TqFloat m_elements[ 4 ][ 4 ];
		/** Flag indicating that this matrix should be treated as identity
		 * irrespective of its contents.
		 */
		bool m_fIdentity;
};

/// Premultiply matrix by vector.
AQSIS_MATH_SHARE CqVector4D operator*( const CqVector4D &vec, const CqMatrix& matrix );

//------------------------------------------------------------------------------
/** \brief Determine whether two matrices are equal to within some tolerance
 *
 * The closeness criterion for the matrices is based on the euclidian norm -
 * ie, the usual distance function between two vectors (sum of squares of the
 * differences between m1 and m2)
 *
 * \param m1
 * \param m2 - matrices to compare
 * \param tolerance - for comparison
 */
AQSIS_MATH_SHARE bool isClose(const CqMatrix& m1, const CqMatrix& m2,
		TqFloat tol = 10*std::numeric_limits<TqFloat>::epsilon());


/// Get the vector transformation associated with the point transformation, m
///
/// The vector transform is the point transform without the translation part
/// of m.
CqMatrix vectorTransform(const CqMatrix& m);

/// Get the normal transformation associated with the point transformation, m
///
/// The normal transform is the same as the vector transform for orthogonal
/// transformations but more complicated in the general case.
CqMatrix normalTransform(const CqMatrix& m);

//==============================================================================
// Implementation details
//==============================================================================

// Constructors
inline CqMatrix::CqMatrix( )
{
	Identity();
}

// construct a diagonal matrix
inline CqMatrix::CqMatrix( const TqFloat xs, const TqFloat ys, const TqFloat zs )
{
	Identity();

	if ( xs != 1.0f || ys != 1.0f || zs != 1.0f )
	{
		m_elements[ 0 ][ 0 ] = xs;
		m_elements[ 1 ][ 1 ] = ys;
		m_elements[ 2 ][ 2 ] = zs;
		m_elements[ 3 ][ 3 ] = 1.0;

		m_fIdentity = false;
	}
}

// Construct a translation matrix
inline CqMatrix::CqMatrix( const CqVector3D& trans )
{
	Identity();

	if ( trans.x() != 0.0f || trans.y() != 0.0f || trans.z() != 0.0f )
	{
		m_fIdentity = false;

		m_elements[ 3 ][ 0 ] = trans.x();
		m_elements[ 3 ][ 1 ] = trans.y();
		m_elements[ 3 ][ 2 ] = trans.z();
	}
}

// Construct a rotation matrix
inline CqMatrix::CqMatrix( const TqFloat angle, const CqVector3D axis )
{
	Identity();

	if ( angle != 0.0f && axis.Magnitude() != 0.0f )
		Rotate( angle, axis );
}

inline CqMatrix::CqMatrix( const CqMatrix &from )
{
	*this = from;
}

inline CqMatrix::CqMatrix(
		const TqFloat r1c1, const TqFloat r1c2, const TqFloat r1c3, const TqFloat r1c4,
		const TqFloat r2c1, const TqFloat r2c2, const TqFloat r2c3, const TqFloat r2c4,
		const TqFloat r3c1, const TqFloat r3c2, const TqFloat r3c3, const TqFloat r3c4,
		const TqFloat r4c1, const TqFloat r4c2, const TqFloat r4c3, const TqFloat r4c4 )
	: m_fIdentity(false)
{
	m_elements[ 0 ][ 0 ] = r1c1;
	m_elements[ 0 ][ 1 ] = r1c2;
	m_elements[ 0 ][ 2 ] = r1c3;
	m_elements[ 0 ][ 3 ] = r1c4;
	m_elements[ 1 ][ 0 ] = r2c1;
	m_elements[ 1 ][ 1 ] = r2c2;
	m_elements[ 1 ][ 2 ] = r2c3;
	m_elements[ 1 ][ 3 ] = r2c4;
	m_elements[ 2 ][ 0 ] = r3c1;
	m_elements[ 2 ][ 1 ] = r3c2;
	m_elements[ 2 ][ 2 ] = r3c3;
	m_elements[ 2 ][ 3 ] = r3c4;
	m_elements[ 3 ][ 0 ] = r4c1;
	m_elements[ 3 ][ 1 ] = r4c2;
	m_elements[ 3 ][ 2 ] = r4c3;
	m_elements[ 3 ][ 3 ] = r4c4;
}

inline CqMatrix::CqMatrix( const TqFloat from[ 4 ][ 4 ] )
{
	*this = from;
}

inline CqMatrix::CqMatrix( const TqFloat from[ 16 ] )
{
	*this = from;
}

inline CqMatrix::CqMatrix( TqFloat f )
{
  Identity();
  if(f != 1.0)
  {
    m_elements[ 0 ][ 0 ] = f;
    m_elements[ 1 ][ 1 ] = f;
    m_elements[ 2 ][ 2 ] = f;
    m_elements[ 3 ][ 3 ] = f;
    m_fIdentity = false;
  }
}

//------------------------------------------------------------------------------
// Special-case identity handling
AQSIS_MATH_SHARE inline void CqMatrix::Identity()
{
	m_fIdentity = true;

	m_elements[ 0 ][ 1 ]
		= m_elements[ 0 ][ 2 ]
		= m_elements[ 0 ][ 3 ]
		= m_elements[ 1 ][ 0 ]
		= m_elements[ 1 ][ 2 ]
		= m_elements[ 1 ][ 3 ]
		= m_elements[ 2 ][ 0 ]
		= m_elements[ 2 ][ 1 ]
		= m_elements[ 2 ][ 3 ]
		= m_elements[ 3 ][ 0 ]
		= m_elements[ 3 ][ 1 ]
		= m_elements[ 3 ][ 2 ]
		= 0.0f;

	m_elements[ 0 ][ 0 ]
		= m_elements[ 1 ][ 1 ]
		= m_elements[ 2 ][ 2 ]
		= m_elements[ 3 ][ 3 ]
		= 1.0f;
}

inline void CqMatrix::SetfIdentity( bool f )
{
	m_fIdentity = f;
}

inline bool CqMatrix::fIdentity() const
{
	return m_fIdentity;
}

//------------------------------------------------------------------------------
// Methods for concatenating transformations onto the current one.
inline void CqMatrix::Scale( const TqFloat S )
{
	if ( S != 1.0f )
		Scale( S, S, S );
}

inline void CqMatrix::Scale( const TqFloat xs, const TqFloat ys, const TqFloat zs )
{
	CqMatrix Scale( xs, ys, zs );
	this->PreMultiply( Scale );
}

inline void CqMatrix::Translate( const CqVector3D& trans )
{
	CqMatrix matTrans( trans );
	this->PreMultiply( matTrans );
}

inline void CqMatrix::Translate( const TqFloat xt, const TqFloat yt, const TqFloat zt )
{
	if ( xt != 0.0f || yt != 0.0f || zt != 0.0f )
		Translate( CqVector3D( xt, yt, zt ) );
}

inline void CqMatrix::ShearX( const TqFloat yh, const TqFloat zh )
{
	CqMatrix Shear;
	Shear.m_fIdentity = false;

	Shear.m_elements[ 0 ][ 1 ] = yh;
	Shear.m_elements[ 0 ][ 2 ] = zh;

	this->PreMultiply( Shear );
}

inline void CqMatrix::ShearY( const TqFloat xh, const TqFloat zh )
{
	CqMatrix Shear;
	Shear.m_fIdentity = false;

	Shear.m_elements[ 1 ][ 0 ] = xh;
	Shear.m_elements[ 1 ][ 2 ] = zh;

	this->PreMultiply( Shear );
}

inline void CqMatrix::ShearZ( const TqFloat xh, const TqFloat yh )
{
	CqMatrix Shear;
	Shear.m_fIdentity = false;

	Shear.m_elements[ 2 ][ 0 ] = xh;
	Shear.m_elements[ 2 ][ 1 ] = yh;

	this->PreMultiply( Shear );
}

inline void CqMatrix::Skew( const TqFloat angle,
                     const TqFloat dx1, const TqFloat dy1, const TqFloat dz1,
                     const TqFloat dx2, const TqFloat dy2, const TqFloat dz2 )
{
	CqMatrix Skew( angle, dx1, dy1, dz1, dx2, dy2, dz2 );

	this->PreMultiply( Skew );
}

//------------------------------------------------------------------------------
// Access to matrix elements.
inline TqFloat CqMatrix::Element( TqInt row, TqInt col ) const
{
	return ( m_elements[ row ][ col ] );
}

inline void CqMatrix::SetElement( TqInt row, TqInt col, TqFloat fValue )
{
	m_elements[ row ][ col ] = fValue;
}

inline TqFloat* CqMatrix::operator[] ( TqInt row )
{
	return( &m_elements[ row ][ 0 ] );
}

inline const TqFloat* CqMatrix::operator[] ( TqInt row ) const
{
	return( &m_elements[ row ][ 0 ] );
}

inline TqFloat* CqMatrix::pElements()
{
	return ( &m_elements[ 0 ][ 0 ] );
}

inline const TqFloat* CqMatrix::pElements() const
{
	return ( &m_elements[ 0 ][ 0 ] );
}

//------------------------------------------------------------------------------
// Operators and functions for matrices arithmetic.

inline CqMatrix CqMatrix::operator*( const CqMatrix &from ) const
{
	CqMatrix temp( *this );
	temp *= from;
	return temp;
}

inline CqVector4D CqMatrix::PreMultiply( const CqVector4D &vec ) const
{
	if ( m_fIdentity )
		return vec;

	CqVector4D	Result;

	Result.x( m_elements[ 0 ][ 0 ] * vec.x()
	          + m_elements[ 0 ][ 1 ] * vec.y()
	          + m_elements[ 0 ][ 2 ] * vec.z()
	          + m_elements[ 0 ][ 3 ] * vec.h() );

	Result.y( m_elements[ 1 ][ 0 ] * vec.x()
	          + m_elements[ 1 ][ 1 ] * vec.y()
	          + m_elements[ 1 ][ 2 ] * vec.z()
	          + m_elements[ 1 ][ 3 ] * vec.h() );

	Result.z( m_elements[ 2 ][ 0 ] * vec.x()
	          + m_elements[ 2 ][ 1 ] * vec.y()
	          + m_elements[ 2 ][ 2 ] * vec.z()
	          + m_elements[ 2 ][ 3 ] * vec.h() );

	Result.h( m_elements[ 3 ][ 0 ] * vec.x()
	          + m_elements[ 3 ][ 1 ] * vec.y()
	          + m_elements[ 3 ][ 2 ] * vec.z()
	          + m_elements[ 3 ][ 3 ] * vec.h() );

	return Result;
}

inline CqMatrix CqMatrix::operator*( const TqFloat S ) const
{
	CqMatrix temp( *this );
	temp *= S;
	return temp;
}

inline CqMatrix &CqMatrix::operator*=( const TqFloat S )
{
	CqMatrix ScaleMatrix( S, S, S );
	this->PreMultiply( ScaleMatrix );
	return *this;
}

inline CqVector4D CqMatrix::operator*( const CqVector4D &vec ) const
{
	if ( m_fIdentity )
		return vec;

	CqVector4D	Result;

	Result.x( m_elements[ 0 ][ 0 ] * vec.x()
	          + m_elements[ 1 ][ 0 ] * vec.y()
	          + m_elements[ 2 ][ 0 ] * vec.z()
	          + m_elements[ 3 ][ 0 ] * vec.h() );

	Result.y( m_elements[ 0 ][ 1 ] * vec.x()
	          + m_elements[ 1 ][ 1 ] * vec.y()
	          + m_elements[ 2 ][ 1 ] * vec.z()
	          + m_elements[ 3 ][ 1 ] * vec.h() );

	Result.z( m_elements[ 0 ][ 2 ] * vec.x()
	          + m_elements[ 1 ][ 2 ] * vec.y()
	          + m_elements[ 2 ][ 2 ] * vec.z()
	          + m_elements[ 3 ][ 2 ] * vec.h() );

	Result.h( m_elements[ 0 ][ 3 ] * vec.x()
	          + m_elements[ 1 ][ 3 ] * vec.y()
	          + m_elements[ 2 ][ 3 ] * vec.z()
	          + m_elements[ 3 ][ 3 ] * vec.h() );

	return Result;
}

inline CqVector3D CqMatrix::operator*( const CqVector3D &vec ) const
{
	if ( m_fIdentity )
		return vec;

	CqVector3D	Result;
	TqFloat h = ( m_elements[ 0 ][ 3 ] * vec.x()
	              + m_elements[ 1 ][ 3 ] * vec.y()
	              + m_elements[ 2 ][ 3 ] * vec.z()
	              + m_elements[ 3 ][ 3 ] );
	Result.x( ( m_elements[ 0 ][ 0 ] * vec.x()
	            + m_elements[ 1 ][ 0 ] * vec.y()
	            + m_elements[ 2 ][ 0 ] * vec.z()
	            + m_elements[ 3 ][ 0 ] ) );
	Result.y( ( m_elements[ 0 ][ 1 ] * vec.x()
	            + m_elements[ 1 ][ 1 ] * vec.y()
	            + m_elements[ 2 ][ 1 ] * vec.z()
	            + m_elements[ 3 ][ 1 ] ) );
	Result.z( ( m_elements[ 0 ][ 2 ] * vec.x()
	            + m_elements[ 1 ][ 2 ] * vec.y()
	            + m_elements[ 2 ][ 2 ] * vec.z()
	            + m_elements[ 3 ][ 2 ] ) );

	if(h != 1)
	{
		assert(h != 0);
		TqFloat invh = 1/h;
		Result.x(Result.x()*invh);
		Result.y(Result.y()*invh);
		Result.z(Result.z()*invh);
	}

	return Result;
}

inline CqMatrix CqMatrix::operator+( const CqMatrix &from ) const
{
	CqMatrix temp( *this );
	temp += from;
	return temp;
}

inline CqMatrix &CqMatrix::operator+=( const CqMatrix &from )
{
	m_elements[ 0 ][ 0 ] += from.m_elements[ 0 ][ 0 ];
	m_elements[ 1 ][ 0 ] += from.m_elements[ 1 ][ 0 ];
	m_elements[ 2 ][ 0 ] += from.m_elements[ 2 ][ 0 ];
	m_elements[ 3 ][ 0 ] += from.m_elements[ 3 ][ 0 ];
	m_elements[ 0 ][ 1 ] += from.m_elements[ 0 ][ 1 ];
	m_elements[ 1 ][ 1 ] += from.m_elements[ 1 ][ 1 ];
	m_elements[ 2 ][ 1 ] += from.m_elements[ 2 ][ 1 ];
	m_elements[ 3 ][ 1 ] += from.m_elements[ 3 ][ 1 ];
	m_elements[ 0 ][ 2 ] += from.m_elements[ 0 ][ 2 ];
	m_elements[ 1 ][ 2 ] += from.m_elements[ 1 ][ 2 ];
	m_elements[ 2 ][ 2 ] += from.m_elements[ 2 ][ 2 ];
	m_elements[ 3 ][ 2 ] += from.m_elements[ 3 ][ 2 ];
	m_elements[ 0 ][ 3 ] += from.m_elements[ 0 ][ 3 ];
	m_elements[ 1 ][ 3 ] += from.m_elements[ 1 ][ 3 ];
	m_elements[ 2 ][ 3 ] += from.m_elements[ 2 ][ 3 ];
	m_elements[ 3 ][ 3 ] += from.m_elements[ 3 ][ 3 ];

	m_fIdentity = false;

	return *this;
}

inline CqMatrix CqMatrix::operator-( const CqMatrix &from ) const
{
	CqMatrix temp( *this );
	temp -= from;
	return temp;
}

inline CqMatrix &CqMatrix::operator-=( const CqMatrix &from )
{
	m_elements[ 0 ][ 0 ] -= from.m_elements[ 0 ][ 0 ];
	m_elements[ 1 ][ 0 ] -= from.m_elements[ 1 ][ 0 ];
	m_elements[ 2 ][ 0 ] -= from.m_elements[ 2 ][ 0 ];
	m_elements[ 3 ][ 0 ] -= from.m_elements[ 3 ][ 0 ];
	m_elements[ 0 ][ 1 ] -= from.m_elements[ 0 ][ 1 ];
	m_elements[ 1 ][ 1 ] -= from.m_elements[ 1 ][ 1 ];
	m_elements[ 2 ][ 1 ] -= from.m_elements[ 2 ][ 1 ];
	m_elements[ 3 ][ 1 ] -= from.m_elements[ 3 ][ 1 ];
	m_elements[ 0 ][ 2 ] -= from.m_elements[ 0 ][ 2 ];
	m_elements[ 1 ][ 2 ] -= from.m_elements[ 1 ][ 2 ];
	m_elements[ 2 ][ 2 ] -= from.m_elements[ 2 ][ 2 ];
	m_elements[ 3 ][ 2 ] -= from.m_elements[ 3 ][ 2 ];
	m_elements[ 0 ][ 3 ] -= from.m_elements[ 0 ][ 3 ];
	m_elements[ 1 ][ 3 ] -= from.m_elements[ 1 ][ 3 ];
	m_elements[ 2 ][ 3 ] -= from.m_elements[ 2 ][ 3 ];
	m_elements[ 3 ][ 3 ] -= from.m_elements[ 3 ][ 3 ];

	m_fIdentity = false;

	return *this;
}

inline CqMatrix &CqMatrix::operator=( const CqMatrix &from )
{
	m_elements[ 0 ][ 0 ] = from.m_elements[ 0 ][ 0 ];
	m_elements[ 1 ][ 0 ] = from.m_elements[ 1 ][ 0 ];
	m_elements[ 2 ][ 0 ] = from.m_elements[ 2 ][ 0 ];
	m_elements[ 3 ][ 0 ] = from.m_elements[ 3 ][ 0 ];
	m_elements[ 0 ][ 1 ] = from.m_elements[ 0 ][ 1 ];
	m_elements[ 1 ][ 1 ] = from.m_elements[ 1 ][ 1 ];
	m_elements[ 2 ][ 1 ] = from.m_elements[ 2 ][ 1 ];
	m_elements[ 3 ][ 1 ] = from.m_elements[ 3 ][ 1 ];
	m_elements[ 0 ][ 2 ] = from.m_elements[ 0 ][ 2 ];
	m_elements[ 1 ][ 2 ] = from.m_elements[ 1 ][ 2 ];
	m_elements[ 2 ][ 2 ] = from.m_elements[ 2 ][ 2 ];
	m_elements[ 3 ][ 2 ] = from.m_elements[ 3 ][ 2 ];
	m_elements[ 0 ][ 3 ] = from.m_elements[ 0 ][ 3 ];
	m_elements[ 1 ][ 3 ] = from.m_elements[ 1 ][ 3 ];
	m_elements[ 2 ][ 3 ] = from.m_elements[ 2 ][ 3 ];
	m_elements[ 3 ][ 3 ] = from.m_elements[ 3 ][ 3 ];

	m_fIdentity = from.m_fIdentity;

	return *this;
}

inline CqMatrix &CqMatrix::operator=( const TqFloat from[ 4 ][ 4 ] )
{
	m_elements[ 0 ][ 0 ] = from[ 0 ][ 0 ];
	m_elements[ 1 ][ 0 ] = from[ 1 ][ 0 ];
	m_elements[ 2 ][ 0 ] = from[ 2 ][ 0 ];
	m_elements[ 3 ][ 0 ] = from[ 3 ][ 0 ];
	m_elements[ 0 ][ 1 ] = from[ 0 ][ 1 ];
	m_elements[ 1 ][ 1 ] = from[ 1 ][ 1 ];
	m_elements[ 2 ][ 1 ] = from[ 2 ][ 1 ];
	m_elements[ 3 ][ 1 ] = from[ 3 ][ 1 ];
	m_elements[ 0 ][ 2 ] = from[ 0 ][ 2 ];
	m_elements[ 1 ][ 2 ] = from[ 1 ][ 2 ];
	m_elements[ 2 ][ 2 ] = from[ 2 ][ 2 ];
	m_elements[ 3 ][ 2 ] = from[ 3 ][ 2 ];
	m_elements[ 0 ][ 3 ] = from[ 0 ][ 3 ];
	m_elements[ 1 ][ 3 ] = from[ 1 ][ 3 ];
	m_elements[ 2 ][ 3 ] = from[ 2 ][ 3 ];
	m_elements[ 3 ][ 3 ] = from[ 3 ][ 3 ];

	m_fIdentity = false;

	return *this;
}

inline CqMatrix &CqMatrix::operator=( const TqFloat from[ 16 ] )
{
	m_elements[ 0 ][ 0 ] = from[ 0 ];
	m_elements[ 0 ][ 1 ] = from[ 1 ];
	m_elements[ 0 ][ 2 ] = from[ 2 ];
	m_elements[ 0 ][ 3 ] = from[ 3 ];
	m_elements[ 1 ][ 0 ] = from[ 4 ];
	m_elements[ 1 ][ 1 ] = from[ 5 ];
	m_elements[ 1 ][ 2 ] = from[ 6 ];
	m_elements[ 1 ][ 3 ] = from[ 7 ];
	m_elements[ 2 ][ 0 ] = from[ 8 ];
	m_elements[ 2 ][ 1 ] = from[ 9 ];
	m_elements[ 2 ][ 2 ] = from[ 10 ];
	m_elements[ 2 ][ 3 ] = from[ 11 ];
	m_elements[ 3 ][ 0 ] = from[ 12 ];
	m_elements[ 3 ][ 1 ] = from[ 13 ];
	m_elements[ 3 ][ 2 ] = from[ 14 ];
	m_elements[ 3 ][ 3 ] = from[ 15 ];

	m_fIdentity = false;

	return *this;
}

inline CqMatrix CqMatrix::Transpose() const
{
	CqMatrix temp;

	if ( m_fIdentity )
	{
		temp = *this;
	}
	else
	{
		temp.m_elements[ 0 ][ 0 ] = m_elements[ 0 ][ 0 ];
		temp.m_elements[ 0 ][ 1 ] = m_elements[ 1 ][ 0 ];
		temp.m_elements[ 0 ][ 2 ] = m_elements[ 2 ][ 0 ];
		temp.m_elements[ 0 ][ 3 ] = m_elements[ 3 ][ 0 ];
		temp.m_elements[ 1 ][ 0 ] = m_elements[ 0 ][ 1 ];
		temp.m_elements[ 1 ][ 1 ] = m_elements[ 1 ][ 1 ];
		temp.m_elements[ 1 ][ 2 ] = m_elements[ 2 ][ 1 ];
		temp.m_elements[ 1 ][ 3 ] = m_elements[ 3 ][ 1 ];
		temp.m_elements[ 2 ][ 0 ] = m_elements[ 0 ][ 2 ];
		temp.m_elements[ 2 ][ 1 ] = m_elements[ 1 ][ 2 ];
		temp.m_elements[ 2 ][ 2 ] = m_elements[ 2 ][ 2 ];
		temp.m_elements[ 2 ][ 3 ] = m_elements[ 3 ][ 2 ];
		temp.m_elements[ 3 ][ 0 ] = m_elements[ 0 ][ 3 ];
		temp.m_elements[ 3 ][ 1 ] = m_elements[ 1 ][ 3 ];
		temp.m_elements[ 3 ][ 2 ] = m_elements[ 2 ][ 3 ];
		temp.m_elements[ 3 ][ 3 ] = m_elements[ 3 ][ 3 ];

		temp.m_fIdentity = false;
	}

	return temp;
}

inline CqMatrix operator*( TqFloat S, const CqMatrix& a )
{
	CqMatrix temp( a );
	temp.m_elements[ 0 ][ 0 ] *= S;
	temp.m_elements[ 1 ][ 0 ] *= S;
	temp.m_elements[ 2 ][ 0 ] *= S;
	temp.m_elements[ 3 ][ 0 ] *= S;

	temp.m_elements[ 0 ][ 1 ] *= S;
	temp.m_elements[ 1 ][ 1 ] *= S;
	temp.m_elements[ 2 ][ 1 ] *= S;
	temp.m_elements[ 3 ][ 1 ] *= S;

	temp.m_elements[ 0 ][ 2 ] *= S;
	temp.m_elements[ 1 ][ 2 ] *= S;
	temp.m_elements[ 2 ][ 2 ] *= S;
	temp.m_elements[ 3 ][ 2 ] *= S;

	temp.m_elements[ 0 ][ 3 ] *= S;
	temp.m_elements[ 1 ][ 3 ] *= S;
	temp.m_elements[ 2 ][ 3 ] *= S;
	temp.m_elements[ 3 ][ 3 ] *= S;
	return temp;
}

inline CqVector4D operator*( const CqVector4D &vec, const CqMatrix& matrix )
{
	return ( matrix.PreMultiply( vec ) );
}

inline bool  operator==(const CqMatrix& A, const CqMatrix& B)
{
	if(	(A.m_elements[ 0 ][ 0 ] == B.m_elements[ 0 ][ 0 ]) &&
	        (A.m_elements[ 1 ][ 0 ] == B.m_elements[ 1 ][ 0 ]) &&
	        (A.m_elements[ 2 ][ 0 ] == B.m_elements[ 2 ][ 0 ]) &&
	        (A.m_elements[ 3 ][ 0 ] == B.m_elements[ 3 ][ 0 ]) &&

	        (A.m_elements[ 0 ][ 1 ] == B.m_elements[ 0 ][ 1 ]) &&
	        (A.m_elements[ 1 ][ 1 ] == B.m_elements[ 1 ][ 1 ]) &&
	        (A.m_elements[ 2 ][ 1 ] == B.m_elements[ 2 ][ 1 ]) &&
	        (A.m_elements[ 3 ][ 1 ] == B.m_elements[ 3 ][ 1 ]) &&

	        (A.m_elements[ 0 ][ 2 ] == B.m_elements[ 0 ][ 2 ]) &&
	        (A.m_elements[ 1 ][ 2 ] == B.m_elements[ 1 ][ 2 ]) &&
	        (A.m_elements[ 2 ][ 2 ] == B.m_elements[ 2 ][ 2 ]) &&
	        (A.m_elements[ 3 ][ 2 ] == B.m_elements[ 3 ][ 2 ]) &&

	        (A.m_elements[ 0 ][ 3 ] == B.m_elements[ 0 ][ 3 ]) &&
	        (A.m_elements[ 1 ][ 3 ] == B.m_elements[ 1 ][ 3 ]) &&
	        (A.m_elements[ 2 ][ 3 ] == B.m_elements[ 2 ][ 3 ]) &&
	        (A.m_elements[ 3 ][ 3 ] == B.m_elements[ 3 ][ 3 ]))
		return true;
	else
		return false;
}

inline bool  operator!=(const CqMatrix& A, const CqMatrix& B)
{
	return !(A==B);
}


inline CqMatrix vectorTransform(const CqMatrix& m)
{
	CqMatrix r = m;
	// Remove the transformation and projection parts of the transform.
	r[3][0] = r[3][1] = r[3][2] = 0;
	r[0][3] = r[1][3] = r[2][3] = 0;
	r[3][3] = 1.0;
	return r;
}


inline CqMatrix normalTransform(const CqMatrix& m)
{
	return vectorTransform(m).Inverse().Transpose();
}


} // namespace Aqsis

#endif	// !MATRIX_H_INCLUDED
