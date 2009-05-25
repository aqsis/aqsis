// CqNoise1234
// Copyright (C) 2003-2005, Stefan Gustavson
//
// Contact: stegu@itn.liu.se
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
		\brief Declares the CqNoise1234 class for producing Perlin noise.
		\author Stefan Gustavson (stegu@itn.liu.se)
*/

/*
 * This is a clean, fast, modern and free Perlin noise class in C++.
 * Being a stand-alone class with no external dependencies, it is
 * highly reusable without source code modifications.
 *
 * Note:
 * Replacing the "float" type with "double" can actually make this run faster
 * on some platforms. A templatized version of CqNoise1234 could be useful.
 */

//? Is .h included already?
#ifndef NOISE1234_H_INCLUDED
#define NOISE1234_H_INCLUDED 1

#include	<aqsis/aqsis.h>

namespace Aqsis {

class AQSIS_MATH_SHARE CqNoise1234
{

	public:
		CqNoise1234()
		{}
		~CqNoise1234()
		{}

		/** 1D, 2D, 3D and 4D float Perlin noise, SL "noise()"
		 */
		static TqFloat noise( TqFloat x );
		static TqFloat noise( TqFloat x, TqFloat y );
		static TqFloat noise( TqFloat x, TqFloat y, TqFloat z );
		static TqFloat noise( TqFloat x, TqFloat y, TqFloat z, TqFloat w );

		/** 1D, 2D, 3D and 4D float Perlin periodic noise, SL "pnoise()"
		 */
		static TqFloat pnoise( TqFloat x, TqInt px );
		static TqFloat pnoise( TqFloat x, TqFloat y, TqInt px, TqInt py );
		static TqFloat pnoise( TqFloat x, TqFloat y, TqFloat z, TqInt px, TqInt py, TqInt pz );
		static TqFloat pnoise( TqFloat x, TqFloat y, TqFloat z, TqFloat w,
		                                    TqInt px, TqInt py, TqInt pz, TqInt pw );

	private:
		static unsigned char perm[];
		static TqFloat  grad( TqInt hash, TqFloat x );
		static TqFloat  grad( TqInt hash, TqFloat x, TqFloat y );
		static TqFloat  grad( TqInt hash, TqFloat x, TqFloat y , TqFloat z );
		static TqFloat  grad( TqInt hash, TqFloat x, TqFloat y, TqFloat z, TqFloat t );

};

//-----------------------------------------------------------------------

} // namespace Aqsis

#endif // NOISE1234_H_INCLUDED
