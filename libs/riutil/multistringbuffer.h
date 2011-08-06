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

#ifndef AQSIS_MULTISTRINGBUFFER_INCLUDED
#define AQSIS_MULTISTRINGBUFFER_INCLUDED

#include <cstring>
#include <string>
#include <vector>

namespace Aqsis {

/// Holder for multiple C strings, all in the same big array.
///
/// This is potentially useful as a limited alternative to
/// std::vector<std::string> because:
///   - It's kinder to the allocator than using std::vector<std::string> which
///     does a lot of small allocations, and for which clear() will completely
///     deallocate all the std::string memory.
///   - You can't get an array of C strings out of std::vector<std::string>
///     directly.
class MultiStringBuffer
{
    private:
        /// Storage for all strings
        std::vector<char> m_storage;
        /// Offsets to starts of strings in m_storage.
        std::vector<size_t> m_offsets;
        /// References to start of strings.
        mutable std::vector<const char*> m_cStrings;
    public:
        MultiStringBuffer()
            : m_storage(), m_offsets(), m_cStrings() {}

        /// Add iterator range [begin,end) to the end of the string vector
        template<typename IterT>
        void push_back(IterT begin, IterT end)
        {
            m_offsets.push_back(m_storage.size());
            m_storage.insert(m_storage.end(), begin, end);
            m_storage.push_back(0); // terminating null char.
        }
        /// Add str to the end of the string vector
        void push_back(const std::string& str)
        {
            push_back(str.begin(), str.end());
        }
        void push_back(const char* str)
        {
            push_back(str, str+std::strlen(str));
        }

        /// Clear all strings
        void clear()
        {
            m_storage.clear();
            m_offsets.clear();
        }

        /// Convert to an vector of C-strings
        const std::vector<const char*>& toCstringVec() const
        {
            // iterate through offsets, getting a pointer for each
            // contained string.  We MUST build this once we know
            // we're done with adding things to m_storage, or the
            // pointers will become invalid when m_storage is resized.
            m_cStrings.resize(m_offsets.size());
            if(m_offsets.empty())
                return m_cStrings;
            const char* first = &m_storage[0];
            for(int i = 0, iend = m_offsets.size(); i < iend; ++i)
                m_cStrings[i] = first + m_offsets[i];
            return m_cStrings;
        }
};

}

#endif // AQSIS_MULTISTRINGBUFFER_INCLUDED
// vi: set et:
