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
 * \brief A RIB token class.
 * \author Chris Foster  [chris42f (at) gmail (dot) com]
 */

#ifndef RIBTOKEN_H_INCLUDED
#define RIBTOKEN_H_INCLUDED

#include <string>
#include <iostream>

#include <aqsis/math/math.h>

namespace Aqsis {

/** \brief A class encapsulating a RIB token
 */
class RibToken
{
	public:
		/// Rib token types
		enum Type
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
		/// Construct a token of the given type (default ERROR)
		RibToken(Type type = ERROR);
		/// Construct a INTEGER token from the given integer value
		RibToken(int intVal);
		/// Construct a FLOAT token from the given float value
		RibToken(float floatVal);
		/// Construct a token with a string as the token value
		RibToken(Type type, const std::string& strVal);
		//@}

		/// Assign to the given type
		RibToken& operator=(Type type);
		RibToken& operator=(int i);
		RibToken& operator=(float f);
		/// Set the token to the error type, with the given error message
		void error(const char* message);

		/** Equality operator (mostly for testing purposes)
		 *
		 * \return true if the token type and data are the same.
		 */
		bool operator==(const RibToken& rhs) const;
		/// Format the token to an output stream.
		friend std::ostream& operator<<(std::ostream& outStream, const RibToken& tok);

		//--------------------------------------------------
		/// \name accessors
		//@{
		/// Get the token type.
		Type type() const;
		/// Get an integer value from the token
		int intVal() const;
		/// Get a float value from the token
		float floatVal() const;
		/// Get a string value from the token
		const std::string& stringVal() const;
		//@}

	private:
		friend class RibTokenizer;

		/// Token type
		Type m_type;

		/// Integer value for token
		int m_intVal;
		/// Float value for token
		float m_floatVal;
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
// RibToken implementation
inline RibToken::RibToken(RibToken::Type type)
		: m_type(type),
		m_intVal(0),
		m_floatVal(0),
		m_strVal()
{ }

inline RibToken::RibToken(int intVal)
		: m_type(INTEGER),
		m_intVal(intVal),
		m_floatVal(0),
		m_strVal()
{}

inline RibToken::RibToken(float floatVal)
		: m_type(FLOAT),
		m_intVal(0),
		m_floatVal(floatVal),
		m_strVal()
{}

inline RibToken::RibToken(RibToken::Type type, const std::string& strVal)
		: m_type(type),
		m_intVal(0),
		m_floatVal(0),
		m_strVal(strVal)
{
	assert(type == STRING || type == REQUEST || type == ERROR);
}

inline RibToken& RibToken::operator=(Type type)
{
	m_type = type;
	return *this;
}

inline RibToken& RibToken::operator=(int i)
{
	m_type = INTEGER;
	m_intVal = i;
	return *this;
}

inline RibToken& RibToken::operator=(float f)
{
	m_type = FLOAT;
	m_floatVal = f;
	return *this;
}

inline void RibToken::error(const char* message)
{
	m_type = ERROR;
	m_strVal = message;
}

inline bool RibToken::operator==(const RibToken& rhs) const
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

inline RibToken::Type RibToken::type() const
{
	return m_type;
}

inline int RibToken::intVal() const
{
	assert(m_type == INTEGER);
	return m_intVal;
}

inline float RibToken::floatVal() const
{
	assert(m_type == FLOAT);
	return m_floatVal;
}

inline const std::string& RibToken::stringVal() const
{
	assert(m_type == STRING || m_type == REQUEST || m_type == ERROR);
	return m_strVal;
}

inline std::ostream& operator<<(std::ostream& outStream, const RibToken& tok)
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
		case RibToken::ARRAY_BEGIN:
		case RibToken::ARRAY_END:
		case RibToken::ENDOFFILE:
			break;
		case RibToken::INTEGER:
			outStream << ": " << tok.m_intVal;
			break;
		case RibToken::FLOAT:
			outStream << ": " << tok.m_floatVal;
			break;
		case RibToken::STRING:
			outStream << ": \"" << tok.m_strVal << "\"";
			break;
		case RibToken::REQUEST:
		case RibToken::ERROR:
			outStream << ": " << tok.m_strVal;
			break;
	}
	return outStream;
}

} // namespace Aqsis

#endif // RIBTOKEN_H_INCLUDED
