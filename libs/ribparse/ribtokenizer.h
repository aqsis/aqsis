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

#include <aqsis/ribparser.h> // for TqCommentCallback
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
class AQSIS_RIBPARSER_SHARE RibTokenizer : boost::noncopyable
{
	public:
		/// Comment callback function type
		typedef IqRibParser::TqCommentCallback TqCommentCallback;

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
				const TqCommentCallback& callback = TqCommentCallback());
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
		SqRibPos pos() const;

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
		TqCommentCallback m_commentCallback;

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
inline SqRibPos RibTokenizer::pos() const
{
	return SqRibPos(m_currPos.line, m_currPos.col,
			m_inBuf ? m_inBuf->streamName().c_str() : "null");
}

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
