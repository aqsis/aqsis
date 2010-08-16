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

#include <aqsis/riutil/primvartoken.h>

#include <cstring>

#include <aqsis/util/exception.h>

namespace Aqsis {

namespace {

// Get the next token from in the string "begin"
//
// On entry, end should hold the start of the string in which to search, begin
// is ignored.
//
// On return, the range [begin,end) holds the next token, which is a substring
// delimited by whitespace, or the characters '[' ']'.  The delimiter
// characters [ and ] are considered tokens themselves, and returned when
// found.
//
// Return true if the token is valid, indicating a nonempty returned range.
bool nextToken(const char*& begin, const char*& end)
{
	const char* c = end;
	while(*c && std::strchr(" \t\n", *c)) ++c; // skip whitespace
	begin = c;
	if(*c == '[' || *c == ']')
		++c; // '[' and ']' are "kept delimiters"
	else
		while(*c && !std::strchr(" []\t\n", *c)) ++c; // skip to end of token
	end = c;
	return begin != end;
}

// Could use std::tolower... but it's nice that this is inline & we don't care
// about locales for RIB.
inline char tolower(char c)
{
	if(c >= 65 && c < 91) c += 32;
	return c;
}

// Return true if the lowercase version of the string range [s1,s1end) is equal
// to the string s2.
bool lowerCaseEqual(const char* s1, const char* s1end, const char* s2)
{
	while(s1 < s1end && *s2)
	{
		if(tolower(*s1) != *s2)
			return false;
		++s1; ++s2;
	}
	return s1 == s1end && *s2 == 0;
}

// Get the Ri::TypeSpec interpolation class from the given string range [begin,end)
bool parseIClass(const char* begin, const char* end, Ri::TypeSpec::IClass& iclass)
{
	     if(lowerCaseEqual(begin, end, "constant"))    iclass = Ri::TypeSpec::Constant;
	else if(lowerCaseEqual(begin, end, "uniform"))     iclass = Ri::TypeSpec::Uniform;
	else if(lowerCaseEqual(begin, end, "varying"))     iclass = Ri::TypeSpec::Varying;
	else if(lowerCaseEqual(begin, end, "vertex"))      iclass = Ri::TypeSpec::Vertex;
	else if(lowerCaseEqual(begin, end, "facevarying")) iclass = Ri::TypeSpec::FaceVarying;
	else if(lowerCaseEqual(begin, end, "facevertex"))  iclass = Ri::TypeSpec::FaceVertex;
	else
		return false;
	return true;
}

// Get the Ri::TypeSpec type from the given string range [begin,end)
bool parseType(const char* begin, const char* end, Ri::TypeSpec::Type& type)
{
	     if(lowerCaseEqual(begin, end, "float"))    type = Ri::TypeSpec::Float;
	else if(lowerCaseEqual(begin, end, "point"))    type = Ri::TypeSpec::Point;
	else if(lowerCaseEqual(begin, end, "color"))    type = Ri::TypeSpec::Color;
	else if(lowerCaseEqual(begin, end, "integer"))  type = Ri::TypeSpec::Integer;
	else if(lowerCaseEqual(begin, end, "int"))      type = Ri::TypeSpec::Integer;
	else if(lowerCaseEqual(begin, end, "string"))   type = Ri::TypeSpec::String;
	else if(lowerCaseEqual(begin, end, "vector"))   type = Ri::TypeSpec::Vector;
	else if(lowerCaseEqual(begin, end, "normal"))   type = Ri::TypeSpec::Normal;
	else if(lowerCaseEqual(begin, end, "hpoint"))   type = Ri::TypeSpec::HPoint;
	else if(lowerCaseEqual(begin, end, "matrix"))   type = Ri::TypeSpec::Matrix;
	else if(lowerCaseEqual(begin, end, "mpoint"))   type = Ri::TypeSpec::MPoint;
	else
		return false;
	return true;
}


} // unnamed namespace


//------------------------------------------------------------------------------
// public stuff

#define PARSE_ERROR(token, message)                            \
	AQSIS_THROW_XQERROR(XqParseError, EqE_BadToken,            \
		"invalid token \"" << token << "\": " << message)      \

Ri::TypeSpec parseDeclaration(const char* token, const char** nameStart,
						      const char** nameEnd, const char** error)
{
	const char* begin = 0;
	const char* end = token;  // setup for nextToken.

	// The tokens should have the form
	//
	// class type '[' array_size ']' name
	//
	// where each of the four parts is optional.
	Ri::TypeSpec spec;

#	define RETURN_ERROR(message) do {                                  \
		if(error)                                                      \
		{                                                              \
			*error = message;                                          \
			return spec;                                               \
		}                                                              \
		else                                                           \
			PARSE_ERROR(token, message);                               \
	} while(false)

	bool parsedClass = false;
	bool parsedType = false;
	bool parsedArraylen = false;

#	define NEXT_OR_END do {                                            \
	if(!nextToken(begin, end))                                         \
	{                                                                  \
		if(nameEnd)                                                    \
			RETURN_ERROR("expected token name");                       \
		if(!parsedType && (parsedClass || parsedArraylen))             \
			RETURN_ERROR("type expected");                             \
		return spec;                                                   \
	} } while(false)

	NEXT_OR_END;
	// (1) attempt to parse class
	parsedClass = parseIClass(begin, end, spec.iclass);
	if(parsedClass)
		NEXT_OR_END;
	// (2) attempt to parse type
	parsedType = parseType(begin, end, spec.type);
	if(parsedType)
		NEXT_OR_END;
	// (3) attempt to parse array size
	if(*begin == '[')
	{
		if(!nextToken(begin, end))
			RETURN_ERROR("expected array size after '['");
		char* convEnd = 0;
		spec.arraySize = std::strtol(begin, &convEnd, 10);
		if(convEnd != end)
			RETURN_ERROR("array size must be an integer");
		// Consume a "]" token
		if(!nextToken(begin, end) || *begin != ']')
			RETURN_ERROR("expected ']' after array size");
		parsedArraylen = true;
		NEXT_OR_END;
	}
	if(*begin == ']')
		RETURN_ERROR("unexpected ]");
	// (4) anything remaining corresponds to the name.
	if(nameStart)
		*nameStart = begin;
	if(nameEnd)
		*nameEnd = end;
	if(!parsedType && (parsedClass || parsedArraylen))
		RETURN_ERROR("type expected");

	// Finally check that we've run out of tokens.
	if(nextToken(begin, end))
		RETURN_ERROR("too many words in token");
	return spec;
