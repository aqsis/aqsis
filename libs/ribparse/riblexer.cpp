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
 * \brief RIB lexer implementation
 * \author Chris Foster  [chris42f (at) gmail (dot) com]
 */

#include "riblexer.h"

#include <cctype>
#include <cmath>
#include <cstdio> // for EOF
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
struct CqRibLexer::SqInputState
{
	CqRibInputBuffer inBuf;
	SqSourcePos currPos;
	SqSourcePos nextPos;
	CqRibToken nextTok;
	bool haveNext;
	TqCommentCallback commentCallback;

	SqInputState(std::istream& inStream, const std::string& streamName,
			const SqSourcePos& currPos, const SqSourcePos& nextPos,
			const CqRibToken& nextTok, bool haveNext,
			const TqCommentCallback& callback)
		: inBuf(inStream, streamName),
		currPos(currPos),
		nextPos(nextPos),
		nextTok(nextTok),
		haveNext(haveNext),
		commentCallback(callback)
	{ }
};

//-------------------------------------------------------------------------------
// CqRibLexer implementation

CqRibLexer::CqRibLexer()
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

void CqRibLexer::pushInput(std::istream& inStream, const std::string& streamName,
		const TqCommentCallback& callback)
{
	m_inputStack.push( boost::shared_ptr<SqInputState>(
				new SqInputState(inStream, streamName, m_currPos, m_nextPos,
					m_nextTok, m_haveNext, m_commentCallback)) );
	m_inBuf = &m_inputStack.top()->inBuf;
	m_currPos = SqSourcePos(1,1);
	m_nextPos = SqSourcePos(1,1);
	m_haveNext = false;
	m_commentCallback = callback;
}

