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
		\brief Define system specific options.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is specific.h included already?
#ifndef SPECIFIC_H_INCLUDED
//{
#define SPECIFIC_H_INCLUDED 1

///---------------------------------------------------------------------
/// record which compiler we're using in the object files

//#pragma comment( compiler )
//#pragma warning( disable : 4786 )
//#pragma warning( disable : 4305 )
//#pragma warning( disable : 4244 )

///----------------------------------------------------------------------
///
/// Namespace macros for those cases where they aren't supported
/// by the host compiler. These can also be disabled by setting
/// /D NO_NAMESPACES in the compiler options.

#ifdef  NO_NAMESPACES
#define START_NAMESPACE(x)	/* start disabled namespace x */
#define END_NAMESPACE(x)	/* end disabled namespace x */
#define USING_NAMESPACE(x)  /* using disabled namespace x */
#else
#define START_NAMESPACE(x)	namespace x {
#define END_NAMESPACE(x)	}
#define USING_NAMESPACE(x)  using namespace x;
#endif


///---------------------------------------------------------------------
/// Processor specific optimisations


///---------------------------------------------------------------------
///
/// g_quick_sqrt
/// Faster version of sqrt. This has no error handling, so be damned sure
/// that f is >=0 before calling this function!

inline float g_quick_sqrt(float f)
{
	return(sqrt(f));
}


///---------------------------------------------------------------------
///
/// g_quick_pow
/// Power function approximation for use only with specular functions.

inline float g_quick_pow(float x, float n)
{
	float m=(n+1)*0.25f;
	float y=(m*x +1 -m);
	return ((y<=0) ? 0 : (x*y*y));
}

#endif // SPECIFIC_H_INCLUDED
