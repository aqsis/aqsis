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
		\brief Implements the 3D geometric bound class.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"aqsis.h"
#include	"matrix.h"
#include	"bound.h"

START_NAMESPACE( Aqsis )

//---------------------------------------------------------------------
/** Copy constructor.
 */

CqBound::CqBound( const CqBound& From )
{
	*this = From;
}


//---------------------------------------------------------------------
/** Assignment operator.
 */

CqBound& CqBound::operator=( const CqBound& From )
{
	m_vecMin = From.m_vecMin;
	m_vecMax = From.m_vecMax;

	return ( *this );
}


//---------------------------------------------------------------------
/**
 * Transform the boundary values by the specified matrix.
 * \param matTransform CqMatrix reference to the transformation matrix.
 */

void CqBound::Transform( const CqMatrix& matTransform )
{
	// Transform the cuboid points.
	CqVector3D	avecCuboid[ 8 ];
	avecCuboid[ 0 ] = m_vecMin;
	avecCuboid[ 1 ] = CqVector3D( m_vecMax.x(), m_vecMin.y(), m_vecMin.z() );
	avecCuboid[ 2 ] = CqVector3D( m_vecMin.x(), m_vecMax.y(), m_vecMin.z() );
	avecCuboid[ 3 ] = CqVector3D( m_vecMin.x(), m_vecMin.y(), m_vecMax.z() );
	avecCuboid[ 4 ] = CqVector3D( m_vecMax.x(), m_vecMax.y(), m_vecMin.z() );
	avecCuboid[ 5 ] = CqVector3D( m_vecMin.x(), m_vecMax.y(), m_vecMax.z() );
	avecCuboid[ 6 ] = CqVector3D( m_vecMax.x(), m_vecMin.y(), m_vecMax.z() );
	avecCuboid[ 7 ] = m_vecMax;

	m_vecMin = CqVector3D( FLT_MAX, FLT_MAX, FLT_MAX );
	m_vecMax = CqVector3D( -FLT_MAX, -FLT_MAX, -FLT_MAX );

	TqInt i;
	for ( i = 0; i < 8; i++ )
	{
		avecCuboid[ i ] = matTransform * avecCuboid[ i ];
		if ( avecCuboid[ i ].x() < m_vecMin.x() )
			m_vecMin.x( avecCuboid[ i ].x() );
		if ( avecCuboid[ i ].y() < m_vecMin.y() )
			m_vecMin.y( avecCuboid[ i ].y() );
		if ( avecCuboid[ i ].z() < m_vecMin.z() )
			m_vecMin.z( avecCuboid[ i ].z() );

		if ( avecCuboid[ i ].x() > m_vecMax.x() )
			m_vecMax.x( avecCuboid[ i ].x() );
		if ( avecCuboid[ i ].y() > m_vecMax.y() )
			m_vecMax.y( avecCuboid[ i ].y() );
		if ( avecCuboid[ i ].z() > m_vecMax.z() )
			m_vecMax.z( avecCuboid[ i ].z() );
	}
}


//---------------------------------------------------------------------
/** Combine the specified boundary with the current one, returning a new bound
 * that encapsulates the area of both.
 * \param bound CqBound to add to this.
 */

CqBound CqBound::Combine( const CqBound& bound )
{
	CqBound Result;

	Result.m_vecMax.x( MAX( m_vecMax.x(), bound.m_vecMax.x() ) );
	Result.m_vecMax.y( MAX( m_vecMax.y(), bound.m_vecMax.y() ) );
	Result.m_vecMax.z( MAX( m_vecMax.z(), bound.m_vecMax.z() ) );

	Result.m_vecMin.x( MIN( m_vecMin.x(), bound.m_vecMin.x() ) );
	Result.m_vecMin.y( MIN( m_vecMin.y(), bound.m_vecMin.y() ) );
	Result.m_vecMin.z( MIN( m_vecMin.z(), bound.m_vecMin.z() ) );

	return ( Result );
}


//---------------------------------------------------------------------
/** Expand this bound to encapsulate the specified bound.
 * \param bound CqBound to add to this.
 */

CqBound& CqBound::Encapsulate( const CqBound& bound )
{
	m_vecMax.x( MAX( m_vecMax.x(), bound.m_vecMax.x() ) );
	m_vecMax.y( MAX( m_vecMax.y(), bound.m_vecMax.y() ) );
	m_vecMax.z( MAX( m_vecMax.z(), bound.m_vecMax.z() ) );

	m_vecMin.x( MIN( m_vecMin.x(), bound.m_vecMin.x() ) );
	m_vecMin.y( MIN( m_vecMin.y(), bound.m_vecMin.y() ) );
	m_vecMin.z( MIN( m_vecMin.z(), bound.m_vecMin.z() ) );

	return( *this );
}


//---------------------------------------------------------------------
/** Expand this bound to encapsulate the specified point.
 * \param v CqVector3D to expand bound to include.
 */

CqBound& CqBound::Encapsulate( const CqVector3D& v )
{
	m_vecMax.x( MAX( m_vecMax.x(), v.x() ) );
	m_vecMax.y( MAX( m_vecMax.y(), v.y() ) );
	m_vecMax.z( MAX( m_vecMax.z(), v.z() ) );

	m_vecMin.x( MIN( m_vecMin.x(), v.x() ) );
	m_vecMin.y( MIN( m_vecMin.y(), v.y() ) );
	m_vecMin.z( MIN( m_vecMin.z(), v.z() ) );

	return( *this );
}


//---------------------------------------------------------------------
/** Expand this bound to encapsulate the specified point.
 * \param v CqVector2D to expand bound to include.
 */

CqBound& CqBound::Encapsulate( const CqVector2D& v )
{
	m_vecMax.x( MAX( m_vecMax.x(), v.x() ) );
	m_vecMax.y( MAX( m_vecMax.y(), v.y() ) );

	m_vecMin.x( MIN( m_vecMin.x(), v.x() ) );
	m_vecMin.y( MIN( m_vecMin.y(), v.y() ) );

	return( *this );
}

//----------------------------------------------------------------------
/** Outputs a bound to an output stream.
 * \param Stream Stream to output the matrix to.
 * \param Bound The bound to output.
 * \return The new state of Stream.
 */

std::ostream &operator<<( std::ostream &Stream, const CqBound &Bound )
{
	CqVector3D min = Bound.vecMin();
	CqVector3D max = Bound.vecMax();
	CqVector3D cross = Bound.vecCross();

	Stream << min << "-->" << max << "  |  Cross: " << cross << std::ends;
	return ( Stream );
}


//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )
