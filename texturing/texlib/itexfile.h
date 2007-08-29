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

#include "texturetile.h"

namespace Aqsis {

//------------------------------------------------------------------------------
class IqTextureInputFile
{
	public:
		virtual ~IqTextureInputFile() = 0;

		/** \brief Read a tile or strip from the tiff file.
		 *
		 * \throw XqTiffError if the requested tile is outside the bounds of the image
		 *
		 * \param x - tile column index (counting from top left, starting with 0)
		 * \param y - tile row index (counting from top left, starting with 0)
		 * \return a tile containing the desired data.
		 */
		template<typename T>
		virtual CqTextureTile<T>::TqPtr readTile(const TqUint x, const TqUint y) = 0;
		/** \brief Read the entire image into a single buffer.
		 */
		template<typename T>
		virtual CqTextureTile<T>::TqPtr readImage() = 0;

		/** \brief Get the image index for multi-image files like TIFF.
		 *
		 * \return The image index, or 0 if the format doesn't support mutiple images.
		 */
		virtual TqUint index();
		/** \brief Set the image index for multi-image files like TIFF.
		 *
		 * Has no effect for images which aren't multi-index.
		 */
		virtual void setIndex(TqUint newIndex);

		/** \brief Get the image width
		 */
		virtual TqUint width() = 0;
		/** \brief Get the image height
		 */
		virtual TqUint height() = 0;
		/** \brief Get the tile width
		 *
		 * \return The tile width, or total image width if the image is not tiled.
		 */
		virtual TqUint tileWidth() = 0;
		/** \brief Get the tile height
		 *
		 * \return The tile height, or total image height if the image is not tiled.
		 */
		virtual TqUint tileHeight() = 0;
};


//------------------------------------------------------------------------------

} // namespace Aqsis

#endif // ITEXFILE_H_INCLUDED
