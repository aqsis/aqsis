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
		\brief Declares the geometric boundary handling class.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef BOUND_H_INCLUDED
#define BOUND_H_INCLUDED 1

#include <aqsis/aqsis.h>

#include <vector>
#include <float.h>

#include <aqsis/math/matrix.h>
#include "plane.h"
#include <aqsis/math/vector2d.h>

namespace Aqsis {

//----------------------------------------------------------------------
/** \class CqBound
 * Class specifying a 3D geometric bound.
 */

class CqBound
{
	public:
		CqBound( const TqFloat* pBounds )
		{
			if ( pBounds )
			{
				m_vecMin.x( pBounds[ 0 ] );
				m_vecMin.y( pBounds[ 2 ] );
				m_vecMin.z( pBounds[ 4 ] );
				m_vecMax.x( pBounds[ 1 ] );
				m_vecMax.y( pBounds[ 3 ] );
				m_vecMax.z( pBounds[ 5 ] );
			}
		}

		CqBound( TqFloat XMin = FLT_MAX, TqFloat YMin = FLT_MAX, TqFloat ZMin = FLT_MAX, TqFloat XMax = -FLT_MAX, TqFloat YMax = -FLT_MAX, TqFloat ZMax = -FLT_MAX )
		{
			m_vecMin.x( XMin );
			m_vecMin.y( YMin );
			m_vecMin.z( ZMin );
			m_vecMax.x( XMax );
			m_vecMax.y( YMax );
			m_vecMax.z( ZMax );
		}

		CqBound( const CqVector3D& vecMin, const CqVector3D& vecMax )
		{
			m_vecMin = vecMin;
			m_vecMax = vecMax;
		}

		CqBound( const CqBound& From );
		~CqBound()
		{}

		const	CqVector3D&	vecMin() const
		{
			return ( m_vecMin );
		}

		CqVector3D&	vecMin()
		{
			return ( m_vecMin );
		}

		const	CqVector3D&	vecMax() const
		{
			return ( m_vecMax );
		}

		CqVector3D&	vecMax()
		{
			return ( m_vecMax );
		}

		CqVector3D vecCross() const
		{
			return( m_vecMax - m_vecMin );
		}

		TqFloat	Volume() const
		{
			return sqrt( Volume2() );
		}

		TqFloat Volume2() const
		{
			return( vecCross().Magnitude2() );
		}

		CqBound&	operator=( const CqBound& From );

		// Fills passed vector array with the 8 points of a bounding box
		void 		getBoundCuboid(CqVector3D cuboid[8]);

		void		Transform( const CqMatrix&	matTransform );
		void		Encapsulate( const CqBound* const bound );
		void		Encapsulate( const CqVector3D& v );
		void		Encapsulate( const CqVector2D& v );

		bool	Contains2D( const CqBound* const b ) const
		{
			if ( ( b->vecMin().x() >= vecMin().x() && b->vecMax().x() <= vecMax().x() ) &&
			        ( b->vecMin().y() >= vecMin().y() && b->vecMax().y() <= vecMax().y() ) )
				return ( true );
			else
				return ( false );
		}
		bool	Contains3D( const CqVector3D& v ) const
		{
			if ( ( v.x() >= m_vecMin.x() && v.x() <= m_vecMax.x() ) &&
			        ( v.y() >= m_vecMin.y() && v.y() <= m_vecMax.y() ) &&
			        ( v.z() >= m_vecMin.z() && v.z() <= m_vecMax.z() ) )
				return ( true );
			else
				return ( false );
		}
		bool	Contains2D( const CqVector2D& v ) const
		{
			if ( ( v.x() < m_vecMin.x() || v.x() > m_vecMax.x() ) ||
			        ( v.y() < m_vecMin.y() || v.y() > m_vecMax.y() ) )
				return ( false );
			else
				return ( true );
		}

		bool	Intersects( const CqVector2D& min, const CqVector2D& max ) const
		{
			if( min.x() > m_vecMax.x() || min.y() > m_vecMax.y() ||
			        max.x() < m_vecMin.x() || max.y() < m_vecMin.y() )
				return ( false );
			else
				return ( true );
		}

		enum EqPlaneSide
		{
		    Side_Outside = -1,
		    Side_Both = 0,
		    Side_Inside = 1,
	};

