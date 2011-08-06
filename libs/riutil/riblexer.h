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
 *
 * \brief RIB lexer interface
 * \author Chris Foster [chris42f (at) g mail (d0t) com]
 */

#ifndef AQSIS_RIBLEXER_H_INCLUDED
#define AQSIS_RIBLEXER_H_INCLUDED

#include <iostream>
#include <string>

#include <boost/function.hpp>

#include <aqsis/riutil/ricxx.h> // for array types.

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief A lexer for RIB-like file formats.
 *
 *
 * The renderman interface bytestream (RIB) is a sequence of renderman
 * interface call requests.  RIB can be broken into a stream of tokens
 * consisting of a few basic types:
 *   - interface call names ("requests")
 *   - strings
 *   - integers
 *   - floats
 *   - arrays of the above
 *
 * RibLexer does the hard work of lexing a RIB stream efficiently in either of
 * the ascii or binary formats.  It can also handle gzipped streams of the
 * same.  The lexer itself is concerned purely with collecting valid RIB
 * tokens from the input stream; it is completely ignorant of the semantics of
 * any requests or request parameter lists.  All semantic analysis is
 * offloaded to an appropriate RIB parser class.
 *
 * Since RIB is a simple format, it usually suffices to know the current
 * request name to correctly parse the incoming arguments, obtained using
 * nextRequest().  In the odd cases where this isn't quite enough, the
 * function peekNextType may be used.  Required arguments to a request should
 * be obtained using the functions getInt, getFloat, getString, getIntArray,
 * getFloatArray, and getStringArray.  When parsing parameter lists,
 * getIntParam, getFloatParam, and getStringParam should be used for
 * extracting parameter values.  (These allow the parameter value to come as
 * either a single value, or an array.)  The lifetime of any arrays or strings
 * returned by the get*() methods is managed by RibLexer, and is valid up until
 * the next call of nextRequest().
 *
 * Whenever an unexpected type is encountered in the underlying stream, an
 * XqParseError is thrown.  To recover from such an error, the user should
 * call discardUntilRequest() to discard tokens up until the next request.
 * The streamPos() function may be used to determine the stream position for
 * error reporting.
 *
 * The input stream connected to the lexer may be changed on the fly using the
 * pushInput() and popInput() functions which control an internal stack of
 * input streams.  This functionality supports the needs of standard interface
 * functions like ReadArchive and RiProcRunProgram which require the input
 * context to be changed between requests.
 */
class RibLexer
{
    public:
        // Array token types used by the lexer
        typedef Ri::IntArray     IntArray;
        typedef Ri::FloatArray   FloatArray;
        typedef Ri::StringArray  StringArray;

        /// Callback function type for passing comments.
        typedef boost::function<void (const std::string&)> CommentCallback;

        /// RIB token types used by the peekNextType() function
        enum TokenType
        {
            Tok_Int,
            Tok_Float,
            Tok_String,
            Tok_Array,
            Tok_RequestEnd
        };

        //--------------------------------------------------
        /** \brief Construct and return a RibLexer instance
         *
         * The initial input stream is a null stream so parseNextRequest() has
         * no effect until pushInput() is called.
         *
         * \param handler - Object which will be called by the lexer to parse
         * the details of various RIB requests.
         */
        static RibLexer* create();
        /// Destroy the lexer.
        ///
        /// Should be used instead of delete, particularly on DLL platforms.
        static void destroy(RibLexer* lexer);

        //--------------------------------------------------
        /// \name Error recovery
        //@{
        /** Error recovery: discard tokens until the next request
         *
         * Recover from an error by reading and discarding tokens from the RIB
         * stream up until the next request, as per the RISpec.
         */
        virtual void discardUntilRequest() = 0;
        /** \brief Get the position in the currently connected RIB
         *
         * The current name and position of the RIB on top of the stream stack
         * is returned in the handy format
         *     "<stream_name>:<line_number> (col <col_number>)"
         * which is useful for error reporting.
         */
        virtual std::string streamPos() = 0;
        //@}

