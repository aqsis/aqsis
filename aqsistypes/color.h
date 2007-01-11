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
		\brief Declares the CqColor class for handling generic 3 element colors.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is color.h included already?
#ifndef COLOR_H_INCLUDED
//{
#define COLOR_H_INCLUDED 1

#include	"aqsis.h"

#include	"vector3d.h"

#include <iostream>

START_NAMESPACE( Aqsis )


//-----------------------------------------------------------------------
/** \class CqColor
 * Class to store and manipulate three component color information.
 */

class COMMON_SHARE CqColor
{
	public:
		/// Default constructor.
		CqColor() : m_fRed( 0.0f ), m_fGreen( 0.0f ), m_fBlue( 0.0f )
		{}
		/** Component constructor
		 * \param fRed red component 0.0-1.0
		 * \param fGreen green component 0.0-1.0
		 * \param fBlue blue component 0.0-1.0
		 */
		CqColor( TqFloat fRed, TqFloat fGreen, TqFloat fBlue ) :
				m_fRed( fRed ),
				m_fGreen( fGreen ),
				m_fBlue( fBlue )
		{}
		CqColor( TqFloat f ) :
				m_fRed( f ),
				m_fGreen( f ),
				m_fBlue( f )
		{}
		/** 3D vector constructor.
		 * \param From the vector to copy the component values from.
		 */
		CqColor( const CqVector3D& From )
		{
			*this = From;
		}
		/** Array component constructor.
		 * \param From array of floats to use as the components.
		 */
		CqColor( const TqFloat From[ 3 ] ) :
				m_fRed( From[ 0 ] ),
				m_fGreen( From[ 1 ] ),
				m_fBlue( From[ 2 ] )
		{}
		~CqColor()
		{}

		/** Get the red component.
		 * \return float red component 0.0-1.0.
		 */
		TqFloat	fRed() const
		{
			return ( m_fRed );
		}
		/** Set the red component.
		 * \param fValue the new value for the red component 0.0-1.0.
		 */
		void	SetfRed( const TqFloat fValue )
		{
			m_fRed = fValue;
		}
		/** Get the freen component.
		 * \return float green component 0.0-1.0.
		 */
		TqFloat	fGreen() const
		{
			return ( m_fGreen );
		}
		/** Set the green component.
		 * \param fValue the new value for the green component 0.0-1.0.
		 */
		void	SetfGreen( const TqFloat fValue )
		{
			m_fGreen = fValue;
		}
		/** Get the blue component.
		 * \return float blue component 0.0-1.0.
		 */
		TqFloat	fBlue() const
		{
			return ( m_fBlue );
		}
		/** Set the blue component.
		 * \param fValue the new value for the blue component 0.0-1.0.
		 */
		void	SetfBlue( const TqFloat fValue )
		{
			m_fBlue = fValue;
		}
		/** Get the color as indiviual components.
		 * \param pfRed pointer to the area to store the red component.
		 * \param pfGreen pointer to the area to store the green component.
		 * \param pfBlue pointer to the area to store the blue component.
		 */
		void	GetColorRGB( TqFloat* pfRed, TqFloat* pfGreen, TqFloat* pfBlue )
		{
			*pfRed = m_fRed;
			*pfGreen = m_fGreen;
			*pfBlue = m_fBlue;
		}
		/** Set the color as individual components.
		 * \param fRed the new value for the red component 0.0-1.0.
		 * \param fGreen the new value for the green component 0.0-1.0.
		 * \param fBlue the new value for the blue component 0.0-1.0.
		 */
		void	SetColorRGB( TqFloat fRed, TqFloat fGreen, TqFloat fBlue )
		{
			m_fRed = fRed;
			m_fGreen = fGreen;
			m_fBlue = fBlue;
		}

		CqColor	rgbtohsv() const;
		CqColor	rgbtohsl() const;
		CqColor	rgbtoXYZ() const;
		CqColor	rgbtoxyY() const;
		CqColor	rgbtoYIQ() const;
		CqColor	hsvtorgb() const;
		CqColor	hsltorgb() const;
		CqColor	XYZtorgb() const;
		CqColor	xyYtorgb() const;
		CqColor	YIQtorgb() const;

