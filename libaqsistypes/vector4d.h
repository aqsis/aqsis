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
		\brief Declares the CqVector4D class which encapsulates a homogenous 4D vector.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef VECTOR4D_H_INCLUDED
#define VECTOR4D_H_INCLUDED 1

#include	<iostream>

#include	"aqsis.h"

START_NAMESPACE( Aqsis )

//-----------------------------------------------------------------------

class CqVector3D;

//----------------------------------------------------------------------
/** \class CqVector4D
 * \brief Define class structure for 4D homogeneous vector.
 */

class CqVector4D
{
public:
    CqVector4D() : m_x( 0.0f ), m_y( 0.0f ), m_z( 0.0f ), m_h( 1.0f )
    {}
    CqVector4D( TqFloat x, TqFloat y, TqFloat z, TqFloat h = 1.0f ) : m_x( x ), m_y( y ), m_z( z ), m_h( h )
    {}
    CqVector4D( const CqVector3D &From );
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
    /** Get the h component.
     */
    TqFloat	h() const
    {
        return ( m_h );
    }
    /** Set the h component.
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
        case 0:	return ( m_x );	break;
        case 1: return ( m_y );	break;
        case 2: return ( m_z );	break;
        case 3: return ( m_h );	break;
        default: return ( m_h );	break;
        }
    }

    /** Array based component access.
     * \param i Integer component index, 0-3.
     * \return Appropriate component, or h if index is invalid.
     */
    const TqFloat&	operator[] ( TqInt i ) const
    {
        switch ( i )
        {
        case 0:	return ( m_x );	break;
        case 1: return ( m_y );	break;
        case 2: return ( m_z );	break;
        case 3: return ( m_h );	break;
        default: return ( m_h );	break;
        }
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

    CqVector4D& operator= ( const CqVector3D &From );
    CqVector4D& operator+=( const CqVector4D &From );
    CqVector4D& operator-=( const CqVector4D &From );
    CqVector4D& operator%=( const CqVector4D &From );
    CqVector4D& operator*=( const TqFloat Scale );
    CqVector4D& operator/=( const TqFloat Scale );
    TqBool	operator==( const CqVector4D &Cmp ) const;
    TqBool	operator!=( const CqVector4D &Cmp ) const;
    TqBool	operator>=( const CqVector4D &Cmp ) const;
    TqBool	operator<=( const CqVector4D &Cmp ) const;
    TqBool	operator>( const CqVector4D &Cmp ) const;
    TqBool	operator<( const CqVector4D &Cmp ) const;

    friend CqVector4D	operator*( const TqFloat f, const CqVector4D& v )
    {
        CqVector4D r( v ); return ( r *= f );
    }
    friend CqVector4D	operator*( const CqVector4D& v, const TqFloat f )
    {
        CqVector4D r( v ); return ( r *= f );
    }
    friend CqVector4D	operator/( const TqFloat f, const CqVector4D& v )
    {
        CqVector4D r( v ); return ( r /= f );
    }
    friend CqVector4D	operator/( const CqVector4D& v, const TqFloat f )
    {
        CqVector4D r( v ); return ( r /= f );
    }

    friend CqVector4D	operator+( const CqVector4D& a, const CqVector4D& b )
    {
        CqVector4D r( a ); return ( r += b );
    }
    friend CqVector4D	operator-( const CqVector4D& a, const CqVector4D& b )
    {
        CqVector4D r( a ); return ( r -= b );
    }
    friend CqVector4D	operator-( const CqVector4D& v )
    {
        return ( CqVector4D( -v.m_x, -v.m_y, -v.m_z, v.m_h ) );
    } // Negation

    friend TqFloat	operator*( const CqVector4D& a, const CqVector4D& b );
    friend CqVector4D	operator%( const CqVector4D& a, const CqVector4D& b );	// X product
    friend std::ostream &operator<<( std::ostream &Stream, CqVector4D &Vector );

protected:
    TqFloat	m_x;	///< X component.
    TqFloat	m_y;	///< Y component.
    TqFloat	m_z;	///< Z component.
    TqFloat	m_h;	///< H component.
}
;

//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !VECTOR4D_H_INCLUDED