void CqRibLexer::popInput()
{
	assert(!m_inputStack.empty());
	// Restore the current state to the state before the previous pushInput()
	const SqInputState& restoreState = *m_inputStack.top();
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

/** \brief Scan the next token from the underlying input stream.
 *
 * Optimization note: this is intentionally all one big function, since there's
 * a performance hit of perhaps 10% or so associated with splitting the ascii
 * and binary decoding up.  It's slightly more natural to handle all the cases
 * at once.
 */
CqRibToken CqRibLexer::scanNext()
{
	if(!m_inBuf)
		return CqRibToken(CqRibToken::ENDOFFILE);
	if(m_arrayElementsRemaining >= 0)
	{
		// If we're currently decoding a float array, return the next element,
		// or an array end token if we've reached the end.
		--m_arrayElementsRemaining;
		if(m_arrayElementsRemaining < 0)
			return CqRibToken(CqRibToken::ARRAY_END);
		else
			return CqRibToken(decodeFloat32(*m_inBuf));
	}
	// Else determine the next token.  The while loop is to ignore whitespace
	// and comments.
	while(true)
	{
		CqRibInputBuffer::TqOutputType c = m_inBuf->get();
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
				return readNumber(*m_inBuf);
			case '"':
				return readString(*m_inBuf);
			case '[':
				return CqRibToken(CqRibToken::ARRAY_BEGIN);
			case ']':
				return CqRibToken(CqRibToken::ARRAY_END);
			default:
				// If a character is nothing else, it's assumed to represent
				// the start of a RI request.
				m_inBuf->unget();
				return readRequest(*m_inBuf);
			case EOF:
				return CqRibToken(CqRibToken::ENDOFFILE);

			//------------------------------------------------------------
			// Binary rib decoding
			//------------------------------------------------------------
			// See the RISpec Appendix C.2.  All characters between 0200 and
			// 0377 are part of the binary encoding.
			case 0200: case 0201: case 0202: case 0203: // b  to  bbbb
				// Decode integers - the encoded token has the form
				//   0200 + w  |  <value>
				// where <value> is w+1 bytes with MSB first.
				return CqRibToken(static_cast<TqInt>(decodeInt(*m_inBuf, c - 0200 + 1)));
			case 0204: case 0205: case 0206: case 0207: // .b to bbb.b
			case 0210: case 0211: case 0212: case 0213: // ._b to bb.bb
			case 0214: case 0215: case 0216: case 0217: // .__b to b.bbb
				// Decode fixed point numbers.  The encoded token has the form
				//   0200 + 4*d + w  |  <value>
				// where <value> is w+1 bytes specifying a fixed point
				// number with the radix point ("decimal" point) located d
				// bytes into the number from the right.
				return CqRibToken(decodeFixedPoint(*m_inBuf,
					((c - 0200) & 0x03) + 1,  // total bytes
					((c - 0200) >> 2) & 0x03  // location of radix point
				));
			case 0220: case 0221: case 0222: case 0223:
			case 0224: case 0225: case 0226: case 0227:
			case 0230: case 0231: case 0232: case 0233:
			case 0234: case 0235: case 0236: case 0237:
				// Decode strings shorter than 16 bytes.  The encoded token has the form
				//   0220 + w  |  <string>
				// where <string> contains w bytes.
				return CqRibToken(CqRibToken::STRING, decodeString(*m_inBuf, c - 0220));
			case 0240: case 0241: case 0242: case 0243:
				// Decode strings of length >= 16 bytes.  The encoded token has the form
				//   0240 + l  |  <length>  |  <string>
				// where <length> is l bytes long, 0 <= l <= 3, and <string> is
				// of length l+1.
				return CqRibToken(CqRibToken::STRING,
						decodeString(*m_inBuf, decodeInt(*m_inBuf, c - 0240 + 1)));
			case 0244:
				// Decode a 32-bit floating point value.  The encoded token has the form
				//   0244  |  <value>
				// where <value> is 4 bytes in IEEE floating point format,
				// transmitted from MSB to LSB
				return CqRibToken(decodeFloat32(*m_inBuf));
			case 0245:
				// Decode a 64-bit floating point value.  The encoded token has the form
				//   0244  |  <value>
				// where <value> is 8 bytes in IEEE floating point format,
				// transmitted from MSB to LSB
				return CqRibToken(static_cast<TqFloat>(decodeFloat64(*m_inBuf)));
			case 0246:
				// Read encoded RI request.  The encoded token has the form
				//   0246  |  <code>
				// where <code> is a single byte indexing a previously encoded
				// request.
				return lookupEncodedRequest(static_cast<TqUint8>(m_inBuf->get()));
			case 0247:
			case 0250: case 0251: case 0252: case 0253:
			case 0254: case 0255: case 0256: case 0257:
			case 0260: case 0261: case 0262: case 0263:
			case 0264: case 0265: case 0266: case 0267:
			case 0270: case 0271: case 0272: case 0273:
			case 0274: case 0275: case 0276: case 0277:
			case 0300: case 0301: case 0302: case 0303:
			case 0304: case 0305: case 0306: case 0307:
				return CqRibToken(CqRibToken::ERROR, "reserved byte encountered");
			case 0310: case 0311: case 0312: case 0313:
				// Decode an array of 32-bit floats.  The encoded token has the form
				//   0310 + l  |  <length>  |  <array>
				// where <length> is l+1 bytes long, 0 <= l <= 3, and <array> is
				// an array of <length> 32-bit floating point values in IEEE format.
				m_arrayElementsRemaining = decodeInt(*m_inBuf, c - 0310 + 1);
				return CqRibToken(CqRibToken::ARRAY_BEGIN);
			case 0314:
				// Define encoded RI request.  The encoded token has the form
				//   0314  |  <code>  |  <string>
				// where <code> is the single byte index for the encoded
				// request, and <string> is a string specifying the ASCII form
				// of the request.
				{
					TqUint8 code = static_cast<TqUint8>(m_inBuf->get());
					CqRibToken requestNameTok = scanNext();
					if(requestNameTok.type() != CqRibToken::STRING)
						return CqRibToken(CqRibToken::ERROR,
								"expected string missing from encoded "
								"request definition");
					else if(requestNameTok.stringVal().empty())
						return CqRibToken(CqRibToken::ERROR,
								"empty string not valid in encoded "
								"request definition");
					defineEncodedRequest(code, requestNameTok);
				}
				return scanNext();
			case 0315: case 0316:
				// Define encoded string.  The encoded token has the form
				//   0315 + w  |  <code>  |  <string>
				// where <code> is a length w+1 integer, and <string> is a
				// string in any format.
				{
					TqInt code = decodeInt(*m_inBuf, c - 0315 + 1);
					CqRibToken stringNameTok = scanNext();
					if(stringNameTok.type() != CqRibToken::STRING)
						return CqRibToken(CqRibToken::ERROR,
								"expected string missing from encoded "
								"string definition");
					defineEncodedString(code, stringNameTok);
				}
				return scanNext();
			case 0317: case 0320:
				// Look up predefined string.  The encoded token has the form
				//   0317 + w  |  <code>
				// where <code> is a length w+1 unsigned integer representing
				// the index of a previously defined string.
				return lookupEncodedString(decodeInt(*m_inBuf, c - 0317 + 1));
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
				return CqRibToken(CqRibToken::ERROR, "reserved byte encountered");
			case 0377:
				// 0377 signals the end of a RunProgram block when we're
				// reading from a pipe.
				return CqRibToken(CqRibToken::ENDOFFILE);
		}
	}
}

/// Read in an ASCII number (integer or real)
CqRibToken CqRibLexer::readNumber(CqRibInputBuffer& inBuf)
{
	CqRibInputBuffer::TqOutputType c = 0;
	TqInt sign = 1;
	TqInt intResult = 0;
	TqFloat floatResult = 0;
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
					return CqRibToken(CqRibToken::ERROR,
							"Expected at least one digit in float");
				TqFloat thisPlaceVal = 0.1;
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
					return CqRibToken(floatResult);
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
				return CqRibToken(CqRibToken::ERROR, "Expected a digit");
			inBuf.unget();
			return CqRibToken(intResult);
			break;
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
		return CqRibToken(CqRibToken::ERROR, "Expected digits in float exponent");
	TqInt exponent = 0;
	while(std::isdigit(c))
	{
		exponent *= 10;
		exponent += c - '0';
		c = inBuf.get();
	}
	exponent *= sign;
	inBuf.unget();
	floatResult *= std::pow(10.0,exponent);
	return CqRibToken(floatResult);
}

/** \brief Read in a string
 *
 * Assumes that the leading '"' has already been read.
 */
CqRibToken CqRibLexer::readString(CqRibInputBuffer& inBuf)
{
	// Assume leading '"' has already been read.
	CqRibToken outTok(CqRibToken::STRING, "");
	std::string& outString = outTok.m_strVal;
	outString.reserve(30);
	bool stringFinished = false;
	while(!stringFinished)
	{
		CqRibInputBuffer::TqOutputType c = inBuf.get();
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
					case '8':
					case '9':
						{
							// read in a sequence of (up to) three octal digits
							unsigned char octalChar = c - '0';
							c = inBuf.get();
							for(TqInt i = 0; i < 2 && std::isdigit(c); i++ )
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
			case EOF:
				return CqRibToken(CqRibToken::ERROR,
						"End of file found while scanning string");
				break;
			default:
				outString += c;
				break;
		}
	}
	return outTok;
}

/// Read in a RIB request
CqRibToken CqRibLexer::readRequest(CqRibInputBuffer& inBuf)
{
	CqRibToken outTok(CqRibToken::REQUEST, "");
	std::string& name = outTok.m_strVal;
	name.reserve(30);
	while(true)
	{
		CqRibInputBuffer::TqOutputType c = inBuf.get();
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
	return outTok;
}

/// Read in and discard a comment.
void CqRibLexer::readComment(CqRibInputBuffer& inBuf)
{
	CqRibInputBuffer::TqOutputType c = inBuf.get();
	if(m_commentCallback)
	{
		std::string comment;
		while(c != EOF && c != '\n' && c != '\r' && c != 0377)
		{
			comment += c;
			c = inBuf.get();
		}
		m_commentCallback(comment);
	}
	else
	{
		while(c != EOF && c != '\n' && c != '\r' && c != 0377)
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
inline TqUint32 CqRibLexer::decodeInt(CqRibInputBuffer& inBuf, TqInt numBytes)
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
inline TqFloat CqRibLexer::decodeFixedPoint(CqRibInputBuffer& inBuf,
		TqInt numBytes, TqInt radixPos)
{
	assert(radixPos > 0);
	TqUint32 mag = decodeInt(inBuf, numBytes - radixPos);
	TqUint32 frac = decodeInt(inBuf, min(radixPos, numBytes));
	return mag + static_cast<TqFloat>(frac)/(1 << (8*radixPos));
}

/** \brief Decode a 32-bit IEEE floating point number.
 *
 * Bits are arranged from MSB to LSB
 *
 * \param inBuf - bytes are read from this input buffer.
 */
inline TqFloat CqRibLexer::decodeFloat32(CqRibInputBuffer& inBuf)
{
	// union to avoid illegal type-punning
	union UqFloatInt32 {
		TqFloat f;
		TqUint32 i;
	};
	UqFloatInt32 conv;
	conv.i = decodeInt(inBuf, 4);
	return conv.f;
}

/** \brief Decode a 64-bit IEEE floating point number.
 *
 * Bits are arranged from MSB to LSB
 *
 * \param inBuf - bytes are read from this input buffer.
 */
inline TqDouble CqRibLexer::decodeFloat64(CqRibInputBuffer& inBuf)
{
	// union to avoid illegal type-punning
	union UqFloatInt64 {
		TqDouble d;
		boost::uint64_t i;
	};
	UqFloatInt64 conv;
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
std::string CqRibLexer::decodeString(CqRibInputBuffer& inBuf, TqInt numBytes)
{
	std::string str;
	str.reserve(numBytes);
	for(TqInt i = 0; i < numBytes; ++i)
		str += static_cast<char>(inBuf.get());
	return str;
}

/** \brief Associate a request name with a numeric code
 *
 * \param code - single byte which the request will be associated with.
 * \param requestNameTok - token holding the request name
 */
inline void CqRibLexer::defineEncodedRequest(TqUint8 code,
		const CqRibToken& requestNameTok)
{
	m_encodedRequests[code] = requestNameTok.stringVal();
}

/// Look up a previously defined encoded request.
inline CqRibToken CqRibLexer::lookupEncodedRequest(TqUint8 code) const
{
	const std::string& str = m_encodedRequests[code];
	if(str.empty())
		return CqRibToken(CqRibToken::ERROR,
				"encoded request not previously defined");
	else
		return CqRibToken(CqRibToken::REQUEST, str);
}

/// Associate a string with a numeric code
inline void CqRibLexer::defineEncodedString(TqInt code,
		const CqRibToken& stringNameTok)
{
	m_encodedStrings[code] = stringNameTok.stringVal();
}

/// Look up a previously defined encoded string.
inline CqRibToken CqRibLexer::lookupEncodedString(TqInt code) const
{
	TqEncodedStringMap::const_iterator pos = m_encodedStrings.find(code);
	if(pos != m_encodedStrings.end())
		return CqRibToken(CqRibToken::STRING, pos->second);
	else
		return CqRibToken(CqRibToken::ERROR,
				"encoded string not previously defined");
}

} // namespace Aqsis
