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
 * \brief Declare input and output interface specifications which should be
 * implemented by all classes wrapping texture files.
 *
 * \author Chris Foster
 */

#ifndef ITEXFILE_H_INCLUDED
#define ITEXFILE_H_INCLUDED

#include "aqsis.h"

#include <string>
#include <map>

#include "texturetile.h"

namespace Aqsis {

class IqTexAttribute
{
	public:
}

template<typename T>
class CqTexAttributeTyped
{
	public:
	private:
};

class CqTexFileHeader
{
	public:
		typedef std::map<std::string, boost::shared_ptr<IqTexAttribute> > TqAttrMap;
		CqTexFileHeader();
		virtual ~CqTexFileHeader();
	protected:
		virtual const boost::shared_ptr<IqTexAttriubute> loadUnderlyingAttribute(const std::string &, );
	private:
		TqAttrMap m_attributes;
		TqInt m_width;
		TqInt m_height;
		TqInt m_tileWidth;
		TqInt m_tileHeight;
};

class IqTextureHeader
{
	public:
		// Access to commonly defined attributes here:

		// Access to data about the names and types of channels
		CqChannelList channels();
		void setChannels(const CqChannelList& channels);

		// General access to attributes
		/** \brief Get an attribute by name
		 */
		template<typename T>
		const T& findAttribute(const std::string& name);
};


//------------------------------------------------------------------------------
class IqTexInputFile
{
	public:
		virtual ~IqTextureInputFile() = 0;

		/** \brief Read a tile from the file.
		 *
		 * \param x - tile column index (counting from top left, starting with 0)
		 * \param y - tile row index (counting from top left, starting with 0)
		 * \return a tile containing the desired data.
		 */
		template<typename T>
		virtual CqTextureTile<T>::TqPtr readTile(const TqInt x, const TqInt y) = 0;
		/** \brief Read the entire image into a single buffer.
		 */
		template<typename T>
		virtual CqTextureTile<T>::TqPtr readPixels() = 0;

		/** \brief Get the image index for multi-image files like TIFF.
		 *
		 * \return The image index, or 0 if the format doesn't support mutiple images.
		 */
		virtual TqInt index();
		/** \brief Set the image index for multi-image files like TIFF.
		 *
		 * Has no effect for images which aren't multi-index.
		 */
		virtual void setIndex(TqInt newIndex);

};


//------------------------------------------------------------------------------

} // namespace Aqsis

#endif // ITEXFILE_H_INCLUDED
