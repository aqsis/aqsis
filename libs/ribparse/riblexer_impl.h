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

/** \file
 * \brief RIB lexer implementation header
 * \author Chris Foster [chris42f (at) g mail (d0t) com]
 */

#ifndef AQSIS_RIBLEXER_IMPL_INCLUDED 
#define AQSIS_RIBLEXER_IMPL_INCLUDED 

#include "riblexer.h"

#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>

#include "ribtokenizer.h"

namespace Aqsis {

class RibToken;

//------------------------------------------------------------------------------
/// Implementation of the RibLexer interface
///
/// See RibLexer for docs on interface functions.
class RibLexerImpl : public RibLexer
{
    public:
        RibLexerImpl();

        /// public RibLexer interface
        virtual void discardUntilRequest();
        virtual std::string streamPos();

        virtual void pushInput(std::istream& inStream,
                               const std::string& streamName,
                               const CommentCallback& callback = CommentCallback());
        virtual void popInput();

        virtual const char* nextRequest();
        virtual int getInt();
        virtual float getFloat();
        virtual const char* getString();
        virtual IntArray getIntArray();
        virtual FloatArray getFloatArray(int length = -1);
        virtual StringArray getStringArray();

        virtual TokenType peekNextType();

        virtual IntArray getIntParam();
        virtual FloatArray getFloatParam();
        virtual StringArray getStringParam();

    private:
        /// \brief A pool of buffers into which RIB arrays will be read.
        ///
        /// The pool serves two purposes:
        ///   * Maintains ownership of the allocated buffers.
        ///   * Attempts to avoid a lot of dynamic allocation.
        ///
        /// A potential problem with the current implementation is that the memory for
        /// individual buffers isn't shared, so the size of the buffers might become
        /// quite large and unbalaced over time.
        ///
        template<typename BufferT>
        class BufferPool
        {
            private:
                /// List of buffers
                boost::ptr_vector<BufferT > m_buffers;
                /// Index of next available buffer.
                size_t m_next;
            public:
                BufferPool() : m_buffers(), m_next(0) { }
                /// Get the next available buffer from the pool.
                BufferT& getBuf()
                {
                    if(m_next >= m_buffers.size())
                        m_buffers.push_back(new BufferT());
                    BufferT& buf = m_buffers[m_next];
                    ++m_next;
                    buf.clear();
                    return buf;
                }
                /// Mark all buffers in the pool as unused.
                void markUnused() { m_next = 0; }
        };

        /// Holder for multiple C strings, all in the same big array.
        class MultiStringBuffer
        {
            private:
                /// Storage for all strings
                std::vector<char> m_storage;
                /// Offsets to starts of strings in m_storage.
                std::vector<size_t> m_offsets;
                /// References to start of strings.
                std::vector<const char*> m_stringBegin;
            public:
                MultiStringBuffer()
                    : m_storage(), m_offsets(), m_stringBegin() {}

                /// Add str to the end of the string vector
                void push_back(const std::string& str)
                {
                    m_offsets.push_back(m_storage.size());
                    m_storage.insert(m_storage.end(), str.begin(), str.end());
                    m_storage.push_back(0); // terminating null char.
                }

                /// Clear all strings
                void clear()
                {
                    m_storage.clear();
                    m_offsets.clear();
                }

                /// Convert to an Ri::StringArray.
                Ri::StringArray toRiArray()
                {
                    if(m_offsets.empty())
                        return Ri::StringArray();
                    // iterate through offsets, getting a pointer for each
                    // contained string.  We MUST build this once we know
                    // we're done with adding things to m_storage, or the
                    // pointers will become invalid when m_storage is resized.
                    const char* first = &m_storage[0];
                    m_stringBegin.clear();
                    m_stringBegin.reserve(m_offsets.size());
                    for(int i = 0, iend = m_offsets.size(); i < iend; ++i)
                        m_stringBegin.push_back(first + m_offsets[i]);
                    return Ri::StringArray(&m_stringBegin[0],
                                           m_stringBegin.size());
                }
        };

        void tokenError(const char* expected, const RibToken& badTok);

        // Tokenizer object (a lower level lexer of sorts)
        RibTokenizer m_tokenizer;

        // pools of parsed arrays for the current request.
        BufferPool<std::string> m_stringPool;
        BufferPool<std::vector<float> > m_floatArrayPool;
        BufferPool<std::vector<int> >   m_intArrayPool;
        BufferPool<MultiStringBuffer>   m_stringArrayPool;
};


} // namespace Aqsis

#endif // AQSIS_RIBLEXER_IMPL_INCLUDED 

// vi: set et:
