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
		\brief Compiler specific options and settings.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef AQSIS_COMPILER_H_INCLUDED
#define AQSIS_COMPILER_H_INCLUDED 1


/** Define the system being compiled on.
 */
#define	AQSIS_SYSTEM_WIN32	1

/** Make sure that including windows.h doesn't define the min and max macros,
 * which conflict with other uses of min and max (Aqsis::min, std::min etc.)
 */
#define NOMINMAX

/** Define the compiler.
 */
#ifdef __GNUC__
#define AQSIS_COMPILER_GCC		1
#else
#if _MSC_VER < 1300
#define	AQSIS_COMPILER_MSVC6	1
#else
#define AQSIS_COMPILER_MSVC7	1
#endif
#endif

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


#if defined(AQSIS_COMPILER_MSVC6) || defined(AQSIS_COMPILER_MSVC7)
#pragma comment( compiler )
#pragma warning( disable : 4786 )
#pragma warning( disable : 4305 )
#pragma warning( disable : 4244 )

#include <sys/stat.h>

#define S_ISDIR(a) (a & S_IFDIR)
#endif

#define SHARED_LIBRARY_SUFFIX ".dll"

#ifdef	AQSIS_STATIC_LINK

#  define  COMMON_SHARE

#else // !AQSIS_STATIC_LINK

#    ifdef COMMON_EXPORTS
#      define COMMON_SHARE __declspec(dllexport)
#    else
#      define COMMON_SHARE __declspec(dllimport)
#    endif

#endif	// AQSIS_STATIC_LINK

#endif // AQSIS_COMPILER_H_INCLUDED
