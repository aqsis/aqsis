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
		\brief Implements the CqMatrix 4D homogenous matrix class.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<math.h>
#include	"matrix.h"

START_NAMESPACE(Aqsis)

//---------------------------------------------------------------------
/** Scale matrix constructor
 * \param xs Float scale in x.
 * \param ys Float scale in y.
 * \param zs Float scale in z.
 */

CqMatrix::CqMatrix(const TqFloat xs, const TqFloat ys, const TqFloat zs)
{
	Identity();

	if(xs==1.0f && ys==1.0f && zs==1.0f)
		m_fIdentity=TqTrue;
	else	
	{
		m_aaElement[0][0]=xs;
		m_aaElement[1][1]=ys;
		m_aaElement[2][2]=zs;

		m_fIdentity=TqFalse;
	}
}


//---------------------------------------------------------------------
/** Translate constructor. Constructs a translation matrix translating by a given vector.
 * \param Trans	The vector by which to translate.
 */

CqMatrix::CqMatrix(const CqVector3D& Trans)
{
	Identity();

	if(Trans.x()==0.0f && Trans.y()==0.0f && Trans.z()==0.0f)
		m_fIdentity=TqTrue;
	else
	{
		m_fIdentity=TqFalse;

		m_aaElement[3][0]=Trans.x();
		m_aaElement[3][1]=Trans.y();
		m_aaElement[3][2]=Trans.z();
	}
}


//---------------------------------------------------------------------
/** Rotate matrix constructor
 * \param Angle	The angle to rotate by.
 * \param Axis The axis about which to rotate.
 */

CqMatrix::CqMatrix(const TqFloat Angle, const CqVector3D Axis)
{
	Identity();
	
	if(Angle!=0.0f && Axis.Magnitude()!=0.0f)
		Rotate(Angle, Axis);
}


//---------------------------------------------------------------------
/** Copy constructor.
 */

CqMatrix::CqMatrix(const CqMatrix &From)
{
	*this=From;	
}

//---------------------------------------------------------------------
/** Constructor from a 2D float array.
 * \param From 2D float array to copy data from.
 */

CqMatrix::CqMatrix(TqFloat From[4][4])
{
	*this=From;	
}


//---------------------------------------------------------------------
/** Constructor from a float array.
 * \param From 1D float array to copy data from.
 */

CqMatrix::CqMatrix(TqFloat From[16])
{
	*this=From;	
}

//---------------------------------------------------------------------
/** Turn the matrix into an identity.
 */

void CqMatrix::Identity()
{
	m_aaElement[0][1]=
	m_aaElement[0][2]=
	m_aaElement[0][3]=
	m_aaElement[1][0]=
	m_aaElement[1][2]=
	m_aaElement[1][3]=
	m_aaElement[2][0]=
	m_aaElement[2][1]=
	m_aaElement[2][3]=
	m_aaElement[3][0]=
	m_aaElement[3][1]=
	m_aaElement[3][2]=0.0f;

	m_aaElement[0][0]=
	m_aaElement[1][1]=
	m_aaElement[2][2]=
	m_aaElement[3][3]=1.0f;

	m_fIdentity=TqTrue;
}


//---------------------------------------------------------------------
/** Scale matrix uniformly in all three axes.
 * \param S	The amount to scale by.
 */

void CqMatrix::Scale(const TqFloat S)
{
	if(S!=1.0f)
		Scale(S, S, S);
}

//---------------------------------------------------------------------
/** Scale matrix in three axes.
 * \param xs X scale factor.
 * \param ys Y scale factor.
 * \param zs Z scale factor.
 */

void CqMatrix::Scale(const TqFloat xs, const TqFloat ys, const TqFloat zs)
{
	CqMatrix Scale(xs, ys, zs);
	
	this->PreMultiply(Scale);
}


//---------------------------------------------------------------------
/** Rotates this matrix by a given rotation angle about a given axis through the origin.
 * \param Angle	The angle to rotate by.
 * \param Axis The axis about which to rotate.
 */

void CqMatrix::Rotate(const TqFloat Angle, const CqVector3D Axis)
{
	if(Angle!=0.0f)
	{
		CqMatrix	R;
		R.Identity();
		CqVector3D	RotAxis=Axis;
		R.m_fIdentity=TqFalse;

		RotAxis.Unit();

		TqFloat	s=static_cast<TqFloat>(sin(Angle));
		TqFloat	c=static_cast<TqFloat>(cos(Angle));
		TqFloat	t=1.0f-c;

		R.m_aaElement[0][0]=t*RotAxis.x()*RotAxis.x()+c;
		R.m_aaElement[1][1]=t*RotAxis.y()*RotAxis.y()+c;
		R.m_aaElement[2][2]=t*RotAxis.z()*RotAxis.z()+c;

		TqFloat	txy=t*RotAxis.x()*RotAxis.y();
		TqFloat	sz =s*RotAxis.z();

		R.m_aaElement[0][1]=txy+sz;
		R.m_aaElement[1][0]=txy-sz;

		TqFloat	txz=t*RotAxis.x()*RotAxis.z();
		TqFloat	sy =s*RotAxis.y();

		R.m_aaElement[0][2]=txz-sy;
		R.m_aaElement[2][0]=txz+sy;

		TqFloat	tyz=t*RotAxis.y()*RotAxis.z();
		TqFloat	sx =s*RotAxis.x();

		R.m_aaElement[1][2]=tyz+sx;
		R.m_aaElement[2][1]=tyz-sx;

		this->PreMultiply(R);
	}
}


