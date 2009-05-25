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
 * \brief Scanline-oriented pixel access for TIFF input.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 *
 */

#ifndef TIFFINPUTFILE_H_INCLUDED
#define TIFFINPUTFILE_H_INCLUDED

#include <aqsis/aqsis.h>

#include <string>

#include <boost/shared_ptr.hpp>

#include <aqsis/tex/io/itexinputfile.h>
#include "tiffdirhandle.h"

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief Scanline-oriented input class for TIFF files.
 *
 * This class puts a scanline interface onto strip-based or tiled TIFF files,
 * and attempts to hide a lot of the complexity of the TIFF format behind a
 * uniform interface.
 *
 * For cases of unusual internal formats, the class falls back on the generic
 * RGBA image handling built into libTIFF.
 */
class AQSIS_TEX_SHARE CqTiffInputFile : public IqMultiTexInputFile
{
	public:
		CqTiffInputFile(const boostfs::path& fileName);
		/** \param Read the tiff from an input stream rather than a file.
		 *
		 * \param inStream - Stream to read from.  This is passed to the
		 * underlying tiff (tiffxx) library.
		 */
		CqTiffInputFile(std::istream& inStream);

		// inherited
		virtual boostfs::path fileName() const;
		virtual EqImageFileType fileType() const;
		virtual const CqTexFileHeader& header() const;

		virtual void setImageIndex(TqInt newIndex);
		virtual TqInt imageIndex() const;
		virtual TqInt numSubImages() const;

	private:
		// inherited
		virtual void readPixelsImpl(TqUint8* buffer, TqInt startLine,
				TqInt numScanlines) const;

		/// Read pixels from a tiff file with data stored in strips.
		void readPixelsStripped(TqUint8* buffer, TqInt startLine,
				TqInt numScanlines) const;
		/// Read pixels from a tiff file with data stored in tiles.
		void readPixelsTiled(TqUint8* buffer, TqInt startLine,
				TqInt numScanlines) const;
		/// Read pixels in a strange formats using generic RGBA reading
		void readPixelsRGBA(TqUint8* buffer, TqInt startLine,
				TqInt numScanlines) const;

		/** \brief Initializations for directory-specific data
		 *
		 * \throw XqInternal if newDir is outside the valid range of
		 * directories = 0...numSubImages()-1
		 * \param newDir - new image directory to set.
		 */
		void setDirectory(tdir_t newDir);

		/// Header information
		CqTexFileHeader m_header;
		/// Handle to the underlying TIFF structure.
		boost::shared_ptr<CqTiffFileHandle> m_fileHandle;
		/// Index to the tiff directory
		tdir_t m_imageIndex;
};


//==============================================================================
// Implementation details
//==============================================================================

inline EqImageFileType CqTiffInputFile::fileType() const
{
	return ImageFile_Tiff;
}

inline const CqTexFileHeader& CqTiffInputFile::header() const
{
	return m_header;
}

inline TqInt CqTiffInputFile::imageIndex() const
{
	return m_imageIndex;
}

} // namespace Aqsis

#endif // TIFFINPUTFILE_H_INCLUDED
