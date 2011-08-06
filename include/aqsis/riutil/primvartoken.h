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
#include <aqsis/riutil/ricxx.h>

namespace Aqsis {

/// Parse type and name information from a string.
///
/// In the renderman interface, interface types come bundled together in a
/// "token" string of format
///
///   [  [ class ]  type  [ '[' count ']' ]  ]   name
///
/// where the square brackets indicate optional components.  This function
/// parses tokens of that form.  For parts of the token which aren't present,
/// defaults are:
///   * class = Uniform
///   * type = Unknown
///   * count = 1  (array size)
///
/// If nameEnd is non-null, it's considered an error for the name token not to
/// be present; otherwise it is optional.  The name part of the token string is
/// returned as the subrange [nameStart,nameEnd)
///
/// If an error is detected during parsing (ie., the token doesn't have the
/// format described above), it is stored into the error string when the error
/// parameter is non-null.  If error is null, an XqParseError is thrown
/// instead.
///
AQSIS_RIUTIL_SHARE
Ri::TypeSpec parseDeclaration(const char* token,
						      const char** nameStart = 0,
						      const char** nameEnd = 0,
						      const char** error = 0);

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
		/// Construct from the given typespec & name.
		CqPrimvarToken(const Ri::TypeSpec& spec, const std::string& name);
		/// Construct using type information from token
		///
		/// \see parseDeclaration
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
};

/// Stream insertion operator.
inline std::ostream& operator<<(std::ostream& out, const CqPrimvarToken& tok);


/// Convert an Ri::TypeSpec to the types used in CqPrimvarToken
///
/// iclass and type are used to return the associated EqVariableClass and
/// EqVariableType respectively.  Both of these are optional; they are ignored
/// if they're NULL.
AQSIS_RIUTIL_SHARE
void typeSpecToEqTypes(EqVariableClass* iclass, EqVariableType* type,
					   const Ri::TypeSpec& spec);

/// Convert type portion of CqPrimvarToken to Ri::TypeSpec
///
/// This function exists for compatibility between the older CqPrimvarToken
/// which is used internally and the public-interface Ri::TypeSpec.
AQSIS_RIUTIL_SHARE
Ri::TypeSpec toTypeSpec(const CqPrimvarToken& tok);


/// Get the token string corresponding to param.spec() and param.name()
AQSIS_RIUTIL_SHARE
std::string tokenString(const Ri::Param& param);

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
	out << tok.Class() << " " << tok.type();
	if(tok.count() != 1)
		out << "[" << tok.count() << "]";
	out << " " << tok.name();
	return out;
}

} // namespace Aqsis

#endif // PRIMVARTOKEN_H_INCLUDED
