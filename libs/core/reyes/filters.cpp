// Aqsis
// Copyright 1997 - 2001, Paul C. Gregory
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
		\brief Implement the filters of the RenderMan API functions.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	<aqsis/aqsis.h>

#include	<stdarg.h>
#include	<list>

#include	<aqsis/ri/ri.h>
#include	<aqsis/math/math.h>

namespace Aqsis {

// Mitchell Filter Declarations
class CqMitchellFilter {
public:
   // CqMitchellFilter Public Methods
   CqMitchellFilter(TqFloat b, TqFloat c, TqFloat xw, TqFloat yw)
   {
      B = b;
      C = c;
      invXWidth = 1.0f/xw;
      invYWidth = 1.0f/yw;
   }
   TqFloat Evaluate(TqFloat x, TqFloat y) const {
      return Evaluate(x * invXWidth) * Evaluate(y * invYWidth);
   }
   TqFloat Evaluate(TqFloat x) const {
      x = fabsf(2.f * x);
      if (x > 1.f)
         return ((-B - 6*C) * x*x*x + (6*B + 30*C) * x*x +
         (-12*B - 48*C) * x + (8*B + 24*C)) * (1.f/6.f);
      else
         return ((12 - 9*B - 6*C) * x*x*x +
         (-18 + 12*B + 6*C) * x*x +
         (6 - 2*B)) * (1.f/6.f);
   }
private:
   TqFloat B, C;
   TqFloat invXWidth, invYWidth;
};

} // namespace Aqsis

//----------------------------------------------------------------------
// RiGaussianFilter
// Gaussian filter used as a possible value passed to RiPixelFilter.
//
extern "C" 
 RtFloat	RiGaussianFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	/*
	 *  d = distance from origin
	 *  w = filterwidth ([COOK84a] article used 1.5)
	 *      For here use sqrt( (xwidth/2)*(xwidth/2) + (ywidth/2)*(ywidth/2) ).
	 *      Simplifying:
	 *
	 *          w = sqrt( (xwidth*xwidth)/2 + (ywidth*ywidth)/2 )
	 *          w = sqrt( (xwidth*xwidth + ywidth*ywidth)/2 )
	 *        w*w = (xwidth*xwidth + ywidth*ywidth)/2
	 *
	 *  if (d < filterwidth) then 0
	 *  else  exp(-d*d) - exp(-w*w)
	 *
	 */
	//RtFloat d,d2,w,w2;
	//
	///* d = sqrt(x*x+y*y), d*d = (x*x+y*y)  */
	//d2 = (x*x+y*y);
	//d = sqrt(d2);
	//
	//w2 = 0.5*(xwidth*xwidth + ywidth*ywidth);
	//w = sqrt(w2);
	//
	//if(d>w)
	//	return(0.0);
	//else
	//	return(exp(-d2) - exp(-w2));

	// The above version falls faster than the one used by the 3.2 spec
	//   PRMan and RenderDotC.  Since all three match exactly, might as
	//   well change to the code below:
	x /= xwidth;
	y /= ywidth;

	return exp( -8.0 * ( x * x + y * y ) );
}


//----------------------------------------------------------------------
// RiMitchellFilter
// Mitchell filter used as a possible value passed to RiPixelFIlter.
//
extern "C" 
RtFloat	RiMitchellFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	Aqsis::CqMitchellFilter mc(1/3.0f, 1/3.0f, xwidth, ywidth);

	return mc.Evaluate(x, y);
}

//----------------------------------------------------------------------
// RiBoxFilter
// Box filter used as a possible value passed to RiPixelFIlter.
//
extern "C" 
RtFloat	RiBoxFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	/* [UPST89] -- (RC p. 178) says that x and y will be in the
	 *    following intervals:
	 *           -xwidth/2 <= x <= xwidth/2
	 *           -ywidth/2 <= y <= ywidth/2
	 *    These constraints on x and y really simplifies the
	 *       the following code to just return (1.0).
	 *
	 */
	return Aqsis::min( ( fabs( x ) <= xwidth / 2.0 ? 1.0 : 0.0 ),
	            ( fabs( y ) <= ywidth / 2.0 ? 1.0 : 0.0 ) );
}


//----------------------------------------------------------------------
// RiTriangleFilter
// Triangle filter used as a possible value passed to RiPixelFilter
//
extern "C" 
RtFloat	RiTriangleFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	RtFloat	hxw = xwidth / 2.0;
	RtFloat	hyw = ywidth / 2.0;
	RtFloat	absx = fabs( x );
	RtFloat	absy = fabs( y );

	/* This function can be simplified as well by not worrying about
	 *    returning zero if the sample is beyond the filter window.
	 */
	return Aqsis::min( ( absx <= hxw ? ( hxw - absx ) / hxw : 0.0 ),
	            ( absy <= hyw ? ( hyw - absy ) / hyw : 0.0 ) );
}


