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

#include "aqsis.h"

#include "itexoutputfile.h"
#include "tiffdirhandle.h"

namespace Aqsis {

//------------------------------------------------------------------------------
class CqTiffOutputFile : public IqTexOutputFile
{
	public:
		/** \brief Construct a tiff output file with the given file name.
		 *
		 * \throw XqInternal if the file cannot be opened for writing.
		 *
		 * \param fileName - name for the new file.
		 * \param header - header data.
		 */
		CqTiffOutputFile(const std::string& fileName, const CqTexFileHeader& header);
		/** \brief Construct a tiff output file writing to the given stream.
		 */
		CqTiffOutputFile(std::ostream& outStream, const CqTexFileHeader& header);

		virtual ~CqTiffOutputFile() {};

		virtual inline const std::string& fileName() const;
		virtual inline const char* fileType();
		virtual inline const CqTexFileHeader& header() const;
		virtual inline TqInt currentLine() const;
	private:
		virtual void writePixelsImpl(TqUchar* buffer, TqInt numScanlines);

		std::string m_fileName;
		CqTexFileHeader m_header;
		TqInt m_currentLine;
		boost::shared_ptr<CqTiffFileHandle> m_fileHandle;
};


//==============================================================================
// Implementation of inline functions and templates
//==============================================================================

inline const std::string& CqTiffOutputFile::fileName() const
{
	return m_fileName;
}

inline const char* CqTiffOutputFile::fileType()
{
	return tiffFileTypeString;
}

inline const CqTexFileHeader& CqTiffOutputFile::header() const
{
	return m_header;
}

TqInt CqTiffOutputFile::currentLine() const
{
	return m_currentLine;
}

} // namespace Aqsis

#endif // TIFFOUTPUTFILE_H_INCLUDED
