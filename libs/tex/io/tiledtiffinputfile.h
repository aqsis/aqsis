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
 * \brief Tiled TIFF input interface.
 *
 * \author Chris Foster [chris42f (at) gmail (d0t) com]
 */

#ifndef ITILEDTIFFINPUTFILE_H_INCLUDED
#define ITILEDTIFFINPUTFILE_H_INCLUDED

#include <aqsis/aqsis.h>

#include <vector>

#include <aqsis/tex/io/itiledtexinputfile.h>
#include "tiffdirhandle.h"

namespace Aqsis {

/** \brief Input interface for tiled TIFF images, allowing reading of
 * individual tiles.
 *
 * This interface is designed to support the fairly specialized usage of a
 * disk-backed tiled texture array.  As such, reading the tiles should be as
 * efficient as possible.  In order to simplify the interface, some assumptions
 * are made which restrict the interface to a subset of possible TIFF files:
 *   - All subimages are tiled
 *   - The tiles in any subimage are the same size as the tiles in any other subimage.
 *   - The pixel format is directly addressable (8, 16, 32 bits per channel)
 *   - Pixel channels are stored interleaved rather than "planar"
 *   - Probably misc. other restrictions (see tiffdirhandle.cpp)
 */
class AQSIS_TEX_SHARE CqTiledTiffInputFile : public IqTiledTexInputFile
{
	public:
		/** \brief Open a tiled TIFF file and setup the input interface.
		 *
		 * \throw XqBadTexture if the TIFF file doesn't satisfy the tiling
		 * assumptions.
		 */
		CqTiledTiffInputFile(const boostfs::path& fileName);

		virtual boostfs::path fileName() const;
		virtual EqImageFileType fileType() const;
		virtual const CqTexFileHeader& header(TqInt index = 0) const;
		virtual SqTileInfo tileInfo() const;

		virtual TqInt numSubImages() const;
		virtual TqInt width(TqInt index) const;
		virtual TqInt height(TqInt index) const;
	private:
		virtual void readTileImpl(TqUint8* buffer, TqInt tileX, TqInt tileY,
				TqInt subImageIdx, const SqTileInfo tileSize) const;

		/// Header information
		std::vector<boost::shared_ptr<CqTexFileHeader> > m_headers;
		/// Handle to the underlying TIFF structure.
		boost::shared_ptr<CqTiffFileHandle> m_fileHandle;
		/// Number of directories in the TIFF file.
		tdir_t m_numDirs;
		/// Tile information
		SqTileInfo m_tileInfo;
		/// Image width
		std::vector<TqInt> m_widths;
		/// Image height
		std::vector<TqInt> m_heights;
};

} // namespace Aqsis

#endif // ITILEDTIFFINPUTFILE_H_INCLUDED
