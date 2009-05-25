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
 * \brief A scanline-based output interface for tiff files.
 *
 * \author Chris Foster
 */

#ifndef TIFFOUTPUTFILE_H_INCLUDED
#define TIFFOUTPUTFILE_H_INCLUDED

#include <aqsis/aqsis.h>

#include <aqsis/tex/io/itexoutputfile.h>

namespace Aqsis {

class CqTiffFileHandle;

//------------------------------------------------------------------------------
/** \brief Scanline-based output for the TIFF file format.
 */
class AQSIS_TEX_SHARE CqTiffOutputFile : public IqMultiTexOutputFile
{
	public:
		/** \brief Construct a tiff output file with the given file name.
		 *
		 * \throw XqInternal if the file cannot be opened for writing.
		 *
		 * \param fileName - name for the new file.
		 * \param header - header data.
		 */
		CqTiffOutputFile(const boostfs::path& fileName, const CqTexFileHeader& header);
		/** \brief Construct a tiff output file writing to the given stream.
		 */
		CqTiffOutputFile(std::ostream& outStream, const CqTexFileHeader& header);

		// inherited
		virtual boostfs::path fileName() const;
		virtual EqImageFileType fileType();
		virtual const CqTexFileHeader& header() const;
		virtual TqInt currentLine() const;
		virtual void newSubImage(TqInt width, TqInt height);
		virtual void newSubImage(const CqTexFileHeader& header);

	private:
		/// Write pixel buffer to file as scanlines
		void writeScanlinePixels(const CqMixedImageBuffer& buffer);
		/// Write pixel buffer to file in tiled format
		void writeTiledPixels(const CqMixedImageBuffer& buffer);
		// inherited
		virtual void writePixelsImpl(const CqMixedImageBuffer& buffer);

		/// Perform shared initializations needed in construction.
		void initialize();
		/// Move to the next subimage, initializing it with the data in header.
		void nextSubImage(const CqTexFileHeader& header);

		/// File header
		CqTexFileHeader m_header;
		/// Scanline at which next output will be written to.
		TqInt m_currentLine;
		/// Handle to the underlying tiff file.
		boost::shared_ptr<CqTiffFileHandle> m_fileHandle;
};


} // namespace Aqsis

#endif // TIFFOUTPUTFILE_H_INCLUDED
