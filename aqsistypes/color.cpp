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
		\brief Implements the CqColor class for handling generic 3 element colors.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<math.h>

#include	"aqsis.h"

#include	"color.h"
#include	"matrix.h"

START_NAMESPACE( Aqsis )


/// Global white color
CqColor	gColWhite( 1, 1, 1 );
/// Global black color
CqColor	gColBlack( 0, 0, 0 );
/// Global pure red color
CqColor	gColRed( 1, 0, 0 );
/// Global pure green color
CqColor	gColGreen( 0, 1, 0 );
/// Global pure blue color
CqColor	gColBlue( 0, 0, 1 );

/// The value of an undefined color component.
#define	UNDEFINED -1

//---------------------------------------------------------------------
/* Color space conversion.
 */

CqColor CqColor::rgbtohsv() const
{
	TqFloat R = m_fRed, G = m_fGreen, B = m_fBlue;
	TqFloat H = UNDEFINED, S, V;
	// RGB_TO_HSV from Foley, van Dam, Feiner, Hughes 2nd Ed. Pg 592
	TqFloat max = MAX( m_fRed, m_fGreen );
	max = MAX( max, m_fBlue );
	TqFloat min = MIN( m_fRed, m_fGreen );
	min = MIN( min, m_fBlue );
	TqFloat diff = max - min;

	V = max;

	// Next Saturation
	if ( max != 0.0 )
		S = diff / max;
	else
		S = 0.0;

	// Next Hue
	if ( S == 0.0 )
		H = UNDEFINED;	// UNDEFINED
	else
	{
		TqFloat r_dist = ( max - R ) / diff;
		TqFloat g_dist = ( max - G ) / diff;
		TqFloat b_dist = ( max - B ) / diff;

		if ( R == max )
			H = b_dist - g_dist;
		else if ( G == max )
			H = 2.0 + r_dist - b_dist;
		else if ( B == max )
			H = 4.0 + g_dist - r_dist;

		H *= 60.0;
		if ( H < 0.0 )
			H += 360.0;
	}

	return ( CqColor( H / 360.0, S, V ) );
}


//---------------------------------------------------------------------
/** Color space conversion.
 */

CqColor CqColor::rgbtohsl() const
{

	static TqFloat Small_Value = 0.0000001;

	TqFloat H = UNDEFINED, S = 0.0f, L;
	TqFloat R = m_fRed, G = m_fGreen, B = m_fBlue;
	// RGB_TO_HLS from Foley, van Dam, Feiner, Hughes 2nd Ed. Pg 595
	TqFloat max = MAX( R, G );
	max = MAX( max, B );
	TqFloat min = MIN( R, G );
	min = MIN( min, B );
	TqFloat diff = max - min;

	L = ( max + min ) / 2;

	// Next Saturation
	if ( fabs( diff ) <= Small_Value )
	{
		L = 0.0;
		H = UNDEFINED;
	}
	else
	{
		if ( L < 0.5 )
			S = diff / ( max + min );
		else
			S = diff / ( 2.0 - max - min );

		TqFloat r_dist = ( max - R ) / diff;
		TqFloat g_dist = ( max - G ) / diff;
		TqFloat b_dist = ( max - B ) / diff;

		// Next Hue
		if ( R == max )
			H = b_dist - g_dist;
		else if ( G == max )
			H = 2.0 + r_dist - b_dist;
		else if ( B == max )
			H = 4.0 + g_dist - r_dist;

		H *= 60;
		if ( H < 0 )
			H += 360;
	}

	return ( CqColor( H / 360.0, S, L ) );
}


//---------------------------------------------------------------------
/** Color space conversion.
 */

CqColor CqColor::rgbtoXYZ() const
{
	CqColor c( *this );
	return ( c );
}

//---------------------------------------------------------------------
/** Color space conversion.
 */

CqColor CqColor::rgbtoxyY() const
{
	CqColor c( *this );
	return ( c );
}


//---------------------------------------------------------------------
/** Color space conversion.
 */

