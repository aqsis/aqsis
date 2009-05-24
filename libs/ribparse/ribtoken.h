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

#ifndef RIBTOKEN_H_INCLUDED
#define RIBTOKEN_H_INCLUDED

#include <aqsis/aqsis.h>

#include <string>
#include <iostream>

#include <aqsis/math/math.h>

namespace Aqsis {

/** \brief A class encapsulating a RIB token
 */
class CqRibToken
{
	public:
		/// Rib token types
		enum EqType
		{
			ARRAY_BEGIN,
			ARRAY_END,
			STRING,
			INTEGER,
			FLOAT,
			REQUEST,
			ERROR,
			ENDOFFILE
		};

		//--------------------------------------------------
		/// \name Constructors
		//@{
		/// Construct a token of the given type (default value)
		CqRibToken(EqType type = ERROR);
		/// Construct a INTEGER token from the given integer value
		CqRibToken(TqInt intVal);
		/// Construct a FLOAT token from the given float value
		CqRibToken(TqFloat floatVal);
		/// Construct a token with a string as the token value
		CqRibToken(EqType type, const std::string& strVal);
		//@}

		/** Equality operator (mostly for testing purposes)
		 *
		 * \return true if the token type and data are the same.
		 */
		bool operator==(const CqRibToken& rhs) const;
		/// Format the token to an output stream.
		friend std::ostream& operator<<(std::ostream& outStream, const CqRibToken& tok);

		//--------------------------------------------------
		/// \name accessors
		//@{
		/// Get the token type.
		EqType type() const;
		/// Get an integer value from the token
		TqInt intVal() const;
		/// Get a float value from the token
		TqFloat floatVal() const;
		/// Get a string value from the token
		const std::string& stringVal() const;
		//@}

	private:
		friend class CqRibLexer;

		/// Token type
		EqType m_type;

		/// Integer value for token
		TqInt m_intVal;
		/// Float value for token
		TqFloat m_floatVal;
		/// String value for token
		std::string m_strVal;
		// note: boost::variant was tried for the mutually exclusive values
		// above, but turned out to have a performance hit of about 7% of total
		// lexer time for a simple test which read tokens from a large RIB and
		// sent them to stdout with operator<<.  (g++-4.1, boost-1.34.1)
};



//==============================================================================
// Implementation details
//==============================================================================
// CqRibToken implementation
inline CqRibToken::CqRibToken(CqRibToken::EqType type)
		: m_type(type),
		m_intVal(0),
		m_floatVal(0),
		m_strVal()
{ }

inline CqRibToken::CqRibToken(TqInt intVal)
		: m_type(INTEGER),
		m_intVal(intVal),
		m_floatVal(0),
		m_strVal()
{}

inline CqRibToken::CqRibToken(TqFloat floatVal)
		: m_type(FLOAT),
		m_intVal(0),
		m_floatVal(floatVal),
		m_strVal()
{}

inline CqRibToken::CqRibToken(CqRibToken::EqType type, const std::string& strVal)
		: m_type(type),
		m_intVal(0),
		m_floatVal(0),
		m_strVal(strVal)
{
	assert(type == STRING || type == REQUEST || type == ERROR);
}

inline bool CqRibToken::operator==(const CqRibToken& rhs) const
{
	if(m_type != rhs.m_type)
		return false;
	switch(m_type)
	{
		case ARRAY_BEGIN:
		case ARRAY_END:
		case ENDOFFILE:
		case ERROR:
		default:
			return true;
		case INTEGER:
			return m_intVal == rhs.m_intVal;
		case FLOAT:
			return isClose(m_floatVal, rhs.m_floatVal);
		case STRING:
		case REQUEST:
			return m_strVal == rhs.m_strVal;
	}
}

inline CqRibToken::EqType CqRibToken::type() const
{
	return m_type;
}

inline TqInt CqRibToken::intVal() const
{
	assert(m_type == INTEGER);
	return m_intVal;
}

inline TqFloat CqRibToken::floatVal() const
{
	assert(m_type == FLOAT);
	return m_floatVal;
}

inline const std::string& CqRibToken::stringVal() const
{
	assert(m_type == STRING || m_type == REQUEST || m_type == ERROR);
	return m_strVal;
}

inline std::ostream& operator<<(std::ostream& outStream, const CqRibToken& tok)
{
	static const char* tokenNames[] = {
		"ARRAY_BEGIN",
		"ARRAY_END",
		"STRING",
		"INTEGER",
		"FLOAT",
		"REQUEST",
		"ERROR",
		"ENDOFFILE"
	};
	outStream << tokenNames[tok.m_type];
	switch(tok.m_type)
	{
		case CqRibToken::ARRAY_BEGIN:
		case CqRibToken::ARRAY_END:
		case CqRibToken::ENDOFFILE:
			break;
		case CqRibToken::INTEGER:
			outStream << ": " << tok.m_intVal;
			break;
		case CqRibToken::FLOAT:
			outStream << ": " << tok.m_floatVal;
			break;
		case CqRibToken::STRING:
			outStream << ": \"" << tok.m_strVal << "\"";
			break;
		case CqRibToken::REQUEST:
		case CqRibToken::ERROR:
			outStream << ": " << tok.m_strVal;
			break;
	}
	return outStream;
}

} // namespace Aqsis

#endif // RIBTOKEN_H_INCLUDED
