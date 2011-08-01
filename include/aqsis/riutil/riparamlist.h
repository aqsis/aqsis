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
 * \brief Class wrapping renderman parameter lists in a C++ interface.
 *
 * \author Chris Foster [chris42f (at) gmail (d0t) com]
 */

#ifndef RIPARAMLIST_H_INCLUDED
#define RIPARAMLIST_H_INCLUDED

#include <aqsis/aqsis.h>

#include <vector>
#include <string>

#include <aqsis/ri/ritypes.h>
#include <aqsis/riutil/ricxx.h>

namespace Aqsis {

/** \brief Encapsulate a pair of renderman (token,value) parameter arrays.
 *
 * The C interface specifies optional parameters in the Ri*V() functions via a
 * pair of arrays, along with a parameter count.  This interface can be
 * improved upon when passing the parameters around between C++ functions.
 * This class is an attempt to make a more friendly encapsulation of the
 * parameter list interface, and should be particularly useful for passing
 * options between renderman interface calls and the non-core libraries such as
 * aqsistex.
 *
 * \deprecated Use the more recent Ri::ParamList instead.  This is just a
 * wrapper for now.
 */
class CqRiParamList
{
	public:
		/** \brief Construct the parameter list from the associated C-interface types.
		 *
		 * \param tokens - array of parameter description strings ("tokens")
		 * \param values - array of value pointers for the parameters
		 * \param count - length of the tokens and values arrays.
		 */
		CqRiParamList(const Ri::ParamList& pList);

		/** \brief Find a pointer for the given token name
		 *
		 * \param name - parameter name to search for.
		 * \return A pointer to the value associated with the given name, or
		 *         null if no value is found.
		 */
		template<typename T>
		const T* find(const std::string& name) const;

		/** \brief Find a value for the given token name
		 *
		 * \param name - parameter name to search for.
		 * \return The value associated with the given name, or
		 *         the provided default value if one isn't found.
		 */
		template<typename T>
		const T& find(const std::string& name, const T& defVal) const;

	private:
		const Ri::ParamList& m_pList;
};


//==============================================================================
// Implementation details
//==============================================================================
// Implementation of CqRiParamList
inline CqRiParamList::CqRiParamList(const Ri::ParamList& pList)
	: m_pList(pList)
{ }

namespace detail
{
	template<typename T> struct TypeToRiSpecType { };
	template<> struct TypeToRiSpecType<RtFloat>      { static const Ri::TypeSpec::Type value = Ri::TypeSpec::Float; };
	template<> struct TypeToRiSpecType<RtInt>        { static const Ri::TypeSpec::Type value = Ri::TypeSpec::Integer; };
	template<> struct TypeToRiSpecType<RtConstToken> { static const Ri::TypeSpec::Type value = Ri::TypeSpec::String; };
}

template<typename T>
inline const T* CqRiParamList::find(const std::string& name) const
{
	for(size_t i = 0; i < m_pList.size(); ++i)
	{
		if(name == m_pList[i].name() &&
		   m_pList[i].spec().storageType() == detail::TypeToRiSpecType<T>::value)
			return reinterpret_cast<const T*>(m_pList[i].data());
	}
	return 0;
}

template<typename T>
inline const T& CqRiParamList::find(const std::string& name, const T& defVal) const
{
	if(const T* val = find<T>(name))
		return *val;
	else
		return defVal;
}

#endif // RIPARAMLIST_H_INCLUDED

} // namespace Aqsis
