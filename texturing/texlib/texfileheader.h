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


#ifndef IMAGEHEADER_H_INCLUDED
#define IMAGEHEADER_H_INCLUDED

#include "aqsis.h"

#include <map>
#include <iostream>

#include <boost/any.hpp>
#include <boost/shared_ptr.hpp>

#include "exception.h"

class CqChannelList;
struct SqTileInfo;

//------------------------------------------------------------------------------
/** \brief Wrapper for image file metadata
 *
 *
 */
class CqTexFileHeader
{
	public:
		typedef std::map<std::string, boost::any> TqAttributeMap;
		typedef TqAttributeMap::const_iterator const_iterator;

		CqTexFileHeader();

		template<typename T>
		inline void addAttribute(const std::string& name, const T& value);

		inline TqInt width() const;
		inline TqInt height() const;

		// Access to data about the names and types of channels
		inline const CqChannelList& channels() const;

		// General access to attributes
		/** \brief Get an attribute by name
		 *
		 * \return a pointer to the desired attribute, or NULL if not present.
		 */
		template<typename T>
		inline const T* findAttribute(const std::string& name) const;
		/** \brief Get an attribute by name
		 *
		 * \throw XqInternal if the named attribute is not present, or of the wrong type.
		 *
		 * \return a reference to the desired attribute
		 */
		template<typename T>
		inline const T& operator[](const std::string& name) const;

		/// Iterator interface to all attributes
		inline const_iterator begin() const;
		inline const_iterator end() const;

	private:
		void addStandardAttributes();

		TqAttributeMap m_attributeMap;
};


//==============================================================================
// Implementation of inline functions and templates
//==============================================================================

CqTexFileHeader::CqTexFileHeader()
	: m_attributeMap()
{
	addStandardAttributes();
}

template<typename T>
inline void CqTexFileHeader::addAttribute(const std::string& name, const T& value)
{
	TqAttributeMap::iterator iter = m_attributeMap.find(name);
	if(iter != m_attributeMap.end() && iter->second->type() != typeid(T))
	{
		throw XqInternal("Cannot assign a different type to a pre-existing attribute",
				__FILE__, __LINE__);
	}
	else
		m_attributeMap[name] = value;
}

inline TqInt CqTexFileHeader::width() const
{
	return boost::any_cast<TqInt>((*this)["width"]);
}

inline TqInt CqTexFileHeader::height() const
{
	return boost::any_cast<TqInt>((*this)["height"]);
}

inline const CqChannelList& CqTexFileHeader::channels() const
{
	return *boost::any_cast<const boost::shared_ptr<CqChannelList>& >((*this)["channels"]);
}

template<typename T>
inline const T* CqTexFileHeader::findAttribute(const std::string& name) const
{
	const_iterator iter = m_attributeMap.find(name);
	if(iter == m_attributeMap.end())
		return 0;
	return & boost::any_cast<const T&>(iter->second);
}

template<typename T>
inline const T& CqTexFileHeader::operator[](const std::string& name) const
{
	const_iterator iter = m_attributeMap.find(name);
	if(iter == m_attributeMap.end())
		throw XqInternal("Cannot cast attribute to incompatible type.");
	return boost::any_cast<const T&>(iter->second);
}

inline const_iterator CqTexFileHeader::begin() const
{
	return m_attributeMap.begin();
}

inline const_iterator CqTexFileHeader::end() const
{
	return m_attributeMap.end();
}

#endif // IMAGEHEADER_H_INCLUDED
