/*
Aqsis
Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.
* Neither the name of the software's owners nor the names of its
  contributors may be used to endorse or promote products derived from this
  software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

(This is the New BSD license)
*/

/** \file
 * \brief Compiler specific options and settings.
 */

#ifndef AQSIS_CONFIG_H_INCLUDED
#define AQSIS_CONFIG_H_INCLUDED

/*----------------------------------------------------------------------------*/
/* Config from cmake system introspection */

#cmakedefine AQSIS_HAVE_STDINT_H

/* Define the system being compiled on. */
#define AQSIS_SYSTEM_${AQSIS_SYSTEM_TYPE} 1


/*----------------------------------------------------------------------------*/
/* system setup for windows */
#ifdef AQSIS_SYSTEM_WIN32

/* Make sure that including windows.h doesn't define the min and max macros,
 * which conflict with other uses of min and max (Aqsis::min, std::min etc.) */
#ifndef	NOMINMAX
#define NOMINMAX
#endif

/* Make sure that the math constants from math.h are defined - that is, M_PI
 * etc.
 */
#ifndef _USE_MATH_DEFINES
#	define _USE_MATH_DEFINES
#endif

/* Make sure we don't try to use the syslog stuff on windows */
#define AQSIS_NO_SYSLOG

/* Define the compiler */
#ifdef __GNUC__
#define AQSIS_COMPILER_GCC		1
#else
#if _MSC_VER < 1300
#define	AQSIS_COMPILER_MSVC6	1
#else
#define AQSIS_COMPILER_MSVC7	1
#endif
#endif

/* Faster windows compilation, and less bloat */
#define WIN32_LEAN_AND_MEAN

#if defined(AQSIS_COMPILER_MSVC6) || defined(AQSIS_COMPILER_MSVC7)
	/* Disable some warnings on MSVC */
#	pragma comment( compiler )
#	pragma warning( disable : 4786 )
#	pragma warning( disable : 4305 )
#	pragma warning( disable : 4244 )
#	pragma warning( disable : 4251 )
#	pragma warning( disable : 4996 )
#	pragma warning( disable : 4290 )
	/* Disable warnings about unsafe arguments to STL iterators */
#	define _SCL_SECURE_NO_WARNINGS
#endif

#define SHARED_LIBRARY_SUFFIX ".dll"

/* Macros for DLL import/export
 *
 * Only defined when we're using dynamic linking (the default).
 *
 * These are setup so that the build will export the necessary symbols whenever
 * it's compiling files for a DLL, and import those symbols when it's merely
 * using them from a separate DLL.  To enable export during the build, the
 * build script should define the appropriate *_EXPORTS macro, for example,
 * AQSIS_MATH_EXPORTS.
 */
#ifdef AQSIS_STATIC_LINK
#	define AQSIS_CORE_SHARE
#	define AQSIS_MATH_SHARE
#	define AQSIS_RIUTIL_SHARE
#	define AQSIS_RI_SHARE
#	define AQSIS_SHADERVM_SHARE
#	define AQSIS_SLCOMP_SHARE
#	define AQSIS_SLXARGS_SHARE
#	define AQSIS_TEX_SHARE
#	define AQSIS_UTIL_SHARE
#else
#	ifdef AQSIS_CORE_EXPORTS
#		define AQSIS_CORE_SHARE __declspec(dllexport)
#	else
#		define AQSIS_CORE_SHARE __declspec(dllimport)
#	endif
#	ifdef AQSIS_MATH_EXPORTS
#		define AQSIS_MATH_SHARE __declspec(dllexport)
#	else
#		define AQSIS_MATH_SHARE __declspec(dllimport)
#	endif
#	ifdef AQSIS_RIUTIL_EXPORTS
#		define AQSIS_RIUTIL_SHARE __declspec(dllexport)
#	else
#		define AQSIS_RIUTIL_SHARE __declspec(dllimport)
#	endif
#	ifdef AQSIS_RI_EXPORTS
#		define AQSIS_RI_SHARE __declspec(dllexport)
#	else
#		define AQSIS_RI_SHARE __declspec(dllimport)
#	endif
#	ifdef AQSIS_SHADERVM_EXPORTS
#		define AQSIS_SHADERVM_SHARE __declspec(dllexport)
#	else
#		define AQSIS_SHADERVM_SHARE __declspec(dllimport)
#	endif
#	ifdef AQSIS_SLCOMP_EXPORTS
#		define AQSIS_SLCOMP_SHARE __declspec(dllexport)
#	else
#		define AQSIS_SLCOMP_SHARE __declspec(dllimport)
#	endif
#	ifdef AQSIS_SLXARGS_EXPORTS
#		define AQSIS_SLXARGS_SHARE __declspec(dllexport)
#	else
#		define AQSIS_SLXARGS_SHARE __declspec(dllimport)
#	endif
#	ifdef AQSIS_TEX_EXPORTS
#		define AQSIS_TEX_SHARE __declspec(dllexport)
#	else
#		define AQSIS_TEX_SHARE __declspec(dllimport)
#	endif
#	ifdef AQSIS_UTIL_EXPORTS
#		define AQSIS_UTIL_SHARE __declspec(dllexport)
#	else
#		define AQSIS_UTIL_SHARE __declspec(dllimport)
#	endif
#endif


#define AQSIS_EXPORT __declspec(dllexport)

/*----------------------------------------------------------------------------*/
/* system setup for POSIX */
#else

/* If on a BeOS platform add this, as it is mainly Posix, but needs some
 * changes. */
#ifdef __BEOS__
#	define AQSIS_SYSTEM_BEOS 1
#	define SOMAXCONN 128
#endif

/* If compiling on Apple platform, set the system identifier
 * AQSIS_SYSTEM_MACOSX, MacOSX is basically Posix, but with some small
 * differences.
 */
#ifdef __APPLE__
#	define AQSIS_SYSTEM_MACOSX 1
#endif

/* Define the compiler. */
#define AQSIS_COMPILER_GCC 1

#define SHARED_LIBRARY_SUFFIX ".so"

/* Macros for DLL import/export on win32.  Unneeded on posix so they're
 * defined to be empty. */
#define AQSIS_CORE_SHARE
#define AQSIS_MATH_SHARE
#define AQSIS_RIUTIL_SHARE
#define AQSIS_RI_SHARE
#define AQSIS_SHADERVM_SHARE
#define AQSIS_SLCOMP_SHARE
#define AQSIS_SLXARGS_SHARE
#define AQSIS_TEX_SHARE
#define AQSIS_UTIL_SHARE

#define AQSIS_EXPORT


/*----------------------------------------------------------------------------*/
#endif

#endif /* AQSIS_CONFIG_H_INCLUDED */
