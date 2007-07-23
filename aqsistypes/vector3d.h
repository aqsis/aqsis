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
		\brief Declares the CqVector3D class which encapsulates a 3D vector/point/normal.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef VECTOR3D_H_INCLUDED
#define VECTOR3D_H_INCLUDED 1

#include	<iostream>

#include	"aqsis.h"
#include	"aqsismath.h"

#include	"vector2d.h"

START_NAMESPACE( Aqsis )

//-----------------------------------------------------------------------

class CqVector4D;
class CqColor;

//----------------------------------------------------------------------
/** \class CqVector3D
 * \brief Define class structure for 3D vector.
 */

class COMMON_SHARE CqVector3D
{
	public:
		CqVector3D() : m_x(0.0f), m_y(0.0f), m_z(0.0f)
		{}
		CqVector3D( const CqVector2D &From ) : m_x( From.x() ), m_y( From.y() ), m_z( 0 )
		{}
		CqVector3D( const CqColor &From );
		CqVector3D( TqFloat x, TqFloat y, TqFloat z ) : m_x( x ), m_y( y ), m_z( z )
		{}
		CqVector3D( TqFloat f ) : m_x( f ), m_y( f ), m_z( f )
		{}
		CqVector3D( const CqVector4D &From );
		CqVector3D( const TqFloat Array[ 3 ] ) : m_x( Array[ 0 ] ), m_y( Array[ 1 ] ), m_z( Array[ 2 ] )
		{}
		~CqVector3D()
		{}

		/** Get the x component.
		 */
		TqFloat	x() const
		{
			return ( m_x );
		}
		/** Set the x component.
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
		 */
		void	y( TqFloat y )
		{
			m_y = y;
		}
		/** Get the z component.
		 */
		TqFloat	z() const
		{
			return ( m_z );
		}
		/** Set the z component.
		 */
		void	z( TqFloat z )
		{
			m_z = z;
		}

		/** Array based component access.
		 * \param i Integer component index, 0-2.
		 * \return Appropriate component, or z if index is invalid.
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
				case 2:
					return ( m_z );
					break;
				default:
					break;
			}
			return ( m_z );
		}

		/** Array based component access.
		 * \param i Integer component index, 0-2.
		 * \return Appropriate component, or z if index is invalid.
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
				case 2:
					return ( m_z );
					break;
				default:
					break;
			}
			return ( m_z );
		}

		/** Get the length squared.
		 */
		TqFloat	Magnitude2() const
		{
			return ( ( m_x * m_x ) + ( m_y * m_y ) + ( m_z * m_z ) );
		}
		/** Get the length.
		 */
		TqFloat	Magnitude() const
		{
			TqFloat mag2 = Magnitude2();
			return ( mag2<=0.0f? 0.0f : sqrt( mag2 ) );
		}
		CqVector3D&	Unit()
		{
			TqFloat Mag = Magnitude();
			if( Mag > 0.0f )
			{
				m_x /= Mag;
				m_y /= Mag;
				m_z /= Mag;
			}
			return(*this);
		}

		CqVector3D& operator= ( const CqVector4D &From );
		CqVector3D& operator= ( const CqColor &From );
		/** Addition assignment operator.
		 */
		CqVector3D& operator+=( const CqVector3D &From )
		{
			m_x += From.m_x;
			m_y += From.m_y;
			m_z += From.m_z;
			return ( *this );
		}
		/** Component wise addition assignment operator.
		 */
		CqVector3D& operator+=( const TqFloat f )
		{
			m_x += f;
			m_y += f;
			m_z += f;
			return ( *this );
		}
		/** Subtraction assignment operator.
		 */
		CqVector3D& operator-=( const CqVector3D &From )
		{
			m_x -= From.m_x;
			m_y -= From.m_y;
			m_z -= From.m_z;
			return ( *this );
		}
		/** Component wise subtraction assignment operator.
		 */
		CqVector3D& operator-=( const TqFloat f )
		{
			m_x -= f;
			m_y -= f;
			m_z -= f;
			return ( *this );
		}
		CqVector3D& operator%=( const CqVector3D &From );
		/** Component wise scale assignment operator.
		 */
		CqVector3D& operator*=( const TqFloat Scale )
		{
			m_x *= Scale;
			m_y *= Scale;
			m_z *= Scale;
			return ( *this );
		}
		/** Scale assignment operator.
		 */
		CqVector3D& operator*=( const CqVector3D &Scale )
		{
			m_x *= Scale.m_x;
			m_y *= Scale.m_y;
			m_z *= Scale.m_z;
			return ( *this );
		}
		/** Inverse scale assignment operator.
		 */
		CqVector3D& operator/=( const CqVector3D &Scale )
		{
			m_x /= Scale.m_x;
			m_y /= Scale.m_y;
			m_z /= Scale.m_z;
			return ( *this );
		}
		/** Component wise inverse scale assignment operator.
		 */
		CqVector3D& operator/=( const TqFloat Scale )
		{
			m_x /= Scale;
			m_y /= Scale;
			m_z /= Scale;
			return ( *this );
		}

		/** Component wise equality operator.
		 */
		bool	operator==( const CqVector3D &Cmp ) const
		{
			return ( ( m_x == Cmp.m_x ) && ( m_y == Cmp.m_y ) && ( m_z == Cmp.m_z ) );
		}
		/** Component wise inequality operator.
		 */
		bool	operator!=( const CqVector3D &Cmp ) const
		{
			return ( ( m_x != Cmp.m_x ) || ( m_y != Cmp.m_y ) || ( m_z != Cmp.m_z ) );
		}
		/** Component wise greater than or equal to operator.
		 */
		bool	operator>=( const CqVector3D &Cmp ) const
		{
			return ( ( m_x >= Cmp.m_x ) && ( m_y >= Cmp.m_y ) && ( m_z >= Cmp.m_z ) );
		}
		/** Component wise less than or equal to operator.
		 */
		bool	operator<=( const CqVector3D &Cmp ) const
		{
			return ( ( m_x <= Cmp.m_x ) && ( m_y <= Cmp.m_y ) && ( m_z <= Cmp.m_z ) );
		}
		/** Component wise greater than operator.
		 */
		bool	operator>( const CqVector3D &Cmp ) const
		{
			return ( ( m_x > Cmp.m_x ) && ( m_y > Cmp.m_y ) && ( m_z > Cmp.m_z ) );
		}
		/** Component wise less than operator.
		 */
		bool	operator<( const CqVector3D &Cmp ) const
		{
			return ( ( m_x < Cmp.m_x ) && ( m_y < Cmp.m_y ) && ( m_z < Cmp.m_z ) );
		}

		/** Determine a componentwise minimum for two vectors
		 */
		friend CqVector3D min(const CqVector3D a, const CqVector3D b);

		/** Determine a componentwise maximum for two vectors
		 */
		friend CqVector3D max(const CqVector3D a, const CqVector3D b);

		/** Clamp the components of a vector to between two given vectors.
		 */
		friend CqVector3D clamp(const CqVector3D v, const CqVector3D min, const CqVector3D max);

		/** \brief Linearly interpolate between two colors
		 *
		 * \param t - interpolation parameter; a floating point between 0 and 1.
		 * \param c0 - color corresponding to t = 0
		 * \param c1 - color corresponding to t = 1
		 */
		template<typename T>
		friend CqVector3D lerp(const T t, const CqVector3D v0, const CqVector3D v1);

		friend CqVector3D	operator+( const TqFloat f, const CqVector3D& v )
		{
			return CqVector3D( f + v.x(), f + v.y(), f + v.z() );
		}
		friend CqVector3D	operator+( const CqVector3D& v, const TqFloat f )
		{
			CqVector3D r( v );
			return ( r += f );
		}
		friend CqVector3D	operator-( const TqFloat f, const CqVector3D& v )
		{
			return CqVector3D( f -v.x(), f - v.y(), f - v.z() );
		}
		friend CqVector3D	operator-( const CqVector3D& v, const TqFloat f )
		{
			CqVector3D r( v );
			return ( r -= f );
		}
		friend CqVector3D	operator*( const TqFloat f, const CqVector3D& v )
		{
			return CqVector3D( f * v.x(), f * v.y(), f * v.z() );
		}
		friend CqVector3D	operator*( const CqVector3D& v, const TqFloat f )
		{
			CqVector3D r( v );
			return ( r *= f );
		}
		friend CqVector3D	operator/( const TqFloat f, const CqVector3D& v )
		{
			return CqVector3D( f / v.x(), f / v.y(), f / v.z() );
		}
		friend CqVector3D	operator/( const CqVector3D& v, const TqFloat f )
		{
			CqVector3D r( v );
			return ( r /= f );
		}

		friend CqVector3D	operator+( const CqVector3D& a, const CqVector3D& b )
		{
			CqVector3D r( a );
			return ( r += b );
		}
		friend CqVector3D	operator-( const CqVector3D& a, const CqVector3D& b )
		{
			CqVector3D r( a );
			return ( r -= b );
		}
		friend CqVector3D	operator/( const CqVector3D& a, const CqVector3D& b )
		{
			CqVector3D r( a );
			return ( r /= b );
		}
		friend CqVector3D	operator-( const CqVector3D& v )
		{
			return ( CqVector3D( -v.m_x, -v.m_y, -v.m_z ) );
		} // Negation

		friend TqFloat	operator*( const CqVector3D& a, const CqVector3D& b )
		{
			return ( a.m_x * b.m_x + a.m_y * b.m_y + a.m_z * b.m_z );
		} // Dot product
		COMMON_SHARE friend CqVector3D	operator%( const CqVector3D& a, const CqVector3D& b );	// X product
		COMMON_SHARE friend std::ostream &operator<<( std::ostream &Stream, const CqVector3D &Vector );

	protected:
		TqFloat	m_x;	///< X component.
		TqFloat	m_y;	///< Y component.
		TqFloat	m_z;	///< Z component.
}
;


//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
// Inline function implementations.
//-----------------------------------------------------------------------
inline CqVector3D min(const CqVector3D a, const CqVector3D b)
{
	return CqVector3D(min(a.m_x, b.m_x), min(a.m_y, b.m_y), min(a.m_z, b.m_z));
}

inline CqVector3D max(const CqVector3D a, const CqVector3D b)
{
	return CqVector3D(max(a.m_x, b.m_x), max(a.m_y, b.m_y), max(a.m_z, b.m_z));
}

inline CqVector3D clamp(const CqVector3D v, const CqVector3D min, const CqVector3D max)
{
	return CqVector3D(clamp(v.m_x, min.m_x, max.m_x),
			clamp(v.m_y, min.m_y, max.m_y),
			clamp(v.m_z, min.m_z, max.m_z));
}

template<typename T>
inline CqVector3D lerp(const T t, const CqVector3D v0, const CqVector3D v1)
{
	return CqVector3D((1-t)*v0.m_x + t*v1.m_x,
			       (1-t)*v0.m_y + t*v1.m_y,
			       (1-t)*v0.m_z + t*v1.m_z);
}

//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// VECTOR3D_H_INCLUDED
