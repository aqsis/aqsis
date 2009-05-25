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
 * \brief Scanline-oriented pixel access for Aqsis z-file input.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 *
 */

#ifndef ZINPUTFILE_H_INCLUDED
#define ZINPUTFILE_H_INCLUDED

#include <aqsis/aqsis.h>

#include <fstream>

#include <aqsis/tex/io/itexinputfile.h>

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief Scanline-oriented input class for Aqsis Z files.
 *
 * An aqsis Z-file is a very simple platform-dependent binary format for depth
 * buffers:
 */
class AQSIS_TEX_SHARE CqZInputFile : public IqTexInputFile
{
	public:
		/** \brief Open a z-file from a file name
		 */
		CqZInputFile(const boostfs::path& fileName);

		// inherited
		virtual boostfs::path fileName() const;
		virtual EqImageFileType fileType() const;
		virtual const CqTexFileHeader& header() const;

		virtual void readPixelsImpl(TqUint8* buffer, TqInt startLine,
				TqInt numScanlines) const;
	private:
		/// Read header data from inStream.
		static void readHeader(std::istream& inStream, CqTexFileHeader& header);
		/// Header information
		CqTexFileHeader m_header;
		/// Name of the file
		const boostfs::path m_fileName;
		/// Input stream from which to read data
		mutable std::ifstream m_fileStream;
		/// Position of the start of depth data in the file.
		std::istream::pos_type m_dataBegin;
};

} // namespace Aqsis

#endif // ZINPUTFILE_H_INCLUDED
