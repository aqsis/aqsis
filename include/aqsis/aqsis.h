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
 * \brief Main Aqsis include for basic macros and types needed by all aqsis headers.
 *
 * \author Paul C. Gregory (pgregory@aqsis.org)
 *
 * This file should be included by every aqsis source file.  Moreover, it
 * should be included before any other headers, since aqsis_config.h defines
 * some constants which potentially modify the behaviour of other standard
 * includes.
 *
 * ===================================================================
 * C-compatible header. C++ constructs must be preprocessor-protected.
 * ===================================================================
 */

#ifndef AQSIS_H_INCLUDED
#define AQSIS_H_INCLUDED

#include <assert.h>

#include <aqsis/inttype.h>
#include <aqsis/config.h>

/* macro which stringizes the value held in another macro. */
#define AQSIS_XSTR(s) AQSIS_STR(s)
#define AQSIS_STR(s) #s

#endif 
