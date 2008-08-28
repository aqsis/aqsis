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
 * \brief A RIB token class.
 * \author Chris Foster  [chris42f (at) gmail (dot) com]
 */

#include "riblexer.h"

#include <cctype>
#include <cmath>
#include <iostream>
#include <string>
#include <sstream>

#include "aqsismath.h"

namespace ribparse
{

//-------------------------------------------------------------------------------
// CqRibLexer implementation

CqRibLexer::CqRibLexer(std::istream& inStream)
	: m_inBuf(inStream),
	m_currPos(1,1),
	m_prevPos(0,0),
	m_nextTok(),
	m_haveNext(false),
	m_encodedRequests(256),
	m_encodedStrings(),
	m_arrayElementsRemaining(-1)
{}

namespace {
// Helper functions for decoding binary RIB.

/** \brief Decode an unsigned integer.
 *
 * Bytes are arranged from MSB to LSB; this function can read up to four bytes.
 *
 * \param inBuf - bytes are read from this input buffer.
 * \param numBytes - number of bytes taken up by the integer
 */
TqUint32 decodeInt(CqRibInputBuffer& inBuf, TqInt numBytes)
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
TqFloat decodeFixedPoint(CqRibInputBuffer& inBuf, TqInt numBytes,
		TqInt radixPos)
{
	assert(radixPos > 0);
	TqUint32 mag = decodeInt(inBuf, numBytes - radixPos);
	TqUint32 frac = decodeInt(inBuf, Aqsis::min(radixPos, numBytes));
	return mag + static_cast<TqFloat>(frac)/(1 << (8*radixPos));
}

/** \brief Decode a 32-bit IEEE floating point number.
 *
 * Bits are arranged from MSB to LSB
 *
 * \param inBuf - bytes are read from this input buffer.
 */
TqFloat decodeFloat32(CqRibInputBuffer& inBuf)
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
TqDouble decodeFloat64(CqRibInputBuffer& inBuf)
{
	// union to avoid illegal type-punning
	union UqFloatInt64 {
		TqDouble d;
		TqUint64 i;
	};
	UqFloatInt64 conv;
	conv.i = static_cast<TqUint64>(decodeInt(inBuf, 4)) << 32;
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
std::string decodeString(CqRibInputBuffer& inBuf, TqInt numBytes)
{
	std::string str;
	str.reserve(numBytes);
	for(TqInt i = 0; i < numBytes; ++i)
		str += static_cast<char>(inBuf.get());
	return str;
}

} // unnamed namespace


CqRibToken CqRibLexer::getToken()
{
	m_prevPos = m_currPos;
	if(m_haveNext)
	{
		// Return the next token directly if we already have it.
		m_haveNext = false;
		return m_nextTok;
	}
	if(m_arrayElementsRemaining >= 0)
	{
		// If we're currently decoding a float array, return the next element,
		// or an array end token if we've reached the end.
		--m_arrayElementsRemaining;
		if(m_arrayElementsRemaining < 0)
			return CqRibToken(CqRibToken::ARRAY_END);
		else
			return CqRibToken(decodeFloat32(m_inBuf));
	}
	// Else determine the next token.
	while(true)
	{
		m_currPos = m_inBuf.pos();
		CqRibInputBuffer::TqOutputType c = m_inBuf.get();
		switch(c)
		{
			//------------------------------------------------------------
			// ASCII rib decoding
			//------------------------------------------------------------
			case ' ':
			case '\t':
			case '\n':
				// ignore whitespace
				break;
			case '#':
				if(m_inBuf.get() == 0377)
					// "#\377" signals the end of a RunProgram block
					return CqRibToken(CqRibToken::ENDOFFILE);
				m_inBuf.unget();
				readComment();
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
			case '-': case '+': case '.':
				m_inBuf.unget();
				return readNumber();
			case '"':
				return readString();
			case '[':
				return CqRibToken(CqRibToken::ARRAY_BEGIN);
			case ']':
				return CqRibToken(CqRibToken::ARRAY_END);
			default:
				// If a character is nothing else, it's assumed to represent
				// the start of a RIB request.
				m_inBuf.unget();
				return readRequest();
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
				return CqRibToken(static_cast<TqInt>(decodeInt(m_inBuf, c - 0200 + 1)));
			case 0204: case 0205: case 0206: case 0207: // .b to bbb.b
			case 0210: case 0211: case 0212: case 0213: // ._b to bb.bb
			case 0214: case 0215: case 0216: case 0217: // .__b to b.bbb
				// Decode fixed point numbers.  The encoded token has the form
				//   0200 + 4*d + w  |  <value>
				// where <value> is w+1 bytes specifying a fixed point
				// number with the radix point ("decimal" point) located d
				// bytes into the number from the right.
				return CqRibToken(decodeFixedPoint(m_inBuf,
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
				return CqRibToken(CqRibToken::STRING, decodeString(m_inBuf, c - 0220));
			case 0240: case 0241: case 0242: case 0243:
				// Decode strings of length >= 16 bytes.  The encoded token has the form
				//   0240 + l  |  <length>  |  <string>
				// where <length> is l bytes long, 0 <= l <= 3, and <string> is
				// of length l+1.
				return CqRibToken(CqRibToken::STRING,
						decodeString(m_inBuf, decodeInt(m_inBuf, c - 0240 + 1)));
			case 0244:
				// Decode a 32-bit floating point value.  The encoded token has the form
				//   0244  |  <value>
				// where <value> is 4 bytes in IEEE floating point format,
				// transmitted from MSB to LSB
				return CqRibToken(decodeFloat32(m_inBuf));
			case 0245:
				// Decode a 64-bit floating point value.  The encoded token has the form
				//   0244  |  <value>
				// where <value> is 8 bytes in IEEE floating point format,
				// transmitted from MSB to LSB
				return CqRibToken(static_cast<TqFloat>(decodeFloat64(m_inBuf)));
			case 0246:
				// Read encoded RI request.  The encoded token has the form
				//   0246  |  <code>
				// where <code> is a single byte indexing a previously encoded
				// request.
				{
					const std::string& str
						= m_encodedRequests[static_cast<TqUint8>(m_inBuf.get())];
					if(str.empty())
						return error("encoded request not previously defined");
					else
						return CqRibToken(CqRibToken::REQUEST, str);
				}
			case 0247:
			case 0250: case 0251: case 0252: case 0253:
			case 0254: case 0255: case 0256: case 0257:
			case 0260: case 0261: case 0262: case 0263:
			case 0264: case 0265: case 0266: case 0267:
			case 0270: case 0271: case 0272: case 0273:
			case 0274: case 0275: case 0276: case 0277:
			case 0300: case 0301: case 0302: case 0303:
			case 0304: case 0305: case 0306: case 0307:
				return error("reserved byte encountered");
			case 0310: case 0311: case 0312: case 0313:
				// Decode an array of 32-bit floats.  The encoded token has the form
				//   0310 + l  |  <length>  |  <array>
				// where <length> is l bytes long, 0 <= l <= 3, and <array> is
				// an array of 32-bit floating point of length l values in IEEE format.
				m_arrayElementsRemaining = decodeInt(m_inBuf, c - 0310 + 1);
				return CqRibToken(CqRibToken::ARRAY_BEGIN);
			case 0314:
				// Define encoded RIB request.  The encoded token has the form
				//   0314  |  <code>  |  <string>
				// where <code> is the single byte index for the encoded
				// request, and <string> is a string specifying the ASCII form
				// of the request.
				{
					TqUint8 code = m_inBuf.get();
					CqRibToken requestNameTok = getToken();
					if(requestNameTok.type() != CqRibToken::STRING)
						return error("expected string missing from "
								"encoded request definition");
					m_encodedRequests[code] = requestNameTok.stringVal();
				}
				return getToken();
			case 0315: case 0316:
				// Define encoded string.  The encoded token has the form
				//   0315 + w  |  <code>  |  <string>
				// where <code> is a length w+1 integer, and <string> is a
				// string in any format.
				{
					TqInt code = decodeInt(m_inBuf, c - 0315 + 1);
					CqRibToken stringTok = getToken();
					if(stringTok.type() != CqRibToken::STRING)
						return error("expected string missing from "
								"encoded string definition");
					m_encodedStrings[code] = stringTok.stringVal();
				}
				return getToken();
			case 0317: case 0320:
				// Look up predefined string.  The encoded token has the form
				//   0317 + w  |  <code>
				// where <code> is a length w+1 unsigned integer representing
				// the index of a previously defined string.
				{
					TqEncodedStringMap::const_iterator pos = m_encodedStrings.find(
							decodeInt(m_inBuf, c - 0317 + 1));
					if(pos != m_encodedStrings.end())
						return CqRibToken(CqRibToken::STRING, pos->second);
					else
						return error("encoded string not previously defined");
				}
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
				return error("reserved byte encountered");
			case 0377:
				// 0377 signals the end of a RunProgram block when we're
				// reading from a pipe.
				// TODO: Check with Paul if this is correct.
				return CqRibToken(CqRibToken::ENDOFFILE);
		}
	}
}

void CqRibLexer::ungetToken(const CqRibToken& tok)
{
	assert(!m_haveNext);
	m_currPos = m_prevPos;
	m_haveNext = true;
	m_nextTok = tok;
}

CqRibToken CqRibLexer::readNumber()
{
	CqRibInputBuffer::TqOutputType c = 0;
	TqInt sign = 1;
	TqInt intResult = 0;
	TqFloat floatResult = 0;
	bool haveReadDigit = false;
	c = m_inBuf.get();
	// deal with optional sign
	switch(c)
	{
		case '+':
			c = m_inBuf.get();
			break;
		case '-':
			sign = -1;
			c = m_inBuf.get();
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
		c = m_inBuf.get();
	}
	switch(c)
	{
		case '.':
			{
				// deal with digits to right of decimal point
				c = m_inBuf.get();
				if(!haveReadDigit && !std::isdigit(c))
					return error("Expected at least one digit in float");
				TqFloat thisPlaceVal = 0.1;
				while(std::isdigit(c))
				{
					floatResult += (c - '0') * thisPlaceVal;
					thisPlaceVal *= 0.1;
					c = m_inBuf.get();
				}
				floatResult *= sign;
				if(c != 'e' && c != 'E')
				{
					m_inBuf.unget();
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
				return error("Expected a digit");
			m_inBuf.unget();
			return CqRibToken(intResult);
			break;
	}
	// deal with the exponent
	c = m_inBuf.get();
	sign = 1;
	switch(c)
	{
		case '+':
			c = m_inBuf.get();
			break;
		case '-':
			sign = -1;
			c = m_inBuf.get();
			break;
	}
	if(!std::isdigit(c))
		return error("Expected digits in float exponent");
	TqInt exponent = 0;
	while(std::isdigit(c))
	{
		exponent *= 10;
		exponent += c - '0';
		c = m_inBuf.get();
	}
	exponent *= sign;
	m_inBuf.unget();
	floatResult *= std::pow(10.0,exponent);
	return CqRibToken(floatResult);
}

CqRibToken CqRibLexer::readString()
{
	// Assume leading '"' has already been read.
	CqRibToken outTok(CqRibToken::STRING, "");
	std::string& outString = outTok.m_strVal;
	outString.reserve(30);
	bool stringFinished = false;
	while(!stringFinished)
	{
		CqRibInputBuffer::TqOutputType c = m_inBuf.get();
		switch(c)
		{
			case '"':
				stringFinished = true;
				break;
			case '\\':
				// deal with escape characters
				c = m_inBuf.get();
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
							c = m_inBuf.get();
							for(TqInt i = 0; i < 2 && std::isdigit(c); i++ )
							{
								octalChar = 8*octalChar + (c - '0');
								c = m_inBuf.get();
							}
							m_inBuf.unget();
							outString += octalChar;
						}
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
			case EOF:
				return error("End of file found while scanning string");
			break;
			default:
				outString += c;
		}
	}
	return outTok;
}

CqRibToken CqRibLexer::readRequest()
{
	CqRibToken outTok(CqRibToken::REQUEST, "");
	std::string& name = outTok.m_strVal;
	name.reserve(30);
	while(true)
	{
		CqRibInputBuffer::TqOutputType c = m_inBuf.get();
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
			m_inBuf.unget();
			break;
		}
	}
	return outTok;
}

void CqRibLexer::readComment()
{
	CqRibInputBuffer::TqOutputType c = m_inBuf.get();
	while(c != EOF && c != '\n')
		c = m_inBuf.get();
	m_inBuf.unget();
}

CqRibToken CqRibLexer::error(std::string message)
{
	std::ostringstream oss;
	oss << "Bad token at " << m_inBuf.pos() << ": " << message;
	return CqRibToken(CqRibToken::ERROR, oss.str());
}

} // namespace ribparse
