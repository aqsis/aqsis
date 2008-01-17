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
#define MATRIX_H_INCLUDED 1

#include "aqsis.h"

#include <iostream>

#include "vector3d.h"
#include "vector4d.h"

namespace Aqsis
{

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
class COMMON_SHARE CqMatrix
{
	public:
		/// Tag struct for no-init constructor
		struct NoInit {};

		//--------------------------------------------------
		// Constructors

		/** \brief Construct the identity matrix.
		 */
		CqMatrix();
		/** \brief Construct an uninitalized matrix.
		 */
		CqMatrix(NoInit);
		/** \brief Construct a diagonal matrix
		 *
		 * \param xs scale factor along x.
		 * \param ys scale factor along y.
		 * \param zs scale factor along z.
		 */
		CqMatrix( const TqFloat xs, const TqFloat ys, const TqFloat zs );
		/** \brief Construct a translation matrix.
		 *
		 * \param Trans The vector by which to translate.
		 */
		CqMatrix( const CqVector3D& Trans );
		/** \brief Rotation matrix constructor
		 *
		 * \param Angle The angle to rotate by.
		 * \param Axis The axis about which to rotate.
		 */
		CqMatrix( const TqFloat Angle, const CqVector3D Axis );
		/** \brief Skew matrix constructor
		 *
		 * \param Angle
		 * \param dx1, dy1, dz1
		 * \param dx2, dy2, dz2
		 */
		CqMatrix( const TqFloat angle,
		          const TqFloat dx1, const TqFloat dy1, const TqFloat dz1,
		          const TqFloat dx2, const TqFloat dy2, const TqFloat dz2 );
		/// Copy constructor
		CqMatrix( const CqMatrix &From );
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
		 * \param From - 2D float array to copy data from.
		 */
		CqMatrix( TqFloat From[ 4 ][ 4 ] );
		/** \brief Constructor from a flattened array of matrix elements
		 *
		 * \param From - 1D float array to copy data from.
		 */
		CqMatrix( TqFloat From[ 16 ] );
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
		 * \param S	- amount to scale by.
		 */
		void Scale( const TqFloat S );
		/** \brief Scale matrix in three axes.
		 * \param xs X scale factor.
		 * \param ys Y scale factor.
		 * \param zs Z scale factor.
		 */
		void Scale( const TqFloat xs, const TqFloat ys, const TqFloat zs );
		/** \brief Rotate this matrix by an angle about an axis through the origin.
		 * \param Angle	The angle to rotate by.
		 * \param Axis The axis about which to rotate.
		 */
		void Rotate( const TqFloat Angle, const CqVector3D Axis );
		/** Translates this matrix by a given vector.
		 * \param Trans	The vector by which to translate.
		 */
		void Translate( const CqVector3D& Trans );
		/** Translates this matrix by three axis distances.
		 * \param xt X distance to translate.
		 * \param yt Y distance to translate.
		 * \param zt Z distance to translate.
		 */
		void Translate( const TqFloat xt, const TqFloat yt, const TqFloat zt );
		/** Shears this matrix's X axis according to two shear factors, yh and zh
		 * \param yh Y shear factor.
		 * \param zh Z shear factor.
		 */
		void ShearX( const TqFloat yh, const TqFloat zh );
		/** Shears this matrix's Y axis according to two shear factors, xh and zh
		 * \param xh X shear factor.
		 * \param zh Z shear factor.
		 */
		void ShearY( const TqFloat xh, const TqFloat zh );
		/** Shears this matrix's Z axis according to two shear factors, xh and yh
		 * \param xh X shear factor.
		 * \param yh Y shear factor.
		 */
		void ShearZ( const TqFloat xh, const TqFloat yh );
		/** Skew matrix
		 * \param angle The angle by which to skew the transformation.
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
		 * \param iRow The row index.
		 * \param iColumn The column index.
		 * \return Float value.
		 */
		TqFloat Element( TqInt iRow, TqInt iColumn ) const;
		/** \brief Set the element at the specified row and column index.
		 * \param iRow The row index.
		 * \param iColumn The column index.
		 * \param fValue the value to insert.
		 */
		void SetElement( TqInt iRow, TqInt iColumn, TqFloat fValue );
		/** \brief Get a pointer to the row index specified.
		 * \param iRow The row index.
		 * \return Pointer to array of 4 float values.
		 */
		TqFloat* operator[] ( TqInt iRow );
		/** \brief Get a read only pointer to the row index specified.
		 * \param iRow The row index.
		 * \return Pointer to array of 4 float values.
		 */
		const TqFloat* operator[] ( TqInt iRow ) const;
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
		/// \name Operators and functions for matrices arithmetic.
		//@{
		/** \brief Multiply two matrices together.
		 * \param From The matrix to multiply with this matrix.
		 * \return The resultant multiplied matrix.
		 */
		CqMatrix operator*( const CqMatrix &From ) const;
		/** \brief Apply scale matrix uniformly in all three axes.
		 * \param S The amount by which to scale matrix.
		 * \return Result of scaling this matrix by S.
		 */
		CqMatrix operator*( const TqFloat S ) const;
		/** \brief Multiply a vector by this matrix.
		 * \param Vector - The vector to multiply.
		 * \return The result of multiplying the vector by this matrix.
		 */
		CqVector4D operator*( const CqVector4D &Vector ) const;
		/** \brief Premultiplies this matrix by a vector, returning v*m.
		 *
		 * This is the same as postmultiply the transpose of m by a vector: T(m)*v
		 * \param Vector The vector to multiply.
		 */
		CqVector4D PreMultiply( const CqVector4D &Vector ) const;
		/** \brief Multiply a 3D vector by this matrix.
		 *
		 * The meaning of this is to first construct a 4D vector corresponding
		 * to the homogeneous coordinates: [Vector, 1], and multiply that by the
		 * matrix instead.  We then rescale the result such that the fourth
		 * component again has value 1.  This allows us to represent
		 * perspective transformations via matrix multiplication in homogeneous
		 * coordinates.
		 *
		 * \param Vector The vector to multiply.
		 */
		CqVector3D operator*( const CqVector3D &Vector ) const;
		/** \brief Translate matrix by 4D Vector.
		 * \param Vector The vector to translate by.
		 * \return Result of translating this matrix by the vector.
		 */
		CqMatrix operator+( const CqVector4D &Vector ) const;
		/** \brief Translate matrix by 4D Vector.
		 * \param Vector The vector to translate by.
		 * \return Result of translating this matrix by the vector.
		 */
		CqMatrix operator-( const CqVector4D &Vector ) const;
		/** \brief Add two matrices.
		 * \param From The matrix to add.
		 * \return Result of adding From to this matrix.
		 */
		CqMatrix operator+( const CqMatrix &From ) const;
		/** \brief Subtract two matrices.
		 * \param From The matrix to subtract.
		 * \return Result of subtracting From from this matrix.
		 */
		CqMatrix operator-( const CqMatrix &From ) const;

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
		 * \param From Matrix to copy information from.
		 */
		CqMatrix& operator=( const CqMatrix &From );
		/** \brief Copy function.
		 * \param From Renderman matrix to copy information from.
		 */
		CqMatrix& operator=( TqFloat From[ 4 ][ 4 ] );
		/** \brief Copy function.
		 * \param From Renderman matrix to copy information from.
		 */
		CqMatrix& operator=( TqFloat From[ 16 ] );
		/** \brief Add a given matrix to this matrix.
		 * \param From The matrix to add.
		 */
		CqMatrix& operator+=( const CqMatrix &From );
		/** \brief Subtract a given matrix from this matrix.
		 * \param From The matrix to subtract.
		 */
		CqMatrix& operator-=( const CqMatrix &From );
		/** \brief Translate this matrix by 4D Vector.
		 * \param Vector The vector to translate by.
		 * \return The result of translating this matrix by the specified vector.
		 */
		CqMatrix& operator+=( const CqVector4D &Vector );
		/** \brief Translate this matrix by 4D Vector.
		 * \param Vector The vector to translate by.
		 */
		CqMatrix& operator-=( const CqVector4D &Vector );
		/** \brief Multiply this matrix by specified matrix.
		 *
		 * This takes into account the types of matrices, in an attempt to
		 * speed it up.
		 * \param From The matrix to multiply with this matrix.
		 */
		CqMatrix& operator*=( const CqMatrix &From );
		/** \brief Matrix multiplication of the form a = b * a.
		 * \param From The matrix to multiply with this matrix.
		 */
		CqMatrix& PreMultiply( const CqMatrix &From );
		/** \brief Apply scale matrix uniformly in all three axes to this matrix.
		 * \param S The amount by which to scale this matrix.
		 * \return The result scaling this matrix by S.
		 */
		CqMatrix& operator*=( const TqFloat S );
		/** \brief Scale each element by the specified value.
		 * \param S The amount by which to scale the matrix elements.
		 * \param a The matrix to be scaled.
		 * \return Result of scaling this matrix by S.
		 */
		COMMON_SHARE friend CqMatrix operator*( TqFloat S, const CqMatrix& a );
		//@}

