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
		\brief Implements the CqVector3D class.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<math.h>

#include	"aqsis.h"
#include	"vector3d.h"
#include	"shaderstack.h"
#include	"vector4d.h"

START_NAMESPACE(Aqsis)

//---------------------------------------------------------------------
/** Copy constructor from 4D Vector.
 */

CqVector3D::CqVector3D(const CqVector4D &From)
{
	*this=From;
}


//---------------------------------------------------------------------
/** Copy constructor from color.
 */

CqVector3D::CqVector3D(const CqColor &From)
{
	*this=From;
}


//---------------------------------------------------------------------
/** Rescale this vector to be a unit vector.
 */

void CqVector3D::Unit()
{
	TqFloat Mag=Magnitude();

	m_x/=Mag;
	m_y/=Mag;
	m_z/=Mag;
}

//---------------------------------------------------------------------
/** Cross product of two vectors.
 */

CqVector3D operator%(const CqVector3D &a, const CqVector3D &b)
{
	return(CqVector3D(	(a.m_y*b.m_z)-(a.m_z*b.m_y),
						(a.m_z*b.m_x)-(a.m_x*b.m_z),
						(a.m_x*b.m_y)-(a.m_y*b.m_x) ));
}

//---------------------------------------------------------------------
/** Sets this vector to be the cross product of itself and another vector.
 */

CqVector3D &CqVector3D::operator%=(const CqVector3D &From)
{
	CqVector3D	vecTemp(*this);

	m_x=(vecTemp.m_y*From.m_z)-(vecTemp.m_z*From.m_y);
	m_y=(vecTemp.m_z*From.m_x)-(vecTemp.m_x*From.m_z);
	m_z=(vecTemp.m_x*From.m_y)-(vecTemp.m_y*From.m_x);

	return(*this);
}

//---------------------------------------------------------------------
/** Copy from specified 4D vector.
 */

CqVector3D &CqVector3D::operator=(const CqVector4D &From)
{
	if(From.h()!=1.0)
	{
		m_x=From.x()/From.h();
		m_y=From.y()/From.h();
		m_z=From.z()/From.h();
	}
	else
	{
		m_x=From.x();
		m_y=From.y();
		m_z=From.z();
	}

	return(*this);
}


//---------------------------------------------------------------------
/** Copy from specified color.
 */

CqVector3D &CqVector3D::operator=(const CqColor &From)
{
	m_x=From.fRed();
	m_y=From.fGreen();
	m_z=From.fBlue();

	return(*this);
}


CqVector3D& CqVector3D::operator=(const SqVMStackEntry* pVal)
{
	*this=pVal->m_Point;
	return(*this);
}

//---------------------------------------------------------------------

END_NAMESPACE(Aqsis)
