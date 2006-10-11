// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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
		\brief Declares the CqMatrix 4D homogenous matrix class.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is matrix.h included already?
#ifndef MATRIX_H_INCLUDED
#define MATRIX_H_INCLUDED 1

#include	<iostream>

#include	"aqsis.h"

#include	"vector3d.h"
#include	"vector4d.h"

START_NAMESPACE( Aqsis )

//----------------------------------------------------------------------
/** \class CqMatrix
 * 4x4 Matrix class definition.
 * Access to matrix elements is always row,column from 0.
 */

class CqMatrix
{
	public:
		struct NoInit
			{}
		;

		CqMatrix();
		CqMatrix(NoInit)
		{}
		CqMatrix( const TqFloat xs, const TqFloat ys, const TqFloat zs ); // Scaled ID
		CqMatrix( const CqVector3D& Trans ); // Translated
		CqMatrix( const TqFloat Angle, const CqVector3D Axis ); // Rotation2
		CqMatrix( const TqFloat angle,
		          const TqFloat dx1, const TqFloat dy1, const TqFloat dz1,
		          const TqFloat dx2, const TqFloat dy2, const TqFloat dz2 ); // Skew
		CqMatrix( const CqMatrix &From );
		/** Individual element constructor.
		 * Takes 16 floats for the elements of the matrix.
		 */
		CqMatrix( const TqFloat r1c1, const TqFloat r1c2, const TqFloat r1c3, const TqFloat r1c4,
		          const TqFloat r2c1, const TqFloat r2c2, const TqFloat r2c3, const TqFloat r2c4,
		          const TqFloat r3c1, const TqFloat r3c2, const TqFloat r3c3, const TqFloat r3c4,
		          const TqFloat r4c1, const TqFloat r4c2, const TqFloat r4c3, const TqFloat r4c4 )
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
			m_fIdentity = TqFalse;
		}
		CqMatrix( TqFloat From[ 4 ][ 4 ] );
		CqMatrix( TqFloat From[ 16 ] );
		CqMatrix( TqFloat f );
		~CqMatrix()
		{}

		void	Identity();		// Make identity
		/** Mark this matrix as identity or not.
		 * \param f Bool indicating whether or not this matrix should be considered identity irespective of its contents.
		 */
		void	SetfIdentity( TqBool f )
		{
			m_fIdentity = f;
		}
		TqBool	fIdentity() const
		{
			return ( m_fIdentity );
		}
		void	Scale( const TqFloat S );
		void	Scale( const TqFloat xs, const TqFloat ys, const TqFloat zs );
		void	Rotate( const TqFloat Angle, const CqVector3D Axis );
		void	Translate( const CqVector3D& Trans );
		void	Translate( const TqFloat xt, const TqFloat yt, const TqFloat zt );
		void	ShearX( const TqFloat yh, const TqFloat zh );
		void	ShearY( const TqFloat xh, const TqFloat zh );
		void	ShearZ( const TqFloat xh, const TqFloat yh );
		void	Skew( const TqFloat angle,
		           const TqFloat dx1, const TqFloat dy1, const TqFloat dz1,
		           const TqFloat dx2, const TqFloat dy2, const TqFloat dz2 );
		void	Normalise();

		/** Get the element at the specified row and column index.
		 * \param iRow The row index.
		 * \param iColumn The column index.
		 * \return Float value.
		 */
		TqFloat	Element( TqInt iRow, TqInt iColumn ) const
		{
			return ( m_aaElement[ iRow ][ iColumn ] );
		}
		/** Set the element at the specified row and column index.
		 * \param iRow The row index.
		 * \param iColumn The column index.
		 * \param fValue the value to insert.
		 */
		void	SetElement( TqInt iRow, TqInt iColumn, TqFloat fValue )
		{
			m_aaElement[ iRow ][ iColumn ] = fValue;
		}
		/** Get a pointer to the row index specified.
		 * \param iRow The row index.
		 * \return Pointer to array of 4 float values.
		 */
		TqFloat*	operator[] ( TqInt iRow )
		{
			return( &m_aaElement[ iRow ][ 0 ] );
		}
		/** Get a read only pointer to the row index specified.
		 * \param iRow The row index.
		 * \return Pointer to array of 4 float values.
		 */
		const TqFloat*	operator[] ( TqInt iRow ) const
		{
			return( &m_aaElement[ iRow ][ 0 ] );
		}
		/** Get a pointer to matrix data.
		 * \return Pointer to array of 16 float values.
		 */
		TqFloat*	pElements()
		{
			return ( &m_aaElement[ 0 ][ 0 ] );
		}
		/** Get a read only pointer to matrix data.
		 * \return Pointer to array of 16 float values.
		 */
		const TqFloat* pElements() const
		{
			return ( &m_aaElement[ 0 ][ 0 ] );
		}

		// Binary operators
		CqMatrix	operator*( const CqMatrix &From ) const;
		CqMatrix	operator*( const TqFloat S ) const;
		CqVector4D	operator*( const CqVector4D &Vector ) const;
		CqVector3D	operator*( const CqVector3D &Vector ) const;
		CqMatrix	operator+( const CqVector4D &Vector ) const;
		CqMatrix	operator-( const CqVector4D &Vector ) const;
		CqMatrix	operator+( const CqMatrix &From ) const;
		CqMatrix	operator-( const CqMatrix &From ) const;

		CqMatrix	Inverse() const;
		CqMatrix	Transpose() const;

		CqMatrix&	operator=( const CqMatrix &From );
		CqMatrix&	operator=( TqFloat From[ 4 ][ 4 ] );
		CqMatrix&	operator=( TqFloat From[ 16 ] );
		CqMatrix&	operator+=( const CqMatrix &From );
		CqMatrix&	operator-=( const CqMatrix &From );
		CqMatrix&	operator+=( const CqVector4D &Vector );
		CqMatrix&	operator-=( const CqVector4D &Vector );
		CqMatrix&	operator*=( const CqMatrix &From );
		CqMatrix&	PreMultiply( const CqMatrix &From );
		CqVector4D	PreMultiply( const CqVector4D &Vector ) const;
		CqMatrix&	operator*=( const TqFloat S );

		friend std::ostream &operator<<( std::ostream &Stream, const CqMatrix &Matrix );
		friend std::ostream &operator<<( std::ostream &Stream, CqMatrix &Matrix );
		friend CqMatrix	operator*( TqFloat S, const CqMatrix& a );
		friend bool  operator==(const CqMatrix& A, const CqMatrix& B);
		friend bool  operator!=(const CqMatrix& A, const CqMatrix& B);

		TqFloat	Determinant() const;

	protected:
		TqFloat	m_aaElement[ 4 ][ 4 ];		///< The 4x4 array of float values.
		TqBool	m_fIdentity;			///< Flag indicating that this matrix should be treated as identity irrespective of its contents.
}
;

std::ostream &operator<<( std::ostream &Stream, CqMatrix &Matrix );

//-----------------------------------------------------------------------

CqVector4D operator*( const CqVector4D &Vector, const CqMatrix& Matrix );

END_NAMESPACE( Aqsis )

#endif	// !MATRIX_H_INCLUDED
