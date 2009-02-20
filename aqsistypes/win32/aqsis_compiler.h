/* Aqsis
 / Copyright (C) 1997 - 2001, Paul C. Gregory
 /
 / Contact: pgregory@aqsis.org
 /
 / This library is free software; you can redistribute it and/or
 / modify it under the terms of the GNU General Public
 / License as published by the Free Software Foundation; either
 / version 2 of the License, or (at your option) any later version.
 /
 / This library is distributed in the hope that it will be useful,
 / but WITHOUT ANY WARRANTY; without even the implied warranty of
 / MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 / General Public License for more details.
 /
 / You should have received a copy of the GNU General Public
 / License along with this library; if not, write to the Free Software
 / Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


/** \file
		\brief Compiler specific options and settings.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

/**? Is .h included already? */
#ifndef AQSIS_COMPILER_H_INCLUDED
#define AQSIS_COMPILER_H_INCLUDED 1


/** Define the system being compiled on.
 */
#define	AQSIS_SYSTEM_WIN32	1

/** Make sure that including windows.h doesn't define the min and max macros,
 * which conflict with other uses of min and max (Aqsis::min, std::min etc.)
 */
#ifndef	NOMINMAX
#define NOMINMAX
#endif

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

/** Faster windows compilation, and less bloat
 */
#define WIN32_LEAN_AND_MEAN

/*----------------------------------------------------------------------*/

#if defined(AQSIS_COMPILER_MSVC6) || defined(AQSIS_COMPILER_MSVC7)
#pragma comment( compiler )
#pragma warning( disable : 4786 )
#pragma warning( disable : 4305 )
#pragma warning( disable : 4244 )
#pragma warning( disable : 4251 )

/* Disable the warnings about unsafe arguments to the 
   STL iterators.
*/
#define	_SCL_SECURE_NO_WARNINGS

/* Disable warnings about the alternative safe versions
   of the C runtime functions.
*/
#define _CRT_SECURE_NO_WARNINGS

#include <sys/stat.h>

#define S_ISDIR(a) (a & S_IFDIR)
#endif

#define SHARED_LIBRARY_SUFFIX ".dll"

/** Macros for DLL import/export
 *
 * Only defined when we're using dynamic linking (the default).
 *
 * These are setup so that the build will export the necessary symbols whenever
 * it's compiling files for a DLL, and import those symbols when it's merely
 * using them from a separate DLL.  To enable export during the build, the
 * build script should define the appropriate *_EXPORTS macro, for example,
 * COMMON_EXPORTS.
 */

#ifdef	AQSIS_STATIC_LINK

#	define COMMON_SHARE
#	define AQSISTEX_SHARE
#	define RI_SHARE
#	define SLXARGS_SHARE

#else 

#    ifdef COMMON_EXPORTS
#      define COMMON_SHARE __declspec(dllexport)
#    else
#      define COMMON_SHARE __declspec(dllimport)
#    endif

#    ifdef AQSISTEX_EXPORTS
#      define AQSISTEX_SHARE __declspec(dllexport)
#    else
#      define AQSISTEX_SHARE __declspec(dllimport)
#    endif

#	ifdef RI_EXPORTS
#		define RI_SHARE __declspec(dllexport)
#	else
#		define RI_SHARE __declspec(dllimport)
#	endif

#	ifdef SLXARGS_EXPORTS
#		define SLXARGS_SHARE __declspec(dllexport)
#	else
#		define SLXARGS_SHARE __declspec(dllimport)
#	endif

#endif	

/** Export from a DLL.
 */
#define AQSIS_EXPORT __declspec(dllexport)

#endif 
