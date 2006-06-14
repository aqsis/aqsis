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
		\brief Declares the CqVector2D class which encapsulates a 2D vector.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef VECTOR2D_H_INCLUDED
#define VECTOR2D_H_INCLUDED 1

#include	<iostream>

#include	<math.h>

#include	"aqsis.h"

START_NAMESPACE( Aqsis )

//-----------------------------------------------------------------------

class CqVector3D;
class CqVector4D;

//----------------------------------------------------------------------
/** \class CqVector2D
 * \brief Define class structure for 2D vector.
 */

class CqVector2D
{
	public:
		CqVector2D() : m_x(0.0f), m_y(0.0f)
		{}
		CqVector2D( TqFloat x, TqFloat y ) : m_x( x ), m_y( y )
		{}
		CqVector2D( const CqVector3D &From );
		CqVector2D( const CqVector4D &From );
		~CqVector2D()
		{}

		/** Get the x component.
		 */
		TqFloat	x() const
		{
			return ( m_x );
		}
		/** Set the x component.
		 * \param x Float new value.
		 */
		void	x( TqFloat x )
		{
			m_x = x;
		}
		/** Get the y component.
		 */
		TqFloat	y() const
		{
			return ( m_y );
		}
		/** Set the y component.
		 * \param y Float new value.
		 */
		void	y( TqFloat y )
		{
			m_y = y;
		}

		/** Access the components as an array.
		 * \param i Integer component index, 0 or 1.
		 * \return Appropriate component or y if invalid index.
		 */
		TqFloat&	operator[] ( TqInt i )
		{
			switch ( i )
			{
				case 0:
					return ( m_x );
					break;
				case 1:
					return ( m_y );
					break;
				default:
					break;
			}
			return ( m_y );
		}

		/** Access the components as an array.
		 * \param i Integer component index, 0 or 1.
		 * \return Appropriate component or y if invalid index.
		 */
		const TqFloat&	operator[] ( TqInt i ) const
		{
			switch ( i )
			{
				case 0:
					return ( m_x );
					break;
				case 1:
					return ( m_y );
					break;
				default:
					break;
			}
			return ( m_y );
		}

		/** Get the length squared.
		 */
		TqFloat	Magnitude2() const
		{
			return ( ( m_x * m_x ) + ( m_y * m_y ) );
		}
		/** Get the length.
		 */
		TqFloat	Magnitude() const
		{
			return ( sqrt( ( m_x * m_x ) + ( m_y * m_y ) ) );
		}
		void	Unit()
		{
			TqFloat Mag = Magnitude();

			m_x /= Mag;
			m_y /= Mag;
		}

		CqVector2D& operator= ( const CqVector3D &From );
		CqVector2D& operator= ( const CqVector4D &From );
		/** Addition assignment operator.
		 */
		CqVector2D& operator+=( const CqVector2D &From )
		{
			m_x += From.m_x;
			m_y += From.m_y;
			return ( *this );
		}
		/** Componentwise addition assignment operator.
		 */
		CqVector2D& operator+=( const TqFloat &f )
		{
			m_x += f;
			m_y += f;
			return ( *this );
		}
		/** Subtraction assignment operator.
		 */
		CqVector2D& operator-=( const CqVector2D &From )
		{
			m_x -= From.m_x;
			m_y -= From.m_y;
			return ( *this );
		}
		/** Componentwise subtraction assignment operator.
		 */
		CqVector2D& operator-=( const TqFloat &f )
		{
			m_x -= f;
			m_y -= f;
			return ( *this );
		}
		/** Coponent wise scale operator.
		 */
		CqVector2D& operator*=( const TqFloat Scale )
		{
			m_x *= Scale;
			m_y *= Scale;
			return ( *this );
		}
		/** Scale operator.
		 */
		CqVector2D& operator*=( const CqVector2D &Scale )
		{
			m_x *= Scale.m_x;
			m_y *= Scale.m_y;
			return ( *this );
		}
		/** Inverse scale operator.
		 */
		CqVector2D& operator/=( const CqVector2D &Scale )
		{
			m_x /= Scale.m_x;
			m_y /= Scale.m_y;
			return ( *this );
		}
		/** Component wise inverse scale operator.
		 */
		CqVector2D& operator/=( const TqFloat Scale )
		{
			m_x /= Scale;
			m_y /= Scale;
			return ( *this );
		}
		/** Equality operator.
		 */
		TqBool	operator==( const CqVector2D &Cmp ) const
		{
			return ( ( m_x == Cmp.m_x ) && ( m_y == Cmp.m_y ) );
		}
		/** Inequality operator.
		 */
		TqBool	operator!=( const CqVector2D &Cmp ) const
		{
			return ( ( m_x != Cmp.m_x ) || ( m_y != Cmp.m_y ) );
		}

		friend CqVector2D	operator+( const TqFloat f, const CqVector2D& v )
		{
			return CqVector2D( f + v.x(), f + v.y() );
		}
		friend CqVector2D	operator+( const CqVector2D& v, const TqFloat f )
		{
			CqVector2D r( v );
			return ( r += f );
		}
		friend CqVector2D	operator-( const TqFloat f, const CqVector2D& v )
		{
			return CqVector2D( f -v.x(), f - v.y() );
		}
		friend CqVector2D	operator-( const CqVector2D& v, const TqFloat f )
		{
			CqVector2D r( v );
			return ( r -= f );
		}
		friend CqVector2D	operator*( const TqFloat f, const CqVector2D& v )
		{
			return CqVector2D( f * v.x(), f * v.y() );
		}
		friend CqVector2D	operator*( const CqVector2D& v, const TqFloat f )
		{
			CqVector2D r( v );
			return ( r *= f );
		}
		friend CqVector2D	operator/( const TqFloat f, const CqVector2D& v )
		{
			return CqVector2D( f / v.x(), f / v.y() );
		}
		friend CqVector2D	operator/( const CqVector2D& v, const TqFloat f )
		{
			CqVector2D r( v );
			return ( r /= f );
		}

		friend CqVector2D	operator+( const CqVector2D& a, const CqVector2D& b )
		{
			CqVector2D r( a );
			return ( r += b );
		}
		friend CqVector2D	operator-( const CqVector2D& a, const CqVector2D& b )
		{
			CqVector2D r( a );
			return ( r -= b );
		}
		friend CqVector2D	operator/( const CqVector2D& a, const CqVector2D& b )
		{
			CqVector2D r( a );
			return ( r /= b );
		}
		friend CqVector2D	operator-( const CqVector2D& v )
		{
			return ( CqVector2D( -v.m_x, -v.m_y ) );
		} // Negation

		friend TqFloat	operator*( const CqVector2D& a, const CqVector2D& b )
		{
			return ( a.m_x * b.m_x + a.m_y * b.m_y );
		} // Dot product
		friend std::ostream &operator<<( std::ostream &Stream, const CqVector2D &Vector );

	protected:
		TqFloat	m_x;		///< X component.
		TqFloat	m_y;		///< Y component.
}
;

//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !VECTOR2D_H_INCLUDED
