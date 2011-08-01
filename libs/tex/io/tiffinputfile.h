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