//---------------------------------------------------------------------
/** Translates this matrix by a given vector.
 * \param Trans	The vector by which to translate.
 */

void CqMatrix::Translate(const CqVector3D& Trans)
{
	CqMatrix matTrans(Trans);
	this->PreMultiply(matTrans);
}



//---------------------------------------------------------------------
/** Translates this matrix by three axis distances.
 * \param xt X distance to translate.
 * \param yt Y distance to translate.
 * \param zt Z distance to translate.
 */

void CqMatrix::Translate(const TqFloat xt, const TqFloat yt, const TqFloat zt)
{
	if(xt!=0.0f || yt!=0.0f || zt!=0.0f)
		Translate(CqVector3D(xt, yt, zt));
}	


//---------------------------------------------------------------------
/** Shears this matrix's X axis according to two shear factors, yh and zh
 * \param yh Y shear factor.
 * \param zh Z shear factor.
 */

void CqMatrix::ShearX(const TqFloat yh, const TqFloat zh)
{
	CqMatrix Shear;
	Shear.m_fIdentity=TqFalse;
	
	Shear.m_aaElement[0][1]=yh;
	Shear.m_aaElement[0][2]=zh;

	this->PreMultiply(Shear);
}


//---------------------------------------------------------------------
/** Shears this matrix's Y axis according to two shear factors, xh and zh
 * \param xh X shear factor.
 * \param zh Z shear factor.
 */

void CqMatrix::ShearY(const TqFloat xh, const TqFloat zh)
{
	CqMatrix Shear;
	Shear.m_fIdentity=TqFalse;
	
	Shear.m_aaElement[1][0]=xh;
	Shear.m_aaElement[1][2]=zh;

	this->PreMultiply(Shear);
}

//---------------------------------------------------------------------
/** Shears this matrix's Z axis according to two shear factors, xh and yh
 * \param xh X shear factor.
 * \param yh Y shear factor.
 */

void CqMatrix::ShearZ(const TqFloat xh, const TqFloat yh)
{
	CqMatrix Shear;
	Shear.m_fIdentity=TqFalse;
	
	Shear.m_aaElement[2][0]=xh;
	Shear.m_aaElement[2][1]=yh;

	this->PreMultiply(Shear);
}

//---------------------------------------------------------------------
/** Normalise the matrix, returning the homogenous part of the matrix to 1.
 */

void CqMatrix::Normalise()
{
	for(TqInt i = 0; i < 4; i++)
	{
		for(TqInt j = 0; j < 4; j++)
		{
			m_aaElement[i][j] /= m_aaElement[3][3];
		}
	}
}


//---------------------------------------------------------------------
/** Multiply two matrices together.
 * \param From The matrix to multiply with this matrix.
 * \return The resultant multiplied matrix.
 */

CqMatrix CqMatrix::operator*(const CqMatrix &From) const
{
	CqMatrix Temp(*this);
	Temp*=From;
	return(Temp);
}


//---------------------------------------------------------------------
/** Multiply this matrix by specified matrix. This now takes into account the types of matrices, in an attempt to speed it up.
 * \param From The matrix to multiply with this matrix.
 */