//----------------------------------------------------------------------
// RiCatmullRomFilter
// Catmull Rom filter used as a possible value passed to RiPixelFilter.
//
extern "C" 
RtFloat	RiCatmullRomFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	/*
	 * From page 223 of [MITC88]
	 *
	 * if abs(d) < 1
	 *    f(d) = 1/6*(  (12-9*B-9*C)*abs(d*d*d)
	 *                + (-18 + 12*B + 6*C)*d*d + (6-2*B) )
	 *
	 * if 1 <= abs(d) < 2
	 *    f(d) = 1/6*(  (-B-6*C)*abs(d*d*d)
	 *                + (6*B + 30*C)*d*d
	 *                + (-12*B - 48*C)*d
	 *                + (8*B + 24*C) )
	 *
	 * otherwise  f(d)=0
	 *
	 * -------------------------------------------------------------
	 *  When B = 0.0 and C = 0.5 the filter is a Catmull-Rom cubic spline.
	 *
	 * if abs(d) < 1
	 *    f(d) = 1/6*[  (12-3)*abs(d*d*d) + (-18 + 3)*d*d + (6) ]
	 *
	 * if 1 <= abs(d) < 2
	 *    f(d) = 1/6*[  (-3)*abs(d*d*d) + (15)*d*d + (-24)*d + (12) ]
	 *
	 * otherwise  f(d)=0
	 * -------------------------------------------------------------
	 * Simplifying:
	 *
	 * if abs(d) < 1
	 *    f(d) = (3/2)*abs(d*d*d) - (5/2)*d*d + 1
	 *
	 * if 1 <= abs(d) <2
	 *    f(d) = (-0.5)*abs(d*d*d) + (5/2)*d*d - 4*abs(d) + 2
	 *
	 * otherwise  f(d)=0
	 *
	 */
	/*    RtFloat d, d2;

	    d2 = x * x + y * y; // d*d
	    d = sqrt( d2 ); // distance from origin

	    if ( d < 1 )
	        return ( 1.5 * d * d2 - 2.5 * d2 + 1.0 );
	    else if ( d < 2 )
	        return ( -d * d2 * 0.5 + 2.5 * d2 - 4.0 * d + 2.0 );
	    else
	        return 0.0;*/

	/* RI SPec 3.2 */
	RtFloat r2 = (x*x+y*y);
	RtFloat r = sqrt(r2);
	return (r>=2.0)?0.0:
	       (r<1.0)?(3.0*r*r2-5.0*r2+2.0):(-r*r2+5.0*r2-8.0*r+4.0);
}


//----------------------------------------------------------------------
// RiSincFilter
// Sinc filter used as a possible value passed to RiPixelFilter.
//
extern "C" 
RtFloat	RiSincFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	//RtFloat d;
	//
	//d = sqrt(x*x+y*y);
	//
	//if(d!=0)
	//	return(sin(RI_PI*d)/(RI_PI*d));
	//else
	//	return(1.0);

	// The above is an un-windowed sinc, below is a windowed sinc
	//   function similar in shape to what PRMan 3.9 uses.
	// tburge 5-28-01

	/* Modified version of the RI Spec 3.2 sinc filter to be
	 *   windowed with a positive lobe of a cosine which is half
	 *   of a cosine period.
	 */

	/* Uses a -PI to PI cosine window. */
	if ( x != 0.0 )
	{
		x *= RI_PI;
		x = cos( 0.5 * x / xwidth ) * sin( x ) / x;
	}
	else
	{
		x = 1.0;
	}
	if ( y != 0.0 )
	{
		y *= RI_PI;
		y = cos( 0.5 * y / ywidth ) * sin( y ) / y;
	}
	else
	{
		y = 1.0;
	}

	/* This is a square separable filter and is the 2D Fourier
	 * transform of a rectangular box outlining a lowpass bandwidth
	* filter in the frequency domain.
	*/
	return x*y;
}


//----------------------------------------------------------------------
// RiDiskFilter -- this is in Pixar's ri.h
// Cylindrical filter used as a possible value passed to RiPixelFilter
//
extern "C" 
RtFloat	RiDiskFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
	double d, xx, yy;

	xx = x * x;
	yy = y * y;
	xwidth *= 0.5;
	ywidth *= 0.5;

	d = ( xx ) / ( xwidth * xwidth ) + ( yy ) / ( ywidth * ywidth );
	if ( d < 1.0 )
	{
		return 1.0;
	}
	else
	{
		return 0.0;
	}
}


//----------------------------------------------------------------------
// RiBesselFilter -- this is in Pixar's ri.h
// Besselj0 filter used as a possible value passed to RiPixelFilter
//
extern "C" 
RtFloat	RiBesselFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{

	double d, w, xx, yy;

	xx = x * x;
	yy = y * y;

	xwidth *= 0.5;
	ywidth *= 0.5;

	w = ( xx ) / ( xwidth * xwidth ) + ( yy ) / ( ywidth * ywidth );
	if ( w < 1.0 )
	{
		d = sqrt( xx + yy );
		if ( d != 0.0 )
		{
			/* Half cosine window. */
			w = cos( 0.5 * RI_PI * sqrt( w ) );
			return w * 2*j1( RI_PI * d ) / d;
		}
		else
		{
			return RI_PI;
		}
	}
	else
	{
		return 0.0;
	}
}

//---------------------------------------------------------------------




