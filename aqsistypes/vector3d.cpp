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


/** \file
		\brief Implements the CqVector3D class.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	<math.h>

#include	"aqsis.h"
#include	"vector3d.h"
#include	"vector4d.h"
#include	"color.h"

namespace Aqsis {

//---------------------------------------------------------------------
/** Copy constructor from 4D Vector.
 */

CqVector3D::CqVector3D( const CqVector4D &From )
{
	*this = From;
}


//---------------------------------------------------------------------
/** Copy constructor from color.
 */
CqVector3D::CqVector3D( const CqColor &From )
{
	*this = From;
}

//---------------------------------------------------------------------
/** Copy from specified 4D vector.
 */
CqVector3D &CqVector3D::operator=( const CqVector4D &From )
{
	if ( From.h() != 1.0 )
	{
		m_x = From.x() / From.h();
		m_y = From.y() / From.h();
		m_z = From.z() / From.h();
	}
	else
	{
		m_x = From.x();
		m_y = From.y();
		m_z = From.z();
	}

	return ( *this );
}


//---------------------------------------------------------------------
/** Copy from specified color.
 */

CqVector3D &CqVector3D::operator=( const CqColor &From )
{
	m_x = From.r();
	m_y = From.g();
	m_z = From.b();

	return ( *this );
}


//----------------------------------------------------------------------
/** Outputs a vector to an output stream.
 * \param Stream Stream to output the matrix to.
 * \param Vector The vector to output.
 * \return The new state of Stream.
 */

std::ostream &operator<<( std::ostream &Stream, const CqVector3D &Vector )
{
	Stream << Vector.m_x << "," << Vector.m_y << "," << Vector.m_z;
	return ( Stream );
}


//---------------------------------------------------------------------

} // namespace Aqsis