CqMatrix &CqMatrix::operator*=(const CqMatrix &From)
{
	if(m_fIdentity)
		return(*this=From);
	else if(From.m_fIdentity)
		return(*this);

	CqMatrix A(*this);

	m_aaElement[0][0]=From.m_aaElement[0][0]*A.m_aaElement[0][0]
					 +From.m_aaElement[0][1]*A.m_aaElement[1][0]
					 +From.m_aaElement[0][2]*A.m_aaElement[2][0]
					 +From.m_aaElement[0][3]*A.m_aaElement[3][0];
	m_aaElement[0][1]=From.m_aaElement[0][0]*A.m_aaElement[0][1]
					 +From.m_aaElement[0][1]*A.m_aaElement[1][1]
					 +From.m_aaElement[0][2]*A.m_aaElement[2][1]
					 +From.m_aaElement[0][3]*A.m_aaElement[3][1];
	m_aaElement[0][2]=From.m_aaElement[0][0]*A.m_aaElement[0][2]
					 +From.m_aaElement[0][1]*A.m_aaElement[1][2]
					 +From.m_aaElement[0][2]*A.m_aaElement[2][2]
					 +From.m_aaElement[0][3]*A.m_aaElement[3][2];
	m_aaElement[0][3]=From.m_aaElement[0][0]*A.m_aaElement[0][3]
					 +From.m_aaElement[0][1]*A.m_aaElement[1][3]
					 +From.m_aaElement[0][2]*A.m_aaElement[2][3]
					 +From.m_aaElement[0][3]*A.m_aaElement[3][3];

	m_aaElement[1][0]=From.m_aaElement[1][0]*A.m_aaElement[0][0]
					 +From.m_aaElement[1][1]*A.m_aaElement[1][0]
					 +From.m_aaElement[1][2]*A.m_aaElement[2][0]
					 +From.m_aaElement[1][3]*A.m_aaElement[3][0];
	m_aaElement[1][1]=From.m_aaElement[1][0]*A.m_aaElement[0][1]
					 +From.m_aaElement[1][1]*A.m_aaElement[1][1]
					 +From.m_aaElement[1][2]*A.m_aaElement[2][1]
					 +From.m_aaElement[1][3]*A.m_aaElement[3][1];
	m_aaElement[1][2]=From.m_aaElement[1][0]*A.m_aaElement[0][2]
					 +From.m_aaElement[1][1]*A.m_aaElement[1][2]
					 +From.m_aaElement[1][2]*A.m_aaElement[2][2]
					 +From.m_aaElement[1][3]*A.m_aaElement[3][2];
	m_aaElement[1][3]=From.m_aaElement[1][0]*A.m_aaElement[0][3]
					 +From.m_aaElement[1][1]*A.m_aaElement[1][3]
					 +From.m_aaElement[1][2]*A.m_aaElement[2][3]
					 +From.m_aaElement[1][3]*A.m_aaElement[3][3];

	m_aaElement[2][0]=From.m_aaElement[2][0]*A.m_aaElement[0][0]
					 +From.m_aaElement[2][1]*A.m_aaElement[1][0]
					 +From.m_aaElement[2][2]*A.m_aaElement[2][0]
					 +From.m_aaElement[2][3]*A.m_aaElement[3][0];
	m_aaElement[2][1]=From.m_aaElement[2][0]*A.m_aaElement[0][1]
					 +From.m_aaElement[2][1]*A.m_aaElement[1][1]
					 +From.m_aaElement[2][2]*A.m_aaElement[2][1]
					 +From.m_aaElement[2][3]*A.m_aaElement[3][1];
	m_aaElement[2][2]=From.m_aaElement[2][0]*A.m_aaElement[0][2]
					 +From.m_aaElement[2][1]*A.m_aaElement[1][2]
					 +From.m_aaElement[2][2]*A.m_aaElement[2][2]
					 +From.m_aaElement[2][3]*A.m_aaElement[3][2];
	m_aaElement[2][3]=From.m_aaElement[2][0]*A.m_aaElement[0][3]
					 +From.m_aaElement[2][1]*A.m_aaElement[1][3]
					 +From.m_aaElement[2][2]*A.m_aaElement[2][3]
					 +From.m_aaElement[2][3]*A.m_aaElement[3][3];

	m_aaElement[3][0]=From.m_aaElement[3][0]*A.m_aaElement[0][0]
					 +From.m_aaElement[3][1]*A.m_aaElement[1][0]
					 +From.m_aaElement[3][2]*A.m_aaElement[2][0]
					 +From.m_aaElement[3][3]*A.m_aaElement[3][0];
	m_aaElement[3][1]=From.m_aaElement[3][0]*A.m_aaElement[0][1]
					 +From.m_aaElement[3][1]*A.m_aaElement[1][1]
					 +From.m_aaElement[3][2]*A.m_aaElement[2][1]
					 +From.m_aaElement[3][3]*A.m_aaElement[3][1];
	m_aaElement[3][2]=From.m_aaElement[3][0]*A.m_aaElement[0][2]
					 +From.m_aaElement[3][1]*A.m_aaElement[1][2]
					 +From.m_aaElement[3][2]*A.m_aaElement[2][2]
					 +From.m_aaElement[3][3]*A.m_aaElement[3][2];
	m_aaElement[3][3]=From.m_aaElement[3][0]*A.m_aaElement[0][3]
					 +From.m_aaElement[3][1]*A.m_aaElement[1][3]
					 +From.m_aaElement[3][2]*A.m_aaElement[2][3]
					 +From.m_aaElement[3][3]*A.m_aaElement[3][3];

	m_fIdentity=TqFalse;
	return(*this);
}

//---------------------------------------------------------------------
/** Matrix multiplication of the form a = b * a.
 * \param From The matrix to multiply with this matrix.
 */

