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
		\brief Implements the CqVector2D class.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<math.h>

#include	"aqsis.h"
#include	"vector2d.h"
#include	"vector3d.h"
#include	"vector4d.h"

START_NAMESPACE(Aqsis)

//---------------------------------------------------------------------
/** Copy constructor from 3D Vector.
 */

CqVector2D::CqVector2D(const CqVector3D &From)
	: m_x(From.x()), m_y(From.y())
{
}

//---------------------------------------------------------------------
/** Rescale this vector to be a unit vector.
 */

void CqVector2D::Unit()
{
	TqFloat Mag=Magnitude();

	m_x/=Mag;
	m_y/=Mag;
}


//---------------------------------------------------------------------
/** Copy from specified 3D vector.
 */

CqVector2D &CqVector2D::operator=(const CqVector3D &From)
{
	m_x=From.x();
	m_y=From.y();

	return(*this);
}


//---------------------------------------------------------------------
/** Copy from specified 4D vector.
 */

CqVector2D &CqVector2D::operator=(const CqVector4D &From)
{
	if(From.h()!=1.0)
	{
		m_x=From.x()/From.h();
		m_y=From.y()/From.h();
	}
	else
	{
		m_x=From.x();
		m_y=From.y();
	}

	return(*this);
}


//---------------------------------------------------------------------

END_NAMESPACE(Aqsis)
