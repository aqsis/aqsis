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
