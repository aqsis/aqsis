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
 * \author Chris Foster  [chris42f (at) gmail (dot) com]
 */

#include "ribtokenizer.h"

#include <cctype>
#include <cmath>
#include <iostream>
#include <string>
#include <sstream>

#include <boost/cstdint.hpp> // for uint64_t

#include <aqsis/math/math.h>

namespace Aqsis
{

/** Struct to save the input state of the lexer inside pushInput(), so that it
 * can be restored inside popInput()
 */
struct RibTokenizer::InputState
{
	RibInputBuffer inBuf;
	SourcePos currPos;
	SourcePos nextPos;
	RibToken nextTok;
	bool haveNext;
	CommentCallback commentCallback;

	InputState(std::istream& inStream, const std::string& streamName,
			const SourcePos& currPos, const SourcePos& nextPos,
			const RibToken& nextTok, bool haveNext,
			const CommentCallback& callback)
		: inBuf(inStream, streamName),
		currPos(currPos),
		nextPos(nextPos),
		nextTok(nextTok),
		haveNext(haveNext),
		commentCallback(callback)
	{ }
};

//-------------------------------------------------------------------------------
// RibTokenizer implementation

RibTokenizer::RibTokenizer()
	: m_inBuf(0),
	m_inputStack(),
	m_currPos(1,1),
	m_nextPos(1,1),
	m_nextTok(),
	m_haveNext(false),
	m_commentCallback(),
	m_encodedRequests(256),
	m_encodedStrings(),
	m_arrayElementsRemaining(-1)
{ }

void RibTokenizer::pushInput(std::istream& inStream, const std::string& streamName,
		const CommentCallback& callback)
{
	m_inputStack.push( boost::shared_ptr<InputState>(
				new InputState(inStream, streamName, m_currPos, m_nextPos,
					m_nextTok, m_haveNext, m_commentCallback)) );
	m_inBuf = &m_inputStack.top()->inBuf;
	m_currPos = SourcePos(1,1);
	m_nextPos = SourcePos(1,1);
	m_haveNext = false;
	m_commentCallback = callback;
}

void RibTokenizer::popInput()
{
	assert(!m_inputStack.empty());
	// Restore the current state to the state before the previous pushInput()
	const InputState& restoreState = *m_inputStack.top();
	m_currPos = restoreState.currPos;
	m_nextPos = restoreState.nextPos;
	m_nextTok = restoreState.nextTok;
	m_haveNext = restoreState.haveNext;
	m_commentCallback = restoreState.commentCallback;
	// Pop the stack, and restore the buffer
	m_inputStack.pop();
	if(!m_inputStack.empty())
		m_inBuf = &m_inputStack.top()->inBuf;
	else
		m_inBuf = 0;
}

std::string RibTokenizer::streamPos() const
{
    std::ostringstream msg;
    msg << (m_inBuf ? m_inBuf->streamName() : "null") << ":"
		<< m_currPos.line << " (col " << m_currPos.col << ")";
    return msg.str();
}

/** \brief Scan the next token from the underlying input stream.
 *
 * Optimization note: this is intentionally all one big function, since there's
 * a performance hit of perhaps 10% or so associated with splitting the ascii
 * and binary decoding up.  It's slightly more natural to handle all the cases
 * at once.
 */
void RibTokenizer::scanNext(RibToken& tok)
{
	if(!m_inBuf)
	{
		tok = RibToken::ENDOFFILE;
		return;
	}
	if(m_arrayElementsRemaining >= 0)
	{
		// If we're currently decoding a float array, return the next element,
		// or an array end token if we've reached the end.
		--m_arrayElementsRemaining;
		if(m_arrayElementsRemaining < 0)
		{
			tok = RibToken::ARRAY_END;
			return;
		}
		else
		{
			tok = decodeFloat32(*m_inBuf);
			return;
		}
	}
	// Else determine the next token.  The while loop is to ignore whitespace
	// and comments.
	while(true)
	{
		RibInputBuffer::CharType c = m_inBuf->get();
		m_nextPos = m_inBuf->pos();
		switch(c)
		{
			//------------------------------------------------------------
			// ASCII rib decoding
			//------------------------------------------------------------
			case ' ':
			case '\t':
			case '\n':
			case '\r':
				// ignore whitespace
				break;
			case '#':
				readComment(*m_inBuf);
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
			case '-': case '+': case '.':
				m_inBuf->unget();
				readNumber(*m_inBuf, tok);
				return;
			case '"':
				readString(*m_inBuf, tok);
				return;
			case '[':
				tok = RibToken::ARRAY_BEGIN;
				return;
			case ']':
				tok = RibToken::ARRAY_END;
				return;
			default:
				// If a character is nothing else, it's assumed to represent
				// the start of a RI request.
				m_inBuf->unget();
				readRequest(*m_inBuf, tok);
				return;

			//------------------------------------------------------------
			// Binary rib decoding
			//------------------------------------------------------------
			// See the RISpec Appendix C.2.  All characters between 0200 and
			// 0377 are part of the binary encoding.
			case 0200: case 0201: case 0202: case 0203: // b  to  bbbb
				// Decode integers - the encoded token has the form
				//   0200 + w  |  <value>
				// where <value> is w+1 bytes with MSB first.
				tok = static_cast<int>(decodeInt(*m_inBuf, c - 0200 + 1));
				return;
			case 0204: case 0205: case 0206: case 0207: // .b to bbb.b
			case 0210: case 0211: case 0212: case 0213: // ._b to bb.bb
			case 0214: case 0215: case 0216: case 0217: // .__b to b.bbb
				// Decode fixed point numbers.  The encoded token has the form
				//   0200 + 4*d + w  |  <value>
				// where <value> is w+1 bytes specifying a fixed point
				// number with the radix point ("decimal" point) located d
				// bytes into the number from the right.
				tok = decodeFixedPoint(*m_inBuf,
					((c - 0200) & 0x03) + 1,  // total bytes
					((c - 0200) >> 2) & 0x03  // location of radix point
				);
				return;
			case 0220: case 0221: case 0222: case 0223:
			case 0224: case 0225: case 0226: case 0227:
			case 0230: case 0231: case 0232: case 0233:
			case 0234: case 0235: case 0236: case 0237:
				// Decode strings shorter than 16 bytes.  The encoded token has the form
				//   0220 + w  |  <string>
				// where <string> contains w bytes.
				decodeString(*m_inBuf, c - 0220, tok);
				return;
			case 0240: case 0241: case 0242: case 0243:
				// Decode strings of length >= 16 bytes.  The encoded token has the form
				//   0240 + l  |  <length>  |  <string>
				// where <length> is l+1 bytes long, 0 <= l <= 3, and <string> is
				// of length l.
				decodeString(*m_inBuf, decodeInt(*m_inBuf, c - 0240 + 1), tok);
				return;
			case 0244:
				// Decode a 32-bit floating point value.  The encoded token has the form
				//   0244  |  <value>
				// where <value> is 4 bytes in IEEE floating point format,
				// transmitted from MSB to LSB
				tok = decodeFloat32(*m_inBuf);
				return;
			case 0245:
				// Decode a 64-bit floating point value.  The encoded token has the form
				//   0244  |  <value>
				// where <value> is 8 bytes in IEEE floating point format,
				// transmitted from MSB to LSB
				tok = static_cast<float>(decodeFloat64(*m_inBuf));
				return;
			case 0246:
				// Read encoded RI request.  The encoded token has the form
				//   0246  |  <code>
				// where <code> is a single byte indexing a previously encoded
				// request.
				lookupEncodedRequest(static_cast<TqUint8>(m_inBuf->get()), tok);
				return;
			case 0247:
			case 0250: case 0251: case 0252: case 0253:
			case 0254: case 0255: case 0256: case 0257:
			case 0260: case 0261: case 0262: case 0263:
			case 0264: case 0265: case 0266: case 0267:
			case 0270: case 0271: case 0272: case 0273:
			case 0274: case 0275: case 0276: case 0277:
			case 0300: case 0301: case 0302: case 0303:
			case 0304: case 0305: case 0306: case 0307:
				tok.error("reserved byte encountered");
				return;
			case 0310: case 0311: case 0312: case 0313:
				// Decode an array of 32-bit floats.  The encoded token has the form
				//   0310 + l  |  <length>  |  <array>
				// where <length> is l+1 bytes long, 0 <= l <= 3, and <array> is
				// an array of <length> 32-bit floating point values in IEEE format.
				m_arrayElementsRemaining = decodeInt(*m_inBuf, c - 0310 + 1);
				tok = RibToken::ARRAY_BEGIN;
				return;
			case 0314:
				// Define encoded RI request.  The encoded token has the form
				//   0314  |  <code>  |  <string>
				// where <code> is the single byte index for the encoded
				// request, and <string> is a string specifying the ASCII form
				// of the request.
				{
					TqUint8 code = static_cast<TqUint8>(m_inBuf->get());
					scanNext(tok);
					if(tok.type() != RibToken::STRING)
					{
						tok.error("expected string missing from encoded "
								  "request definition");
						return;
					}
					else if(tok.stringVal().empty())
					{
						tok.error("empty string not valid in encoded "
								  "request definition");
						return;
					}
					// FIXME: m_encodedRequests should be part of the
					// pushed/popped state.
					m_encodedRequests[code] = tok.stringVal();
				}
				break;
			case 0315: case 0316:
				// Define encoded string.  The encoded token has the form
				//   0315 + w  |  <code>  |  <string>
				// where <code> is a length w+1 integer, and <string> is a
				// string in any format.
				{
					int code = decodeInt(*m_inBuf, c - 0315 + 1);
					scanNext(tok);
					if(tok.type() != RibToken::STRING)
					{
						tok.error("expected string missing from encoded "
								  "string definition");
						return;
					}
					m_encodedStrings[code] = tok.stringVal();
				}
				break;
			case 0317: case 0320:
				// Look up predefined string.  The encoded token has the form
				//   0317 + w  |  <code>
				// where <code> is a length w+1 unsigned integer representing
				// the index of a previously defined string.
				lookupEncodedString(decodeInt(*m_inBuf, c - 0317 + 1), tok);
				return;
			case 0321: case 0322: case 0323:
			case 0324: case 0325: case 0326: case 0327:
			case 0330: case 0331: case 0332: case 0333:
			case 0334: case 0335: case 0336: case 0337:
			case 0340: case 0341: case 0342: case 0343:
			case 0344: case 0345: case 0346: case 0347:
			case 0350: case 0351: case 0352: case 0353:
			case 0354: case 0355: case 0356: case 0357:
			case 0360: case 0361: case 0362: case 0363:
			case 0364: case 0365: case 0366: case 0367:
			case 0370: case 0371: case 0372: case 0373:
			case 0374: case 0375: case 0376:
				tok.error("reserved byte encountered");
				return;
			case RibInputBuffer::eof:  // == 0377
				// 0377 signals the end of a RunProgram block when we're
				// reading from a pipe.  It's also used as the handy byte-sized
				// standin for EOF from our input buffer.
				tok = RibToken::ENDOFFILE;
				return;
		}
	}
}

/// Read in an ASCII number (integer or real)
void RibTokenizer::readNumber(RibInputBuffer& inBuf, RibToken& tok)
{
	RibInputBuffer::CharType c = 0;
	int sign = 1;
	int intResult = 0;
	float floatResult = 0;
	bool haveReadDigit = false;
	c = inBuf.get();
	// deal with optional sign
	switch(c)
	{
		case '+':
			c = inBuf.get();
			break;
		case '-':
			sign = -1;
			c = inBuf.get();
			break;
	}
	// deal with digits before decimal point
	if(std::isdigit(c))
		haveReadDigit = true;
	// \todo: Think of a better way to handle the overflow below robustly?
	while(std::isdigit(c))
	{
		intResult *= 10;
		intResult += c - '0';
		// this duplication seems like a waste, but it's robust against overflow...
		floatResult *= 10;
		floatResult += c - '0';
		c = inBuf.get();
	}
	switch(c)
	{
		case '.':
			{
				// deal with digits to right of decimal point
				c = inBuf.get();
				if(!haveReadDigit && !std::isdigit(c))
				{
					tok.error("Expected at least one digit in float");
					return;
				}
				float thisPlaceVal = 0.1;
				while(std::isdigit(c))
				{
					floatResult += (c - '0') * thisPlaceVal;
					thisPlaceVal *= 0.1;
					c = inBuf.get();
				}
				floatResult *= sign;
				if(c != 'e' && c != 'E')
				{
					inBuf.unget();
					tok = floatResult;
					return;
				}
			}
			break;
		case 'e':
		case 'E':
			floatResult *= sign;
			break;
		default:
			// Number is an integer
			intResult *= sign;
			if(!haveReadDigit)
			{
				tok.error("Expected a digit");
				return;
			}
			inBuf.unget();
			tok = intResult;
			return;
	}
	// deal with the exponent
	c = inBuf.get();
	sign = 1;
	switch(c)
	{
		case '+':
			c = inBuf.get();
			break;
		case '-':
			sign = -1;
			c = inBuf.get();
			break;
	}
	if(!std::isdigit(c))
	{
		tok.error("Expected digits in float exponent");
		return;
	}
	int exponent = 0;
	while(std::isdigit(c))
	{
		exponent *= 10;
		exponent += c - '0';
		c = inBuf.get();
	}
	exponent *= sign;
	inBuf.unget();
	floatResult *= std::pow(10.0,exponent);
	tok = floatResult;
}

/** \brief Read in a string
 *
 * Assumes that the leading '"' has already been read.
 */
void RibTokenizer::readString(RibInputBuffer& inBuf, RibToken& tok)
{
	// Assume leading '"' has already been read.
	tok = RibToken::STRING;
	std::string& outString = tok.m_strVal;
	outString.clear();
	bool stringFinished = false;
	while(!stringFinished)
	{
		RibInputBuffer::CharType c = inBuf.get();
		switch(c)
		{
			case '"':
				stringFinished = true;
				break;
			case '\\':
				// deal with escape characters
				c = inBuf.get();
				switch(c)
				{
					case 'n':
						outString += '\n';
						break;
					case 'r':
						outString += '\r';
						break;
					case 't':
						outString += '\t';
						break;
					case 'b':
						outString += '\b';
						break;
					case 'f':
						outString += '\f';
						break;
					case '\\':
						outString += '\\';
						break;
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
						{
							// read in a sequence of (up to) three octal digits
							unsigned char octalChar = c - '0';
							c = inBuf.get();
							for(int i = 0; i < 2 && c >= '0' && c <= '7'; i++ )
							{
								octalChar = 8*octalChar + (c - '0');
								c = inBuf.get();
							}
							inBuf.unget();
							outString += octalChar;
						}
						break;
					case '\r':
						// discard newlines, including "\r\n" pairs.
						if(inBuf.get() != '\n')
							inBuf.unget();
						break;
					case '\n':
						break;
					default:
						// ignore the escape '\' if the following char isn't one of
						// the above.
						outString += c;
						break;
				}
				break;
			case '\r':
				if(inBuf.get() != '\n')
					inBuf.unget();
				outString += '\n';
				break;
			case RibInputBuffer::eof:
				tok.error("End of file found while scanning string");
				return;
			default:
				outString += c;
				break;
		}
	}
}

/// Read in a RIB request
void RibTokenizer::readRequest(RibInputBuffer& inBuf, RibToken& tok)
{
	tok = RibToken::REQUEST;
	std::string& name = tok.m_strVal;
	name.clear();
	while(true)
	{
		RibInputBuffer::CharType c = inBuf.get();
		bool acceptChar = false;
		// Check that
		// 1. c is in the 7-bit ASCII character set.
		if(c >= 0 && c < 0200)
		{
			// 2. c is not whitespace or a special char
			switch(c)
			{
				case ' ':
				case '\t':
				case '\n':
				case '\r':
				case '#':
				case '"':
				case '[':
				case ']':
					break;
				default:
					acceptChar = true;
			}
		}
		// if it's anything else, c continues the request.
		if(acceptChar)
			name += c;
		else
		{
			inBuf.unget();
			break;
		}
	}
}

/// Read in and discard a comment.
void RibTokenizer::readComment(RibInputBuffer& inBuf)
{
	RibInputBuffer::CharType c = inBuf.get();
	if(m_commentCallback)
	{
		std::string comment;
		while(c != '\n' && c != '\r' && c != RibInputBuffer::eof)
		{
			comment += c;
			c = inBuf.get();
		}
		m_commentCallback(comment);
	}
	else
	{
		while(c != '\n' && c != '\r' && c != RibInputBuffer::eof)
			c = inBuf.get();
	}
	inBuf.unget();
}

/** \brief Decode an unsigned integer.
 *
 * Bytes are arranged from MSB to LSB; this function can read up to four bytes.
 *
 * \param inBuf - bytes are read from this input buffer.
 * \param numBytes - number of bytes taken up by the integer
 */
inline TqUint32 RibTokenizer::decodeInt(RibInputBuffer& inBuf, int numBytes)
{
	TqUint32 result = 0;
	switch(numBytes)
	{
		// intentional case fallthrough.
		case 4: result = static_cast<TqUint8>(inBuf.get()) << 24;
		case 3: result += static_cast<TqUint8>(inBuf.get()) << 16;
		case 2: result += static_cast<TqUint8>(inBuf.get()) << 8;
		case 1: result += static_cast<TqUint8>(inBuf.get());
	}
	return result;
}

/** \brief Read and decode a fixed point number.
 *
 * These come as a set of bytes with the location of the radix point
 * ("decimal" point) specified with respect to the right hand side.
 *
 * \param numBytes - total number of bytes.
 * \param radixPos - position of radix ("decimal") point.  Must be positive.
 *
 * For example:
 *
 * \verbatim
 *
 *       b.bbb               .__bb
 *         <--                <---
 *    radixPos = 3         radixPos = 4
 *    numBytes = 4         numBytes = 2
 *
 * \endverbatim
 *
 * \param inBuf - bytes are read from this input buffer.
 */
inline float RibTokenizer::decodeFixedPoint(RibInputBuffer& inBuf,
		int numBytes, int radixPos)
{
	assert(radixPos > 0);
	TqUint32 mag = decodeInt(inBuf, numBytes - radixPos);
	TqUint32 frac = decodeInt(inBuf, min(radixPos, numBytes));
	return mag + static_cast<float>(frac)/(1 << (8*radixPos));
}

/** \brief Decode a 32-bit IEEE floating point number.
 *
 * Bits are arranged from MSB to LSB
 *
 * \param inBuf - bytes are read from this input buffer.
 */
inline float RibTokenizer::decodeFloat32(RibInputBuffer& inBuf)
{
	// union to avoid illegal type-punning
	union FloatInt32 {
		float f;
		TqUint32 i;
	};
	FloatInt32 conv;
	conv.i = decodeInt(inBuf, 4);
	return conv.f;
}

/** \brief Decode a 64-bit IEEE floating point number.
 *
 * Bits are arranged from MSB to LSB
 *
 * \param inBuf - bytes are read from this input buffer.
 */
inline double RibTokenizer::decodeFloat64(RibInputBuffer& inBuf)
{
	// union to avoid illegal type-punning
	union FloatInt64 {
		double d;
		boost::uint64_t i;
	};
	FloatInt64 conv;
	conv.i = static_cast<boost::uint64_t>(decodeInt(inBuf, 4)) << 32;
	conv.i += decodeInt(inBuf, 4);
	return conv.d;
}

/** \brief Read a string of specified length
 *
 * Since this is for use with binary-encoded RIB, no translations are performed
 * on escape characters etc.
 *
 * \param inBuf - bytes are read from this input buffer.
 * \param numBytes - number of bytes taken up by the string
 */
void RibTokenizer::decodeString(RibInputBuffer& inBuf, int numBytes,
							  RibToken& tok)
{
	tok = RibToken::STRING;
	std::string& str = tok.m_strVal;
	str.clear();
	for(int i = 0; i < numBytes; ++i)
		str += static_cast<char>(inBuf.get());
}

/// Look up a previously defined encoded request.
inline void RibTokenizer::lookupEncodedRequest(TqUint8 code, RibToken& tok) const
{
	const std::string& str = m_encodedRequests[code];
	if(str.empty())
		tok.error("encoded request not previously defined");
	else
	{
		tok = RibToken::REQUEST;
		tok.m_strVal = str;
	}
}

/// Look up a previously defined encoded string.
inline void RibTokenizer::lookupEncodedString(int code, RibToken& tok) const
{
	EncodedStringMap::const_iterator pos = m_encodedStrings.find(code);
	if(pos != m_encodedStrings.end())
	{
		tok = RibToken::STRING;
		tok.m_strVal = pos->second;
	}
	else
		tok.error("encoded string not previously defined");
}

} // namespace Aqsis
