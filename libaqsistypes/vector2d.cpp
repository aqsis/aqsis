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

START_NAMESPACE( Aqsis )

//---------------------------------------------------------------------
/** Copy constructor from 3D Vector.
 */

CqVector2D::CqVector2D( const CqVector3D &From )
        : m_x( From.x() ), m_y( From.y() )
{}

//---------------------------------------------------------------------
/** Copy constructor from 4D Vector.
 */

CqVector2D::CqVector2D( const CqVector4D &From )
        : m_x( From.x() / From.h() ), m_y( From.y() / From.h() )
{}
//---------------------------------------------------------------------
/** Copy from specified 3D vector.
 */

CqVector2D &CqVector2D::operator=( const CqVector3D &From )
{
    m_x = From.x();
    m_y = From.y();

    return ( *this );
}


//---------------------------------------------------------------------
/** Copy from specified 4D vector.
 */

CqVector2D &CqVector2D::operator=( const CqVector4D &From )
{
    if ( From.h() != 1.0 )
    {
        m_x = From.x() / From.h();
        m_y = From.y() / From.h();
    }
    else
    {
        m_x = From.x();
        m_y = From.y();
    }

    return ( *this );
}

//----------------------------------------------------------------------
/** Outputs a vector to an output stream.
 * \param Stream Stream to output the matrix to.
 * \param Vector The vector to output.
 * \return The new state of Stream.
 */

std::ostream &operator<<( std::ostream &Stream, const CqVector2D &Vector )
{
    Stream << Vector.m_x << "," << Vector.m_y;
    return ( Stream );
}


//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )
