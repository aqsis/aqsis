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

#ifndef ITILEDTEXINPUTFILE_H_INCLUDED
#define ITILEDTEXINPUTFILE_H_INCLUDED

#include "aqsis.h"

#include <string>

#include "texfileheader.h"

namespace Aqsis {

//------------------------------------------------------------------------------
class AQSISTEX_SHARE IqTiledTexInputFile
{
	public:
		virtual ~IqTiledTexInputFile() {};

		/// get the file name
		virtual const char* fileName() const = 0;

		/// get a string representing the file type
		virtual const char* fileType() const = 0;

		/// Get the file header data
		virtual const CqTexFileHeader& header() const = 0;

		/** \brief Read in a tile.
		 *
		 * Array2DType is a type modelling a simple resizeable 2D array
		 * interface.  It should provide the following methods:
		 *   - void resize(TqInt width, TqInt height, const CqChannelList& channels)
		 *     Resizes the buffer.  (width, height) is the new dimensions for
		 *     the buffer.  channels describes the new desired channel
		 *     structure for the buffer.  If the buffer cannot handle the
		 *     given channel structure it should throw.
		 *   - TqUchar* rawData()
		 *     Gets a raw pointer to the data.
		 *
		 * Tiles are all of the same size.  Tile coordinates are such that
		 * (0,0) is in the top left of the image with (1,0) the tile to the
		 * immediate right of (0,0), etc.
		 *
		 * \param buffer - buffer to read the tile into
		 * \tileX - horizontal tile coordinate, starting from 0 in the top left.
		 * \tileY - vertical tile coordinate, starting from 0 in the top left.
		 */
		template<typename Array2DType>
		void readTile(Array2DType& buffer, TqInt tileX, TqInt tileY) const;

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
		virtual void readTileImpl(TqUchar* buffer, TqInt tileX, TqInt tileY);
};


//==============================================================================
// Implementation of inline functions and templates
//==============================================================================

#if 0
template<typename Array2DType>
void IqTiledTexInputFile::readPixels(Array2DType& buffer, TqInt startLine,
		TqInt numScanlines) const
{
	TqInt imageHeight = header().height();
	// if numScanlines is negative, read until the last line
	if(numScanlines <= 0)
		numScanlines = imageHeight - startLine;
	// check that startLine is in the image range & that the ending line is
	// reasonable.
	if(startLine < 0 || startLine >= imageHeight
			|| startLine + numScanlines > imageHeight)
	{
		AQSIS_THROW(XqInternal, "Attempt to read scanlines " << startLine
				<< " to " << startLine + numScanlines - 1
				<< " outside image boundaries for file \"" << fileName() << "\".");
	}
		AQSIS_THROW(XqInternal, "Attempt to read scanlines outside image boundaries");
	// Resize the buffer to deal with the new data
	buffer.resize(header().width(), numScanlines, header().channelList());
	readPixelsImpl(buffer.rawData(), startLine, numScanlines);
}
#endif

} // namespace Aqsis

#endif // ITILEDTEXINPUTFILE_H_INCLUDED
