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
