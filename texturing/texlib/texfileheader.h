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
#include "channellist.h"
#include "matrix.h"

namespace Aqsis {

/// A box type specifying an image region.
struct SqImageRegion
{
	TqInt width;     ///< width of the box
	TqInt height;    ///< height of the box
	TqInt topLeftX;  ///< x-position of the top left of the box.
	TqInt topLeftY;  ///< y-position of the top left of the box.

	/// Trivial constructor
	inline SqImageRegion(TqInt width = 0, TqInt height = 0,
			TqInt topLeftX = 0, TqInt topLeftY = 0);
};

//------------------------------------------------------------------------------
/** \brief Standard image header attributes.
 *
 * This namespace contains all the "tag" structs which represent possible image
 * attributes to be stored in a CqTexFileHeader
 */
namespace Attr
{
	/** \brief Macro to aid in defining the standard image attributes
	 *
	 * It's not certain that the name() function is necessary at this stage,
	 * but it might prove useful for diagnostics
	 */
#	define AQSIS_IMAGE_ATTR_TAG(attrName, attrType)          \
	struct attrName                                          \
	{                                                        \
		typedef attrType type;                               \
		static const char* name() { return #attrName; }      \
	}

	//--------------------------------------------------
	/** \name Image dimensions
	 *
	 * Image data has dimensions  Width x Height.
	 *
	 * In addition to the image dimensions, the header may specify a
	 * DisplayWindow attribute (mainly for use with cropped images).  The
	 * display window specifies the extent of the non-cropped image.  The
	 * coordinates used are such that the image data of size Width x Height
	 * has top left coordinates of (0,0).  (So negative values of
	 * topLeftX,topLeftY are valid).
	 *
	 * (topLeftX, topLeftY)
	 *  x---Display window-------------------+
	 *  |                                    |
	 *  |                                    |
	 *  |      (0,0)                         |
	 *  |       x----Data window------+      |
	 *  |       |                     | |    |
	 *  |       |                     | |    |
	 *  |       |                     |      |
	 *  |       |                   Height   |
	 *  |       |                     |      |
	 *  |       |                     | |    |
	 *  |       |                     | |    |
	 *  |       |                     | |    |
	 *  |       |                     | v    |
	 *  |       +---------------------+      |
	 *  |        ------ Width ------->       |
	 *  |                                    |
	 *  +------------------------------------+
	 *
	 */
	AQSIS_IMAGE_ATTR_TAG(Width, TqInt);
	AQSIS_IMAGE_ATTR_TAG(Height, TqInt);
	AQSIS_IMAGE_ATTR_TAG(DisplayWindow, SqImageRegion);
	/// aspect ratio = pix_width/pix_height
	AQSIS_IMAGE_ATTR_TAG(PixelAspectRatio, TqFloat);

	//--------------------------------------------------
	/// Channel information
	AQSIS_IMAGE_ATTR_TAG(ChannelList, CqChannelList);

	//--------------------------------------------------
	/// Tile information
	AQSIS_IMAGE_ATTR_TAG(IsTiled, bool);
	AQSIS_IMAGE_ATTR_TAG(TileWidth, TqInt);
	AQSIS_IMAGE_ATTR_TAG(TileHeight, TqInt);

	//--------------------------------------------------
	/// Information strings
	// image creation software
	AQSIS_IMAGE_ATTR_TAG(Software, std::string);
	// computer host name
	AQSIS_IMAGE_ATTR_TAG(HostName, std::string);
	// description of image
	AQSIS_IMAGE_ATTR_TAG(Description, std::string);
	// date and time of creation
	AQSIS_IMAGE_ATTR_TAG(DateTime, std::string);

	//--------------------------------------------------
	/// Transformation matrices
	AQSIS_IMAGE_ATTR_TAG(WorldToScreenMatrix, CqMatrix);
	AQSIS_IMAGE_ATTR_TAG(WorldToCameraMatrix, CqMatrix);

