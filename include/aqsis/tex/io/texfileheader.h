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

#include <aqsis/aqsis.h>

#include <iostream>
#include <map>

#include <boost/any.hpp>
#include <boost/shared_ptr.hpp>

#include <aqsis/tex/io/texfileattributes.h>

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief Wrapper for image file metadata
 *
 * General support for image metadata presents a bit of a problem, since
 * various file types choose to support various types and field names for
 * metadata.
 *
 * For the best possible compile-time checking, we choose to identify the
 * various image attributes with "tag" structs.  These tags live in the
 * Aqsis::Attr namespace.  They collect together the name of the attribute with
 * the type, so handily allow all the type-checking to be done at compile time.
 *
 * For example, to retrieve the image description string, use
 *   header.find<Attr::Description>()
 * which will automatically know it should return a std::string as the
 * appropriate type.
 *
 * Attributes which are guarenteed to be always present have specialized
 * methods to read and write them for convenience and efficiency.
 */
class CqTexFileHeader
{
	private:
		class CqTypeInfoHolder;
		/// Underlying map type.
		typedef std::map<CqTypeInfoHolder, boost::any> TqAttributeMap;
		typedef TqAttributeMap::const_iterator const_iterator;
	public:

		/** \brief Construct a header, empty except for required fields.
		 */
		CqTexFileHeader();

		//---------------------------------------------------------
		/// \name Accessors and modifiers for always-present attributes
		//@{
		/// Get the image width
		TqInt width() const;
		/// Get the image height
		TqInt height() const;
		/// Get the image channel data
		const CqChannelList& channelList() const;

		/// Set the image width
		void setWidth(TqInt width);
		/// Set the image height
		void setHeight(TqInt height);
		/// Get a modifyable ref. to the image channel data.
		CqChannelList& channelList();
		//@}
		//---------------------------------------------------------

		//---------------------------------------------------------
		/// \name Modifiers for image attribute values.
		//@{
		/** \brief Set the value of an attribute with the given tag type
		 *
		 * AttrTagType provides a typedef AttrTagType::type which is the
		 * desired type for the corresponding attribute.
		 */
		template<typename AttrTagType>
		void set(const typename AttrTagType::type& value);

		/** \brief Erase an attribute from the header.
		 *
		 * AttrTagType specifies the attribute to erase.
		 */
		template<typename AttrTagType>
		void erase();

		/** \brief Timestamp the file.
		 *
		 * This adds the Attr::DateTime attribute to the header in the format
		 * given by the tiff standard.  The tiff standard specifies 19
		 * characters for the date and time: "YYYY:MM:DD hh:mm:ss".
		 */
		void setTimestamp();
		//@}

		//---------------------------------------------------------
		/// \name Image attribute accessors/modifiers
		//@{
		/** \brief Get a reference to an attribute
		 *
		 * AttrTagType provides a typedef AttrTagType::type which is the
		 * desired type for the corresponding attribute.
		 *
		 * \throw XqInternal if the named attribute is not present.
		 *
		 * \return a reference to the desired attribute.
		 */
		template<typename AttrTagType>
		typename AttrTagType::type& find();
		/// Get a reference to an attribute (const version)
		template<typename AttrTagType>
		const typename AttrTagType::type& find() const;

		/** \brief Get a reference to an attribute
		 *
		 * If the named attribute is not present, return the default value
		 * given.
		 *
		 * \param defaultVal - default attribute value
		 *
		 * \return a reference to the desired attribute.
		 */
		template<typename AttrTagType>
		const typename AttrTagType::type& find(const typename
				AttrTagType::type& defaultVal) const;

		/** \brief Get a pointer to an attribute
		 *
		 * \return a pointer to the desired attribute, or NULL if not present.
		 */
		template<typename AttrTagType>
		typename AttrTagType::type* findPtr();
		/// Get a pointer to an attribute by name (const version)
		template<typename AttrTagType>
		const typename AttrTagType::type* findPtr() const;
		//@}

	private:
		TqInt m_width;
		TqInt m_height;
		CqChannelList m_channelList;
		TqAttributeMap m_attributeMap;
};


//==============================================================================
// Implementation details
//==============================================================================
/** \brief Wrapper around std::type_info to allow usage as a key type in std::map.
 *
 * Hold onto a reference to std::type_info, and provide operator<
 */
class CqTexFileHeader::CqTypeInfoHolder
{
	private:
		const std::type_info& m_typeInfo;
	public:
		CqTypeInfoHolder(const std::type_info& typeInfo)
			: m_typeInfo(typeInfo)
		{ }
		bool operator<(const CqTypeInfoHolder& rhs) const
		{
			return m_typeInfo.before(rhs.m_typeInfo) != 0;
		}
};

//------------------------------------------------------------------------------
// CqTexFileHeader
inline CqTexFileHeader::CqTexFileHeader()
	: m_width(0),
	m_height(0),
	m_channelList(),
	m_attributeMap()
{ }


inline TqInt CqTexFileHeader::width() const
{
	return m_width;
}

inline TqInt CqTexFileHeader::height() const
{
	return m_height;
}

inline const CqChannelList& CqTexFileHeader::channelList() const
{
	return m_channelList;
}

inline void CqTexFileHeader::setWidth(TqInt width)
{
	assert(m_width >= 0);
	m_width = width;
}

inline void CqTexFileHeader::setHeight(TqInt height)
{
	assert(m_height >= 0);
	m_height = height;
}

inline CqChannelList& CqTexFileHeader::channelList()
{
	return m_channelList;
}

inline void CqTexFileHeader::setTimestamp()
{
	time_t long_time;
	// Get time as long integer.
	time( &long_time );
	// Convert to local time.
	struct tm* ct = localtime( &long_time );
	set<Attr::DateTime>(
			(boost::format("%04d:%02d:%02d %02d:%02d:%02d")
			% (1900 + ct->tm_year) % (ct->tm_mon + 1) % ct->tm_mday
			% ct->tm_hour % ct->tm_min % ct->tm_sec).str()
		);
}


template<typename AttrTagType>
inline void CqTexFileHeader::set(const typename AttrTagType::type& value)
{
	m_attributeMap[CqTypeInfoHolder(typeid(AttrTagType))] = value;
}

template<typename AttrTagType>
void CqTexFileHeader::erase()
{
	m_attributeMap.erase(CqTypeInfoHolder(typeid(AttrTagType)));
}

template<typename AttrTagType>
inline typename AttrTagType::type& CqTexFileHeader::find()
{
	return const_cast<typename AttrTagType::type&>(
			const_cast<const CqTexFileHeader*>(this)->find<AttrTagType>() );
}

template<typename AttrTagType>
inline const typename AttrTagType::type& CqTexFileHeader::find() const
{
	const_iterator iter = m_attributeMap.find(CqTypeInfoHolder(typeid(AttrTagType)));
	if(iter == m_attributeMap.end())
	{
		AQSIS_THROW_XQERROR(XqInternal, EqE_BadFile, "Requested attribute \""
				<< AttrTagType::name() << "\" not present in file header");
	}
	return boost::any_cast<const typename AttrTagType::type&>(iter->second);
}

template<typename AttrTagType>
inline const typename AttrTagType::type& CqTexFileHeader::find(
		const typename AttrTagType::type& defaultVal) const
{
	const typename AttrTagType::type* attr = findPtr<AttrTagType>();
	if(attr)
		return *attr;
	else
		return defaultVal;
}

template<typename AttrTagType>
inline typename AttrTagType::type* CqTexFileHeader::findPtr()
{
	return const_cast<typename AttrTagType::type*>(
			const_cast<const CqTexFileHeader*>(this)->findPtr<AttrTagType>() );
}

template<typename AttrTagType>
inline const typename AttrTagType::type* CqTexFileHeader::findPtr() const
{
	const_iterator iter = m_attributeMap.find(CqTypeInfoHolder(typeid(AttrTagType)));
	if(iter == m_attributeMap.end())
		return 0;
	return & boost::any_cast<const typename AttrTagType::type&>(iter->second);
}


} // namespace Aqsis

#endif // TEXFILEHEADER_H_INCLUDED