		//--------------------------------------------------
		/// \name Other operators
		//@{
		/** \brief Compare two matrices.
		 * \param A One Matrix to be compared.
		 * \param B Second Matrix to be compared with.
		 * \return Result if matrices are equal or not.
		 *
		 * \todo code review Direct floating point comparison is rarely a good
		 * idea.  We probably need some kind of comparison tolerance in here.
		 */
		COMMON_SHARE friend bool  operator==(const CqMatrix& A, const CqMatrix& B);
		/// Return !(A == B)
		COMMON_SHARE friend bool  operator!=(const CqMatrix& A, const CqMatrix& B);
		/** \brief Outputs a matrix to an output stream.
		 * \param Stream Stream to output the matrix to.
		 * \param Matrix The matrix to output.
		 * \return The new state of Stream.
		 */
		COMMON_SHARE friend std::ostream &operator<<( std::ostream &Stream, const CqMatrix &Matrix );
		//@}

	protected:
		/// The 4x4 array of float values.
		TqFloat m_aaElement[ 4 ][ 4 ];
		/** Flag indicating that this matrix should be treated as identity
		 * irrespective of its contents.
		 */
		bool m_fIdentity;
};

/// Premultiply matrix by vector.
COMMON_SHARE CqVector4D operator*( const CqVector4D &Vector, const CqMatrix& Matrix );




//==============================================================================
// Implementation details
//==============================================================================

// Constructors
inline CqMatrix::CqMatrix( )
{
	Identity();
}

inline CqMatrix::CqMatrix(NoInit)
	: m_fIdentity(false)
{}

// construct a diagonal matrix
inline CqMatrix::CqMatrix( const TqFloat xs, const TqFloat ys, const TqFloat zs )
{
	Identity();

	if ( xs != 1.0f || ys != 1.0f || zs != 1.0f )
	{
		m_aaElement[ 0 ][ 0 ] = xs;
		m_aaElement[ 1 ][ 1 ] = ys;
		m_aaElement[ 2 ][ 2 ] = zs;
		m_aaElement[ 3 ][ 3 ] = 1.0;

		m_fIdentity = false;
	}
}

// Construct a translation matrix
inline CqMatrix::CqMatrix( const CqVector3D& Trans )
{
	Identity();

	if ( Trans.x() != 0.0f || Trans.y() != 0.0f || Trans.z() != 0.0f )
	{
		m_fIdentity = false;

		m_aaElement[ 3 ][ 0 ] = Trans.x();
		m_aaElement[ 3 ][ 1 ] = Trans.y();
		m_aaElement[ 3 ][ 2 ] = Trans.z();
	}
}

// Construct a rotation matrix
inline CqMatrix::CqMatrix( const TqFloat Angle, const CqVector3D Axis )
{
	Identity();

	if ( Angle != 0.0f && Axis.Magnitude() != 0.0f )
		Rotate( Angle, Axis );
}

inline CqMatrix::CqMatrix( const CqMatrix &From )
{
	*this = From;
}

inline CqMatrix::CqMatrix(
		const TqFloat r1c1, const TqFloat r1c2, const TqFloat r1c3, const TqFloat r1c4,
		const TqFloat r2c1, const TqFloat r2c2, const TqFloat r2c3, const TqFloat r2c4,
		const TqFloat r3c1, const TqFloat r3c2, const TqFloat r3c3, const TqFloat r3c4,
		const TqFloat r4c1, const TqFloat r4c2, const TqFloat r4c3, const TqFloat r4c4 )
	: m_fIdentity(false)
{
	m_aaElement[ 0 ][ 0 ] = r1c1;
	m_aaElement[ 0 ][ 1 ] = r1c2;
	m_aaElement[ 0 ][ 2 ] = r1c3;
	m_aaElement[ 0 ][ 3 ] = r1c4;
	m_aaElement[ 1 ][ 0 ] = r2c1;
	m_aaElement[ 1 ][ 1 ] = r2c2;
	m_aaElement[ 1 ][ 2 ] = r2c3;
	m_aaElement[ 1 ][ 3 ] = r2c4;
	m_aaElement[ 2 ][ 0 ] = r3c1;
	m_aaElement[ 2 ][ 1 ] = r3c2;
	m_aaElement[ 2 ][ 2 ] = r3c3;
	m_aaElement[ 2 ][ 3 ] = r3c4;
	m_aaElement[ 3 ][ 0 ] = r4c1;
	m_aaElement[ 3 ][ 1 ] = r4c2;
	m_aaElement[ 3 ][ 2 ] = r4c3;
	m_aaElement[ 3 ][ 3 ] = r4c4;
}

inline CqMatrix::CqMatrix( TqFloat From[ 4 ][ 4 ] )
{
	*this = From;
}

inline CqMatrix::CqMatrix( TqFloat From[ 16 ] )
{
	*this = From;
}

inline CqMatrix::CqMatrix( TqFloat f )
{
  Identity();
  if(f != 1.0)
  {
    m_aaElement[ 0 ][ 0 ] = f;
    m_aaElement[ 1 ][ 1 ] = f;
    m_aaElement[ 2 ][ 2 ] = f;
    m_aaElement[ 3 ][ 3 ] = f;
    m_fIdentity = false;
  }
}

//------------------------------------------------------------------------------
// Special-case identity handling
inline void CqMatrix::Identity()
{
	m_fIdentity = true;

	m_aaElement[ 0 ][ 1 ]
		= m_aaElement[ 0 ][ 2 ]
		= m_aaElement[ 0 ][ 3 ]
		= m_aaElement[ 1 ][ 0 ]
		= m_aaElement[ 1 ][ 2 ]
		= m_aaElement[ 1 ][ 3 ]
		= m_aaElement[ 2 ][ 0 ]
		= m_aaElement[ 2 ][ 1 ]
		= m_aaElement[ 2 ][ 3 ]
		= m_aaElement[ 3 ][ 0 ]
		= m_aaElement[ 3 ][ 1 ]
		= m_aaElement[ 3 ][ 2 ]
		= 0.0f;

	m_aaElement[ 0 ][ 0 ]
		= m_aaElement[ 1 ][ 1 ]
		= m_aaElement[ 2 ][ 2 ]
		= m_aaElement[ 3 ][ 3 ]
		= 1.0f;
}

inline void CqMatrix::SetfIdentity( bool f )
{
	m_fIdentity = f;
}

inline bool CqMatrix::fIdentity() const
{
	return ( m_fIdentity );
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

inline void CqMatrix::Translate( const CqVector3D& Trans )
{
	CqMatrix matTrans( Trans );
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

	Shear.m_aaElement[ 0 ][ 1 ] = yh;
	Shear.m_aaElement[ 0 ][ 2 ] = zh;

	this->PreMultiply( Shear );
}

inline void CqMatrix::ShearY( const TqFloat xh, const TqFloat zh )
{
	CqMatrix Shear;
	Shear.m_fIdentity = false;

	Shear.m_aaElement[ 1 ][ 0 ] = xh;
	Shear.m_aaElement[ 1 ][ 2 ] = zh;

	this->PreMultiply( Shear );
}

inline void CqMatrix::ShearZ( const TqFloat xh, const TqFloat yh )
{
	CqMatrix Shear;
	Shear.m_fIdentity = false;

	Shear.m_aaElement[ 2 ][ 0 ] = xh;
	Shear.m_aaElement[ 2 ][ 1 ] = yh;

	this->PreMultiply( Shear );
}

inline void CqMatrix::Skew( const TqFloat angle,
                     const TqFloat dx1, const TqFloat dy1, const TqFloat dz1,
                     const TqFloat dx2, const TqFloat dy2, const TqFloat dz2 )
{
	CqMatrix Skew( angle, dx1, dy1, dz1, dx2, dy2, dz2 );

	this->PreMultiply( Skew );
}

inline void CqMatrix::Normalise()
{
	assert(m_aaElement[ 3 ][ 3 ] != 0);
	for ( TqInt i = 0; i < 4; i++ )
	{
		for ( TqInt j = 0; j < 4; j++ )
		{
			m_aaElement[ i ][ j ] /= m_aaElement[ 3 ][ 3 ];
		}
	}
}

//------------------------------------------------------------------------------
// Access to matrix elements.
inline TqFloat CqMatrix::Element( TqInt iRow, TqInt iColumn ) const
{
	return ( m_aaElement[ iRow ][ iColumn ] );
}

inline void CqMatrix::SetElement( TqInt iRow, TqInt iColumn, TqFloat fValue )
{
	m_aaElement[ iRow ][ iColumn ] = fValue;
}

inline TqFloat* CqMatrix::operator[] ( TqInt iRow )
{
	return( &m_aaElement[ iRow ][ 0 ] );
}

inline const TqFloat* CqMatrix::operator[] ( TqInt iRow ) const
{
	return( &m_aaElement[ iRow ][ 0 ] );
}

inline TqFloat* CqMatrix::pElements()
{
	return ( &m_aaElement[ 0 ][ 0 ] );
}

inline const TqFloat* CqMatrix::pElements() const
{
	return ( &m_aaElement[ 0 ][ 0 ] );
}

//------------------------------------------------------------------------------
// Operators and functions for matrices arithmetic.

inline CqMatrix CqMatrix::operator*( const CqMatrix &From ) const
{
	CqMatrix Temp( *this );
	Temp *= From;
	return ( Temp );
}

inline CqVector4D CqMatrix::PreMultiply( const CqVector4D &Vector ) const
{
	if ( m_fIdentity )
		return ( Vector );

	CqVector4D	Result;

	Result.x( m_aaElement[ 0 ][ 0 ] * Vector.x()
	          + m_aaElement[ 0 ][ 1 ] * Vector.y()
	          + m_aaElement[ 0 ][ 2 ] * Vector.z()
	          + m_aaElement[ 0 ][ 3 ] * Vector.h() );

	Result.y( m_aaElement[ 1 ][ 0 ] * Vector.x()
	          + m_aaElement[ 1 ][ 1 ] * Vector.y()
	          + m_aaElement[ 1 ][ 2 ] * Vector.z()
	          + m_aaElement[ 1 ][ 3 ] * Vector.h() );

	Result.z( m_aaElement[ 2 ][ 0 ] * Vector.x()
	          + m_aaElement[ 2 ][ 1 ] * Vector.y()
	          + m_aaElement[ 2 ][ 2 ] * Vector.z()
	          + m_aaElement[ 2 ][ 3 ] * Vector.h() );

	Result.h( m_aaElement[ 3 ][ 0 ] * Vector.x()
	          + m_aaElement[ 3 ][ 1 ] * Vector.y()
	          + m_aaElement[ 3 ][ 2 ] * Vector.z()
	          + m_aaElement[ 3 ][ 3 ] * Vector.h() );

	return ( Result );
}

inline CqMatrix CqMatrix::operator*( const TqFloat S ) const
{
	CqMatrix Temp( *this );
	Temp *= S;
	return ( Temp );
}

inline CqMatrix &CqMatrix::operator*=( const TqFloat S )
{
	CqMatrix ScaleMatrix( S, S, S );
	this->PreMultiply( ScaleMatrix );
	return ( *this );
}

inline CqVector4D CqMatrix::operator*( const CqVector4D &Vector ) const
{
	if ( m_fIdentity )
		return ( Vector );

	CqVector4D	Result;

	Result.x( m_aaElement[ 0 ][ 0 ] * Vector.x()
	          + m_aaElement[ 1 ][ 0 ] * Vector.y()
	          + m_aaElement[ 2 ][ 0 ] * Vector.z()
	          + m_aaElement[ 3 ][ 0 ] * Vector.h() );

	Result.y( m_aaElement[ 0 ][ 1 ] * Vector.x()
	          + m_aaElement[ 1 ][ 1 ] * Vector.y()
	          + m_aaElement[ 2 ][ 1 ] * Vector.z()
	          + m_aaElement[ 3 ][ 1 ] * Vector.h() );

	Result.z( m_aaElement[ 0 ][ 2 ] * Vector.x()
	          + m_aaElement[ 1 ][ 2 ] * Vector.y()
	          + m_aaElement[ 2 ][ 2 ] * Vector.z()
	          + m_aaElement[ 3 ][ 2 ] * Vector.h() );

	Result.h( m_aaElement[ 0 ][ 3 ] * Vector.x()
	          + m_aaElement[ 1 ][ 3 ] * Vector.y()
	          + m_aaElement[ 2 ][ 3 ] * Vector.z()
	          + m_aaElement[ 3 ][ 3 ] * Vector.h() );

	return ( Result );
}

inline CqVector3D CqMatrix::operator*( const CqVector3D &Vector ) const
{
	if ( m_fIdentity )
		return ( Vector );

	CqVector3D	Result;
	TqFloat h = ( m_aaElement[ 0 ][ 3 ] * Vector.x()
	              + m_aaElement[ 1 ][ 3 ] * Vector.y()
	              + m_aaElement[ 2 ][ 3 ] * Vector.z()
	              + m_aaElement[ 3 ][ 3 ] );
	Result.x( ( m_aaElement[ 0 ][ 0 ] * Vector.x()
	            + m_aaElement[ 1 ][ 0 ] * Vector.y()
	            + m_aaElement[ 2 ][ 0 ] * Vector.z()
	            + m_aaElement[ 3 ][ 0 ] ) );
	Result.y( ( m_aaElement[ 0 ][ 1 ] * Vector.x()
	            + m_aaElement[ 1 ][ 1 ] * Vector.y()
	            + m_aaElement[ 2 ][ 1 ] * Vector.z()
	            + m_aaElement[ 3 ][ 1 ] ) );
	Result.z( ( m_aaElement[ 0 ][ 2 ] * Vector.x()
	            + m_aaElement[ 1 ][ 2 ] * Vector.y()
	            + m_aaElement[ 2 ][ 2 ] * Vector.z()
	            + m_aaElement[ 3 ][ 2 ] ) );

	if(h != 1)
	{
		assert(h != 0);
		TqFloat invh = 1/h;
		Result.x(Result.x()*invh);
		Result.y(Result.y()*invh);
		Result.z(Result.z()*invh);
	}

	return ( Result );
}

inline CqMatrix CqMatrix::operator+( const CqVector4D &Vector ) const
{
	CqMatrix Temp( *this );
	Temp += Vector;
	return ( Temp );
}

inline CqMatrix &CqMatrix::operator+=( const CqVector4D &Vector )
{
	CqMatrix Trans( Vector );
	this->PreMultiply( Trans );
	return ( *this );
}

inline CqMatrix CqMatrix::operator-( const CqVector4D &Vector ) const
{
	CqMatrix Temp( *this );
	Temp -= Vector;
	return ( Temp );
}

inline CqMatrix &CqMatrix::operator-=( const CqVector4D &Vector )
{
	CqVector4D Temp( Vector );

	Temp.x( -Temp.x() );
	Temp.y( -Temp.y() );
	Temp.z( -Temp.z() );

	CqMatrix Trans( Temp );

	this->PreMultiply( Trans );

	return ( *this );
}

inline CqMatrix CqMatrix::operator+( const CqMatrix &From ) const
{
	CqMatrix Temp( *this );
	Temp += From;
	return ( Temp );
}

inline CqMatrix &CqMatrix::operator+=( const CqMatrix &From )
{
	m_aaElement[ 0 ][ 0 ] += From.m_aaElement[ 0 ][ 0 ];
	m_aaElement[ 1 ][ 0 ] += From.m_aaElement[ 1 ][ 0 ];
	m_aaElement[ 2 ][ 0 ] += From.m_aaElement[ 2 ][ 0 ];
	m_aaElement[ 3 ][ 0 ] += From.m_aaElement[ 3 ][ 0 ];
	m_aaElement[ 0 ][ 1 ] += From.m_aaElement[ 0 ][ 1 ];
	m_aaElement[ 1 ][ 1 ] += From.m_aaElement[ 1 ][ 1 ];
	m_aaElement[ 2 ][ 1 ] += From.m_aaElement[ 2 ][ 1 ];
	m_aaElement[ 3 ][ 1 ] += From.m_aaElement[ 3 ][ 1 ];
	m_aaElement[ 0 ][ 2 ] += From.m_aaElement[ 0 ][ 2 ];
	m_aaElement[ 1 ][ 2 ] += From.m_aaElement[ 1 ][ 2 ];
	m_aaElement[ 2 ][ 2 ] += From.m_aaElement[ 2 ][ 2 ];
	m_aaElement[ 3 ][ 2 ] += From.m_aaElement[ 3 ][ 2 ];
	m_aaElement[ 0 ][ 3 ] += From.m_aaElement[ 0 ][ 3 ];
	m_aaElement[ 1 ][ 3 ] += From.m_aaElement[ 1 ][ 3 ];
	m_aaElement[ 2 ][ 3 ] += From.m_aaElement[ 2 ][ 3 ];
	m_aaElement[ 3 ][ 3 ] += From.m_aaElement[ 3 ][ 3 ];

	m_fIdentity = false;

	return ( *this );
}

inline CqMatrix CqMatrix::operator-( const CqMatrix &From ) const
{
	CqMatrix Temp( *this );
	Temp -= From;
	return ( Temp );
}

inline CqMatrix &CqMatrix::operator-=( const CqMatrix &From )
{
	m_aaElement[ 0 ][ 0 ] -= From.m_aaElement[ 0 ][ 0 ];
	m_aaElement[ 1 ][ 0 ] -= From.m_aaElement[ 1 ][ 0 ];
	m_aaElement[ 2 ][ 0 ] -= From.m_aaElement[ 2 ][ 0 ];
	m_aaElement[ 3 ][ 0 ] -= From.m_aaElement[ 3 ][ 0 ];
	m_aaElement[ 0 ][ 1 ] -= From.m_aaElement[ 0 ][ 1 ];
	m_aaElement[ 1 ][ 1 ] -= From.m_aaElement[ 1 ][ 1 ];
	m_aaElement[ 2 ][ 1 ] -= From.m_aaElement[ 2 ][ 1 ];
	m_aaElement[ 3 ][ 1 ] -= From.m_aaElement[ 3 ][ 1 ];
	m_aaElement[ 0 ][ 2 ] -= From.m_aaElement[ 0 ][ 2 ];
	m_aaElement[ 1 ][ 2 ] -= From.m_aaElement[ 1 ][ 2 ];
	m_aaElement[ 2 ][ 2 ] -= From.m_aaElement[ 2 ][ 2 ];
	m_aaElement[ 3 ][ 2 ] -= From.m_aaElement[ 3 ][ 2 ];
	m_aaElement[ 0 ][ 3 ] -= From.m_aaElement[ 0 ][ 3 ];
	m_aaElement[ 1 ][ 3 ] -= From.m_aaElement[ 1 ][ 3 ];
	m_aaElement[ 2 ][ 3 ] -= From.m_aaElement[ 2 ][ 3 ];
	m_aaElement[ 3 ][ 3 ] -= From.m_aaElement[ 3 ][ 3 ];

	m_fIdentity = false;

	return ( *this );
}

inline CqMatrix &CqMatrix::operator=( const CqMatrix &From )
{
	m_aaElement[ 0 ][ 0 ] = From.m_aaElement[ 0 ][ 0 ];
	m_aaElement[ 1 ][ 0 ] = From.m_aaElement[ 1 ][ 0 ];
	m_aaElement[ 2 ][ 0 ] = From.m_aaElement[ 2 ][ 0 ];
	m_aaElement[ 3 ][ 0 ] = From.m_aaElement[ 3 ][ 0 ];
	m_aaElement[ 0 ][ 1 ] = From.m_aaElement[ 0 ][ 1 ];
	m_aaElement[ 1 ][ 1 ] = From.m_aaElement[ 1 ][ 1 ];
	m_aaElement[ 2 ][ 1 ] = From.m_aaElement[ 2 ][ 1 ];
	m_aaElement[ 3 ][ 1 ] = From.m_aaElement[ 3 ][ 1 ];
	m_aaElement[ 0 ][ 2 ] = From.m_aaElement[ 0 ][ 2 ];
	m_aaElement[ 1 ][ 2 ] = From.m_aaElement[ 1 ][ 2 ];
	m_aaElement[ 2 ][ 2 ] = From.m_aaElement[ 2 ][ 2 ];
	m_aaElement[ 3 ][ 2 ] = From.m_aaElement[ 3 ][ 2 ];
	m_aaElement[ 0 ][ 3 ] = From.m_aaElement[ 0 ][ 3 ];
	m_aaElement[ 1 ][ 3 ] = From.m_aaElement[ 1 ][ 3 ];
	m_aaElement[ 2 ][ 3 ] = From.m_aaElement[ 2 ][ 3 ];
	m_aaElement[ 3 ][ 3 ] = From.m_aaElement[ 3 ][ 3 ];

	m_fIdentity = From.m_fIdentity;

	return ( *this );
}

inline CqMatrix &CqMatrix::operator=( TqFloat From[ 4 ][ 4 ] )
{
	m_aaElement[ 0 ][ 0 ] = From[ 0 ][ 0 ];
	m_aaElement[ 1 ][ 0 ] = From[ 1 ][ 0 ];
	m_aaElement[ 2 ][ 0 ] = From[ 2 ][ 0 ];
	m_aaElement[ 3 ][ 0 ] = From[ 3 ][ 0 ];
	m_aaElement[ 0 ][ 1 ] = From[ 0 ][ 1 ];
	m_aaElement[ 1 ][ 1 ] = From[ 1 ][ 1 ];
	m_aaElement[ 2 ][ 1 ] = From[ 2 ][ 1 ];
	m_aaElement[ 3 ][ 1 ] = From[ 3 ][ 1 ];
	m_aaElement[ 0 ][ 2 ] = From[ 0 ][ 2 ];
	m_aaElement[ 1 ][ 2 ] = From[ 1 ][ 2 ];
	m_aaElement[ 2 ][ 2 ] = From[ 2 ][ 2 ];
	m_aaElement[ 3 ][ 2 ] = From[ 3 ][ 2 ];
	m_aaElement[ 0 ][ 3 ] = From[ 0 ][ 3 ];
	m_aaElement[ 1 ][ 3 ] = From[ 1 ][ 3 ];
	m_aaElement[ 2 ][ 3 ] = From[ 2 ][ 3 ];
	m_aaElement[ 3 ][ 3 ] = From[ 3 ][ 3 ];

	m_fIdentity = false;

	return ( *this );
}

inline CqMatrix &CqMatrix::operator=( TqFloat From[ 16 ] )
{
	m_aaElement[ 0 ][ 0 ] = From[ 0 ];
	m_aaElement[ 0 ][ 1 ] = From[ 1 ];
	m_aaElement[ 0 ][ 2 ] = From[ 2 ];
	m_aaElement[ 0 ][ 3 ] = From[ 3 ];
	m_aaElement[ 1 ][ 0 ] = From[ 4 ];
	m_aaElement[ 1 ][ 1 ] = From[ 5 ];
	m_aaElement[ 1 ][ 2 ] = From[ 6 ];
	m_aaElement[ 1 ][ 3 ] = From[ 7 ];
	m_aaElement[ 2 ][ 0 ] = From[ 8 ];
	m_aaElement[ 2 ][ 1 ] = From[ 9 ];
	m_aaElement[ 2 ][ 2 ] = From[ 10 ];
	m_aaElement[ 2 ][ 3 ] = From[ 11 ];
	m_aaElement[ 3 ][ 0 ] = From[ 12 ];
	m_aaElement[ 3 ][ 1 ] = From[ 13 ];
	m_aaElement[ 3 ][ 2 ] = From[ 14 ];
	m_aaElement[ 3 ][ 3 ] = From[ 15 ];

	m_fIdentity = false;

	return ( *this );
}

inline CqMatrix CqMatrix::Transpose() const
{
	CqMatrix Temp;

	if ( m_fIdentity )
	{
		Temp = *this;
	}
	else
	{
		Temp.m_aaElement[ 0 ][ 0 ] = m_aaElement[ 0 ][ 0 ];
		Temp.m_aaElement[ 0 ][ 1 ] = m_aaElement[ 1 ][ 0 ];
		Temp.m_aaElement[ 0 ][ 2 ] = m_aaElement[ 2 ][ 0 ];
		Temp.m_aaElement[ 0 ][ 3 ] = m_aaElement[ 3 ][ 0 ];
		Temp.m_aaElement[ 1 ][ 0 ] = m_aaElement[ 0 ][ 1 ];
		Temp.m_aaElement[ 1 ][ 1 ] = m_aaElement[ 1 ][ 1 ];
		Temp.m_aaElement[ 1 ][ 2 ] = m_aaElement[ 2 ][ 1 ];
		Temp.m_aaElement[ 1 ][ 3 ] = m_aaElement[ 3 ][ 1 ];
		Temp.m_aaElement[ 2 ][ 0 ] = m_aaElement[ 0 ][ 2 ];
		Temp.m_aaElement[ 2 ][ 1 ] = m_aaElement[ 1 ][ 2 ];
		Temp.m_aaElement[ 2 ][ 2 ] = m_aaElement[ 2 ][ 2 ];
		Temp.m_aaElement[ 2 ][ 3 ] = m_aaElement[ 3 ][ 2 ];
		Temp.m_aaElement[ 3 ][ 0 ] = m_aaElement[ 0 ][ 3 ];
		Temp.m_aaElement[ 3 ][ 1 ] = m_aaElement[ 1 ][ 3 ];
		Temp.m_aaElement[ 3 ][ 2 ] = m_aaElement[ 2 ][ 3 ];
		Temp.m_aaElement[ 3 ][ 3 ] = m_aaElement[ 3 ][ 3 ];

		Temp.m_fIdentity = false;
	}

	return ( Temp );
}

inline CqMatrix operator*( TqFloat S, const CqMatrix& a )
{
	CqMatrix Temp( a );
	Temp.m_aaElement[ 0 ][ 0 ] *= S;
	Temp.m_aaElement[ 1 ][ 0 ] *= S;
	Temp.m_aaElement[ 2 ][ 0 ] *= S;
	Temp.m_aaElement[ 3 ][ 0 ] *= S;

	Temp.m_aaElement[ 0 ][ 1 ] *= S;
	Temp.m_aaElement[ 1 ][ 1 ] *= S;
	Temp.m_aaElement[ 2 ][ 1 ] *= S;
	Temp.m_aaElement[ 3 ][ 1 ] *= S;

	Temp.m_aaElement[ 0 ][ 2 ] *= S;
	Temp.m_aaElement[ 1 ][ 2 ] *= S;
	Temp.m_aaElement[ 2 ][ 2 ] *= S;
	Temp.m_aaElement[ 3 ][ 2 ] *= S;

	Temp.m_aaElement[ 0 ][ 3 ] *= S;
	Temp.m_aaElement[ 1 ][ 3 ] *= S;
	Temp.m_aaElement[ 2 ][ 3 ] *= S;
	Temp.m_aaElement[ 3 ][ 3 ] *= S;
	return ( Temp );
}

inline CqVector4D operator*( const CqVector4D &Vector, const CqMatrix& Matrix )
{
	return ( Matrix.PreMultiply( Vector ) );
}

inline bool  operator==(const CqMatrix& A, const CqMatrix& B)
{
	if(	(A.m_aaElement[ 0 ][ 0 ] == B.m_aaElement[ 0 ][ 0 ]) &&
	        (A.m_aaElement[ 1 ][ 0 ] == B.m_aaElement[ 1 ][ 0 ]) &&
	        (A.m_aaElement[ 2 ][ 0 ] == B.m_aaElement[ 2 ][ 0 ]) &&
	        (A.m_aaElement[ 3 ][ 0 ] == B.m_aaElement[ 3 ][ 0 ]) &&

	        (A.m_aaElement[ 0 ][ 1 ] == B.m_aaElement[ 0 ][ 1 ]) &&
	        (A.m_aaElement[ 1 ][ 1 ] == B.m_aaElement[ 1 ][ 1 ]) &&
	        (A.m_aaElement[ 2 ][ 1 ] == B.m_aaElement[ 2 ][ 1 ]) &&
	        (A.m_aaElement[ 3 ][ 1 ] == B.m_aaElement[ 3 ][ 1 ]) &&

	        (A.m_aaElement[ 0 ][ 2 ] == B.m_aaElement[ 0 ][ 2 ]) &&
	        (A.m_aaElement[ 1 ][ 2 ] == B.m_aaElement[ 1 ][ 2 ]) &&
	        (A.m_aaElement[ 2 ][ 2 ] == B.m_aaElement[ 2 ][ 2 ]) &&
	        (A.m_aaElement[ 3 ][ 2 ] == B.m_aaElement[ 3 ][ 2 ]) &&

	        (A.m_aaElement[ 0 ][ 3 ] == B.m_aaElement[ 0 ][ 3 ]) &&
	        (A.m_aaElement[ 1 ][ 3 ] == B.m_aaElement[ 1 ][ 3 ]) &&
	        (A.m_aaElement[ 2 ][ 3 ] == B.m_aaElement[ 2 ][ 3 ]) &&
	        (A.m_aaElement[ 3 ][ 3 ] == B.m_aaElement[ 3 ][ 3 ]))
		return(true);
	else
		return(false);
}

inline bool  operator!=(const CqMatrix& A, const CqMatrix& B)
{
	return(!(A==B));
}

} // namespace Aqsis

#endif	// !MATRIX_H_INCLUDED
