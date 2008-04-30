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
 * \brief Input interface for tiled images.
 *
 * \author Chris Foster
 */

#ifndef ITILEDTEXINPUTFILE_H_INCLUDED
#define ITILEDTEXINPUTFILE_H_INCLUDED

#include "aqsis.h"

#include <string>

#include <boost/shared_ptr.hpp>

#include "imagefiletype.h"
#include "texfileheader.h"

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief Image data input interface for tiled image files.
 *
 * Tiled images are broken up into a set of rectangular subregions called
 * "tiles" which are all of the same size.  Tile coordinates are such that
 * (0,0) is in the top left of the image, with (1,0) being the tile to the
 * immediate right of (0,0), etc.
 *
 * This interface allows tiles to be read from file one at a time.
 *
 * \todo it should also handle multi-image files.
 */
class AQSISTEX_SHARE IqTiledTexInputFile
{
	public:
		virtual ~IqTiledTexInputFile() {};

		//--------------------------------------------------
		/// \name Metadata access
		//@{
		/// Get the file name
		virtual const char* fileName() const = 0;
		/// Get the file type
		virtual EqImageFileType fileType() const = 0;
		/// Get the file header data
		virtual const CqTexFileHeader& header() const = 0;
		/// Get tile dimensions
		virtual SqTileInfo tileInfo() const = 0;
		//@}

		//--------------------------------------------------
		/// \name Functions for accessing multiple sub-images.
		//@{
		/** Set the image index in a multi-image file.
		 *
		 * In general, this function may be expected to modify the image header
		 * to reflect the metadata for the new image level.
		 *
		 * \param newIndex - new index in the multi-image file.
		 */
		virtual void setImageIndex(TqInt newIndex) = 0;
		/** Get the image index for a multi-image file.
		 *
		 * \return the current image index
		 */
		virtual TqInt imageIndex() const = 0;
		/** Get the number of images in the multi-image file.
		 *
		 * \return The number of images
		 */
		virtual TqInt numSubImages() const = 0;
		//@}

		//--------------------------------------------------
		/** \brief Read in a tile.
		 *
		 * ArrayT is a type modelling a simple resizeable 2D array
		 * interface.  It should provide the following methods:
		 *   - void resize(TqInt width, TqInt height, const CqChannelList& channels)
		 *     Resizes the buffer.  (width, height) is the new dimensions for
		 *     the buffer.  channels describes the new desired channel
		 *     structure for the buffer.  If the buffer cannot handle the
		 *     given channel structure it should throw.
		 *   - TqUint8* rawData()
		 *     Gets a raw pointer to the data.
		 *
		 * \param buffer - buffer to read the tile into
		 * \tileX - horizontal tile coordinate, starting from 0 in the top left.
		 * \tileY - vertical tile coordinate, starting from 0 in the top left.
		 */
		template<typename ArrayT>
		void readTile(ArrayT& buffer, TqInt tileX, TqInt tileY) const;

		/** \brief Open a tiled input file.
		 *
		 * Uses magic numbers to determine the file format of the file given by
		 * fileName.  If the format is unknown or the file cannot be opened for
		 * some other reason, throw an exception.
		 *
		 * \param fileName - file to open.  Can be in any of the formats
		 * understood by aqsistex.
		 * \return The newly opened input file
		 */
		static boost::shared_ptr<IqTiledTexInputFile> open(const std::string& fileName);
	protected:
		/** \brief Low-level readTile() function to be overridden by child classes
		 *
		 * The implementation of readTile simply validates the input
		 * parameters against the image dimensions as reported by header(),
		 * sets up the buffer, and calls readPixelsImpl().
		 *
		 * Implementations of readTileImpl() can assume that startLine and
		 * numScanlines specify a valid range.
		 */
		virtual void readTileImpl(TqUint8* buffer, TqInt tileX, TqInt tileY) = 0;
};


template<typename ArrayT>
void IqTiledTexInputFile::readTile(ArrayT& buffer, TqInt tileX, TqInt tileY) const
{
	/// \todo Make sure that getting header().channelList() isn't too expensive
	/// for fast tile access.
	SqTileInfo info = tileInfo();
	buffer.resize(info.width, info.height, header().channelList());
	readTileImpl(buffer.rawData(), tileX, tileY);
}

} // namespace Aqsis

#endif // ITILEDTEXINPUTFILE_H_INCLUDED
