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
 * \brief RIB lexer implementation
 * \author Chris Foster [chris42f (at) g mail (d0t) com]
 */

#include <aqsis/util/exception.h>

#include <sstream>

#include "riblexer_impl.h"

namespace Aqsis {

namespace {

template<typename T>
Ri::Array<T> toRiArray(const std::vector<T>& v)
{
    if(v.empty())
        return Ri::Array<T>();
    return Ri::Array<T>(&v[0], v.size());
}

Ri::StringArray toRiArray(MultiStringBuffer& buf)
{
    return toRiArray(buf.toCstringVec());
}

}


// RibLexerImpl implementation
RibLexerImpl::RibLexerImpl()
    : m_tokenizer(),
    m_stringPool(),
    m_floatArrayPool(),
    m_intArrayPool(),
    m_stringArrayPool()
{ }

void RibLexerImpl::discardUntilRequest()
{
    RibToken::Type nextType = m_tokenizer.peek().type();
    while(nextType != RibToken::REQUEST && nextType != RibToken::ENDOFFILE)
    {
        m_tokenizer.get();
        nextType = m_tokenizer.peek().type();
    }
}

std::string RibLexerImpl::streamPos()
{
    return m_tokenizer.streamPos();
}

void RibLexerImpl::pushInput(std::istream& inStream, const std::string& streamName,
                             const CommentCallback& callback)
{
    m_tokenizer.pushInput(inStream, streamName, callback);
}

void RibLexerImpl::popInput()
{
    m_tokenizer.popInput();
}

const char* RibLexerImpl::nextRequest()
{
    // Mark array pools as free to use.
    m_floatArrayPool.markUnused();
    m_intArrayPool.markUnused();
    m_stringArrayPool.markUnused();
    m_stringPool.markUnused();
    // Get the next token, and make sure it's a request token.
    const RibToken& tok = m_tokenizer.get();
    if(tok.type() == RibToken::ENDOFFILE)
        return 0;
    else if(tok.type() != RibToken::REQUEST)
        tokenError("request", tok);
    std::string& storage = m_stringPool.getBuf();
    // Using assign() rather than operator=() subverts the libstdc++ ref
    // counting for std::string, which is best avoided here since it seems to
    // result in extra churn on the heap.
    //
    // TODO: Replace the use of std::string with a minimal string buffer type
    // with explicit control over memory allocation.
    storage.assign(tok.stringVal().begin(), tok.stringVal().end());
    return storage.c_str();
}

int RibLexerImpl::getInt()
{
    const RibToken& tok = m_tokenizer.get();
    if(tok.type() != RibToken::INTEGER)
        tokenError("integer", tok);
    return tok.intVal();
}

float RibLexerImpl::getFloat()
{
    const RibToken& tok = m_tokenizer.get();
    switch(tok.type())
    {
        case RibToken::INTEGER:
            return tok.intVal();
        case RibToken::FLOAT:
            return tok.floatVal();
        default:
            tokenError("float", tok);
            return 0;
    }
}

const char* RibLexerImpl::getString()
{
    const RibToken& tok = m_tokenizer.get();
    if(tok.type() != RibToken::STRING)
        tokenError("string", tok);
    std::string& storage = m_stringPool.getBuf();
    storage.assign(tok.stringVal().begin(), tok.stringVal().end());
    return storage.c_str();
}

/* Functions for reading arrays.
 *
 * The code duplication between the get*Array() functions here is somewhat
 * alarming, but it's tricky to do something about it without the code becoming
 * an opaque mess of templates.
 *
 * At the very least, we'd need to make the tok.intVal(), tok.floatVal(),
 * tok.stringVal() into a template or overloaded functions taking a reference
 * so that the correct version could be called in a templated readArray().
 * After that we'd still be left with the float case in which it's acceptable
 * for the token to be *either* an integer *or* a float.
 */
RibLexer::IntArray RibLexerImpl::getIntArray()
{
    const RibToken& tok = m_tokenizer.get();
    if(tok.type() != RibToken::ARRAY_BEGIN)
        tokenError("integer array", tok);

    std::vector<int>& buf = m_intArrayPool.getBuf();
    bool parsing = true;
    while(parsing)
    {
        const RibToken& tok = m_tokenizer.get();
        switch(tok.type())
        {
            case RibToken::INTEGER:
                buf.push_back(tok.intVal());
                break;
            case RibToken::ARRAY_END:
                parsing = false;
                break;
            default:
                tokenError("integer array element", tok);
                break;
        }
    }
    return toRiArray(buf);
}

RibLexer::FloatArray RibLexerImpl::getFloatArray(int length)
{
    std::vector<float>& buf = m_floatArrayPool.getBuf();
    if(m_tokenizer.peek().type() == RibToken::ARRAY_BEGIN)
    {
        // Read an array in [ num1 num2 ... num_n ] format

        m_tokenizer.get(); // consume '['
        bool parsing = true;
        while(parsing)
        {
            const RibToken& tok = m_tokenizer.get();
            switch(tok.type())
            {
                case RibToken::INTEGER:
                    buf.push_back(tok.intVal());
                    break;
                case RibToken::FLOAT:
                    buf.push_back(tok.floatVal());
                    break;
                case RibToken::ARRAY_END:
                    parsing = false;
                    break;
                default:
                    tokenError("float array element", tok);
                    break;
            }
        }

        if(length >= 0 && static_cast<int>(buf.size()) != length)
        {
            AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
                "expected " << length << " float array componenets, got "
                << buf.size());
        }
    }
    else if(length >= 0)
    {
        // Read an array in  num1 num2 ... num_n  format (ie, without the usual
        // array delimiters).
        for(int i = 0; i < length; ++i)
            buf.push_back(getFloat());
    }
    else
    {
        tokenError("float array", m_tokenizer.get());
    }
    return toRiArray(buf);
}

