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

#include "tiffoutputfile.h"

namespace Aqsis {

CqTiffOutputFile::CqTiffOutputFile(const std::string& fileName, const CqTexFileHeader& header)
	: m_fileName(fileName),
	m_header(header),
	m_currentLine(0),
	m_fileHandle(new CqTiffFileHandle(fileName.c_str(), "w"))
{
	CqChannelList& channels = m_header.channels();
	// make all channels are the same type.
	if(channels.sharedChannelType() == Channel_TypeUnknown)
	{
		throw XqInternal("TIFF format can only deal with channels of the same type",
				__FILE__, __LINE__);
	}
	// reorder the channels for TIFF output...
	channels.reorderChannels();

	// Use lzw compression if the compression hasn't been specified.
	std::string& compressionStr = m_header.findAttribute<std::string>("compression");
	if(compressionStr == "unknown")
		compressionStr = "lzw";

	// Now load the initial settings into the TIFF.
	CqTiffDirHandle dirHandle(m_fileHandle);
	dirHandle.writeHeader(header);

	/// \todo more checking & validation of the header.
}

void CqTiffOutputFile::writePixelsImpl(TqUchar* buffer, TqInt numScanlines)
{
	/// \todo implementaton
}

} // namespace Aqsis
