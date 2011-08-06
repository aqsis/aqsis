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
 * \brief Declares typedefs for the basic types.
 * \author Paul C. Gregory (pgregory@aqsis.org)
 * \author Chris Foster (chris42f (at) gmail (d0t) com)
 *
 * ===================================================================
 * C-compatible header. C++ constructs must be preprocessor-protected.
 * ===================================================================
 */


#ifndef AQSIS_TYPES_INCLUDED
#define AQSIS_TYPES_INCLUDED

#include <aqsis/config.h>

/** \todo <b>Code review</b> Consider whether we can avoid polluting the global namespace with these typedefs.  C compatibility needs to be assured, so we may have to duplicate the header...
 */
/*--------------------------------------------------------------------------------*/
typedef char TqChar;
typedef unsigned char TqUchar;

/** \todo <b>Code review</b> Deprecate these pointer types?  They're inconsistently used and of dubious usefulness anyway.
 */
typedef char* TqPchar;
typedef unsigned char* TqPuchar;

typedef int TqInt;
typedef unsigned int TqUint;
typedef long TqLong;
typedef unsigned long TqUlong;

typedef short TqShort;
typedef unsigned short TqUshort;

typedef float TqFloat;
typedef double TqDouble;


/*--------------------------------------------------------------------------------
 / Typedefs for integer types with specific sizes.
 /
 / This approach - based on a combination of stdint.h and limit macros from
 / limits.h - is taken from boost/cstdint.hpp.  Unfortunately we have to use
 / our own version here for C compatibility.
*/

#ifdef AQSIS_HAVE_STDINT_H

	/* Use types from the C99 stdint.h header. */
#	include <stdint.h>

	typedef int8_t  TqInt8;
	typedef uint8_t TqUint8;

	typedef int16_t  TqInt16;
	typedef uint16_t TqUint16;

	typedef int32_t  TqInt32;
	typedef uint32_t TqUint32;

#else 

	/* If the stdint.h header is not present, fall back on using the limits
	  macros from limits.h to guess the correct types.
	*/
#	include <limits.h>

#	if UCHAR_MAX == 0xff
		typedef signed char   TqInt8;
		typedef unsigned char TqUint8;
#	else
#		error 8 bit integers not autodetected - please modify aqsis_types.h \
			or contact the aqsis team.
#	endif

#	if USHRT_MAX == 0xffff
		typedef short          TqInt16;
		typedef unsigned short TqUint16;
#	else
#		error 16 bit integers not autodetected - please modify aqsis_types.h \
			or contact the aqsis team.
#	endif

#	if ULONG_MAX == 0xffffffff
		typedef long          TqInt32;
		typedef unsigned long TqUint32;
#	elif UINT_MAX == 0xffffffff
		typedef int           TqInt32;
		typedef unsigned int  TqUint32;
#	else
#		error 32 bit integers not autodetected - please modify aqsis_types.h \
			or contact the aqsis team.
#	endif

#endif 


#endif 
