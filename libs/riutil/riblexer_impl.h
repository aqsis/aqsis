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

/** \file
 * \brief RIB lexer implementation header
 * \author Chris Foster [chris42f (at) g mail (d0t) com]
 */

#ifndef AQSIS_RIBLEXER_IMPL_INCLUDED 
#define AQSIS_RIBLEXER_IMPL_INCLUDED 

#include "riblexer.h"

#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>

#include "multistringbuffer.h"
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
