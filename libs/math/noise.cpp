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
		\brief Implements the CqNoise wrapper class to CqNoise1234 for producing Perlin noise.
		\author Paul C. Gregory (pgregory@aqsis.org)
		\author Stefan Gustavson (stegu@itn.liu.se)
*/

#include <aqsis/math/noise.h>

#include <aqsis/math/noise1234.h>
#include <aqsis/math/vectorcast.h>


namespace Aqsis {

// This macro is slightly differently implemented as
// FLOOR() in "aqsis_types.h". I could have used that version
// instead, but the FLOOR/CEIL/ROUND in "aqsis_types.h" all
// seem unnecessarily complicated to me.
// This is faster because of fewer conditionals, and rounding
// to the nearest integer by using FASTFLOOR(x+0.5) is a lot
// faster than using the ROUND() in "aqsis_types.h".
#define FASTFLOOR(x) ( ((x)>0) ? ((int)x) : ((int)x-1 ) )

// Carefully chosen but somewhat arbitrary x, y, z, t offsets
// for repeated evaluation for vector return types.
#define	O1x	19.34
#define	O1y	7.66
#define	O1z	3.23
#define O1t 2.77

#define	O2x	5.47
#define	O2y	17.85
#define	O2z	11.04
#define	O2t	13.19

// These are actually never used, because SL has no 4D return types
#define	O3x	23.54
#define	O3y	29.11
#define	O3z	31.91
#define	O3t	37.48


//---------------------------------------------------------------------
/** 1D float Perlin noise, SL "noise()"
 */
TqFloat CqNoise::FGNoise1( TqFloat x )
{
	return ( 0.5f * (1.0f + CqNoise1234::noise( x ) ) );
}

//---------------------------------------------------------------------
/** 1D float Perlin periodic noise, SL "pnoise()"
 */
TqFloat CqNoise::FGPNoise1( TqFloat x, TqFloat pfx )
{
	TqInt px;
	pfx = pfx + 0.5f;
	px = FASTFLOOR( pfx );
	return ( 0.5f * ( 1.0f + CqNoise1234::pnoise( x, px ) ) );
}

//---------------------------------------------------------------------
/** 2D float Perlin noise.
 */
TqFloat CqNoise::FGNoise2( TqFloat x, TqFloat y )
{
	return ( 0.5f * ( 1.0f + CqNoise1234::noise( x, y ) ) );
}

//---------------------------------------------------------------------
/** 2D float Perlin periodic noise.
 */
TqFloat CqNoise::FGPNoise2( TqFloat x, TqFloat y, TqFloat pfx, TqFloat pfy )
{
	TqInt px, py;
	pfx = pfx + 0.5;
	pfy = pfy + 0.5;
	px = FASTFLOOR( pfx );
	py = FASTFLOOR( pfy );
	return ( 0.5f * ( 1.0f + CqNoise1234::pnoise( x, y, px, py ) ) );
}

//---------------------------------------------------------------------
/** 3D float Perlin noise.
 */
TqFloat	CqNoise::FGNoise3( const CqVector3D& v )
{
	TqFloat x, y, z;
	x = v.x();
	y = v.y();
	z = v.z();
	return ( 0.5f * ( 1.0f + CqNoise1234::noise( x, y, z ) ) );
}

//---------------------------------------------------------------------
/** 3D float Perlin periodic noise.
 */
TqFloat	CqNoise::FGPNoise3( const CqVector3D& v, const CqVector3D& pv )
{
	TqFloat x, y, z;
	TqFloat pfx, pfy, pfz;
	TqInt px, py, pz;
	x = v.x();
	y = v.y();
	z = v.z();
	pfx = pv.x() + 0.5f; // Temp variables to avoid having the FASTFLOOR() macro
	pfy = pv.y() + 0.5f; // expand to something that is difficult to optimise.
	pfz = pv.z() + 0.5f; // (An inline function fastfloor() would be nicer.)
	px = FASTFLOOR( pfx );
	py = FASTFLOOR( pfy );
	pz = FASTFLOOR( pfz );
	return ( 0.5f * ( 1.0f + CqNoise1234::pnoise( x, y, z, px, py, pz ) ) );
}

//---------------------------------------------------------------------
/** 4D float Perlin noise.
 */
TqFloat	CqNoise::FGNoise4( const CqVector3D& v, TqFloat t )
{
	TqFloat x, y, z;
	x = v.x();
	y = v.y();
	z = v.z();
	return ( 0.5f * ( 1.0f + CqNoise1234::noise( x, y, z, t ) ) );
}

//---------------------------------------------------------------------
/** 4D float Perlin periodic noise.
 */
TqFloat	CqNoise::FGPNoise4( const CqVector3D& v, TqFloat t, const CqVector3D& pv, TqFloat pft )
{
	TqFloat x, y, z;
	TqFloat pfx, pfy, pfz;
	TqInt px, py, pz, pt;
	x = v.x();
	y = v.y();
	z = v.z();
	pfx = pv.x() + 0.5f; // Temp variables to avoid having the FASTFLOOR() macro
	pfy = pv.y() + 0.5f; // expand to something that is difficult to optimise.
	pfz = pv.z() + 0.5f;
	pft = pft + 0.5f;
	px = FASTFLOOR( pfx );
	py = FASTFLOOR( pfy );
	pz = FASTFLOOR( pfz );
	pt = FASTFLOOR( pft );
	return ( 0.5f * ( 1.0f + CqNoise1234::pnoise( x, y, z, t, px, py, pz, pt ) ) );
}

//---------------------------------------------------------------------
/** Vector-valued 1D Perlin noise.
 */
CqVector3D CqNoise::PGNoise1( TqFloat x )
{
	TqFloat a, b, c;
	a = 0.5f * ( 1.0f + CqNoise1234::noise( x ) );
	b = 0.5f * ( 1.0f + CqNoise1234::noise( x + O1x ) );
	c = 0.5f * ( 1.0f + CqNoise1234::noise( x + O2x ) );
	return ( CqVector3D( a, b, c ) );
}

//---------------------------------------------------------------------
/** Vector-valued 1D Perlin periodic noise.
 */
CqVector3D CqNoise::PGPNoise1( TqFloat x, TqFloat pfx )
{
	TqFloat a, b, c;
	TqInt px;
	pfx = pfx + 0.5f;
	px = FASTFLOOR( pfx );
	a = 0.5f * ( 1.0f + CqNoise1234::pnoise( x, px ) );
	b = 0.5f * ( 1.0f + CqNoise1234::pnoise( x + O1x, px ) );
	c = 0.5f * ( 1.0f + CqNoise1234::pnoise( x + O2x, px ) );
	return ( CqVector3D( a, b, c ) );
}

//---------------------------------------------------------------------
/** Vector-valued 2D Perlin noise.
 */
CqVector3D CqNoise::PGNoise2( TqFloat x, TqFloat y )
{
	TqFloat a, b, c;
	a = 0.5f * ( 1.0f + CqNoise1234::noise( x, y ) );
	b = 0.5f * ( 1.0f + CqNoise1234::noise( x + O1x, y + O1y ) );
	c = 0.5f * ( 1.0f + CqNoise1234::noise( x + O2x, y + O2y ) );
	return ( CqVector3D( a, b, c ) );
}

//---------------------------------------------------------------------
/** Vector-valued 2D Perlin periodic noise.
 */
CqVector3D CqNoise::PGPNoise2( TqFloat x, TqFloat y, TqFloat pfx, TqFloat pfy )
{
	TqInt px, py;
	TqFloat a, b, c;
	pfx = pfx + 0.5;
	pfy = pfy + 0.5;
	px = FASTFLOOR( pfx );
	py = FASTFLOOR( pfy );
	a = 0.5f * ( 1.0f + CqNoise1234::pnoise( x, y, px, py ) );
	b = 0.5f * ( 1.0f + CqNoise1234::pnoise( x + O1x, y + O1y, px, py ) );
	c = 0.5f * ( 1.0f + CqNoise1234::pnoise( x + O2x, y + O2y, px, py ) );
	return ( CqVector3D( a, b, c ) );
}

//---------------------------------------------------------------------
/** Vector-valued 3D Perlin noise.
 */
CqVector3D CqNoise::PGNoise3( const CqVector3D& v )
{
	TqFloat x, y, z;
	TqFloat a, b, c;
	x = v.x();
	y = v.y();
	z = v.z();
	a = 0.5f * ( 1.0f + CqNoise1234::noise( x, y, z ) );
	b = 0.5f * ( 1.0f + CqNoise1234::noise( x + O1x, y + O1y, z + O1z ) );
	c = 0.5f * ( 1.0f + CqNoise1234::noise( x + O2x, y + O2y, z + O2z ) );
	return ( CqVector3D( a, b, c ) );
}

//---------------------------------------------------------------------
/** Vector-valued 3D Perlin periodic noise.
 */
CqVector3D CqNoise::PGPNoise3( const CqVector3D& v, const CqVector3D& pv )
{
	TqFloat x, y, z;
	TqFloat a, b, c;
	TqFloat pfx, pfy, pfz;
	TqInt px, py, pz;
	x = v.x();
	y = v.y();
	z = v.z();
	pfx = pv.x() + 0.5f; // Temp variables to avoid having the FASTFLOOR() macro
	pfy = pv.y() + 0.5f; // expand to something that is difficult to optimise.
	pfz = pv.z() + 0.5f; // This might seem stupid, but it *is* actually faster.
	px = FASTFLOOR(pfx);
	py = FASTFLOOR(pfy);
	pz = FASTFLOOR(pfz);
	a = 0.5f * ( 1.0f + CqNoise1234::pnoise( x, y, z, px, py, pz ) );
	b = 0.5f * ( 1.0f + CqNoise1234::pnoise( x + O1x, y + O1y, z + O1z, px, py, pz ) );
	c = 0.5f * ( 1.0f + CqNoise1234::pnoise( x + O2x, y + O2y, z + O2z, px, py, pz ) );
	return ( CqVector3D ( a, b, c ) );
}

//---------------------------------------------------------------------
/** Vector-valued 4D Perlin noise.
 */
CqVector3D CqNoise::PGNoise4( const CqVector3D& v, TqFloat t )
{
	TqFloat x, y, z;
	TqFloat a, b, c;
	x = v.x();
	y = v.y();
	z = v.z();
	a = 0.5f * ( 1.0f + CqNoise1234::noise( x, y, z, t ) );
	b = 0.5f * ( 1.0f + CqNoise1234::noise( x + O1x, y + O1y, z + O1z, t + O1t ) );
	c = 0.5f * ( 1.0f + CqNoise1234::noise( x + O2x, y + O2y, z + O2z, t + O2t ) );
	return ( CqVector3D( a, b, c ) );
}

//---------------------------------------------------------------------
/** Vector-valued 4D Perlin periodic noise.
 */
CqVector3D	CqNoise::PGPNoise4( const CqVector3D& v, TqFloat t, const CqVector3D& pv, TqFloat pft )
{
	TqFloat x, y, z;
	TqFloat a, b, c;
	TqFloat pfx, pfy, pfz;
	TqInt px, py, pz, pt;
	x = v.x();
	y = v.y();
	z = v.z();
	pfx = pv.x() + 0.5f; // Temp variables to avoid having the FASTFLOOR() macro
	pfy = pv.y() + 0.5f; // expand to something that is difficult to optimise.
	pfz = pv.z() + 0.5f;
	pft = pft + 0.5f;
	px = FASTFLOOR( pfx );
	py = FASTFLOOR( pfy );
	pz = FASTFLOOR( pfz );
	pt = FASTFLOOR( pft );
	a = 0.5f * ( 1.0f + CqNoise1234::pnoise( x, y, z, t, px, py, pz, pt ) );
	b = 0.5f * ( 1.0f + CqNoise1234::pnoise( x + O1x, y + O1y, z + O1z, t + O1t, px, py, pz, pt ) );
	c = 0.5f * ( 1.0f + CqNoise1234::pnoise( x + O2x, y + O2y, z + O2z, t + O2t, px, py, pz, pt ) );
	return ( CqVector3D( a, b, c ) );
}


// The Color noise versions below are all simple conversions from Point noise.
// This assumes that Color is always a 3-vector, which is indeed the case now,
// but the Ri spec formally allows for an arbitrary number of color components.
// A change from RGB to a more general Color class would require changes here.
// Hyperspectral renderings are still uncommon, so this is a minor problem.

//---------------------------------------------------------------------
/** Color-valued 1D Perlin noise.
 */
CqColor CqNoise::CGNoise1( TqFloat x )
{
	return vectorCast<CqColor>( PGNoise1( x ) );
}

//---------------------------------------------------------------------
/** Color-valued 1D Perlin periodic noise.
 */
CqColor CqNoise::CGPNoise1( TqFloat x, TqFloat px )
{
	return vectorCast<CqColor>( PGPNoise1( x, px ) );
}

//---------------------------------------------------------------------
/** Color-valued 2D Perlin noise.
 */
CqColor CqNoise::CGNoise2( TqFloat x, TqFloat y )
{
	return vectorCast<CqColor>( PGNoise2( x, y ) );
}

//---------------------------------------------------------------------
/** Color-valued 2D Perlin periodic noise.
 */
CqColor	CqNoise::CGPNoise2( TqFloat x, TqFloat y, TqFloat px, TqFloat py )
{
	return vectorCast<CqColor>( PGPNoise2( x, y, px, py ) );
}

//---------------------------------------------------------------------
/** Color-valued 3D Perlin noise.
 */
CqColor	CqNoise::CGNoise3( const CqVector3D& v )
{
	return vectorCast<CqColor>( PGNoise3( v ) );
}

//---------------------------------------------------------------------
/** Color-valued 3D Perlin periodic noise.
 */
CqColor	CqNoise::CGPNoise3( const CqVector3D& v, const CqVector3D& pv )
{
	return vectorCast<CqColor>( PGPNoise3( v, pv ) );
}

//---------------------------------------------------------------------
/** Color-valued 4D Perlin noise.
 */
CqColor	CqNoise::CGNoise4( const CqVector3D& v, TqFloat t )
{
	return vectorCast<CqColor>( PGNoise4( v, t ) );
}

//---------------------------------------------------------------------
/** Color-valued 4D Perlin periodic noise.
 */
CqColor	CqNoise::CGPNoise4( const CqVector3D& v, TqFloat t, const CqVector3D& pv, TqFloat pt )
{
	return vectorCast<CqColor>( PGPNoise4( v, t, pv, pt ) );
}


} // namespace Aqsis
//---------------------------------------------------------------------
