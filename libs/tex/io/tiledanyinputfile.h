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
 * \brief Tiled input wrapper for files which don't really hold tiled data.
 *
 * \author Chris Foster
 */

#ifndef ITILEDANYINPUTFILE_H_INCLUDED
#define ITILEDANYINPUTFILE_H_INCLUDED

#include <aqsis/aqsis.h>

#include <boost/shared_ptr.hpp>

#include <aqsis/tex/io/itiledtexinputfile.h>

namespace Aqsis {

class IqTexInputFile;

/** \brief Tiled input wrapper interface for non-tiled image formats
 *
 * This interface is a simple wrapper around any image format which can be
 * loaded using the IqTexInputFile interface.  As such, the "tiles" which are
 * provided are the size of the entire image which is likely to cause excessive
 * memory usage.  However, we will warn when using the interface and if the
 * user persists in not correctly mipmapping their files then they probably
 * deserve what they get ;-)
 */
class AQSIS_TEX_SHARE CqTiledAnyInputFile : public IqTiledTexInputFile
{
	public:
		/** \brief Open any texture file and interpret as a tiled file.
		 *
		 * This simply calls through to IqTexInputFile::open(), so fails in the
		 * same ways.
		 */
		CqTiledAnyInputFile(const boostfs::path& fileName);

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

		/// Underlying input file which does the real work.
		boost::shared_ptr<IqTexInputFile> m_texFile;
		/// Tile information
		SqTileInfo m_tileInfo;
};

} // namespace Aqsis

#endif // ITILEDANYINPUTFILE_H_INCLUDED
