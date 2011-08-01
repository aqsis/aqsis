// Aqsis
// Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the software's owners nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// (This is the New BSD license)

/// \file Memory profiling tools
/// \author Chris Foster  chris42f (at) gmail (dot) com

#include "memdebug.h"


#ifdef AQSIS_MEMORY_DEBUGGING
#   include <malloc.h> //< Won't work on windows, I guess?
#endif


namespace Aqsis {

#ifdef AQSIS_MEMORY_DEBUGGING

MemoryLog::MemoryLog(const char* fileName)
    : m_outFile(fileName)
{ }

void MemoryLog::log()
{
    struct mallinfo m = mallinfo();
    m_outFile
        << m.arena    << " "
        << m.ordblks  << " "
        << m.hblks    << " "
        << m.hblkhd   << " "
        << m.uordblks << " "
        << m.fordblks << " "
        << m.keepcost << "\n";
}

#else

// Dummy implementations when AQSIS_MEMORY_DEBUGGING isn't defined.
MemoryLog::MemoryLog(const char* fileName) {}
void MemoryLog::log() {}

#endif

} // namespace Aqsis
