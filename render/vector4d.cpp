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
		\brief Implements the CqVector4D homogenous vector class.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<math.h>

#include	"aqsis.h"
#include	"vector4d.h"
#include	"vector3d.h"

START_NAMESPACE(Aqsis)

//---------------------------------------------------------------------
/** Copy constructor from 3D Vector.
 */

CqVector4D::CqVector4D(const CqVector3D &From)
{
	*this=From;
}

//---------------------------------------------------------------------
/** Return magnitude squared of this vector.
 */

TqFloat CqVector4D::Magnitude2() const
{
	if(m_h==1.0)
		return((m_x*m_x)+(m_y*m_y)+(m_z*m_z));
	else
		return(((m_x*m_x)+(m_y*m_y)+(m_z*m_z))/(m_h*m_h));
}

//---------------------------------------------------------------------
/** Return magnitude of this vector.
 */

TqFloat CqVector4D::Magnitude() const
{
	return(sqrt(Magnitude2()));
}

//---------------------------------------------------------------------
/** Rescale this vector to be a unit vector.
 */

void CqVector4D::Unit()
{
	m_h=(Magnitude());		
}


//---------------------------------------------------------------------
/** Renormalises vector so that h=1.
 */

void CqVector4D::Homogenize()
{
	if(m_h!=1.0)
	{
		m_x/=m_h;	
		m_y/=m_h;	
		m_z/=m_h;	
		m_h=1.0;
	}
}


//---------------------------------------------------------------------
/** Add a vector to this vector.
 */

CqVector4D &CqVector4D::operator+=(const CqVector4D &From)
{
	TqFloat Hom=m_h/From.m_h;
	
	m_x+=From.m_x*Hom;
	m_y+=From.m_y*Hom;
	m_z+=From.m_z*Hom;

	return(*this);
}



//---------------------------------------------------------------------
/** Subtract a vector from this vector.
 */

CqVector4D &CqVector4D::operator-=(const CqVector4D &From)
{
	TqFloat Hom=m_h/From.m_h;
	
	m_x-=From.m_x*Hom;
	m_y-=From.m_y*Hom;
	m_z-=From.m_z*Hom;

	return(*this);
}

//---------------------------------------------------------------------
/** Dot product of two vectors.
 */

TqFloat operator*(const CqVector4D &a, const CqVector4D &From)
{
	CqVector4D	A(a);
	CqVector4D	B(From);

	A.Homogenize();
	B.Homogenize();

	return((A.m_x*B.m_x)+
		   (A.m_y*B.m_y)+
		   (A.m_z*B.m_z));
}


//---------------------------------------------------------------------
/** Cross product of two vectors.
 */

CqVector4D operator%(const CqVector4D &a, const CqVector4D &From)
{
	CqVector4D Temp(a);
	Temp%=From;
	return(Temp);
}


//---------------------------------------------------------------------
/** Sets this vector to be the cross product of itself and another vector.
 */

CqVector4D &CqVector4D::operator%=(const CqVector4D &From)
{
	CqVector4D	A(*this);
	CqVector4D	B(From);

	A.Homogenize();
	B.Homogenize();
	
	m_x=(A.m_y*B.m_z)-(A.m_z*B.m_y);
	m_y=(A.m_z*B.m_x)-(A.m_x*B.m_z);
	m_z=(A.m_x*B.m_y)-(A.m_y*B.m_x);

	return(*this);
}


//---------------------------------------------------------------------
/** Copy from specified 3D vector.
 */

CqVector4D &CqVector4D::operator=(const CqVector3D &From)
{
	m_x=From.x();
	m_y=From.y();
	m_z=From.z();
	m_h=1.0;

	return(*this);
}


//---------------------------------------------------------------------
/** Scale this vector by the specifed scale factor.
 */

CqVector4D &CqVector4D::operator*=(const TqFloat Scale)
{
	m_h/=Scale;

	return(*this);
}	



//---------------------------------------------------------------------
/** Divide this vector by the specifed scale factor.
 */

CqVector4D &CqVector4D::operator/=(const TqFloat Scale)
{
	m_h*=Scale;

	return(*this);
}	


//---------------------------------------------------------------------
/** Compare two vectors for equality.
 */

TqBool CqVector4D::operator==(const CqVector4D &Cmp) const
{
	TqFloat Hom=m_h/Cmp.m_h;

	return((m_x==(Cmp.m_x*Hom)) &&
		   (m_y==(Cmp.m_y*Hom)) &&
		   (m_z==(Cmp.m_z*Hom)));
}	


//---------------------------------------------------------------------
/** Compare two vectors for inequality.
 */

TqBool CqVector4D::operator!=(const CqVector4D &Cmp) const
{
	return(!(*this==Cmp));
}	


//---------------------------------------------------------------------
/** Compare two vectors for greater than or equal.
 */

TqBool CqVector4D::operator>=(const CqVector4D &Cmp) const
{
	TqFloat Hom=m_h/Cmp.m_h;

	return((m_x>=(Cmp.m_x*Hom)) &&
		   (m_y>=(Cmp.m_y*Hom)) &&
		   (m_z>=(Cmp.m_z*Hom)));
}	


//---------------------------------------------------------------------
/** Compare two vectors for less than or equal.
 */

TqBool CqVector4D::operator<=(const CqVector4D &Cmp) const
{
	TqFloat Hom=m_h/Cmp.m_h;

	return((m_x<=(Cmp.m_x*Hom)) &&
		   (m_y<=(Cmp.m_y*Hom)) &&
		   (m_z<=(Cmp.m_z*Hom)));
}	


//---------------------------------------------------------------------
/** Compare two vectors for greater than.
 */

TqBool CqVector4D::operator>(const CqVector4D &Cmp) const
{
	TqFloat Hom=m_h/Cmp.m_h;

	return((m_x>(Cmp.m_x*Hom)) &&
		   (m_y>(Cmp.m_y*Hom)) &&
		   (m_z>(Cmp.m_z*Hom)));
}	


//---------------------------------------------------------------------
/** Compare two vectors for less than.
 */

TqBool CqVector4D::operator<(const CqVector4D &Cmp) const
{
	TqFloat Hom=m_h/Cmp.m_h;

	return((m_x<(Cmp.m_x*Hom)) &&
		   (m_y<(Cmp.m_y*Hom)) &&
		   (m_z<(Cmp.m_z*Hom)));
}	


//---------------------------------------------------------------------

END_NAMESPACE(Aqsis)
 