	//--------------------------------------------------
	/// Compression
	/// Compression type
	AQSIS_IMAGE_ATTR_TAG(Compression, std::string);
	/// compression quality (for lossy compression)
	AQSIS_IMAGE_ATTR_TAG(CompressionQuality, TqInt);
}


//------------------------------------------------------------------------------
/** \brief Wrapper for image file metadata
 *
 * General support for image metadata presents a bit of a problem, since
 * various file types choose to support various types and field names for
 * metadata.
 *
 * For the best possible compile-time checking, we choose to identify the
 * various image attributes with "tag" structs.  These tags live in the
 * Aqsis::Attr namespace defined above.  They collect together the name of the
 * attribute with the type, so handily allow all the type-checking to be done
 * at compile time.
 *
 * For example, to retrieve the width of the image, use
 *   header.find<Attr::Width>()
 * which will automatically know it should return a TqInt as the appropriate
 * type.
 */
COMMON_SHARE class CqTexFileHeader
{
	private:
		class CqTypeInfoHolder;
		/// Underlying map type.
		typedef std::map<CqTypeInfoHolder, boost::any> TqAttributeMap;
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
		/** \brief Set the value of an attribute with the given tag type
		 *
		 * AttrTagType provides a typedef AttrTagType::type which is the
		 * desired type for the corresponding attribute.
		 */
		template<typename AttrTagType>
		inline void set(const typename AttrTagType::type& value);

		//---------------------------------------------------------
		/** \name Image attribute access
		 *
		 * Convenience functions are provided for a few often-used attributes.
		 * All other attributes are accessed via the find() functions.
		 */
		//@{
		/// Get the image width
		inline TqInt width() const;
		/// Get the image height
		inline TqInt height() const;
		/// Get the image channel data
		inline CqChannelList& channelList();
		/// Get the image channel data
		inline const CqChannelList& channelList() const;

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
		inline typename AttrTagType::type& find();
		/// Get a reference to an attribute (const version)
		template<typename AttrTagType>
		inline const typename AttrTagType::type& find() const;

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
		inline const typename AttrTagType::type& find(const typename
				AttrTagType::type& defaultVal) const;

		/** \brief Get a pointer to an attribute
		 *
		 * \return a pointer to the desired attribute, or NULL if not present.
		 */
		template<typename AttrTagType>
		inline typename AttrTagType::type* findPtr();
		/// Get a pointer to an attribute by name (const version)
		template<typename AttrTagType>
		inline const typename AttrTagType::type* findPtr() const;
		//@}

	private:
		void addStandardAttributes();

		TqAttributeMap m_attributeMap;
};


//==============================================================================
// Implementation details
//==============================================================================
inline SqImageRegion::SqImageRegion(TqInt width, TqInt height,
		TqInt topLeftX, TqInt topLeftY)
	: width(width),
	height(height),
	topLeftX(topLeftX),
	topLeftY(topLeftY)
{ }

//------------------------------------------------------------------------------
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
			return m_typeInfo.before(rhs.m_typeInfo);
		}
};

//------------------------------------------------------------------------------
// CqTexFileHeader
inline CqTexFileHeader::CqTexFileHeader()
	: m_attributeMap()
{
	addStandardAttributes();
}

template<typename AttrTagType>
inline void CqTexFileHeader::set(const typename AttrTagType::type& value)
{
	m_attributeMap[CqTypeInfoHolder(typeid(AttrTagType))] = value;
}

inline TqInt CqTexFileHeader::width() const
{
	return find<Attr::Width>();
}

inline TqInt CqTexFileHeader::height() const
{
	return find<Attr::Height>();
}

inline CqChannelList& CqTexFileHeader::channelList()
{
	return find<Attr::ChannelList>();
}

inline const CqChannelList& CqTexFileHeader::channelList() const
{
	return find<Attr::ChannelList>();
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
		throw XqInternal("Cannot find requested attribute", __FILE__, __LINE__);
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
