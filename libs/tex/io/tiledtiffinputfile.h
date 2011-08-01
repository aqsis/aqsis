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