		/// Clamp the components to the range 0.0-1.0.
		void	Clamp()
		{
			if ( m_fRed > 1.0 )
				m_fRed = 1.0;
			if ( m_fGreen > 1.0 )
				m_fGreen = 1.0;
			if ( m_fBlue > 1.0 )
				m_fBlue = 1.0;

			if ( m_fRed < 0.0 )
				m_fRed = 0.0;
			if ( m_fGreen < 0.0 )
				m_fGreen = 0.0;
			if ( m_fBlue < 0.0 )
				m_fBlue = 0.0;
		}

		/** Array based component access.
		 * \param i integer component index, 0-2.
		 * \return a reference to the float value of the appropriate component, returns blue if index is invalid.
		 */
		TqFloat&	operator[] ( TqInt i )
		{
			if ( i==0 )
				return ( m_fRed );
			else if ( i == 1 )
				return ( m_fGreen );
			else
				return ( m_fBlue );
		}
		/** Array based read only component access.
		 * \param i integer component index, 0-2.
		 * \return a constant reference the float value of the appropriate component, returns blue if index is invalid.
		 */
		const TqFloat&	operator[] ( TqInt i ) const
		{
			if ( i==0 )
				return ( m_fRed );
			else if ( i == 1 )
				return ( m_fGreen );
			else
				return ( m_fBlue );
		}
		/** Copy value from a 3D vector.
		 * \param From the vector to get the color cmoponents from.
		 * \return a reference to this color.
		 */
		CqColor&	operator=( const CqVector3D& From )
		{
			m_fRed = From.x();
			m_fGreen = From.y();
			m_fBlue = From.z();

			return ( *this );
		}
		/** Additive assign operator.
		 * \param colFrom the color to add to this.
		 * \return a reference to this color.
		 */
		CqColor&	operator+=( const CqColor &colFrom )
		{
			m_fRed += colFrom.m_fRed;
			m_fGreen += colFrom.m_fGreen;
			m_fBlue += colFrom.m_fBlue;
			return ( *this );
		}
		/** Subtractive assign operator.
		 * \param colFrom the color to subtract from this.
		 * \return a reference to this color.
		 */
		CqColor&	operator-=( const CqColor &colFrom )
		{
			m_fRed -= colFrom.m_fRed;
			m_fGreen -= colFrom.m_fGreen;
			m_fBlue -= colFrom.m_fBlue;
			return ( *this );
		}
		/** Component wise multiplicative assign operator.
		 * \param colFrom the color to multiply this with.
		 * \return a reference to this color.
		 */
		CqColor&	operator*=( const CqColor& colFrom )
		{
			m_fRed *= colFrom.m_fRed;
			m_fGreen *= colFrom.m_fGreen;
			m_fBlue *= colFrom.m_fBlue;
			return ( *this );
		}
		/** Component wise multiplicative assign operator.
		 * \param fScale the float to multiply each component with.
		 * \return a reference to this color.
		 */
		CqColor&	operator*=( TqFloat fScale )
		{
			m_fRed *= fScale;
			m_fGreen *= fScale;
			m_fBlue *= fScale;
			return ( *this );
		}
		/** Component wise divisive assign operator.
		 * \param fScale the float to divide each component by.
		 * \return a reference to this color.
		 */
		CqColor&	operator/=( const TqFloat fScale )
		{
			m_fRed /= fScale;
			m_fGreen /= fScale;
			m_fBlue /= fScale;
			return ( *this );
		}
		/** Component wise divisive assign operator.
		 * \param colFrom the color to divide this by.
		 * \return a reference to this color.
		 */
		CqColor&	operator/=( const CqColor& colFrom )
		{
			m_fRed /= colFrom.m_fRed;
			m_fGreen /= colFrom.m_fGreen;
			m_fBlue /= colFrom.m_fBlue;
			return ( *this );
		}
		/** Component wise additive assign operator.
		 * \param Add the float to add to each component.
		 * \return a reference to this color.
		 */
		CqColor&	operator+=( const TqFloat Add )
		{
			m_fRed += Add;
			m_fGreen += Add;
			m_fBlue += Add;
			return ( *this );
		}
		/** Component wise subtractive assign operator.
		 * \param Sub the float to subtract from each component.
		 * \return a reference to this color.
		 */
		CqColor&	operator-=( const TqFloat Sub )
		{
			m_fRed -= Sub;
			m_fGreen -= Sub;
			m_fBlue -= Sub;
			return ( *this );
		}
		/** Component wise equality operator.
		 * \param colCmp the color to compare this with.
		 * \return boolean indicating equality.
		 */
		TqBool	operator==( const CqColor &colCmp ) const
		{
			return ( ( m_fRed == colCmp.m_fRed ) && ( m_fGreen == colCmp.m_fGreen ) && ( m_fBlue == colCmp.m_fBlue ) );
		}
		/** Component wise inequality operator.
		 * \param colCmp the color to compare this with.
		 * \return boolean indicating inequality.
		 */
		TqBool	operator!=( const CqColor &colCmp ) const
		{
			return ( !( *this == colCmp ) );
		}
		/** Component wise greater than or equal to operator.
		 * \param colCmp the color to compare this with.
		 * \return boolean indicating each component is greater than or equal to its counterpart in the argument.
		 */
		TqBool	operator>=( const CqColor &colCmp ) const
		{
			return ( ( m_fRed >= colCmp.m_fRed ) && ( m_fGreen >= colCmp.m_fGreen ) && ( m_fBlue >= colCmp.m_fBlue ) );
		}
		/** Component wise less than or equal to operator.
		 * \param colCmp the color to compare this with.
		 * \return boolean indicating each component is less than or equal to its counterpart in the argument.
		 */
		TqBool	operator<=( const CqColor &colCmp ) const
		{
			return ( ( m_fRed <= colCmp.m_fRed ) && ( m_fGreen <= colCmp.m_fGreen ) && ( m_fBlue <= colCmp.m_fBlue ) );
		}
		/** Component wise greater than to operator.
		 * \param colCmp the color to compare this with.
		 * \return boolean indicating each component is greater than its counterpart in the argument.
		 */
		TqBool	operator>( const CqColor &colCmp ) const
		{
			return ( ( m_fRed > colCmp.m_fRed ) && ( m_fGreen > colCmp.m_fGreen ) && ( m_fBlue > colCmp.m_fBlue ) );
		}
		/** Component wise less than to operator.
		 * \param colCmp the color to compare this with.
		 * \return boolean indicating each component is less than its counterpart in the argument.
		 */
		TqBool	operator<( const CqColor &colCmp ) const
		{
			return ( ( m_fRed < colCmp.m_fRed ) && ( m_fGreen < colCmp.m_fGreen ) && ( m_fBlue < colCmp.m_fBlue ) );
		}

