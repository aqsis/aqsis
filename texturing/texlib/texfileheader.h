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
 * \brief Declare classes for dealing with image file metadata
 *
 * \author Chris Foster
 */


#ifndef TEXFILEHEADER_H_INCLUDED
#define TEXFILEHEADER_H_INCLUDED

#include "aqsis.h"

#include <map>
#include <iostream>

#include <boost/any.hpp>
#include <boost/shared_ptr.hpp>

#include "exception.h"

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief Wrapper for image file metadata
 *
 *
 */
class CqTexFileHeader
{
	private:
		/// Underlying map type.
		typedef std::map<std::string, boost::any> TqAttributeMap;
	public:
		/** \brief Const iterator type
		 *
		 * Dereferencing this iterator yields objects of type
		 * std::pair<std::string, boost::any>, where the first represents the
		 * attribute name, and second element represents the value.
		 */
		typedef TqAttributeMap::const_iterator const_iterator;

		/** Construct a header, empty except for required fields.
		 */
		inline CqTexFileHeader();

		//---------------------------------------------------------
		/** \brief Set the value of an attribute with the given name
		 *
		 * If an attribute with "name" already exists, then the type of the new
		 * and current values must be the same.  If the types are incompatible,
		 * throw an XqInternal.
		 */
		template<typename T>
		inline void setAttribute(const std::string& name, const T& value);

		//---------------------------------------------------------
		/** \name Attribute access
		 *
		 * The findAttribute functions provide typesafe access to the
		 * underlying image attributes.
		 */
		//@{

		/** \brief Get a reference to an attribute by name
		 *
		 * \throw XqInternal if the named attribute is not present, or of the
		 * wrong type.
		 *
		 * \return a reference to the desired attribute.
		 */
		template<typename T>
		inline T& findAttribute(const std::string& name);
		/// Get a reference to an attribute by name (const version)
		template<typename T>
		inline const T& findAttribute(const std::string& name) const;

		/** \brief Get a pointer to an attribute by name
		 *
		 * \return a pointer to the desired attribute, or NULL if not present.
		 */
		template<typename T>
		inline T* findAttributePtr(const std::string& name);
		/// Get a pointer to an attribute by name (const version)
		template<typename T>
		inline const T* findAttributePtr(const std::string& name) const;

		//@}


		//---------------------------------------------------------
		/// \name Standard iterator interface to all contained attributes
		//@{
		inline const_iterator begin() const;
		inline const_iterator end() const;
		//@}

	private:
		void addStandardAttributes();

		TqAttributeMap m_attributeMap;
};


//==============================================================================
// Implementation of inline functions and templates
//==============================================================================

inline CqTexFileHeader::CqTexFileHeader()
	: m_attributeMap()
{
	addStandardAttributes();
}

template<typename T>
inline void CqTexFileHeader::setAttribute(const std::string& name, const T& value)
{
	TqAttributeMap::iterator iter = m_attributeMap.find(name);
	if(iter != m_attributeMap.end() && iter->second.type() != typeid(T))
	{
		throw XqInternal("Cannot assign a different type to a pre-existing attribute",
				__FILE__, __LINE__);
	}
	else
		m_attributeMap[name] = value;
}

template<typename T>
inline T& CqTexFileHeader::findAttribute(const std::string& name)
{
	return const_cast<T&>(
			const_cast<const CqTexFileHeader*>(this)->findAttribute<T>(name) );
}

template<typename T>
inline const T& CqTexFileHeader::findAttribute(const std::string& name) const
{
	const_iterator iter = m_attributeMap.find(name);
	if(iter == m_attributeMap.end())
		throw XqInternal("Cannot find attribute with requested name",
				__FILE__, __LINE__);
	if(iter->second.type() != typeid(T))
		throw XqInternal("Cannot cast attribute to the requested type",
				__FILE__, __LINE__);
	return boost::any_cast<const T&>(iter->second);
}

template<typename T>
inline T* CqTexFileHeader::findAttributePtr(const std::string& name)
{
	return const_cast<T*>(
			const_cast<const CqTexFileHeader*>(this)->findAttributePtr<T>(name) );
}

template<typename T>
inline const T* CqTexFileHeader::findAttributePtr(const std::string& name) const
{
	const_iterator iter = m_attributeMap.find(name);
	if(iter == m_attributeMap.end() || iter->second.type() != typeid(T))
		return 0;
	return & boost::any_cast<const T&>(iter->second);
}

inline CqTexFileHeader::const_iterator CqTexFileHeader::begin() const
{
	return m_attributeMap.begin();
}

inline CqTexFileHeader::const_iterator CqTexFileHeader::end() const
{
	return m_attributeMap.end();
}

} // namespace Aqsis

#endif // TEXFILEHEADER_H_INCLUDED