        //--------------------------------------------------
        /// \name Stream management
        //@{
        /** \brief Push a stream onto the input stack
         *
         * The stream will remain as the current input stream until a
         * corresponding call to popInput() is encountered.
         *
         * Note that the stream should be opened in binary mode so that no
         * translation of newline characters is performed.  If not, any binary
         * encoded RIB will be read incorrectly.
         *
         * According to the RISpec, it's possible to install a callback function
         * to perform a custom action when the lexer encounters a comment (see
         * the section describing RiReadArchive).  The comment callback function
         * is called immediately whenever a comment is encountered in the RIB
         * stream.  The argument to the callback is the comment string from one
         * character after the leading # up to and not including the next end of
         * line character.
         *
         * Performance consideration: If the input stream corresponds to
         * std::cin and high performance is desired, the user should call
         * std::ios_base::sync_with_stdio(false) to encourage buffering directly
         * by the C++ iostream library.  If not, bytes are likely to be read one
         * at a time, resulting in poor lexer performance (measured to be
         * approximately a factor of two slower on linux/g++/amd64).
         *
         * \param inStream - stream from which RIB will be read
         * \param streamName - name of the input stream
         * \param commentCallback - callback function for handling comments in
         *                          the RIB stream.  Defaults to an empty
         *                          callback which corresponds to discarding
         *                          all comments.
         */
        virtual void pushInput(std::istream& inStream,
                const std::string& streamName,
                const CommentCallback& callback = CommentCallback()) = 0;
        /** \brief Pop a stream off the input stack
         *
         * If the stream is the last on the input stack, the lexer reverts to
         * null input for which parseNextRequest() has no effect.
         */
        virtual void popInput() = 0;
        //@}

        //--------------------------------------------------
        /// \name Functions for reading tokens.
        //@{
        /** \brief Reset state and read next request name
         *
         * This function first resets the lexer state, invalidating any
         * references to values obtained for the previous request.  The next
         * token is then read and returned if it's a request name.  If not,
         * an XqParseError is thrown.
         *
         * \return the next request name, or 0 if end of stream is reached.
         */
        virtual const char* nextRequest() = 0;

        /// Read an integer from the input
        virtual int getInt() = 0;
        /// Read a float from the input
        virtual float getFloat() = 0;
        /// Read a string from the input
        virtual const char* getString() = 0;

        /** \brief Read an array of integers from the input.
         *
         * \return a reference to the array which is valid until parsing the
         * next request commences.
         */
        virtual IntArray getIntArray() = 0;
        /** \brief Read an array of floats from the input
         *
         * The array can be in two formats.  The default is an array of
         * indeterminate length, in the format
         *
         *   '[' num_1 num_2 ... num_n ']'
         *
         * However, if the length parameter is non-negative, the function also
         * parses arrays without the delimiting brackets, that is, of the form
         *
         *   num_1 num_2 ... num_n
         *
         * \param length - number of elements expected in the array.  When
         * non-negative, this also allows arrays without delimiting brackets to
         * be parsed.
         *
         * \return a reference to the array which is valid until parsing the
         * next request commences.
         */
        virtual FloatArray getFloatArray(int length = -1) = 0;
        /** \brief Read an array of strings from the input
         *
         * \return a reference to the array which is valid until parsing the
         * next request commences.
         */
        virtual StringArray getStringArray() = 0;

        /** \brief Return the type of the next RIB token in the stream.
         *
         * Some requests such as SubdivisionMesh or Basis have multiple
         * allowable forms, so the request handler may need to decide which
         * type to read next from the lexer.
         *
         * \return The type of the next request parameter in the input.  If the
         * next token is not a valid request parameter Tok_RequestEnd is
         * returned.
         */
        virtual TokenType peekNextType() = 0;

        /// Read an integer or integer array from the input as an array
        virtual IntArray getIntParam() = 0;
        /// Read an float or float array from the input as an array
        virtual FloatArray getFloatParam() = 0;
        /// Read an string or string array from the input as an array
        virtual StringArray getStringParam() = 0;
        //@}

        virtual ~RibLexer() {}
};


} // namespace Aqsis

#endif // AQSIS_RIBLEXER_H_INCLUDED

// vi: set et:
