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
		\brief Declares the CqVector4D class which encapsulates a homogenous 4D vector.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef VECTOR4D_H_INCLUDED
#define VECTOR4D_H_INCLUDED 1

#include <aqsis/aqsis.h>

#include <iostream>

#include <aqsis/math/math.h>

namespace Aqsis {

//----------------------------------------------------------------------
/** \brief A homogeneous length-4 vector.
 *
 * Homogenous coordinates have the property that any scaled version of a vector
 * is considered equal to that vector.  That is, we have
 *
 *   v1 == a * v1
 *
 * for any number a.
 *
 * This equivilance relation seems a little odd at first sight, but is a
 * convenient representation for certian types of splines, in particular NURBS.
 * It's also a convenient when doing perspective transformations.
 *
 * A 3D vector can be extraced from a 4D homogenous vector by putting the
 * 4-vector in a canonical form where the last componenet is equal to 1:
 *
 * (x,y,z,h)  -->  (x/h,y/h,z/h,1)
 *
 * By the equivilance relation, these two vectors are still considered "equal".
 * The associated 3-vector is just the first three componenets.
 *
 * (Note that this and many other operations involve division by the last
 * element, and will give rubbish if it is 0.)
 *
 * Note also that homogenous vectors have odd and slightly counterintuitive
 * rules for addition, subtraction, scalar multiplication etc.  These are *not*
 * the same as the rules for taking linear combinations on a normal vector.
 */
class CqVector4D
{
	public:
		CqVector4D() : m_x( 0.0f ), m_y( 0.0f ), m_z( 0.0f ), m_h( 1.0f )
		{}
		explicit CqVector4D(TqFloat f) : m_x(f), m_y(f), m_z(f), m_h(1)
		{}
		CqVector4D( TqFloat x, TqFloat y, TqFloat z, TqFloat h = 1.0f ) : m_x( x ), m_y( y ), m_z( z ), m_h( h )
		{}
		CqVector4D( const TqFloat Array[ 4 ] ) : m_x( Array[ 0 ] ), m_y( Array[ 1 ] ), m_z( Array[ 2 ] ), m_h( Array[ 3 ] )
		{}
		~CqVector4D()
		{}

		/** Get the x component.
		 */
		TqFloat	x() const
		{
			return ( m_x );
		}
		/// Get a reference to the x-componenet.
		TqFloat& x()
		{
			return m_x;
		}
		/** Set the x component.
		 * \deprecated.  Use the x() method returning a non-const reference.
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
		/// Get a reference to the y-componenet.
		TqFloat& y()
		{
			return m_y;
		}
		/** Set the y component.
		 * \deprecated.  Use the y() method returning a non-const reference.
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
		/// Get a reference to the z-componenet.
		TqFloat& z()
		{
			return m_z;
		}
		/** Set the z component.
		 * \deprecated.  Use the z() method returning a non-const reference.
		 */
		void	z( TqFloat z )
		{
			m_z = z;
		}
		/** Get the h component.
		 */
		TqFloat	h() const
		{
			return ( m_h );
		}
		/// Get a reference to the h-componenet.
		TqFloat& h()
		{
			return m_h;
		}
		/** Set the h component.
		 * \deprecated.  Use the h() method returning a non-const reference.
		 */
		void	h( TqFloat h )
		{
			m_h = h;
		}

		/** Array based component access.
		 * \param i Integer component index, 0-3.
		 * \return Appropriate component, or h if index is invalid.
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
				case 3:
					return ( m_h );
					break;
				default:
					break;
			}
			return ( m_h );
		}

		/** Array based component access.
		 * \param i Integer component index, 0-3.
		 * \return Appropriate component, or h if index is invalid.
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
				case 3:
					return ( m_h );
					break;
				default:
					break;
			}
			return ( m_h );
		}

		/** Get the length squared.
		 */
		TqFloat	Magnitude2() const;
		/** Get the length.
		 */
		TqFloat	Magnitude() const;

		void	Unit()
		{
			m_h = ( Magnitude() );
		}
		void	Homogenize()
		{
			if ( m_h != 1.0 )
			{
				m_x /= m_h;
				m_y /= m_h;
				m_z /= m_h;
				m_h = 1.0;
			}
		}

		CqVector4D& operator+=( const CqVector4D &From );
		CqVector4D& operator-=( const CqVector4D &From );
		CqVector4D& operator%=( const CqVector4D &From );
		CqVector4D& operator*=( const TqFloat Scale );
		CqVector4D& operator/=( const TqFloat Scale );
		bool	operator==( const CqVector4D &Cmp ) const;
		bool	operator!=( const CqVector4D &Cmp ) const;
		bool	operator>=( const CqVector4D &Cmp ) const;
		bool	operator<=( const CqVector4D &Cmp ) const;
		bool	operator>( const CqVector4D &Cmp ) const;
		bool	operator<( const CqVector4D &Cmp ) const;

