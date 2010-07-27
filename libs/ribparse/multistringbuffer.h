// Aqsis
// Copyright (C) 1997 - 2007, Paul C. Gregory
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
        std::vector<const char*> m_cStrings;
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
        const std::vector<const char*>& toCstringVec()
        {
            // iterate through offsets, getting a pointer for each
            // contained string.  We MUST build this once we know
            // we're done with adding things to m_storage, or the
            // pointers will become invalid when m_storage is resized.
            const char* first = &m_storage[0];
            m_cStrings.resize(m_offsets.size());
            for(int i = 0, iend = m_offsets.size(); i < iend; ++i)
                m_cStrings[i] = first + m_offsets[i];
            return m_cStrings;
        }
};

}

#endif // AQSIS_MULTISTRINGBUFFER_INCLUDED
// vi: set et:
