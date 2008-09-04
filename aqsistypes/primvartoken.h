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
 */

#ifndef PRIMVARTOKEN_H_INCLUDED
#define PRIMVARTOKEN_H_INCLUDED

#include "aqsis.h"

#include <string>

#include "primvartype.h"

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief Class holding primitive variable name and type
 *
 * Each primitive variable attached to a piece of geometry in renderman has a
 * name and some associated type data:
 *
 *   - The primvar _class_ specifies how interpolation should happen over the
 *     primitive surface.
 *   - The primvar _type_ specifies the type of data (eg, "float", "vector",
 *     "matrix" etc)
 *   - The primvar _array size_ specifies the number of elements of the given
 *     type which are attached to each vertex (or other "attachment point").
 *
 */
class CqPrimvarToken
{
	public:
		/// Trivial constructor.
		CqPrimvarToken(const std::string& name, EqVariableClass Class,
				EqVariableType type, TqInt arraySize);
		/** \brief Parse type and name information from an RtToken string.
		 *
		 * In the renderman interface, strings come bundled together in an
		 * "RtToken" of format
		 *
		 *   [class]  [type]  [ '['array_size']' ]  name
		 *
		 * where the class, type and array size are optional.  This constructor
		 * parses tokens of that form.  For parts which aren't present,
		 * defaults are:
		 *   * class = class_uniform
		 *   * type = type_invalid
		 *   * array_size = 1
		 *
		 * \param token - token string to parse.
		 * \todo Cleanup this to use a const version of RtToken...
		 */
		CqPrimvarToken(const char* token);
		/** \brief Parse type information from an RtToken string
		 *
		 * \param typeToken has the form
		 *
		 *   [class]  type  [ '['array_size']' ]
		 *
		 * \param name is the primvar name, and may not be empty.
		 *
		 * \see CqPrimvarToken(const char* token)
		 */
		CqPrimvarToken(const char* typeToken, const std::string& name);

		/// \name Accessors
		//@{
		/// get the primvar name
		const std::string& name() const;
		/// get the primvar class
		EqVariableClass Class() const;
		/// get the primvar type
		EqVariableType type() const;
		/// get the primvar array size.
		TqInt arraySize() const;
		//@}

		/** \brief comparison operator for sorted containers.
		 *
		 * Having operator< implemented allows us to construct sorted
		 * containers of CqPrimvarToken's
		 */
//		bool operator<(const CqPrimvarToken rhs);
	private:
		std::string m_name;
		/// name hash for fast lookups
//		TqUlong m_nameHash;
		EqVariableClass m_class;
		EqVariableType m_type;
		TqInt m_arraySize;

		/** Parse a primitive variable token
		 *
		 * m_class, m_type, m_arraySize and m_name will be extracted if present
		 * and in the correct order.  m_class and m_type will be set to invalid
		 * if not present.
		 */
		void parse(const char* token);
};


//==============================================================================
// Implementation details
//==============================================================================

inline CqPrimvarToken::CqPrimvarToken(const std::string& name,
		EqVariableClass Class, EqVariableType type, TqInt arraySize)
	: m_name(name),
	m_class(Class),
	m_type(type),
	m_arraySize(arraySize)
{
	assert(m_arraySize > 0);
}

inline const std::string& CqPrimvarToken::name() const
{
	return m_name;
}

inline EqVariableClass CqPrimvarToken::Class() const
{
	return m_class;
}

inline EqVariableType CqPrimvarToken::type() const
{
	return m_type;
}

inline TqInt CqPrimvarToken::arraySize() const
{
	return m_arraySize;
}

} // namespace Aqsis

#endif // PRIMVARTOKEN_H_INCLUDED