		TqInt whichSideOf(const CqPlane* const plane) const
		{
			bool inside = false;
			bool outside = false;


			CqVector3D p(m_vecMin.x(), m_vecMin.y(), m_vecMin.z());
			if (plane->whichSide(p) == CqPlane::Space_Positive)
				inside = true;
			else
				outside = true;

			p.z(m_vecMax.z());	// xmin, ymin, zmax
			if (plane->whichSide(p) == CqPlane::Space_Positive)
			{
				inside = true;
				if (outside)
					return(Side_Both);	// Both sides
			}
			else
			{
				outside = true;
				if (inside)
					return(Side_Both);	// Both sides
			}

			p.z(m_vecMin.z());
			p.y(m_vecMax.y());	// xmin, ymax, zmin
			if (plane->whichSide(p) == CqPlane::Space_Positive)
			{
				inside = true;
				if (outside)
					return(Side_Both);	// Both sides
			}
			else
			{
				outside = true;
				if (inside)
					return(Side_Both);	// Both sides
			}
			p.z(m_vecMax.z());	// xmin, ymax, zmax
			if (plane->whichSide(p) == CqPlane::Space_Positive)
			{
				inside = true;
				if (outside)
					return(Side_Both);
			}
			else
			{
				outside = true;
				if (inside)
					return(Side_Both);
			}
			p.x(m_vecMax.x());
			p.y(m_vecMin.y());
			p.z(m_vecMin.z());	// xmax, ymin, zmin
			if (plane->whichSide(p) == CqPlane::Space_Positive)
			{
				inside = true;
				if (outside)
					return(Side_Both);
			}
			else
			{
				outside = true;
				if (inside)
					return(Side_Both);
			}
			p.z(m_vecMax.z());	// xmax, ymin, zmax
			if (plane->whichSide(p) == CqPlane::Space_Positive)
			{
				inside = true;
				if (outside)
					return(Side_Both);
			}
			else
			{
				outside = true;
				if (inside)
					return(Side_Both);
			}
			p.z(m_vecMin.z());
			p.y(m_vecMax.y());	// xmax, ymax, zmin
			if (plane->whichSide(p) == CqPlane::Space_Positive)
			{
				inside = true;
				if (outside)
					return(Side_Both);
			}
			else
			{
				outside = true;
				if (inside)
					return(Side_Both);
			}
			p.z(m_vecMax.z());	// xmax, ymax, zmax
			if (plane->whichSide(p) == CqPlane::Space_Positive)
			{
				inside = true;
				if (outside)
					return(Side_Both);
			}
			else
			{
				outside = true;
				if (inside)
					return(Side_Both);
			}
			return(inside ? Side_Inside : Side_Outside);
		}

		friend std::ostream &operator<<( std::ostream &Stream, const CqBound &Bound );

	private:
		CqVector3D	m_vecMin;
		CqVector3D	m_vecMax;
};

//----------------------------------------------------------------------
/** \class CqBoundList
 * Class specifying a list of CqBounds.
 */
class CqBoundList
{
	public:
		CqBoundList()
		{}
		~CqBoundList()
		{
			//for ( std::vector<CqBound*>::iterator i = m_Bounds.begin(); i != m_Bounds.end(); i++ )
			//	delete ( *i );
		}

		/** Clear the list
		 */
		void Clear()
		{
			//for ( std::vector<CqBound*>::iterator i = m_Bounds.begin(); i != m_Bounds.end(); i++ )
			//	delete ( *i );
			m_Bounds.clear();
			m_Times.clear();
		}

		void SetSize( TqInt size )
		{
			m_Bounds.resize( size );
			m_Times.resize( size );
		}
		/** Add a bound to the current list
		 * \param index The index to set in the list.
		 * \param bound The CqBound to add
		 * \param time The shutter time that this bound becomes valid at. The bound
		 *               is valid until the time of the next bound or until 1.0 if there are no more bounds.
		 */
		void	Set( TqInt index, CqBound bound, TqFloat time )
		{
			assert( index < Size() );
			m_Bounds[ index ] = bound;
			m_Times[ index ] = time;
		}
		TqInt Size() const
		{
			return m_Bounds.size();
		}

		CqBound& GetBound( TqInt i )
		{
			return m_Bounds[ i ];
		}
		const CqBound& GetBound( TqInt i ) const
		{
			return m_Bounds[ i ];
		}
		TqFloat GetTime( TqInt i ) const
		{
			return m_Times[ i ];
		}

	private:
		std::vector<CqBound> m_Bounds;
		std::vector<TqFloat> m_Times;
};

//-----------------------------------------------------------------------

} // namespace Aqsis

#endif // BOUND_H_INCLUDED