CqMatrix& CqMatrix::PreMultiply(const CqMatrix &From)
{
	if(m_fIdentity)
		return(*this=From);
	else if(From.m_fIdentity)
		return(*this);

	CqMatrix A(*this);

	m_aaElement[0][0]=A.m_aaElement[0][0]*From.m_aaElement[0][0]
				   +A.m_aaElement[0][1]*From.m_aaElement[1][0]
				   +A.m_aaElement[0][2]*From.m_aaElement[2][0]
				   +A.m_aaElement[0][3]*From.m_aaElement[3][0];
	m_aaElement[0][1]=A.m_aaElement[0][0]*From.m_aaElement[0][1]
				   +A.m_aaElement[0][1]*From.m_aaElement[1][1]
				   +A.m_aaElement[0][2]*From.m_aaElement[2][1]
				   +A.m_aaElement[0][3]*From.m_aaElement[3][1];
	m_aaElement[0][2]=A.m_aaElement[0][0]*From.m_aaElement[0][2]
				   +A.m_aaElement[0][1]*From.m_aaElement[1][2]
				   +A.m_aaElement[0][2]*From.m_aaElement[2][2]
				   +A.m_aaElement[0][3]*From.m_aaElement[3][2];
	m_aaElement[0][3]=A.m_aaElement[0][0]*From.m_aaElement[0][3]
				   +A.m_aaElement[0][1]*From.m_aaElement[1][3]
				   +A.m_aaElement[0][2]*From.m_aaElement[2][3]
				   +A.m_aaElement[0][3]*From.m_aaElement[3][3];

	m_aaElement[1][0]=A.m_aaElement[1][0]*From.m_aaElement[0][0]
				   +A.m_aaElement[1][1]*From.m_aaElement[1][0]
				   +A.m_aaElement[1][2]*From.m_aaElement[2][0]
				   +A.m_aaElement[1][3]*From.m_aaElement[3][0];
	m_aaElement[1][1]=A.m_aaElement[1][0]*From.m_aaElement[0][1]
				   +A.m_aaElement[1][1]*From.m_aaElement[1][1]
				   +A.m_aaElement[1][2]*From.m_aaElement[2][1]
				   +A.m_aaElement[1][3]*From.m_aaElement[3][1];
	m_aaElement[1][2]=A.m_aaElement[1][0]*From.m_aaElement[0][2]
				   +A.m_aaElement[1][1]*From.m_aaElement[1][2]
				   +A.m_aaElement[1][2]*From.m_aaElement[2][2]
				   +A.m_aaElement[1][3]*From.m_aaElement[3][2];
	m_aaElement[1][3]=A.m_aaElement[1][0]*From.m_aaElement[0][3]
				   +A.m_aaElement[1][1]*From.m_aaElement[1][3]
				   +A.m_aaElement[1][2]*From.m_aaElement[2][3]
				   +A.m_aaElement[1][3]*From.m_aaElement[3][3];

	m_aaElement[2][0]=A.m_aaElement[2][0]*From.m_aaElement[0][0]
				   +A.m_aaElement[2][1]*From.m_aaElement[1][0]
				   +A.m_aaElement[2][2]*From.m_aaElement[2][0]
				   +A.m_aaElement[2][3]*From.m_aaElement[3][0];
	m_aaElement[2][1]=A.m_aaElement[2][0]*From.m_aaElement[0][1]
				   +A.m_aaElement[2][1]*From.m_aaElement[1][1]
				   +A.m_aaElement[2][2]*From.m_aaElement[2][1]
				   +A.m_aaElement[2][3]*From.m_aaElement[3][1];
	m_aaElement[2][2]=A.m_aaElement[2][0]*From.m_aaElement[0][2]
				   +A.m_aaElement[2][1]*From.m_aaElement[1][2]
				   +A.m_aaElement[2][2]*From.m_aaElement[2][2]
				   +A.m_aaElement[2][3]*From.m_aaElement[3][2];
	m_aaElement[2][3]=A.m_aaElement[2][0]*From.m_aaElement[0][3]
				   +A.m_aaElement[2][1]*From.m_aaElement[1][3]
				   +A.m_aaElement[2][2]*From.m_aaElement[2][3]
				   +A.m_aaElement[2][3]*From.m_aaElement[3][3];

	m_aaElement[3][0]=A.m_aaElement[3][0]*From.m_aaElement[0][0]
				   +A.m_aaElement[3][1]*From.m_aaElement[1][0]
				   +A.m_aaElement[3][2]*From.m_aaElement[2][0]
				   +A.m_aaElement[3][3]*From.m_aaElement[3][0];
	m_aaElement[3][1]=A.m_aaElement[3][0]*From.m_aaElement[0][1]
				   +A.m_aaElement[3][1]*From.m_aaElement[1][1]
				   +A.m_aaElement[3][2]*From.m_aaElement[2][1]
				   +A.m_aaElement[3][3]*From.m_aaElement[3][1];
	m_aaElement[3][2]=A.m_aaElement[3][0]*From.m_aaElement[0][2]
				   +A.m_aaElement[3][1]*From.m_aaElement[1][2]
				   +A.m_aaElement[3][2]*From.m_aaElement[2][2]
				   +A.m_aaElement[3][3]*From.m_aaElement[3][2];
	m_aaElement[3][3]=A.m_aaElement[3][0]*From.m_aaElement[0][3]
				   +A.m_aaElement[3][1]*From.m_aaElement[1][3]
				   +A.m_aaElement[3][2]*From.m_aaElement[2][3]
				   +A.m_aaElement[3][3]*From.m_aaElement[3][3];

	m_fIdentity=TqFalse;
	return(*this);
}

//---------------------------------------------------------------------
/** Premultiplies this matrix by a vector, returning v*m. This is the same as postmultiply the transpose of m by a vector: T(m)*v
 * \param Vector The vector to multiply.
 * \return The result of multiplying the vector by this matrix.
 */

CqVector4D CqMatrix::PreMultiply(const CqVector4D &Vector) const
{
	if(m_fIdentity)
		return(Vector);

	CqVector4D	Result;
	
	Result.x(m_aaElement[0][0]*Vector.x()
	  		+m_aaElement[0][1]*Vector.y()
	  		+m_aaElement[0][2]*Vector.z()
	  		+m_aaElement[0][3]*Vector.h());
	   	
	Result.y(m_aaElement[1][0]*Vector.x()
	  		+m_aaElement[1][1]*Vector.y()
	  		+m_aaElement[1][2]*Vector.z()
	  		+m_aaElement[1][3]*Vector.h());

	Result.z(m_aaElement[2][0]*Vector.x()
	  		+m_aaElement[2][1]*Vector.y()
	  		+m_aaElement[2][2]*Vector.z()
	  		+m_aaElement[2][3]*Vector.h());

	Result.h(m_aaElement[3][0]*Vector.x()
	  		+m_aaElement[3][1]*Vector.y()
	  		+m_aaElement[3][2]*Vector.z()
	  		+m_aaElement[3][3]*Vector.h());

	return(Result);
}



//---------------------------------------------------------------------
/** Apply scale matrix uniformly in all three axes.
 * \param S The amount by which to scale matrix.
 * \return Result of scaling this matrix by S.
 */

CqMatrix CqMatrix::operator*(const TqFloat S) const
{
	CqMatrix Temp(*this);
	Temp*=S;
	return(Temp);
}


//---------------------------------------------------------------------
/** Apply scale matrix uniformly in all three axes to this matrix.
 * \param S The amount by which to scale this matrix.
 * \return The result scaling this matrix by S.
 */

CqMatrix &CqMatrix::operator*=(const TqFloat S)
{
	CqMatrix ScaleMatrix(S, S, S);

	this->PreMultiply(ScaleMatrix);	

	return(*this);
}


//---------------------------------------------------------------------
/** Multiply a vector by this matrix.
 * \param Vector	: The vector to multiply.
 * \return The result of multiplying the vector by this matrix.
 */

CqVector4D CqMatrix::operator*(const CqVector4D &Vector) const
{
	if(m_fIdentity)
		return(Vector);

	CqVector4D	Result;
	
	Result.x(m_aaElement[0][0]*Vector.x()
	  		+m_aaElement[1][0]*Vector.y()
	  		+m_aaElement[2][0]*Vector.z()
	  		+m_aaElement[3][0]*Vector.h());
	   	
	Result.y(m_aaElement[0][1]*Vector.x()
	  		+m_aaElement[1][1]*Vector.y()
	  		+m_aaElement[2][1]*Vector.z()
	  		+m_aaElement[3][1]*Vector.h());

	Result.z(m_aaElement[0][2]*Vector.x()
	  		+m_aaElement[1][2]*Vector.y()
	  		+m_aaElement[2][2]*Vector.z()
	  		+m_aaElement[3][2]*Vector.h());

	Result.h(m_aaElement[0][3]*Vector.x()
	  		+m_aaElement[1][3]*Vector.y()
	  		+m_aaElement[2][3]*Vector.z()
	  		+m_aaElement[3][3]*Vector.h());

	return(Result);
}


//---------------------------------------------------------------------
/** Multiply a vector by this matrix.
 * \param Vector The vector to multiply.
 * \return The result of multiplying the vector by this matrix.
 */

CqVector3D CqMatrix::operator*(const CqVector3D &Vector) const
{
	if(m_fIdentity)
		return(Vector);

	CqVector3D	Result;
	
	TqFloat h=(m_aaElement[0][3]*Vector.x()
	  		  +m_aaElement[1][3]*Vector.y()
	  		  +m_aaElement[2][3]*Vector.z()
	  		  +m_aaElement[3][3]);

	Result.x((m_aaElement[0][0]*Vector.x()
	  		 +m_aaElement[1][0]*Vector.y()
	  		 +m_aaElement[2][0]*Vector.z()
	  		 +m_aaElement[3][0])/h);
	   	
	Result.y((m_aaElement[0][1]*Vector.x()
	  		 +m_aaElement[1][1]*Vector.y()
	  		 +m_aaElement[2][1]*Vector.z()
	  		 +m_aaElement[3][1])/h);

	Result.z((m_aaElement[0][2]*Vector.x()
	  		 +m_aaElement[1][2]*Vector.y()
	  		 +m_aaElement[2][2]*Vector.z()
	  		 +m_aaElement[3][2])/h);
	

	return(Result);
}


//---------------------------------------------------------------------
/** Translate matrix by 4D Vector.
 * \param Vector The vector to translate by.
 * \return Result of translating this matrix by the vector.
 */

CqMatrix CqMatrix::operator+(const CqVector4D &Vector) const
{
 	CqMatrix Temp(*this);
	Temp+=Vector;
	return(Temp);
}


//---------------------------------------------------------------------
/** Translate this matrix by 4D Vector.
 * \param Vector The vector to translate by.
 * \return The result of translating this matrix by the specified vector.
 */

CqMatrix &CqMatrix::operator+=(const CqVector4D &Vector)
{
	CqMatrix Trans(Vector);
	
	this->PreMultiply(Trans);

	return(*this);
}


//---------------------------------------------------------------------
/** Translate matrix by 4D Vector.
 * \param Vector The vector to translate by.
 * \return Result of translating this matrix by the vector.
 */

CqMatrix CqMatrix::operator-(const CqVector4D &Vector) const
{
 	CqMatrix Temp(*this);
	Temp-=Vector;
	return(Temp);
}


//---------------------------------------------------------------------
/** Translate this matrix by 4D Vector.
 * \param Vector The vector to translate by.
 */

CqMatrix &CqMatrix::operator-=(const CqVector4D &Vector)
{
	CqVector4D Temp(Vector);	

	Temp.x(-Temp.x());
	Temp.y(-Temp.y());
	Temp.z(-Temp.z());
	
	CqMatrix Trans(Temp);
	
	this->PreMultiply(Trans);

	return(*this);
}


//---------------------------------------------------------------------
/** Add two matrices.
 * \param From The matrix to add.
 * \return Result of adding From to this matrix.
 */

CqMatrix CqMatrix::operator+(const CqMatrix &From) const
{
 	CqMatrix Temp(*this);
	Temp+=From;
	return(Temp);
}


//---------------------------------------------------------------------
/** Add a given matrix to this matrix.
 * \param From The matrix to add.
 */

CqMatrix &CqMatrix::operator+=(const CqMatrix &From)
{
	m_aaElement[0][0]+=From.m_aaElement[0][0];
	m_aaElement[1][0]+=From.m_aaElement[1][0];
	m_aaElement[2][0]+=From.m_aaElement[2][0];
	m_aaElement[3][0]+=From.m_aaElement[3][0];
	m_aaElement[0][1]+=From.m_aaElement[0][1];
	m_aaElement[1][1]+=From.m_aaElement[1][1];
	m_aaElement[2][1]+=From.m_aaElement[2][1];
	m_aaElement[3][1]+=From.m_aaElement[3][1];
	m_aaElement[0][2]+=From.m_aaElement[0][2];
	m_aaElement[1][2]+=From.m_aaElement[1][2];
	m_aaElement[2][2]+=From.m_aaElement[2][2];
	m_aaElement[3][2]+=From.m_aaElement[3][2];
	m_aaElement[0][3]+=From.m_aaElement[0][3];
	m_aaElement[1][3]+=From.m_aaElement[1][3];
	m_aaElement[2][3]+=From.m_aaElement[2][3];
	m_aaElement[3][3]+=From.m_aaElement[3][3];

	m_fIdentity=TqFalse;

	return(*this);
}


//---------------------------------------------------------------------
/** Subtract two matrices.
 * \param From The matrix to subtract.
 * \return Result of subtracting From from this matrix.
 */

CqMatrix CqMatrix::operator-(const CqMatrix &From) const
{
 	CqMatrix Temp(*this);
	Temp-=From;
	return(Temp);
}


//---------------------------------------------------------------------
/** Subtract a given matrix from this matrix.
 * \param From The matrix to subtract.
 */

CqMatrix &CqMatrix::operator-=(const CqMatrix &From)
{
	m_aaElement[0][0]-=From.m_aaElement[0][0];
	m_aaElement[1][0]-=From.m_aaElement[1][0];
	m_aaElement[2][0]-=From.m_aaElement[2][0];
	m_aaElement[3][0]-=From.m_aaElement[3][0];
	m_aaElement[0][1]-=From.m_aaElement[0][1];
	m_aaElement[1][1]-=From.m_aaElement[1][1];
	m_aaElement[2][1]-=From.m_aaElement[2][1];
	m_aaElement[3][1]-=From.m_aaElement[3][1];
	m_aaElement[0][2]-=From.m_aaElement[0][2];
	m_aaElement[1][2]-=From.m_aaElement[1][2];
	m_aaElement[2][2]-=From.m_aaElement[2][2];
	m_aaElement[3][2]-=From.m_aaElement[3][2];
	m_aaElement[0][3]-=From.m_aaElement[0][3];
	m_aaElement[1][3]-=From.m_aaElement[1][3];
	m_aaElement[2][3]-=From.m_aaElement[2][3];
	m_aaElement[3][3]-=From.m_aaElement[3][3];

	m_fIdentity=TqFalse;

	return(*this);
}


//---------------------------------------------------------------------
/** Copy function.
 * \param From Matrix to copy information from.
 */

CqMatrix &CqMatrix::operator=(const CqMatrix &From)
{
	m_aaElement[0][0]=From.m_aaElement[0][0];
	m_aaElement[1][0]=From.m_aaElement[1][0];
	m_aaElement[2][0]=From.m_aaElement[2][0];
	m_aaElement[3][0]=From.m_aaElement[3][0];
	m_aaElement[0][1]=From.m_aaElement[0][1];
	m_aaElement[1][1]=From.m_aaElement[1][1];
	m_aaElement[2][1]=From.m_aaElement[2][1];
	m_aaElement[3][1]=From.m_aaElement[3][1];
	m_aaElement[0][2]=From.m_aaElement[0][2];
	m_aaElement[1][2]=From.m_aaElement[1][2];
	m_aaElement[2][2]=From.m_aaElement[2][2];
	m_aaElement[3][2]=From.m_aaElement[3][2];
	m_aaElement[0][3]=From.m_aaElement[0][3];
	m_aaElement[1][3]=From.m_aaElement[1][3];
	m_aaElement[2][3]=From.m_aaElement[2][3];
	m_aaElement[3][3]=From.m_aaElement[3][3];

	m_fIdentity=From.m_fIdentity;

 	return(*this);
}

//---------------------------------------------------------------------
/** Copy function.
 * \param From Renderman matrix to copy information from.
 */

CqMatrix &CqMatrix::operator=(TqFloat From[4][4])
{
	m_aaElement[0][0]=From[0][0];
	m_aaElement[1][0]=From[1][0];
	m_aaElement[2][0]=From[2][0];
	m_aaElement[3][0]=From[3][0];
	m_aaElement[0][1]=From[0][1];
	m_aaElement[1][1]=From[1][1];
	m_aaElement[2][1]=From[2][1];
	m_aaElement[3][1]=From[3][1];
	m_aaElement[0][2]=From[0][2];
	m_aaElement[1][2]=From[1][2];
	m_aaElement[2][2]=From[2][2];
	m_aaElement[3][2]=From[3][2];
	m_aaElement[0][3]=From[0][3];
	m_aaElement[1][3]=From[1][3];
	m_aaElement[2][3]=From[2][3];
	m_aaElement[3][3]=From[3][3];

	m_fIdentity=TqFalse;

 	return(*this);
}

//---------------------------------------------------------------------
/** Copy function.
 * \param From Renderman matrix to copy information from.
 */

CqMatrix &CqMatrix::operator=(TqFloat From[16])
{
	m_aaElement[0][0]=From[0];
	m_aaElement[0][1]=From[1];
	m_aaElement[0][2]=From[2];
	m_aaElement[0][3]=From[3];
	m_aaElement[1][0]=From[4];
	m_aaElement[1][1]=From[5];
	m_aaElement[1][2]=From[6];
	m_aaElement[1][3]=From[7];
	m_aaElement[2][0]=From[8];
	m_aaElement[2][1]=From[9];
	m_aaElement[2][2]=From[10];
	m_aaElement[2][3]=From[11];
	m_aaElement[3][0]=From[12];
	m_aaElement[3][1]=From[13];
	m_aaElement[3][2]=From[14];
	m_aaElement[3][3]=From[15];

	m_fIdentity=TqFalse;

 	return(*this);
}


//---------------------------------------------------------------------
/** Returns the inverse of this matrix using an algorithm from Graphics
 * \param Gems IV (p554) - Gauss-Jordan elimination with partial pivoting
 */

CqMatrix CqMatrix::Inverse() const
{
	CqMatrix b;		// b evolves from identity into inverse(a)
	CqMatrix a(*this);	// a evolves from original matrix into identity

	if(m_fIdentity)
	{
		b = *this;
	}
	else
	{
		b.Identity();
		b.m_fIdentity=TqFalse;

		TqInt i;
		TqInt j;
		TqInt i1;

		// Loop over cols of a from left to right, eliminating above and below diag
		for(j = 0; j < 4; j++)	// Find largest pivot in column j among rows j..3
		{
			i1 = j;
			for(i = j + 1; i < 4; i++)
			{
				if(fabs(a.m_aaElement[i][j]) > fabs(a.m_aaElement[i1][j]))
				{
					i1 = i;
				}
			}

			if (i1 != j)
			{
				// Swap rows i1 and j in a and b to put pivot on diagonal
				TqFloat temp;

				temp = a.m_aaElement[i1][0];
				a.m_aaElement[i1][0] = a.m_aaElement[j][0];
				a.m_aaElement[j][0] = temp;
				temp = a.m_aaElement[i1][1];
				a.m_aaElement[i1][1] = a.m_aaElement[j][1];
				a.m_aaElement[j][1] = temp;
				temp = a.m_aaElement[i1][2];
				a.m_aaElement[i1][2] = a.m_aaElement[j][2];
				a.m_aaElement[j][2] = temp;
				temp = a.m_aaElement[i1][3];
				a.m_aaElement[i1][3] = a.m_aaElement[j][3];
				a.m_aaElement[j][3] = temp;

				temp = b.m_aaElement[i1][0];
				b.m_aaElement[i1][0] = b.m_aaElement[j][0];
				b.m_aaElement[j][0] = temp;
				temp = b.m_aaElement[i1][1];
				b.m_aaElement[i1][1] = b.m_aaElement[j][1];
				b.m_aaElement[j][1] = temp;
				temp = b.m_aaElement[i1][2];
				b.m_aaElement[i1][2] = b.m_aaElement[j][2];
				b.m_aaElement[j][2] = temp;
				temp = b.m_aaElement[i1][3];
				b.m_aaElement[i1][3] = b.m_aaElement[j][3];
				b.m_aaElement[j][3] = temp;
			}

			// Scale row j to have a unit diagonal
			if(a.m_aaElement[j][j] == 0.0f)
			{
				// Can't invert a singular matrix!
//				throw XssMatrix(MatrixErr_Singular, __LINE__, ssMakeWide(__FILE__));
			}
			TqFloat scale = 1.0f/a.m_aaElement[j][j];
			b.m_aaElement[j][0] *= scale;
			b.m_aaElement[j][1] *= scale;
			b.m_aaElement[j][2] *= scale;
			b.m_aaElement[j][3] *= scale;
			// all elements to left of a[j][j] are already zero
			for (i1=j+1; i1<4; i1++)
			{
				a.m_aaElement[j][i1] *= scale;
			}
			a.m_aaElement[j][j] = 1.0f;

			// Eliminate off-diagonal elements in column j of a, doing identical ops to b
			for(i = 0; i < 4; i++)
			{
				if(i != j)
				{
					scale = a.m_aaElement[i][j];
					b.m_aaElement[i][0] -= scale * b.m_aaElement[j][0];
					b.m_aaElement[i][1] -= scale * b.m_aaElement[j][1];
					b.m_aaElement[i][2] -= scale * b.m_aaElement[j][2];
					b.m_aaElement[i][3] -= scale * b.m_aaElement[j][3];

					// all elements to left of a[j][j] are zero
					// a[j][j] is 1.0
					for (i1=j+1; i1<4; i1++)
					{
						a.m_aaElement[i][i1] -= scale * a.m_aaElement[j][i1];
					}
					a.m_aaElement[i][j] = 0.0f;
				}
			}
		}
	}

	return b;
}

//---------------------------------------------------------------------
/** Returns the transpose of this matrix
 */

CqMatrix CqMatrix::Transpose() const
{
	CqMatrix Temp;

	if(m_fIdentity)
	{
		Temp = *this;
	}
	else
	{
		Temp.m_aaElement[0][0] = m_aaElement[0][0];
		Temp.m_aaElement[0][1] = m_aaElement[1][0];
		Temp.m_aaElement[0][2] = m_aaElement[2][0];
		Temp.m_aaElement[0][3] = m_aaElement[3][0];
		Temp.m_aaElement[1][0] = m_aaElement[0][1];
		Temp.m_aaElement[1][1] = m_aaElement[1][1];
		Temp.m_aaElement[1][2] = m_aaElement[2][1];
		Temp.m_aaElement[1][3] = m_aaElement[3][1];
		Temp.m_aaElement[2][0] = m_aaElement[0][2];
		Temp.m_aaElement[2][1] = m_aaElement[1][2];
		Temp.m_aaElement[2][2] = m_aaElement[2][2];
		Temp.m_aaElement[2][3] = m_aaElement[3][2];
		Temp.m_aaElement[3][0] = m_aaElement[0][3];
		Temp.m_aaElement[3][1] = m_aaElement[1][3];
		Temp.m_aaElement[3][2] = m_aaElement[2][3];
		Temp.m_aaElement[3][3] = m_aaElement[3][3];

		Temp.m_fIdentity=TqFalse;
	}
	
	return(Temp);
}


