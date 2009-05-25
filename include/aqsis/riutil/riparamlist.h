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
 * \todo This class should be aware of the type of a renderman token - it
 * should hold parsed token information taken from the renderman symbol table
 * rather than plain old strings.
 */
class AQSIS_RIUTIL_SHARE CqRiParamList
{
	public:
		/** \brief Construct the parameter list from the associated C-interface types.
		 *
		 * \param tokens - array of parameter description strings ("tokens")
		 * \param values - array of value pointers for the parameters
		 * \param count - length of the tokens and values arrays.
		 */
		CqRiParamList(RtToken tokens[], RtPointer values[], TqInt count);

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
		/** \brief Extract token names from a token array.
		 *
		 * \param tokNames - extracted token names are placed here.
		 * \param tokens - full tokens (may be type and class qualified)
		 * \param tokCount - number of tokens.
		 */
		static void extractTokenNames(std::vector<std::string>& tokNames,
				RtToken* tokens, TqInt tokCount);
		/// token name array
		std::vector<std::string> m_tokenNames;
		/// 
		const RtPointer* m_values;
		TqInt m_count;
};


//==============================================================================
// Implementation details
//==============================================================================
// Implementation of CqRiParamList
inline CqRiParamList::CqRiParamList(RtToken tokens[],
		RtPointer values[], TqInt count)
	: m_tokenNames(),
	m_values(values),
	m_count(count)
{
	extractTokenNames(m_tokenNames, tokens, count);
}

template<typename T>
inline const T* CqRiParamList::find(const std::string& name) const
{
	for(TqInt i = 0; i < m_count; ++i)
	{
		if(name == m_tokenNames[i])
			return reinterpret_cast<T*>(m_values[i]);
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