CqColor CqColor::rgbtoYIQ() const
{
	static CqMatrix matRGBtoYIQ( 0.299, 0.587, 0.114, 0,
	                             0.596, -0.274, -0.322, 0,
	                             0.212, -0.523, 0.311, 0,
	                             0, 0, 0, 1 );

	CqColor c = CqColor( matRGBtoYIQ * CqVector3D( m_fRed, m_fGreen, m_fBlue ) );

	return ( c );
}


//---------------------------------------------------------------------
/** Color space conversion.
 */

CqColor CqColor::hsvtorgb() const
{
	TqFloat H = m_fRed * 360.0, S = m_fGreen, V = m_fBlue;
	TqFloat R = 0.0f, G = 0.0f, B = 0.0f;
	// HSV_TO_RGB from Foley, van Dam, Feiner, Hughes 2nd Ed. Pg 593
	if ( S == 0.0 )
	{
		assert( H < 0 );	// If s==0 then h MUST be undefined.
		R = G = B = V;
	}
	else
	{
		if ( H == 360 )
			H = 0;
		H /= 60;	// H is now in [0,6]
		TqInt i = static_cast<TqInt>( FLOOR( H ) );
		TqFloat f = H - i;
		TqFloat p = V * ( 1.0 - S );
		TqFloat q = V * ( 1.0 - ( S * f ) );
		TqFloat t = V * ( 1.0 - ( S * ( 1.0 - f ) ) );
		switch ( i )
		{
				case 0:
				R = V;
				G = t;
				B = p;
				break;
				case 1:
				R = q;
				G = V;
				B = p;
				break;
				case 2:
				R = p;
				G = V;
				B = t;
				break;
				case 3:
				R = p;
				G = q;
				B = V;
				break;
				case 4:
				R = t;
				G = p;
				B = V;
				break;
				case 5:
				R = V;
				G = p;
				B = q;
				break;
		}
	}

	return ( CqColor( R, G, B ) );
}


//---------------------------------------------------------------------
/** Color space conversion.
 */

static TqFloat HSLValue( TqFloat n1, TqFloat n2, TqFloat hue )
{
	TqFloat Value;

	if ( hue > 360 )
		hue -= 360;
	else if ( hue < 0 )
		hue += 360;

	if ( hue < 60 )
		Value = n1 + ( n2 - n1 ) * hue / 60;
	else if ( hue < 180 )
		Value = n2;
	else if ( hue < 240 )
		Value = n1 + ( n2 - n1 ) * ( 240 - hue ) / 60;
	else
		Value = n1;

	return ( Value );
}


//---------------------------------------------------------------------
/** Color space conversion.
 */

CqColor CqColor::hsltorgb() const
{
	TqFloat R, G, B, H = m_fRed * 360.0, S = m_fGreen, L = m_fBlue;
	// HLS_TO_RGB from Foley, van Dam, Feiner, Hughes 2nd Ed. Pg 596

	TqFloat m2, m1;
	if ( L <= 0.5 )
		m2 = L * ( 1.0 + S );
	else
		m2 = L + S - ( L * S );
	m1 = 2 * L - m2;

	if ( S == 0 )
	{
		assert( m_fRed < 0 );
		R = G = B = L;
	}
	else
	{
		R = HSLValue( m1, m2, H + 120 );
		G = HSLValue( m1, m2, H );
		B = HSLValue( m1, m2, H - 120 );
	}

	return ( CqColor( R, G, B ) );
}


//---------------------------------------------------------------------
/** Color space conversion.
 */

CqColor CqColor::XYZtorgb() const
{
	CqColor c( *this );
	return ( c );
}


//---------------------------------------------------------------------
/** Color space conversion.
 */

CqColor CqColor::xyYtorgb() const
{
	CqColor c( *this );
	return ( c );
}


//---------------------------------------------------------------------
/** Color space conversion.
 */

CqColor CqColor::YIQtorgb() const
{
	static CqMatrix matRGBtoYIQ( 1, 0.956, 0.621, 0,
	                             1, -0.272, -0.647, 0,
	                             1, -1.105, 1.702, 0,
	                             0, 0, 0, 1 );

	CqColor c = CqColor( matRGBtoYIQ * CqVector3D( m_fRed, m_fGreen, m_fBlue ) );

	return ( c );
}

//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )


