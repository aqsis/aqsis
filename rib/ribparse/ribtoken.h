// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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

#include "aqsis.h"

#include <string>

#include <boost/intrusive_ptr.hpp>

namespace ribparse {

/** \brief A class encapsulating a RIB token
 */
class CqRibToken
{
	public:
		/// Rib token types
		enum EqType
		{
			BEGIN_ARRAY,
			END_ARRAY,
			STRING,
			INTEGER,
			FLOAT,
			REQUEST,
			COMMENT,
			ERROR,
			ENDOFFILE,
			NONE
		};

		//--------------------------------------------------
		/// \name Constructors
		//@{
		/// Default constructor - initializes with NONE
		CqRibToken();
		/// Construct a INTEGER token from the given integer value
		CqRibToken(TqInt intVal);
		/// Construct a FLOAT token from the given float value
		CqRibToken(TqFloat floatVal);
		/// Construct a token with a string as the token value
		CqRibToken(EqType type, const std::string& strVal);
		/// Construct a token of the given type (no value)
		CqRibToken(EqType type);
		/// Destructor
		~CqRibToken();
		//@}

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
		class SqStringHolder;

		/// Token type
		EqType m_type;
		/// Integer value for token
		TqInt m_intVal;
		/// Float value for token
		TqFloat m_floatVal;
		/// String value for token
		boost::intrusive_ptr<const SqStringHolder> m_strVal;
};



//==============================================================================
// Implementation details
//==============================================================================

/// Struct combining a std::string with intrusive reference counting
struct CqRibToken::SqStringHolder
{
	public:
		std::string str;

		SqStringHolder(const std::string& str)
			: str(str)
		{ }
	private:
		mutable TqInt m_refCount;

		/// Increase the reference count; required for boost::intrusive_ptr
		friend inline void intrusive_ptr_add_ref(const SqStringHolder* ptr)
		{
			++ptr->m_refCount;
		}
		/// Decrease the reference count; required for boost::intrusive_ptr
		friend inline void intrusive_ptr_release(const SqStringHolder* ptr)
		{
			if(--ptr->m_refCount == 0)
				delete ptr;
		}
};

//--------------------------------------------------
// CqRibToken implementation
inline CqRibToken::CqRibToken()
		: m_type(NONE),
		m_intVal(0),
		m_floatVal(0),
		m_strVal()
{}

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
		m_strVal(new SqStringHolder(strVal))
{
	assert(type == STRING || type == REQUEST || type == COMMENT);
}

inline CqRibToken::CqRibToken(CqRibToken::EqType type)
		: m_type(type),
		m_intVal(0),
		m_floatVal(0),
		m_strVal()
{
	assert(type == BEGIN_ARRAY || type == END_ARRAY || type == ENDOFFILE
			|| type == NONE);
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
	assert(m_type == STRING || m_type == REQUEST || m_type == COMMENT);
	return m_strVal->str;
}

} // namespace ribparse

#endif // RIBTOKEN_H_INCLUDED