RibLexer::StringArray RibLexerImpl::getStringArray()
{
    const RibToken& tok = m_tokenizer.get();
    if(tok.type() != RibToken::ARRAY_BEGIN)
        tokenError("string array", tok);

    MultiStringBuffer& buf = m_stringArrayPool.getBuf();
    bool parsing = true;
    while(parsing)
    {
        const RibToken& tok = m_tokenizer.get();
        switch(tok.type())
        {
            case RibToken::STRING:
                buf.push_back(tok.stringVal());
                break;
            case RibToken::ARRAY_END:
                parsing = false;
                break;
            default:
                tokenError("string array element", tok);
                break;
        }
    }
    return toRiArray(buf);
}

RibLexer::TokenType RibLexerImpl::peekNextType()
{
    switch(m_tokenizer.peek().type())
    {
        case RibToken::ARRAY_BEGIN:
            return Tok_Array;
        case RibToken::STRING:
            return Tok_String;
        case RibToken::INTEGER:
            return Tok_Int;
        case RibToken::FLOAT:
            return Tok_Float;
        default:
            return Tok_RequestEnd;
    }
}

RibLexer::IntArray RibLexerImpl::getIntParam()
{
    if(m_tokenizer.peek().type() == RibToken::INTEGER)
    {
        std::vector<int>& buf = m_intArrayPool.getBuf();
        buf.push_back(m_tokenizer.get().intVal());
        return toRiArray(buf);
    }
    return getIntArray();
}

RibLexer::FloatArray RibLexerImpl::getFloatParam()
{
    switch(m_tokenizer.peek().type())
    {
        case RibToken::INTEGER:
            {
                std::vector<float>& buf = m_floatArrayPool.getBuf();
                buf.push_back(m_tokenizer.get().intVal());
                return toRiArray(buf);
            }
        case RibToken::FLOAT:
            {
                std::vector<float>& buf = m_floatArrayPool.getBuf();
                buf.push_back(m_tokenizer.get().floatVal());
                return toRiArray(buf);
            }
        default:
            return getFloatArray();
    }
}

RibLexer::StringArray RibLexerImpl::getStringParam()
{
    if(m_tokenizer.peek().type() == RibToken::STRING)
    {
        // special case where next token is a single string.
        MultiStringBuffer& buf = m_stringArrayPool.getBuf();
        buf.push_back(m_tokenizer.get().stringVal());
        return toRiArray(buf);
    }
    return getStringArray();
}


/** \brief Throw an error from encountering an unexpected token.
 *
 * An error string is generated with the form:
 *
 * expected <expected> before <token_description>
 *
 * where <expected> is a string describing the expected token and
 * <token_description> is generated from the bad token provided.
 *
 * \param expected - string describing the expected token
 * \param badTok - the problematic token which was actually obtained.
 */
void RibLexerImpl::tokenError(const char* expected, const RibToken& badTok)
{
    std::ostringstream msg;

    msg << "expected " << expected << " before ";
    switch(badTok.type())
    {
        case RibToken::ARRAY_BEGIN:
            msg << "'['";
            break;
        case RibToken::ARRAY_END:
            msg << "']'";
            break;
        case RibToken::ENDOFFILE:
            msg << "end of file";
            // Put ENDOFFILE back into the input, since not doing so may cause
            // problems for streams which only provide a single ENDOFFILE
            // token before blocking (eg, ProcRunProgram pipes).
            m_tokenizer.unget();
            break;
        case RibToken::INTEGER:
            msg << "integer [= " << badTok.intVal() << "]";
            break;
        case RibToken::FLOAT:
            msg << "float [= " << badTok.floatVal() << "]";
            break;
        case RibToken::STRING:
            msg << "string [= \"" << badTok.stringVal() << "\"]";
            break;
        case RibToken::REQUEST:
            msg << "request [= " << badTok.stringVal() << "]";
            // For unexpected REQUEST tokens we back up by one token so that
            // the next call to nextRequest() can start afresh with the
            // new request.
            m_tokenizer.unget();
            break;
        case RibToken::ERROR:
            msg << "bad token [" << badTok.stringVal() << "]";
            break;
    }

    AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax, msg.str());
}


//------------------------------------------------------------------------------
// RibLexer create/destroy functions.
RibLexer* RibLexer::create()
{
    return new RibLexerImpl();
}

void RibLexer::destroy(RibLexer* lexer)
{
    delete lexer;
}

} // namespace Aqsis

// vi: set et:
