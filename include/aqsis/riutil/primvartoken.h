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

#include <aqsis/aqsis.h>

#include <string>

#include <aqsis/riutil/interpclasscounts.h>
#include <aqsis/riutil/primvartype.h>

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
class AQSIS_RIUTIL_SHARE CqPrimvarToken
{
	public:

		/** \brief Default constructor.
		 * Equivalent to CqPrimvarToken(class_invalid, type_invalid, 0, "")
		 */
		CqPrimvarToken();
		/// Trivial constructor.
		CqPrimvarToken(EqVariableClass Class, EqVariableType type,
				TqInt count, const std::string& name);
		/** \brief Parse type and name information from an RtToken string.
		 *
		 * In the renderman interface, strings come bundled together in an
		 * "RtToken" of format
		 *
		 *   [  [ class ]  type  [ '[' count ']' ]  ]   name
		 *
		 * where the square brackets indicate optional components.  This
		 * constructor parses tokens of that form.  For parts of the token
		 * which aren't present, defaults are:
		 *   * class = class_uniform  (or class_invaild if type == type_invalid)
		 *   * type = type_invalid
		 *   * count = 1  (array size)
		 *
		 * \throw XqParseError whenever the token fails to have the format
		 * shown above.
		 *
		 * \param token - token string to parse.
		 * \todo Cleanup this to use a const version of RtToken...
		 */
		explicit CqPrimvarToken(const char* token);
		/** \brief Parse type information from an RtToken string
		 * 
		 * \throw XqParseError if typeToken fails to have the required form.
		 *
		 * \param typeToken has the form  [ [class]  type  [ '['array_size']' ] ]
		 * \param name is the primvar name, and may not be empty.
		 *
		 * \see CqPrimvarToken(const char* token)
		 */
		CqPrimvarToken(const char* typeToken, const char* name);

		//--------------------------------------------------
		/// \name Accessors
		//@{
		/// get the primvar name
		const std::string& name() const;
		/// get the primvar class
		EqVariableClass Class() const;
		/// get the primvar type
		EqVariableType type() const;
		/// get the primvar array size.  Non-arrays have a count of 1.
		TqInt count() const;
		//@}

		//--------------------------------------------------
		/// \name Storage information for RI array-representation
		//@{
		/** \brief Number of float/int/string elements needed to represent a
		 * single value of the token type.
		 *
		 * \param numColorComponents - The number of color components.  This
		 * is three by default, but may be more or less in principle since the
		 * renderman interface has facility to deal with spectral color.
		 *
		 * \return The number of float/int/strings needed to store a single
		 * element of a primitive variable of this type and array length.  For
		 * example, a "vector[2]" would need 3*2 floats per element.
		 */
		TqInt storageCount(TqInt numColorComponents = 3) const;
		/** \brief Number of float/int/string elements needed to represent a
		 * primvar of the token type.
		 *
		 * \param classCounts - counts for the various interpolation classes
		 * \param numColorComponents - The number of color components.
		 */
		TqInt storageCount(const SqInterpClassCounts& classCounts,
				TqInt numColorComponents = 3) const;
		/** \brief Return the type of array required to store a variable of this type.
		 *
		 * The result is one of type_float, type_integer, type_string, or
		 * type_invalid.  For the first three, values of the token type can be
		 * stored in an array of the corresponding type: TqFloat, TqInt or
		 * std::string.
		 */
		EqVariableType storageType() const;
		//@}

		/** \brief Determine whether the token is correctly typed.
		 *
		 * A token is incorrectly typed when the class and type are not fully
		 * specified.  For example, tokens which come from a string which only
		 * specifies the name leave the type unspecified.
		 */
		bool hasType() const;
		/** \brief Compare two tokens for equality
		 *
		 * Tokens are equal if their name, size and array counts are equal.
		 */
		bool operator==(const CqPrimvarToken& rhs) const;
	private:
		EqVariableClass m_class;
		EqVariableType m_type;
		TqInt m_count;
		std::string m_name;

		void parse(const char* token);
};

/// Stream insertion operator.
inline std::ostream& operator<<(std::ostream& out, const CqPrimvarToken& tok);

//==============================================================================
// Implementation details
//==============================================================================

inline CqPrimvarToken::CqPrimvarToken()
	: m_class(class_invalid),
	m_type(type_invalid),
	m_count(1),
	m_name()
{ }

inline CqPrimvarToken::CqPrimvarToken(EqVariableClass Class, EqVariableType type,
		TqInt count, const std::string& name)
	: m_class(Class),
	m_type(type),
	m_count(count),
	m_name(name)
{
	assert(m_count >= 0);
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

inline TqInt CqPrimvarToken::count() const
{
	return m_count;
}

inline TqInt CqPrimvarToken::storageCount(TqInt numColorComponents) const
{
	TqInt typeCount = 0;
	switch(m_type)
	{
		case type_float:
		case type_integer:
		case type_bool:
		case type_string:
			typeCount = 1;
			break;
		case type_triple:
		case type_point:
		case type_normal:
		case type_vector:
			typeCount = 3;
			break;
		case type_color:
			typeCount = numColorComponents;
			break;
		case type_hpoint:
			typeCount = 4;
			break;
		case type_matrix:
		case type_sixteentuple:
			typeCount = 16;
			break;
		case type_void:
			break;
		default:
			assert(0 && "storage length unknown for type");
			break;
	}
	return typeCount*m_count;
}

inline TqInt CqPrimvarToken::storageCount(const SqInterpClassCounts& classCounts,
		TqInt numColorComponents) const
{
	TqInt singleVarCount = storageCount(numColorComponents);
	switch(m_class)
	{
		case class_constant:    return singleVarCount;
		case class_uniform:     return singleVarCount*classCounts.uniform;
		case class_varying:     return singleVarCount*classCounts.varying;
		case class_vertex:      return singleVarCount*classCounts.vertex;
		case class_facevarying: return singleVarCount*classCounts.facevarying;
		case class_facevertex:  return singleVarCount*classCounts.facevertex;
		default:
			assert(0 && "Can't get storage count for class_invalid");
			return 0;
	}
}

inline EqVariableType CqPrimvarToken::storageType() const
{
	switch(m_type)
	{
		// Parameters representable as float arrays
		case type_float:
		case type_point:
		case type_vector:
		case type_normal:
		case type_hpoint:
		case type_matrix:
		case type_color:
			return type_float;
		// Parameters representable as integer arrays
		case type_bool:
		case type_integer:
			return type_integer;
		// Parameters representable as string arrays
		case type_string:
			return type_string;
		// Invalid parameters.
		case type_invalid:
		case type_void:
		case type_triple:
		case type_sixteentuple:
		default:
			return type_invalid;
	}
}

inline bool CqPrimvarToken::hasType() const
{
	return m_type != type_invalid && m_class != class_invalid;
}

inline bool CqPrimvarToken::operator==(const CqPrimvarToken& rhs) const
{
	return m_type == rhs.m_type && m_class == rhs.m_class
		&& m_count == rhs.m_count && m_name == rhs.m_name;
}

inline std::ostream& operator<<(std::ostream& out, const CqPrimvarToken& tok)
{
	out << tok.Class() << " " << tok.type() << "[" << tok.count() << "] "
		<< tok.name();
	return out;
}

} // namespace Aqsis

#endif // PRIMVARTOKEN_H_INCLUDED