#undef NEXT_OR_END
#undef RETURN_ERROR
}

CqPrimvarToken::CqPrimvarToken(const Ri::TypeSpec& spec,
							   const std::string& name)
	: m_class(class_invalid),
	m_type(type_invalid),
	m_count(-1),
	m_name(name)
{
	typeSpecToEqTypes(m_class, m_type, m_count, spec);
}

CqPrimvarToken::CqPrimvarToken(const char* token)
	: m_class(class_invalid),
	m_type(type_invalid),
	m_count(-1),
	m_name()
{
	assert(token != 0);
	const char* beginName = 0;
	const char* endName = 0;
	Ri::TypeSpec spec = parseDeclaration(token, &beginName, &endName);
	m_name.assign(beginName, endName);
	typeSpecToEqTypes(m_class, m_type, m_count, spec);
}

CqPrimvarToken::CqPrimvarToken(const char* typeToken, const char* name)
	: m_class(class_invalid),
	m_type(type_invalid),
	m_count(1),
	m_name()
{
	assert(typeToken != 0);
	const char* beginName = 0;
	Ri::TypeSpec spec = parseDeclaration(typeToken, &beginName);
	if(beginName)
		AQSIS_THROW_XQERROR(XqParseError, EqE_BadToken,
			"invalid token: unexpected name \"" << beginName << "\" in type string \""
			<< typeToken << "\"");
	m_name = name;
	typeSpecToEqTypes(m_class, m_type, m_count, spec);
}


//------------------------------------------------------------------------------
void typeSpecToEqTypes(EqVariableClass& iclass, EqVariableType& type,
					   int& count, const Ri::TypeSpec& spec)
{
    switch(spec.type)
    {
        case Ri::TypeSpec::Float:   type = type_float;   break;
        case Ri::TypeSpec::Point:   type = type_point;   break;
        case Ri::TypeSpec::Vector:  type = type_vector;  break;
        case Ri::TypeSpec::Normal:  type = type_normal;  break;
        case Ri::TypeSpec::HPoint:  type = type_hpoint;  break;
        case Ri::TypeSpec::Matrix:  type = type_matrix;  break;
        case Ri::TypeSpec::Color:   type = type_color;   break;
        case Ri::TypeSpec::Integer: type = type_integer; break;
        case Ri::TypeSpec::String:  type = type_string;  break;
        default:                    type = type_invalid; break;
    }
    switch(spec.iclass)
    {
        case Ri::TypeSpec::Constant:    iclass = class_constant;    break;
        case Ri::TypeSpec::Uniform:     iclass = class_uniform;     break;
        case Ri::TypeSpec::Varying:     iclass = class_varying;     break;
        case Ri::TypeSpec::Vertex:      iclass = class_vertex;      break;
        case Ri::TypeSpec::FaceVarying: iclass = class_facevarying; break;
        case Ri::TypeSpec::FaceVertex:  iclass = class_facevertex;  break;
		default:                        iclass = class_invalid;     break;
    }
	count = spec.arraySize;
}

Ri::TypeSpec toTypeSpec(const CqPrimvarToken& tok)
{
    Ri::TypeSpec spec;
    switch(tok.type())
    {
        case type_float:   spec.type = Ri::TypeSpec::Float;   break;
        case type_point:   spec.type = Ri::TypeSpec::Point;   break;
        case type_vector:  spec.type = Ri::TypeSpec::Vector;  break;
        case type_normal:  spec.type = Ri::TypeSpec::Normal;  break;
        case type_hpoint:  spec.type = Ri::TypeSpec::HPoint;  break;
        case type_matrix:  spec.type = Ri::TypeSpec::Matrix;  break;
        case type_color:   spec.type = Ri::TypeSpec::Color;   break;
        case type_integer: spec.type = Ri::TypeSpec::Integer; break;
        case type_string:  spec.type = Ri::TypeSpec::String;  break;
        default:           spec.type = Ri::TypeSpec::Unknown; break;
    }
    switch(tok.Class())
    {
        case class_constant:    spec.iclass = Ri::TypeSpec::Constant; break;
        case class_uniform:     spec.iclass = Ri::TypeSpec::Uniform;  break;
        case class_varying:     spec.iclass = Ri::TypeSpec::Varying;  break;
        case class_vertex:      spec.iclass = Ri::TypeSpec::Vertex;   break;
        case class_facevarying: spec.iclass = Ri::TypeSpec::FaceVarying; break;
        case class_facevertex:  spec.iclass = Ri::TypeSpec::FaceVertex;  break;
        default:                spec.type = Ri::TypeSpec::Unknown;    break;
    }
    spec.arraySize = tok.count();
    return spec;
}

std::string tokenString(const Ri::Param& param)
{
	std::ostringstream oss;
	oss << CqPrimvarToken(param.spec(), param.name());
	return oss.str();
}

} // namespace Aqsis
