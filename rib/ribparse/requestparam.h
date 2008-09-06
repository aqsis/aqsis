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
 * \brief RI request parameter types.
 * \author Chris Foster [chris42f (at) gmail (d0t) com]
 */

#ifndef REQUESTPARAM_H_INCLUDED
#define REQUESTPARAM_H_INCLUDED

#include "aqsis.h"

#include <boost/any.hpp>

#include "exception.h"
#include "primvartoken.h"

namespace Aqsis {

//------------------------------------------------------------------------------
/// Enumeration of all possible parameter types
enum EqRiParamType
{
	ParamType_Int,
	ParamType_Float,
	ParamType_String,
	ParamType_IntArray,
	ParamType_FloatArray,
	ParamType_StringArray
};

//------------------------------------------------------------------------------
/// Metafunction from type constants to the associated types.
template<EqRiParamType ty> struct getRiParamType
{
	typedef TqInt type;
};

/** \brief A typesafe holder class for multiple request types.
 *
 * This class is intended to be passed around by value, since it should only
 * hold references to arrays rather than arrays themselves.
 */
class CqRequestParam
{
	public:
		template<typename T>
		CqRequestParam(const CqPrimvarToken& token, EqRiParamType type, T value);
		/// Get the type of the data held in the parameter.
		EqRiParamType type() const;
		/// Get the token of the data held in the parameter.
		const CqPrimvarToken& token() const;

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
		CqPrimvarToken m_token;
		EqRiParamType m_type;
		boost::any m_value;
};


/// integer array type for RI request array paramters
typedef std::vector<TqInt> TqRiIntArray;
/// float array type for RI request array paramters
typedef std::vector<TqFloat> TqRiFloatArray;
/// string array type for RI request array paramters
typedef std::vector<std::string> TqRiStringArray;

/** Parameter list of (name, value) pairs.
 *
 * TODO: This needs to be refactored to remove duplication between it and
 * CqRiParamList; we should use a common ABC, or a single concrete class if
 * possible.
 */
typedef std::vector<CqRequestParam> TqRiParamList;


//==============================================================================
// Implementation details
//==============================================================================
#define SPECIALIZE_GET_RI_PARAM_TYPE(typeEnumVal, typeName)          \
template<>                                                           \
struct getRiParamType<typeEnumVal>                                   \
{                                                                    \
	typedef typeName type;                                           \
};
SPECIALIZE_GET_RI_PARAM_TYPE(ParamType_Float, TqFloat);
SPECIALIZE_GET_RI_PARAM_TYPE(ParamType_String, std::string);
SPECIALIZE_GET_RI_PARAM_TYPE(ParamType_IntArray, const std::vector<TqInt>&);
SPECIALIZE_GET_RI_PARAM_TYPE(ParamType_FloatArray, const std::vector<TqFloat>&);
SPECIALIZE_GET_RI_PARAM_TYPE(ParamType_StringArray, const std::vector<std::string>&);
#undef SPECIALIZE_GET_RI_PARAM_TYPE


//------------------------------------------------------------------------------
template<typename T>
inline CqRequestParam::CqRequestParam(const CqPrimvarToken& token,
		EqRiParamType type, T value)
	: m_token(token),
	m_type(type),
	m_value(value)
{ }

inline EqRiParamType CqRequestParam::type() const
{
	return m_type;
}

inline const CqPrimvarToken& CqRequestParam::token() const
{
	return m_token;
}


template<EqRiParamType t>
inline typename getRiParamType<t>::type CqRequestParam::value() const
{
	// any_cast will throw in the release build.
	assert(m_type == t);
	return boost::any_cast<typename getRiParamType<t>::type>(m_value);
}


} // namespace Aqsis
#endif // REQUESTPARAM_H_INCLUDED