		/** Component wise friend addition operator.
		 * \param f float to add to each component.
		 * \param c color to add to.
		 * \return new color representing addition. 
		 */
		friend CqColor	operator+( const TqFloat f, const CqColor& c )
		{
			CqColor r( c );
			return ( r += f );
		}
		/** Component wise friend addition operator.
		 * \param c color to add to.
		 * \param f float to add to each component.
		 * \return new color representing addition. 
		 */
		friend CqColor	operator+( const CqColor& c, const TqFloat f )
		{
			CqColor r( c );
			return ( r += f );
		}
		/** Component wise friend subtraction operator.
		 * \param f float to subtract from each component.
		 * \param c color to subtract from.
		 * \return new color representing subtraction. 
		 */
		friend CqColor	operator-( const TqFloat f, const CqColor& c )
		{
			CqColor r( f, f, f );
			return ( r -= c );
		}
		/** Component wise friend subtraction operator.
		 * \param c color to subtract from.
		 * \param f float to subtract from each component.
		 * \return new color representing subtraction. 
		 */
		friend CqColor	operator-( const CqColor& c, const TqFloat f )
		{
			CqColor r( c );
			return ( r -= f );
		}
		/** Component wise friend multiplication operator.
		 * \param f float to multiply each component with.
		 * \param c color to multiply with.
		 * \return new color representing multiplication. 
		 */
		friend CqColor	operator*( const TqFloat f, const CqColor& c )
		{
			CqColor r( f, f, f );
			return ( r *= c );
		}
		/** Component wise friend multiplication operator.
		 * \param c color to multiply with.
		 * \param f float to multiply each component with.
		 * \return new color representing multiplication. 
		 */
		friend CqColor	operator*( const CqColor& c, const TqFloat f )
		{
			CqColor r( c );
			return ( r *= f );
		}
		/** Component wise friend division operator.
		 * \param f float to divide each component by.
		 * \param c color to divide.
		 * \return new color representing division. 
		 */
		friend CqColor	operator/( const TqFloat f, const CqColor& c )
		{
			CqColor r( f, f, f );
			return ( r /= c );
		}
		/** Component wise friend division operator.
		 * \param c color to divide.
		 * \param f float to divide each component by.
		 * \return new color representing division. 
		 */
		friend CqColor	operator/( const CqColor& c, const TqFloat f )
		{
			CqColor r( c );
			return ( r /= f );
		}