		friend CqVector4D	operator*( const TqFloat f, const CqVector4D& v )
		{
			CqVector4D r( v );
			return ( r *= f );
		}
		friend CqVector4D	operator*( const CqVector4D& v, const TqFloat f )
		{
			CqVector4D r( v );
			return ( r *= f );
		}
		friend CqVector4D	operator/( const TqFloat f, const CqVector4D& v )
		{
			CqVector4D r( v );
			return ( r /= f );
		}
		friend CqVector4D	operator/( const CqVector4D& v, const TqFloat f )
		{
			CqVector4D r( v );
			return ( r /= f );
		}

		friend CqVector4D	operator+( const CqVector4D& a, const CqVector4D& b )
		{
			CqVector4D r( a );
			return ( r += b );
		}
		friend CqVector4D	operator-( const CqVector4D& a, const CqVector4D& b )
		{
			CqVector4D r( a );
			return ( r -= b );
		}
		friend CqVector4D	operator-( const CqVector4D& v )
		{
			return ( CqVector4D( -v.m_x, -v.m_y, -v.m_z, v.m_h ) );
		} // Negation

		AQSIS_MATH_SHARE friend TqFloat	operator*( const CqVector4D& a, const CqVector4D& b );
		AQSIS_MATH_SHARE friend CqVector4D	operator%( const CqVector4D& a, const CqVector4D& b );	// X product
		AQSIS_MATH_SHARE friend std::ostream &operator<<( std::ostream &Stream, const CqVector4D &Vector );

	protected:
		TqFloat	m_x;	///< X component.
		TqFloat	m_y;	///< Y component.
		TqFloat	m_z;	///< Z component.
		TqFloat	m_h;	///< H component.
}
;

//-----------------------------------------------------------------------
/** \brief Determine whether two vectors are equal to within some tolerance
 *
 * The closeness criterion for homogenous 4-vectors is based on the euclidian
 * norm of the associated 3D vector.  That is, we take the homogenous
 * difference between v1 and v2, and compare that to the length of v1 and v2
 * scaled by the tolerance.  In symbols (remember that all operations are
 * homogenous!)
 *
 * length(v1 - v2) < tol*max(length(v1), length(v2));
 *
 * \param v1, v2 - vectors to compare
 * \param tolerance for comparison
 */
bool isClose(const CqVector4D& v1, const CqVector4D& v2,
		TqFloat tol = 10*std::numeric_limits<TqFloat>::epsilon());



//==============================================================================
// Implementation details
//==============================================================================

//---------------------------------------------------------------------
/** Return magnitude squared of this vector.
 */

inline TqFloat CqVector4D::Magnitude2() const
{
	if ( m_h == 1.0 )
		return ( ( m_x * m_x ) + ( m_y * m_y ) + ( m_z * m_z ) );
	else
		return ( ( ( m_x * m_x ) + ( m_y * m_y ) + ( m_z * m_z ) ) / ( m_h * m_h ) );
}

//---------------------------------------------------------------------
/** Return magnitude of this vector.
 */

inline TqFloat CqVector4D::Magnitude() const
{
	return ( sqrt( Magnitude2() ) );
}


//---------------------------------------------------------------------
/** Add a vector to this vector.
 */

inline CqVector4D &CqVector4D::operator+=( const CqVector4D &From )
{
	TqFloat Hom = m_h / From.m_h;

	m_x += From.m_x * Hom;
	m_y += From.m_y * Hom;
	m_z += From.m_z * Hom;

	return ( *this );
}



//---------------------------------------------------------------------
/** Subtract a vector from this vector.
 */

inline CqVector4D &CqVector4D::operator-=( const CqVector4D &From )
{
	TqFloat Hom = m_h / From.m_h;

	m_x -= From.m_x * Hom;
	m_y -= From.m_y * Hom;
	m_z -= From.m_z * Hom;

	return ( *this );
}

//---------------------------------------------------------------------
/** Dot product of two vectors.
 */

inline TqFloat operator*( const CqVector4D &a, const CqVector4D &From )
{
	CqVector4D	A( a );
	CqVector4D	B( From );

	A.Homogenize();
	B.Homogenize();

	return ( ( A.m_x * B.m_x ) +
	         ( A.m_y * B.m_y ) +
	         ( A.m_z * B.m_z ) );
}


//---------------------------------------------------------------------
/** Cross product of two vectors.
 */

inline CqVector4D operator%( const CqVector4D &a, const CqVector4D &From )
{
	CqVector4D Temp( a );
	Temp %= From;
	return ( Temp );
}


//---------------------------------------------------------------------
/** Sets this vector to be the cross product of itself and another vector.
 */

inline CqVector4D &CqVector4D::operator%=( const CqVector4D &From )
{
	CqVector4D	A( *this );
	CqVector4D	B( From );

	A.Homogenize();
	B.Homogenize();

	m_x = ( A.m_y * B.m_z ) - ( A.m_z * B.m_y );
	m_y = ( A.m_z * B.m_x ) - ( A.m_x * B.m_z );
	m_z = ( A.m_x * B.m_y ) - ( A.m_y * B.m_x );

	return ( *this );
}


//---------------------------------------------------------------------
/** Scale this vector by the specifed scale factor.
 */

inline CqVector4D &CqVector4D::operator*=( const TqFloat Scale )
{
	/*	if(Scale != 0.0)
		{
		    m_h /= Scale;
		}
		else
		{
			m_x = m_y = m_z = 0.0;
		}
	*/

	m_x *= Scale;
	m_z *= Scale;
	m_y *= Scale;

	return ( *this );
}



//---------------------------------------------------------------------
/** Divide this vector by the specifed scale factor.
 */

inline CqVector4D &CqVector4D::operator/=( const TqFloat Scale )
{
	m_h *= Scale;

	return ( *this );
}


//---------------------------------------------------------------------
/** Compare two vectors for equality.
 */

inline bool CqVector4D::operator==( const CqVector4D &Cmp ) const
{
	/*    TqFloat Hom = m_h / Cmp.m_h;
	 
	    return ( ( m_x == ( Cmp.m_x * Hom ) ) &&
	             ( m_y == ( Cmp.m_y * Hom ) ) &&
	             ( m_z == ( Cmp.m_z * Hom ) ) );
	*/

	// exact equality of floats isn't actually very useful because of slight
	// differences due to rounding errors. a fuzzy compare is generally more useful.
	// if the vectors have different h values the above is unlikely to
	// return true, so we might as well just test for exact equality of each component.
	return ((m_x == Cmp.m_x) &&
	        (m_y == Cmp.m_y) &&
	        (m_z == Cmp.m_z) &&
	        (m_h == Cmp.m_h));
}


//---------------------------------------------------------------------
/** Compare two vectors for inequality.
 */

inline bool CqVector4D::operator!=( const CqVector4D &Cmp ) const
{
	return ( !( *this == Cmp ) );
}


//---------------------------------------------------------------------
/** Compare two vectors for greater than or equal.
 */

inline bool CqVector4D::operator>=( const CqVector4D &Cmp ) const
{
	TqFloat Hom = m_h / Cmp.m_h;

	return ( ( m_x >= ( Cmp.m_x * Hom ) ) &&
	         ( m_y >= ( Cmp.m_y * Hom ) ) &&
	         ( m_z >= ( Cmp.m_z * Hom ) ) );
}


//---------------------------------------------------------------------
/** Compare two vectors for less than or equal.
 */

inline bool CqVector4D::operator<=( const CqVector4D &Cmp ) const
{
	TqFloat Hom = m_h / Cmp.m_h;

	return ( ( m_x <= ( Cmp.m_x * Hom ) ) &&
	         ( m_y <= ( Cmp.m_y * Hom ) ) &&
	         ( m_z <= ( Cmp.m_z * Hom ) ) );
}


//---------------------------------------------------------------------
/** Compare two vectors for greater than.
 */

inline bool CqVector4D::operator>( const CqVector4D &Cmp ) const
{
	TqFloat Hom = m_h / Cmp.m_h;

	return ( ( m_x > ( Cmp.m_x * Hom ) ) &&
	         ( m_y > ( Cmp.m_y * Hom ) ) &&
	         ( m_z > ( Cmp.m_z * Hom ) ) );
}


//---------------------------------------------------------------------
/** Compare two vectors for less than.
 */

inline bool CqVector4D::operator<( const CqVector4D &Cmp ) const
{
	TqFloat Hom = m_h / Cmp.m_h;

	return ( ( m_x < ( Cmp.m_x * Hom ) ) &&
	         ( m_y < ( Cmp.m_y * Hom ) ) &&
	         ( m_z < ( Cmp.m_z * Hom ) ) );
}


//----------------------------------------------------------------------
/** Outputs a vector to an output stream.
 * \param Stream Stream to output the matrix to.
 * \param Vector The vector to output.
 * \return The new state of Stream.
 */

inline std::ostream &operator<<( std::ostream &Stream, const CqVector4D &Vector )
{
	Stream << Vector.m_x << "," << Vector.m_y << "," << Vector.m_z << "," << Vector.m_h;
	return ( Stream );
}

//------------------------------------------------------------------------------
inline bool isClose(const CqVector4D& v1, const CqVector4D& v2, TqFloat tol)
{
	TqFloat diff2 = (v1 - v2).Magnitude2();
	TqFloat tol2 = tol*tol;
	return diff2 <= tol2*v1.Magnitude2() || diff2 <= tol2*v2.Magnitude2();
}


} // namespace Aqsis

#endif	// !VECTOR4D_H_INCLUDED
