// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is matrix.h included already?
#ifndef MATRIX_H_INCLUDED
#define MATRIX_H_INCLUDED 1

#include	<iostream>

#include	"aqsis.h"
#include	"specific.h"
#include	"vector3d.h"
#include	"vector4d.h"

#define		_qShareName	CORE
#include	"share.h"

START_NAMESPACE(Aqsis)

//----------------------------------------------------------------------
/** \class CqMatrix 
 * 4x4 Matrix class definition.
 * Access to matrix elements is always row,column from 0.
 */

class _qShareC CqMatrix
{
	public:
		_qShareM				CqMatrix() : m_fIdentity(TqTrue) {}
		_qShareM				CqMatrix(const TqFloat xs, const TqFloat ys, const TqFloat zs); // Scaled ID
		_qShareM				CqMatrix(const CqVector3D& Trans); // Translated
		_qShareM				CqMatrix(const TqFloat Angle, const CqVector3D Axis); // Rotation2
		_qShareM				CqMatrix(const CqMatrix &From);
								/** Individual element constructor.
								 * Takes 16 floats for the elements of the matrix.
								 */
		_qShareM				CqMatrix(const TqFloat r1c1, const TqFloat r1c2, const TqFloat r1c3, const TqFloat r1c4,
										  const TqFloat r2c1, const TqFloat r2c2, const TqFloat r2c3, const TqFloat r2c4,
										  const TqFloat r3c1, const TqFloat r3c2, const TqFloat r3c3, const TqFloat r3c4,
										  const TqFloat r4c1, const TqFloat r4c2, const TqFloat r4c3, const TqFloat r4c4)
											{
												m_aaElement[0][0]=r1c1; m_aaElement[0][1]=r1c2; m_aaElement[0][2]=r1c3; m_aaElement[0][3]=r1c4;
												m_aaElement[1][0]=r2c1; m_aaElement[1][1]=r2c2; m_aaElement[1][2]=r2c3; m_aaElement[1][3]=r2c4;
												m_aaElement[2][0]=r3c1; m_aaElement[2][1]=r3c2; m_aaElement[2][2]=r3c3; m_aaElement[2][3]=r3c4;
												m_aaElement[3][0]=r4c1; m_aaElement[3][1]=r4c2; m_aaElement[3][2]=r4c3; m_aaElement[3][3]=r4c4;
												m_fIdentity=TqFalse;
											}
		_qShareM				CqMatrix(TqFloat From[4][4]);
		_qShareM				CqMatrix(TqFloat From[16]);
		_qShareM				~CqMatrix() {}

		_qShareM	void		Identity();		// Make identity
								/** Mark this matrix as identity or not.
								 * \param f Bool indicating whether or not this matrix should be considered identity irespective of its contents.
								 */
		_qShareM	void		SetfIdentity(TqBool f)	{m_fIdentity=f;}
		_qShareM	void		Scale(const TqFloat S);
		_qShareM	void		Scale(const TqFloat xs, const TqFloat ys, const TqFloat zs);
		_qShareM	void		Rotate(const TqFloat Angle, const CqVector3D Axis);
		_qShareM	void		Translate(const CqVector3D& Trans);
		_qShareM	void		Translate(const TqFloat xt, const TqFloat yt, const TqFloat zt);
		_qShareM	void		ShearX(const TqFloat yh, const TqFloat zh);
		_qShareM	void		ShearY(const TqFloat xh, const TqFloat zh);
		_qShareM	void		ShearZ(const TqFloat xh, const TqFloat yh);
		_qShareM	void		Normalise();

								/** Get the element at the specified row and column index.
								 * \param iRow The row index.
								 * \param iColumn The column index.
								 * \return Float value.
								 */
		_qShareM	TqFloat		Element(TqInt iRow,TqInt iColumn) const	
								{
									return(m_aaElement[iRow][iColumn]);
								}
								/** Set the element at the specified row and column index.
								 * \param iRow The row index.
								 * \param iColumn The column index.
								 * \param fValue the value to insert.
								 */
		_qShareM	void		SetElement(TqInt iRow,TqInt iColumn, TqFloat fValue)
								{
									m_aaElement[iRow][iColumn]=fValue;
								}
								/** Get a pointer to the row index specified.
								 * \param iRow The row index.
								 * \return Pointer to array of 4 float values.
								 */
		_qShareM	TqFloat*	 operator[](TqInt iRow)			
								{
									return(&m_aaElement[iRow][0]);
								}
								/** Get a read only pointer to the row index specified.
								 * \param iRow The row index.
								 * \return Pointer to array of 4 float values.
								 */
		_qShareM	const TqFloat*	 operator[](TqInt iRow) const	
								{
									return(&m_aaElement[iRow][0]); 
								}
								/** Get a pointer to matrix data.
								 * \return Pointer to array of 16 float values.
								 */
		_qShareM	TqFloat*		pElements()	
								{
									return(&m_aaElement[0][0]);
								}
								/** Get a read only pointer to matrix data.
								 * \return Pointer to array of 16 float values.
								 */
		_qShareM	const TqFloat*  pElements() const	
								{
									return(&m_aaElement[0][0]);
								}

		// Binary operators
		_qShareM	CqMatrix	operator*(const CqMatrix &From) const;
		_qShareM	CqMatrix	operator*(const TqFloat S) const;
		_qShareM	CqVector4D	operator*(const CqVector4D &Vector) const;
		_qShareM	CqVector3D	operator*(const CqVector3D &Vector) const;
		_qShareM	CqMatrix	operator+(const CqVector4D &Vector) const;
		_qShareM	CqMatrix	operator-(const CqVector4D &Vector) const;
		_qShareM	CqMatrix	operator+(const CqMatrix &From) const;
		_qShareM	CqMatrix	operator-(const CqMatrix &From) const;

		_qShareM	CqMatrix	Inverse() const;
		_qShareM	CqMatrix	Transpose() const;

		_qShareM	CqMatrix&	operator=(const CqMatrix &From);
		_qShareM	CqMatrix&	operator=(TqFloat From[4][4]);
		_qShareM	CqMatrix&	operator=(TqFloat From[16]);
		_qShareM	CqMatrix&	operator+=(const CqMatrix &From);
		_qShareM	CqMatrix&	operator-=(const CqMatrix &From);
		_qShareM	CqMatrix&	operator+=(const CqVector4D &Vector);
		_qShareM	CqMatrix&	operator-=(const CqVector4D &Vector);
		_qShareM	CqMatrix&	operator*=(const CqMatrix &From);
		_qShareM	CqMatrix&	PreMultiply(const CqMatrix &From);
		_qShareM	CqVector4D	PreMultiply(const CqVector4D &Vector) const;
		_qShareM	CqMatrix&	operator*=(const TqFloat S);
			
			friend _qShare std::ostream &operator<<(std::ostream &Stream, CqMatrix &Matrix);
			friend CqMatrix	operator*(TqFloat f, const CqMatrix& a);

		_qShareM	TqFloat		Determinant() const;

	protected:
			TqFloat	m_aaElement[4][4];		///< The 4x4 array of float values.
			TqBool	m_fIdentity;			///< Flag indicating that this matrix should be treated as identity irrespective of its contents.
};

_qShare	std::ostream &operator<<(std::ostream &Stream, CqMatrix &Matrix);

//-----------------------------------------------------------------------

_qShare	CqVector4D operator*(const CqVector4D &Vector, const CqMatrix& Matrix);

END_NAMESPACE(Aqsis)

#endif	// !MATRIX_H_INCLUDED
