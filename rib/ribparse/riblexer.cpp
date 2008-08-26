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

#include <string>
#include <sstream>
#include <cmath>
#include <ctype.h>
#include <iostream>

#include "riblexer.h"

namespace ribparse
{

//-------------------------------------------------------------------------------
// CqRibLexer implementation

CqRibLexer::CqRibLexer(std::istream& inStream)
	: m_inBuf(inStream),
	m_currPos(1,1),
	m_prevPos(0,0),
	m_nextTok(),
	m_haveNext(false)
{}

CqRibToken CqRibLexer::getToken()
{
	m_prevPos = m_currPos;
	// Return the next token directly if we already have it.
	if(m_haveNext)
	{
		m_haveNext = false;
		return m_nextTok;
	}
	// Else determine the next token.
	while(true)
	{
		m_currPos = m_inBuf.pos();
		CqRibInputBuffer::TqOutputType c = m_inBuf.get();
		switch(c)
		{
			case ' ':
			case '\t':
			case '\n':
				// ignore whitespace
				break;
			case '#':
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
				return CqRibToken(CqRibToken::BEGIN_ARRAY);
			case ']':
				return CqRibToken(CqRibToken::END_ARRAY);
			case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
			case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
			case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
			case 's': case 't': case 'u': case 'v': case 'w': case 'x':
			case 'y': case 'z': case 'A': case 'B': case 'C': case 'D':
			case 'E': case 'F': case 'G': case 'H': case 'I': case 'J':
			case 'K': case 'L': case 'M': case 'N': case 'O': case 'P':
			case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V':
			case 'W': case 'X': case 'Y': case 'Z':
				m_inBuf.unget();
				return readRequest();
			case EOF:
				return CqRibToken(CqRibToken::ENDOFFILE);
			default:
				return error("unrecognized character");
		}
		// TODO: integrate binary rib handling
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
	// TODO: Think of a better way to handle the overflow below robustly?
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
	std::string outString;
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
	return CqRibToken(CqRibToken::STRING, outString);
}

CqRibToken CqRibLexer::readRequest()
{
	// TODO: use a better buffer than a plain old std::string?
	std::string name;
	name.reserve(30);
	while(true)
	{
		CqRibInputBuffer::TqOutputType c = m_inBuf.get();
		if(std::isalpha(c))
			name += c;
		else
		{
			m_inBuf.unget();
			break;
		}
	}
	return CqRibToken(CqRibToken::REQUEST, name);
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
