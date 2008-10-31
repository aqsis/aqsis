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
 *
 * \brief Primitive variable token parsing machinary.
 * \author Chris Foster [chris42f (at) g mail (d0t) com]
 * \author loosely based on CqInlineParse by Lionel J. Lacour (intuition01@online.fr)
 */

#include "primvartoken.h"

#include <boost/algorithm/string/case_conv.hpp>

#include "exception.h"

namespace Aqsis {

namespace {

/** Tokenizer class for primvar class/type/arraysize tokens.
 */
class CqPrimvarTokenizer
{
	private:
		const char* m_currPos;
	public:
		CqPrimvarTokenizer(const char* tokenStr)
			: m_currPos(tokenStr)
		{ }
		/** \brief Get the next token.
		 *
		 *
		 * \return The next token.  These consist of pretty much anything other
		 * than whitespace or the two characters "# .  The characters [ and ]
		 * are treated as tokens in their own right.
		 */
		std::string get()
		{
			const char* m_wordBegin = m_currPos;
			while(true)
			{
				switch(*m_currPos)
				{
					case ' ': case '\t': case '\n':
						if(m_wordBegin < m_currPos)
							return std::string(m_wordBegin, m_currPos);
						m_wordBegin = m_currPos + 1;
						break;
					case 0:
						return std::string(m_wordBegin, m_currPos);
					case '#': case '"':
						AQSIS_THROW(XqParseError, "invalid character '"
								<< *m_currPos << "' in primvar type declaration");
						break;
					case '[': case ']':
						if(m_wordBegin < m_currPos)
						{
							// return the token which was ended by '[' or ']'
							return std::string(m_wordBegin, m_currPos);
						}
						else
						{
							// [ and ] are "kept delimiters" - return one of them.
							++m_currPos;
							return std::string(m_currPos-1, m_currPos);
						}
					default:
						break;
				}
				++m_currPos;
			}
		}
};

/// Convert a token to lower case before passing it to enumCast.
template<typename EnumT>
EnumT lowercaseEnumCast(std::string tok)
{
	boost::to_lower(tok);
	return enumCast<EnumT>(tok);
}

} // unnamed namespace


// Helper for uniformly formatted errors.
#define PARSE_ERROR(token, message) \
	AQSIS_THROW(XqParseError, "invalid primvar \"" << token << "\": " << message)

/** Parse a primitive variable token
 *
 * m_class, m_type, m_arraySize and m_name will be extracted if present
 * and in the correct order.  m_class and m_type will be set to invalid
 * if not present.
 */
void CqPrimvarToken::parse(const char* tokenStr)
{
	CqPrimvarTokenizer tokenizer(tokenStr);
	std::string tok;

#define NEXT_OR_END if((tok = tokenizer.get()) == "") return

	// The tokens should have the form
	//
	// class type '[' array_size ']' name
	//
	// where each of the four parts is optional.

	// (1) attempt to parse class
	NEXT_OR_END;
	m_class = lowercaseEnumCast<EqVariableClass>(tok);
	if(m_class != class_invalid)
		NEXT_OR_END;
	// (2) attempt to parse type
	m_type = lowercaseEnumCast<EqVariableType>(tok);
	if(m_type != type_invalid)
	{
		NEXT_OR_END;
	}
	else if(tok == "int")
	{
		// Kludge - support "int" for type_integer, even though it's not
		// standard since some content expects it.
		m_type = type_integer;
		NEXT_OR_END;
	}
	// (3) attempt to parse array size
	if(tok == "[")
	{
		tok = tokenizer.get();
		if(tok == "")
			PARSE_ERROR(tokenStr, "expected array size after '['");
		// Convert to integer.
		std::istringstream in(tok);
		in >> m_arraySize;
		// check array size is positive and nothing is left over in the token.
		if(m_arraySize <= 0 || in.get() != EOF)
			PARSE_ERROR(tokenStr, "array size must be a positive integer");
		// Consume a "]" token
		if(tokenizer.get() != "]")
			PARSE_ERROR(tokenStr, "expected ']' after array size");
		NEXT_OR_END;
	}
	if(tok == "]")
		PARSE_ERROR(tokenStr, "']' is not a valid name");
	// (4) anything remaining corresponds to the name.
	m_name = tok;

#undef NEXT_OR_END

	// Finally check that we've run out of tokens.
	if(tokenizer.get() != "")
		PARSE_ERROR(tokenStr, "too many words in token");
}

//-----------------------------------------
// public methods

CqPrimvarToken::CqPrimvarToken(const char* token)
	: m_class(class_invalid),
	m_type(type_invalid),
	m_arraySize(0),
	m_name()
{
	if(token)
		parse(token);
	if(m_class == class_invalid)
		m_class = class_uniform;
	if(m_name == "")
		PARSE_ERROR(token, "expected name in token");
}

CqPrimvarToken::CqPrimvarToken(const char* typeToken, const std::string& name)
	: m_class(class_invalid),
	m_type(type_invalid),
	m_arraySize(0),
	m_name()
{
	if(typeToken)
		parse(typeToken);
	if(m_name != "")
		AQSIS_THROW(XqParseError, "invalid primvar: unexpected name \""
				<< m_name << "\" in type string \"" << typeToken << "\"");
	m_name = name;
	if(m_class == class_invalid)
		m_class = class_uniform;
}

} // namespace Aqsis
