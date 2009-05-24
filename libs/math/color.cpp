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
		\brief Implements the CqColor class for handling generic 3 element colors.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include <aqsis/math/color.h>

#include <aqsis/math/matrix.h>
#include <aqsis/math/vectorcast.h>

namespace Aqsis {


// Global white color
const CqColor gColWhite(1, 1, 1);
// Global black color
const CqColor gColBlack(0, 0, 0);

// The value of an undefined color component.
static const TqFloat UNDEFINED = -1;

//---------------------------------------------------------------------
CqColor rgbtohsv(const CqColor& col)
{
	TqFloat R = col.r(), G = col.g(), B = col.b();
	TqFloat H = UNDEFINED, S, V;
	// RGB_TO_HSV from Foley, van Dam, Feiner, Hughes 2nd Ed. Pg 592
	TqFloat maxComp = max(max(R, G), B);
	TqFloat minComp = min(min(R, G), B);
	TqFloat diff = maxComp - minComp;

	V = maxComp;

	// Next Saturation
	if ( maxComp != 0.0 )
		S = diff / maxComp;
	else
		S = 0.0;

	// Next Hue
	if ( S == 0.0 )
		H = UNDEFINED;	// UNDEFINED
	else
	{
		TqFloat r_dist = ( maxComp - R ) / diff;
		TqFloat g_dist = ( maxComp - G ) / diff;
		TqFloat b_dist = ( maxComp - B ) / diff;

		if ( R == maxComp )
			H = b_dist - g_dist;
		else if ( G == maxComp )
			H = 2.0 + r_dist - b_dist;
		else if ( B == maxComp )
			H = 4.0 + g_dist - r_dist;

		H *= 60.0;
		if ( H < 0.0 )
			H += 360.0;
	}

	return ( CqColor( H / 360.0, S, V ) );
}

CqColor rgbtohsl(const CqColor& col)
{
	const TqFloat Small_Value = 0.0000001;

	TqFloat H = UNDEFINED, S = 0.0f, L;
	TqFloat R = col.r(), G = col.g(), B = col.b();
	// RGB_TO_HLS from Foley, van Dam, Feiner, Hughes 2nd Ed. Pg 595
	TqFloat maxComp = max(max(R, G), B);
	TqFloat minComp = min(min(R, G), B);
	TqFloat diff = maxComp - minComp;

	L = ( maxComp + minComp ) / 2;

	// Next Saturation
	if ( fabs( diff ) <= Small_Value )
	{
		L = 0.0;
		H = UNDEFINED;
	}
	else
	{
		if ( L < 0.5 )
			S = diff / ( maxComp + minComp );
		else
			S = diff / ( 2.0 - maxComp - minComp );

		TqFloat r_dist = ( maxComp - R ) / diff;
		TqFloat g_dist = ( maxComp - G ) / diff;
		TqFloat b_dist = ( maxComp - B ) / diff;

		// Next Hue
		if ( R == maxComp )
			H = b_dist - g_dist;
		else if ( G == maxComp )
			H = 2.0 + r_dist - b_dist;
		else if ( B == maxComp )
			H = 4.0 + g_dist - r_dist;

		H *= 60;
		if ( H < 0 )
			H += 360;
	}

	return ( CqColor( H / 360.0, S, L ) );
}

CqColor rgbtoXYZ(const CqColor& col)
{
	assert(0 && "Not implemented!");
	return col;
}
CqColor rgbtoxyY(const CqColor& col)
{
	assert(0 && "Not implemented!");
	return col;
}

CqColor rgbtoYIQ(const CqColor& col)
{
	const CqMatrix matRGBtoYIQ( 0.299, 0.587, 0.114, 0,
	                             0.596, -0.274, -0.322, 0,
	                             0.212, -0.523, 0.311, 0,
	                             0, 0, 0, 1 );

	return vectorCast<CqColor>(matRGBtoYIQ * vectorCast<CqVector3D>(col));
}

CqColor hsvtorgb(const CqColor& col)
{
	TqFloat H = col.r() * 360.0, S = col.g(), V = col.b();
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
		TqInt i = lfloor(H);
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

	return CqColor(R, G, B);
}

/// Helper function for hsltorgb().
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

CqColor hsltorgb(const CqColor& col)
{
	TqFloat R, G, B, H = col.r() * 360.0, S = col.g(), L = col.b();
	// HLS_TO_RGB from Foley, van Dam, Feiner, Hughes 2nd Ed. Pg 596

	TqFloat m2, m1;
	if ( L <= 0.5 )
		m2 = L * ( 1.0 + S );
	else
		m2 = L + S - ( L * S );
	m1 = 2 * L - m2;

	if ( S == 0 )
	{
		assert( col.r() < 0 );
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

CqColor XYZtorgb(const CqColor& col)
{
	assert(0 && "Not implemented!");
	return col;
}

CqColor xyYtorgb(const CqColor& col)
{
	assert(0 && "Not implemented!");
	return col;
}

CqColor YIQtorgb(const CqColor& col)
{
	const CqMatrix matYIQtoRGB( 1, 0.956, 0.621, 0,
	                             1, -0.272, -0.647, 0,
	                             1, -1.105, 1.702, 0,
	                             0, 0, 0, 1 );

	return vectorCast<CqColor>(matYIQtoRGB * vectorCast<CqVector3D>(col));
}

//---------------------------------------------------------------------

} // namespace Aqsis


