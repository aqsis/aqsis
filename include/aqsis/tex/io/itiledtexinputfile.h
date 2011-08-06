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
 * \brief Input interface for tiled images.
 *
 * \author Chris Foster
 */

#ifndef ITILEDTEXINPUTFILE_H_INCLUDED
#define ITILEDTEXINPUTFILE_H_INCLUDED

#include <aqsis/aqsis.h>

#include <boost/shared_ptr.hpp>

#include <aqsis/util/file.h>
#include <aqsis/tex/io/imagefiletype.h>
#include <aqsis/tex/io/texfileheader.h>

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief Image data input interface for tiled image files.
 *
 * Tiled images are broken up into a set of rectangular subregions called
 * "tiles" which are all of the same size.  Tile coordinates are such that
 * (0,0) is in the top left of the image, with (1,0) being the tile to the
 * immediate right of (0,0), etc.
 *
 * This interface allows tiles to be read from file one at a time, while also
 * providing convenient access to various data about the tiling.
 *
 * Tiled images may contain multiple sub-images stored within the single file.
 * The interface allows random access to tiles from the various sub-images for
 * convenience.  A useful simplifying assumption is that the tiles for all
 * subimages are assumed to be of the same size when using this interface.
 *
 * Efficiency of random tile access depends strongly on the exact image type
 * being used to store the tile data.  Some image formats like TIFF don't model
 * such random access directly, which means switching between subimages may be
 * costly.  If this becomes an issue, an alternative would be to provide a
 * clone() function to make a copy of the backend, and use separate copies to
 * access separate subimages.
 */
class AQSIS_TEX_SHARE IqTiledTexInputFile
{
	public:
		virtual ~IqTiledTexInputFile() {};

		//--------------------------------------------------
		/// \name Metadata access
		//@{
		/// Get the file name
		virtual boostfs::path fileName() const = 0;
		/// Get the file type
		virtual EqImageFileType fileType() const = 0;
		/** Get the file header data
		 *
		 * For file types which support multiple sets of header data (eg,
		 * TIFF), index specifies the header data set to consider.
		 *
		 * \param index - Header data set to return.  Child classes should
		 *                return the primary set of header data if index is
		 *                larger than the maximum header.
		 */
		virtual const CqTexFileHeader& header(TqInt index = 0) const = 0;
		/** \brief Get tile dimensions as used by readTile().
		 *
		 * Note that this may be different from the tile dimensions as reported
		 * in the file metadata header.  The header should report correctly on
		 * the structure of the underlying file, while tileInfo() reports the
		 * effective tile size used by the interface.  This is to allow file
		 * formats which aren't tiled at all to share the interface for
		 * convenience.  (In these cases, the obvious strategy is to consider
		 * the whole image to be a single tile.)
		 */
		virtual SqTileInfo tileInfo() const = 0;
		//@}

		//--------------------------------------------------
		/// \name Access to information about sub-images.
		//@{
		/** Get the number of images in the multi-image file.
		 *
		 * \return The number of images
		 */
		virtual TqInt numSubImages() const = 0;
		/// Get the width of image with the given index
		virtual TqInt width(TqInt index) const = 0;
		/// Get the height of image with the given index
		virtual TqInt height(TqInt index) const = 0;
		//@}

		//--------------------------------------------------
		/** \brief Random read access to tile data.
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
		 * Often the dimensions of an image are not a multiple of the tile
		 * size.  In this case, tiles on the right hand side and bottom of the
		 * image are resized to exactly fit the image edges.
		 *
		 * \param buffer - buffer to read the tile into
		 * \param tileX - horizontal tile coordinate, starting from 0 in the top left.
		 * \param tileY - vertical tile coordinate, starting from 0 in the top left.
		 * \param subImageIdx - subimage index from which to read the tile.
		 */
		template<typename ArrayT>
		void readTile(ArrayT& buffer, TqInt tileX, TqInt tileY,
				TqInt subImageIdx) const;

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
		static boost::shared_ptr<IqTiledTexInputFile> open(const boostfs::path& fileName);
		/** \brief Open any image file using the tiled interface.
		 *
		 * Sometimes it may be useful to wrap any image up in a tiled
		 * interface.  This function returns a tiled image interface wrapping
		 * any image type which can be read using the IqTexInputFile interface.
		 *
		 * \note Using such images as tiled images is not guaranteed to be
		 * memory efficient!
		 *
		 * \param fileName - file to open.  Can be in any of the formats
		 * understood by aqsistex.
		 * \return The newly opened input file
		 */
		static boost::shared_ptr<IqTiledTexInputFile> openAny(const boostfs::path& fileName);

	protected:
		/** \brief Low-level readTile() function to be overridden by child classes
		 *
		 * The implementation of readTile simply validates the input parameters
		 * against the image dimensions as reported by header(), resizes the
		 * buffer, and calls readPixelsImpl() to do the work.
		 *
		 * \param buffer - Pointer to raw data buffer of sufficient size for
		 *                 the tile data.
		 * \param tileX - x-coordinate for tile
		 * \param tileY - y-coordinate for tile
		 * \param subImageIdx - subimage index
		 * \param tileSize - Size of the tile to be read.  (May be truncated at
		 *                   image bottom or right.)
		 */
		virtual void readTileImpl(TqUint8* buffer, TqInt tileX, TqInt tileY,
				TqInt subImageIdx, const SqTileInfo tileSize) const = 0;
};


template<typename ArrayT>
void IqTiledTexInputFile::readTile(ArrayT& buffer, TqInt tileX, TqInt tileY,
		TqInt subImageIdx) const
{
	SqTileInfo tInfo = tileInfo();
	TqInt w = width(subImageIdx);
	TqInt h = height(subImageIdx);
	// Modify the tile size in the case where a tile of the natural size would
	// tile fall off the image edge.  
	if((tileX + 1)*tInfo.width > w)
		tInfo.width = w - tileX*tInfo.width;
	if((tileY + 1)*tInfo.height > h)
		tInfo.height = h - tileY*tInfo.height;
	assert(tInfo.width > 0);
	assert(tInfo.height > 0);
	assert(subImageIdx >= 0);
	assert(subImageIdx < numSubImages());
	buffer.resize(tInfo.width, tInfo.height, header().channelList());
	readTileImpl(buffer.rawData(), tileX, tileY, subImageIdx, tInfo);
}

} // namespace Aqsis

#endif // ITILEDTEXINPUTFILE_H_INCLUDED
