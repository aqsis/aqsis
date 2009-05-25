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
		\brief Declares the CqNoise class for producing Perlin noise.
		\author Paul C. Gregory (pgregory@aqsis.org)
		\author Stefan Gustavson (stegu@itn.liu.se)
*/

//? Is .h included already?
#ifndef NOISE_H_INCLUDED
#define NOISE_H_INCLUDED 1

#include	<aqsis/aqsis.h>
#include	<aqsis/math/vector3d.h>
#include	<aqsis/math/color.h>

namespace Aqsis {

//----------------------------------------------------------------------
/** \class CqNoise
 * Wrapper class for CqNoise1234, to produce float and vector Perlin noise
 * over 1D to 4D domains.
 */

class AQSIS_MATH_SHARE CqNoise
{
	public:

		CqNoise()
		{}

		~CqNoise()
		{}

		// These are functions with "SL-friendly" parameter lists and return types,
		// which each invoke one or three calls to one of the functions in CqNoise1234.

		static	TqFloat	FGNoise1( TqFloat x );
		static  TqFloat FGPNoise1( TqFloat x, TqFloat px );
		static	TqFloat	FGNoise2( TqFloat x, TqFloat y );
		static	TqFloat	FGPNoise2( TqFloat x, TqFloat y, TqFloat px, TqFloat py );
		static	TqFloat	FGNoise3( const CqVector3D& v );
		static	TqFloat	FGPNoise3( const CqVector3D& v, const CqVector3D& pv );
		static	TqFloat	FGNoise4( const CqVector3D& v, const TqFloat t );
		static	TqFloat	FGPNoise4( const CqVector3D& v, const TqFloat t, const CqVector3D& pv, const TqFloat pt );
		static	CqVector3D	PGNoise1( TqFloat x );
		static	CqVector3D	PGPNoise1( TqFloat x, TqFloat px );
		static	CqVector3D	PGNoise2( TqFloat x, TqFloat y );
		static	CqVector3D	PGPNoise2( TqFloat x, TqFloat y, TqFloat px, TqFloat py );
		static	CqVector3D	PGNoise3( const CqVector3D& v );
		static	CqVector3D	PGPNoise3( const CqVector3D& v, const CqVector3D& pv );
		static	CqVector3D	PGNoise4( const CqVector3D& v, TqFloat t );
		static	CqVector3D	PGPNoise4( const CqVector3D& v, TqFloat t, const CqVector3D& pv, TqFloat pt );
		static	CqColor	CGNoise1( TqFloat x );
		static	CqColor	CGPNoise1( TqFloat x, TqFloat px );
		static	CqColor	CGNoise2( TqFloat x, TqFloat y );
		static	CqColor	CGPNoise2( TqFloat x, TqFloat y, TqFloat px, TqFloat py );
		static	CqColor	CGNoise3( const CqVector3D& v );
		static	CqColor	CGPNoise3( const CqVector3D& v, const CqVector3D& pv );
		static	CqColor	CGNoise4( const CqVector3D& v, TqFloat t );
		static	CqColor	CGPNoise4( const CqVector3D& v, TqFloat t, const CqVector3D& pv, TqFloat pt );

};

//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	// !NOISE_H_INCLUDED