		/** Component wise friend addition operator.
		 * \param a color to add to.
		 * \param b color to add.
		 * \return new color representing addition. 
		 */
		friend CqColor	operator+( const CqColor& a, const CqColor& b )
		{
			CqColor r( a );
			return ( r += b );
		}
		/** Component wise friend subtraction operator.
		 * \param a color to subtract from.
		 * \param b color to subtract.
		 * \return new color representing subtraction. 
		 */
		friend CqColor	operator-( const CqColor& a, const CqColor& b )
		{
			CqColor r( a );
			return ( r -= b );
		}
		/** Component wise friend multiplication operator.
		 * \param a color to multiply.
		 * \param b color to multiply by.
		 * \return new color representing multiplication. 
		 */
		friend CqColor	operator*( const CqColor& a, const CqColor& b )
		{
			CqColor r( a );
			return ( r *= b );
		}
		/** Component wise friend division operator.
		 * \param a color to divide.
		 * \param b color to divide by.
		 * \return new color representing division. 
		 */
		friend CqColor	operator/( const CqColor& a, const CqColor& b )
		{
			CqColor r( a );
			return ( r /= b );
		}
		/** Component wise friend negation operator.
		 * \param a color to negate.
		 * \return new color representing negation. 
		 */
		friend CqColor	operator-( const CqColor& a )
		{
			return ( CqColor( -a.m_fRed, -a.m_fGreen, -a.m_fBlue ) );
		} // Negation
		friend CqColor	operator%( const CqColor& a, const CqColor& b )
		{
			return ( a );
		} // Implemented purely for the use of the shader VM, we have macros
		  // that rely on the implementation of various operators for all
		  // supported shader types.
		/** Component wide stream output operator.
		 *\param Stream output stream.
		 *\param a color to serialize.
		 *\return input stream.
		 */
		friend std::ostream& operator<<( std::ostream& Stream, const CqColor& a )
		{
			Stream << a.m_fRed << " " << a.m_fGreen << " " << a.m_fBlue;
			return Stream;
		}

	private:
		TqFloat	m_fRed,     				///< the red component 0.0-1.0
		m_fGreen,     			///< the green component 0.0-1.0
		m_fBlue;			///< the blue component 0.0-1.0
}
;


/// Static white color
COMMON_SHARE extern CqColor	gColWhite;
/// Static black color
COMMON_SHARE extern CqColor	gColBlack;
/// Static red color
COMMON_SHARE extern CqColor	gColRed;
/// Static green color
COMMON_SHARE extern CqColor	gColGreen;
/// Static blue color
COMMON_SHARE extern CqColor	gColBlue;

//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )


//}  // End of #ifdef _H_INCLUDED
#endif
