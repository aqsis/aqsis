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
 * \author Chris Foster
 */

#ifndef ITILEDTIFFINPUTFILE_H_INCLUDED
#define ITILEDTIFFINPUTFILE_H_INCLUDED

#include "aqsis.h"

#include "itiledtexinputfile.h"
#include "tiffdirhandle.h"

namespace Aqsis {

/** \brief Input interface for tiled TIFF images, allowing reading of individual tiles.
 *
 */
class AQSISTEX_SHARE CqTiledTiffInputFile : public IqTiledTexInputFile
{
	public:
		CqTiledTiffInputFile(const std::string& fileName);

		virtual const char* fileName() const;
		virtual EqImageFileType fileType() const;
		virtual const CqTexFileHeader& header() const;
		virtual SqTileInfo tileInfo() const;

		virtual void setImageIndex(TqInt newIndex);
		virtual TqInt imageIndex() const;
		virtual TqInt numSubImages() const;
	protected:
		virtual void readTileImpl(TqUint8* buffer, TqInt tileX, TqInt tileY,
				const SqTileInfo tileSize) const;

		/** \brief Initializations for directory-specific data
		 *
		 * \throw XqInternal if newDir is outside the valid range of
		 * directories = 0...numSubImages()-1
		 * \throw XqBadTexture if the directory has an unsupported data format
		 *        (eg, isn't tiled)
		 * \param newDir - new image directory to set.
		 */
		void setDirectory(tdir_t newDir);

		/// Header information
		CqTexFileHeader m_header;
		/// Handle to the underlying TIFF structure.
		boost::shared_ptr<CqTiffFileHandle> m_fileHandle;
		/// Index to the tiff directory
		tdir_t m_imageIndex;
		/// Number of directories in the TIFF file.
		tdir_t m_numDirs;
		/// Tile information
		SqTileInfo m_tileInfo;
		/// Image width
		TqInt m_width;
		/// Image height
		TqInt m_height;
};

} // namespace Aqsis

#endif // ITILEDTIFFINPUTFILE_H_INCLUDED
