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

#include "tiffdirhandle.h"

namespace Aqsis {

CqTiffOutputFile::CqTiffOutputFile(const std::string& fileName, const CqTexFileHeader& header)
	: m_header(header),
	m_currentLine(0),
	m_fileHandle(new CqTiffFileHandle(fileName.c_str(), "w"))
{
	initialize();
}

CqTiffOutputFile::CqTiffOutputFile(std::ostream& outStream, const CqTexFileHeader& header)
	: m_header(header),
	m_currentLine(0),
	m_fileHandle(new CqTiffFileHandle(outStream))
{
	initialize();
}

const char* CqTiffOutputFile::fileName() const
{
	return m_fileHandle->fileName().c_str();
}

void CqTiffOutputFile::initialize()
{
	CqChannelList& channelList = m_header.channelList();
	// make all channels are the same type.
	if(channelList.sharedChannelType() == Channel_TypeUnknown)
		AQSIS_THROW(XqInternal, "tiff cannot store multiple pixel types in the same image");

	// Maybe we should check here whether channels need reordering by name...

	// Use lzw compression if the compression hasn't been specified.
	std::string& compressionStr = m_header.find<Attr::Compression>();
	if(compressionStr == "unknown")
		compressionStr = "lzw";

	// Timestamp the file.
	m_header.setTimestamp();

	/// \todo more checking & validation of the header.

	// Now load the initial settings into the TIFF.
	CqTiffDirHandle dirHandle(m_fileHandle);
	dirHandle.writeHeader(m_header);
}

void CqTiffOutputFile::writePixelsImpl(const CqMixedImageBuffer& buffer)
{
	if(!buffer.channelList().channelTypesMatch(header().channelList()))
		AQSIS_THROW(XqInternal, "Buffer and file channels don't match");
	CqTiffDirHandle dirHandle(m_fileHandle);
	// Simplest possible implementation using scanline TIFF I/O.  Could use
	// Strip-based IO if performance is ever a problem here.
	const TqUint8* rawBuf = buffer.rawData();
	const TqInt rowStride = buffer.channelList().bytesPerPixel()*buffer.width();
	const TqInt endLine = m_currentLine + buffer.height();
	for(TqInt line = m_currentLine; line < endLine; ++line)
	{
		TIFFWriteScanline(dirHandle.tiffPtr(), reinterpret_cast<tdata_t>(const_cast<TqUint8*>(rawBuf)),
				static_cast<uint32>(line));
		rawBuf += rowStride;
	}
	m_currentLine = endLine;
}

} // namespace Aqsis
