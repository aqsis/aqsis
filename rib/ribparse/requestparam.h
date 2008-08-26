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
 * \brief RIB request parameter types.
 * \author Chris Foster [chris42f (at) gmail (d0t) com]
 */

#ifndef REQUESTPARAM_H_INCLUDED
#define REQUESTPARAM_H_INCLUDED

#include "aqsis.h"

#include <boost/any.hpp>

#include "exception.h"

namespace ribparse {

//------------------------------------------------------------------------------
/// Enumeration of all possible parameter types
enum EqRiParamType
{
	Param_Int,
	Param_Float,
	Param_String,
	Param_IntArray,
	Param_FloatArray,
	Param_StringArray
};

//------------------------------------------------------------------------------

// forward declaration
template<EqRiParamType t> struct getRiParamType;

/** \brief A typesafe holder class for multiple request types.
 *
 * This class is intended to be passed around by value, since it should only
 * hold references to arrays rather than arrays themselves.
 */
class CqRibRequestParam
{
	public:
		/// Get the type of the data held in the parameter.
		EqRiParamType type() const;

		/** \brief Return the value held in the parameter
		 *
		 * This function is runtime typesafe - it lets you ask for a type based
		 * on EqRiParamType, and returns the correct type if the parameter
		 * holds it.  Otherwise an error is thrown.
		 *
		 * \param typeConst - enumeration value indicating the type to return.
		 */
		template<EqRiParamType typeConst>
		typename getRiParamType<typeConst>::type value() const;
	private:
		EqRiParamType m_type;
		boost::any m_value;
};


/** \brief A RIB parameter with associated name.
 *
 * This is designed to form part of the optional name,value pair "parameter
 * list" for a RIB request.
 */
struct SqNamedRibRequestParam
{
	/// Name of the parameter
	std::string name;
	/// value for the parameter
	CqRibRequestParam param;
};

/// integer array type for RIB request array paramters
typedef std::vector<TqInt> TqRibIntArray;
/// float array type for RIB request array paramters
typedef std::vector<TqFloat> TqRibFloatArray;
/// string array type for RIB request array paramters
typedef std::vector<std::string> TqRibStringArray;
/** Parameter list of (name, value) pairs.
 *
 * TODO: This needs to be refactored to remove duplication between it and
 * CqRiParamList; we should use a common ABC, or a single concrete class if
 * possible.
 */
typedef std::vector<SqNamedRibRequestParam> TqRibParamList;


//==============================================================================
// Implementation details
//==============================================================================
template<EqRiParamType ty>
struct getRiParamType
{
	typedef TqInt type;
};
#define SPECIALIZE_GET_RI_PARAM_TYPE(typeEnumVal, typeName)          \
template<>                                                           \
struct getRiParamType<typeEnumVal>                                   \
{                                                                    \
	typedef typeName type;                                           \
};
SPECIALIZE_GET_RI_PARAM_TYPE(Param_Float, TqFloat);
SPECIALIZE_GET_RI_PARAM_TYPE(Param_String, std::string);
SPECIALIZE_GET_RI_PARAM_TYPE(Param_IntArray, const std::vector<TqInt>&);
SPECIALIZE_GET_RI_PARAM_TYPE(Param_FloatArray, const std::vector<TqFloat>&);
SPECIALIZE_GET_RI_PARAM_TYPE(Param_StringArray, const std::vector<std::string>&);
#undef SPECIALIZE_GET_RI_PARAM_TYPE


//------------------------------------------------------------------------------
inline EqRiParamType CqRibRequestParam::type() const
{
	return m_type;
}

template<EqRiParamType t>
inline typename getRiParamType<t>::type CqRibRequestParam::value() const
{
	// any_cast will throw in the release build.
	assert(m_type == t);
	return boost::any_cast<typename getRiParamType<t>::type>(m_value);
}


} // namespace ribparse
#endif // REQUESTPARAM_H_INCLUDED
