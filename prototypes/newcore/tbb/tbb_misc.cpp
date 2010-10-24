/*
    Copyright 2005-2010 Intel Corporation.  All Rights Reserved.

    This file is part of Threading Building Blocks.

    Threading Building Blocks is free software; you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    version 2 as published by the Free Software Foundation.

    Threading Building Blocks is distributed in the hope that it will be
    useful, but WITHOUT ANY WARRANTY; without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Threading Building Blocks; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    As a special exception, you may use this file as part of a free software
    library without restriction.  Specifically, if other files instantiate
    templates or use macros or inline functions from this file, or you compile
    this file and link it with other files to produce an executable, this
    file does not by itself cause the resulting executable to be covered by
    the GNU General Public License.  This exception does not however
    invalidate any other reasons why the executable file might be covered by
    the GNU General Public License.
*/

// Source file for miscellaneous entities that are infrequently referenced by
// an executing program.

#include "tbb/tbb_stddef.h"
//#include "tbb_assert_impl.h" // Out-of-line TBB assertion handling routines are instantiated here.
//#include "tbb/tbb_exception.h"
#include "tbb/tbb_machine.h"
//#include "tbb_misc.h"
#include <cstdio>
#include <cstdlib>

#if !TBB_USE_EXCEPTIONS && _MSC_VER
    // Suppress "C++ exception handler used, but unwind semantics are not enabled" warning in STL headers
    #pragma warning (push)
    #pragma warning (disable: 4530)
#endif

#include <cstring>

#if !TBB_USE_EXCEPTIONS && _MSC_VER
    #pragma warning (pop)
#endif

using namespace std;

#if !__TBB_RML_STATIC
#if __TBB_x86_32

#include "tbb/atomic.h"

// in MSVC environment, int64_t defined in tbb::internal namespace only (see tbb_stddef.h)
#if _MSC_VER
using tbb::internal::int64_t;
#endif

//! Handle 8-byte store that crosses a cache line.
extern "C" void __TBB_machine_store8_slow( volatile void *ptr, int64_t value ) {
    for( tbb::internal::atomic_backoff b;; b.pause() ) {
        int64_t tmp = *(int64_t*)ptr;
        if( __TBB_CompareAndSwap8(ptr,value,tmp)==tmp )
            break;
    }
}

#endif /* __TBB_x86_32 */
#endif /* !__TBB_RML_STATIC */