//---------------------------------------------------------------------
/** A utility function, used by CqMatrix::GetDeterminant
 * Calculates the determinant of a 2x2 matrix
 */

static TqFloat det2x2(TqFloat a, TqFloat b, TqFloat c, TqFloat d)
{
	return a * d - b * c;
}

//---------------------------------------------------------------------
/** A utility function, used by CqMatrix::GetDeterminant
 * Calculates the determinant of a 3x3 matrix
 */

static TqFloat det3x3(TqFloat a1, TqFloat a2, TqFloat a3,
					   TqFloat b1, TqFloat b2, TqFloat b3,
					   TqFloat c1, TqFloat c2, TqFloat c3)
{
	return a1 * det2x2(b2, b3, c2, c3) -
		   b1 * det2x2(a2, a3, c2, c3) +
		   c1 * det2x2(a2, a3, b2, b3);
}


//---------------------------------------------------------------------
/** Returns the determinant of this matrix using an algorithm from
 * Graphics Gems I (p768)
 */

TqFloat CqMatrix::Determinant() const
{
	// Assign to individual variable names to aid selecting correct elements
	TqFloat a1 = m_aaElement[0][0];
	TqFloat b1 = m_aaElement[0][1];
	TqFloat c1 = m_aaElement[0][2];
	TqFloat d1 = m_aaElement[0][3];

	TqFloat a2 = m_aaElement[1][0];
	TqFloat b2 = m_aaElement[1][1];
	TqFloat c2 = m_aaElement[1][2];
	TqFloat d2 = m_aaElement[1][3];

	TqFloat a3 = m_aaElement[2][0];
	TqFloat b3 = m_aaElement[2][1];
	TqFloat c3 = m_aaElement[2][2];
	TqFloat d3 = m_aaElement[2][3];

	TqFloat a4 = m_aaElement[3][0];
	TqFloat b4 = m_aaElement[3][1];
	TqFloat c4 = m_aaElement[3][2];
	TqFloat d4 = m_aaElement[3][3];

	return a1 * det3x3(b2, b3, b4, c2, c3, c4, d2, d3, d4) -
		   b1 * det3x3(a2, a3, a4, c2, c3, c4, d2, d3, d4) +
		   c1 * det3x3(a2, a3, a4, b2, b3, b4, d2, d3, d4) -
		   d1 * det3x3(a2, a3, a4, b2, b3, b4, c2, c3, c4);
}

//----------------------------------------------------------------------
/** Outputs a matrix to an output stream.
 * \param Stream Stream to output the matrix to.
 * \param Matrix The matrix to output.
 * \return The new state of Stream.
 */

std::ostream &operator<<(std::ostream &Stream, CqMatrix &Matrix)
{
	Stream << "|" << Matrix.m_aaElement[0][0] <<
			  "," << Matrix.m_aaElement[0][1] <<
			  "," << Matrix.m_aaElement[0][2] <<
			  "," << Matrix.m_aaElement[0][3] << "|" << std::endl <<
			  "|" << Matrix.m_aaElement[1][0] <<
			  "," << Matrix.m_aaElement[1][1] <<
			  "," << Matrix.m_aaElement[1][2] <<
			  "," << Matrix.m_aaElement[1][3] << "|" << std::endl <<
			  "|" << Matrix.m_aaElement[2][0] <<
			  "," << Matrix.m_aaElement[2][1] <<
			  "," << Matrix.m_aaElement[2][2] <<
			  "," << Matrix.m_aaElement[2][3] << "|" << std::endl <<
			  "|" << Matrix.m_aaElement[3][0] <<
			  "," << Matrix.m_aaElement[3][1] <<
			  "," << Matrix.m_aaElement[3][2] <<
			  "," << Matrix.m_aaElement[3][3] << "|" << std::ends;														

	return(Stream);
}


//---------------------------------------------------------------------
/** Scale each element by the specified value.
 * \param S The amount by which to scale the matrix elements.
 * \return Result of scaling this matrix by S.
 */

CqMatrix operator*(TqFloat S, const CqMatrix& a)
{
	CqMatrix Temp(a);
	Temp.m_aaElement[0][0]*=S;
	Temp.m_aaElement[1][0]*=S;
	Temp.m_aaElement[2][0]*=S;
	Temp.m_aaElement[3][0]*=S;

	Temp.m_aaElement[0][1]*=S;
	Temp.m_aaElement[1][1]*=S;
	Temp.m_aaElement[2][1]*=S;
	Temp.m_aaElement[3][1]*=S;

	Temp.m_aaElement[0][2]*=S;
	Temp.m_aaElement[1][2]*=S;
	Temp.m_aaElement[2][2]*=S;
	Temp.m_aaElement[3][2]*=S;

	Temp.m_aaElement[0][3]*=S;
	Temp.m_aaElement[1][3]*=S;
	Temp.m_aaElement[2][3]*=S;
	Temp.m_aaElement[3][3]*=S;
	return(Temp);
}


//---------------------------------------------------------------------
/** Premultiply matrix by vector.
 */

CqVector4D operator*(const CqVector4D &Vector, const CqMatrix& Matrix)
{
	return(Matrix.PreMultiply(Vector));
}

//---------------------------------------------------------------------

END_NAMESPACE(Aqsis)
