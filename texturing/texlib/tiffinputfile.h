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

#include "aqsis.h"

#include <string>

#include <boost/shared_ptr.hpp>

#include "itexinputfile.h"
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
COMMON_SHARE class CqTiffInputFile : public IqTexInputFile
{
	public:
		CqTiffInputFile(const std::string& fileName);
		/// Constructor which takes an input stream rather than a file name
		CqTiffInputFile(std::istream& inStream);

		virtual const char* fileName() const;
		inline virtual const char* fileType() const;
		inline virtual const CqTexFileHeader& header() const;

		virtual void setImageIndex(TqInt newIndex);
		inline virtual TqInt imageIndex() const;
		virtual TqInt numImages() const;

	private:
		virtual void readPixelsImpl(TqUchar* buffer, TqInt startLine,
				TqInt numScanlines) const;

		/** \brief Initializations for directory-specific data
		 *
		 * \throw XqInternal if newDir is outside the valid range of
		 * directories = 0...numImages()-1
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

inline const char* CqTiffInputFile::fileType() const
{
	return "tiff";
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
