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
 * \brief A RIB lexer class.
 * \author Chris Foster  [chris42f (at) gmail (dot) com]
 */

#ifndef AQSIS_RIBTOKENIZER_H_INCLUDED
#define AQSIS_RIBTOKENIZER_H_INCLUDED

#include <aqsis/aqsis.h>

#include <iosfwd>
#include <vector>
#include <map>
#include <stack>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include "riblexer.h" // for CommentCallback
#include "ribinputbuffer.h"
#include "ribtoken.h"

namespace Aqsis
{

//------------------------------------------------------------------------------
/** \brief A lexical analyser for the Renderman Interface Byetstream.
 *
 * The lexer translates input characters into a sequence of tokens of type
 * RibToken.  The input can be either in ASCII or binary RIB format.  In
 * addition the lexer supports decompression of a RIB stream from the gzip
 * format.
 *
 * Performance consideration: If the input stream corresponds to std::cin and
 * high performance is desired, the user should call
 * std::ios_base::sync_with_stdio(false) to encourage buffering directly by the
 * C++ iostream library.  If not, bytes are likely to be read one at a time,
 * resulting in significantly poor lexer performance (measured to be
 * approximately a factor of two slower on linux/g++/amd64).
 */
class RibTokenizer : boost::noncopyable
{
	public:
		/// Comment callback function type
		typedef RibLexer::CommentCallback CommentCallback;

		/** Create a new lexer without input stream.
		 *
		 * get() operations will return ENDOFFILE tokens until pushInput() is
		 * called.
		 */
		RibTokenizer();

		/** \brief Push a stream onto the input stack
		 *
		 * The stream should be opened in binary mode so that no translation of
		 * newline characters is performed.
		 *
		 * \param inStream - new stream from which RIB will be read.
		 * \param streamName - Name of the stream, used for error messages.
		 * \param callback - Callback function used when the lexer encounters a
		 *                   comment token.
		 */
		void pushInput(std::istream& inStream, const std::string& streamName,
				const CommentCallback& callback = CommentCallback());
		/** \brief Pop a stream off the input stack
		 *
		 * If the stream is the last on the input stack, the lexer reverts to
		 * using null input and will always return EOF tokens.
		 */
		void popInput();

		/** \brief Get the next token.
		 * \return The next token from the input stream.
		 */
		const RibToken& get();
		/// Put the previous token back into the input.
		void unget();
		/** \brief Look at but don't remove the next token from the input sequence
		 * \return the next token from the input stream
		 */
		const RibToken& peek();

		/** Return the position in the input file
		 *
		 * \return The position in the input file for the previous token
		 *         obtained with get().
		 */
		std::string streamPos() const;

	private:
		typedef std::map<int, std::string> EncodedStringMap;
		struct InputState;

		//--------------------------------------------------
		/// \name ASCII RIB decoding functions
		//@{
		static void readNumber(RibInputBuffer& inBuf, RibToken& tok);
		static void readString(RibInputBuffer& inBuf, RibToken& tok);
		static void readRequest(RibInputBuffer& inBuf, RibToken& tok);
		void readComment(RibInputBuffer& inBuf);
		//@}

		//--------------------------------------------------
		/// \name Binary RIB decoding functions
		//@{
		static TqUint32 decodeInt(RibInputBuffer& inBuf, int numBytes);
		static float decodeFixedPoint(RibInputBuffer& inBuf,
				int numBytes, int radixPos);
		static float decodeFloat32(RibInputBuffer& inBuf);
		static double decodeFloat64(RibInputBuffer& inBuf);
		static void decodeString(RibInputBuffer& inBuf, int numBytes,
								 RibToken& tok);
		void lookupEncodedRequest(TqUint8 code, RibToken& tok) const;
		void lookupEncodedString(int code, RibToken& tok) const;
		//@}

		// scan next token from underlying input stream.
		void scanNext(RibToken& tok);

		//--------------------------------------------------
		// Member data
		/// Input buffer from which characters are read.
		RibInputBuffer* m_inBuf;
		/// Stack of input buffers.
		std::stack<boost::shared_ptr<InputState> > m_inputStack;
		/// source position of previous token in input stream
		SourcePos m_currPos;
		/// source position of latest token read from the input stream
		SourcePos m_nextPos;
		/// next token, if already read.
		RibToken m_nextTok;
		/// flag indicating whether we've already got the next token.
		bool m_haveNext;

		/// Pointer to callback function for handling comments
		CommentCallback m_commentCallback;

		/// Array of encoded requests for binary RIB decoding
		std::vector<std::string> m_encodedRequests;
		/// Map of encoded strings for binary RIB decoding
		EncodedStringMap m_encodedStrings;

		/// Number of array elements remaining in current encoded float array.
		int m_arrayElementsRemaining;
};


//==============================================================================
// Implementation details
//==============================================================================
// RibTokenizer functions
inline const RibToken& RibTokenizer::get()
{
	if(!m_haveNext)
		scanNext(m_nextTok);
	m_haveNext = false;
	m_currPos = m_nextPos;
	return m_nextTok;
}

inline void RibTokenizer::unget()
{
	assert(!m_haveNext);
	m_haveNext = true;
}

inline const RibToken& RibTokenizer::peek()
{
	if(!m_haveNext)
	{
		scanNext(m_nextTok);
		m_haveNext = true;
	}
	return m_nextTok;
}

} // namespace Aqsis

#endif // AQSIS_RIBTOKENIZER_H_INCLUDED
