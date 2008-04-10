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

#include <cstring>  // for memcpy.

#include <boost/scoped_array.hpp>

#include "aqsismath.h"
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

EqImageFileType CqTiffOutputFile::fileType()
{
	return ImageFile_Tiff;
}

const CqTexFileHeader& CqTiffOutputFile::header() const
{
	return m_header;
}

TqInt CqTiffOutputFile::currentLine() const
{
	return m_currentLine;
}

void CqTiffOutputFile::initialize()
{
	// make sure all channels are the same type.
	if(m_header.channelList().sharedChannelType() == Channel_TypeUnknown)
		AQSIS_THROW(XqInternal, "tiff cannot store multiple pixel types in the same image");

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

void CqTiffOutputFile::newSubImage(TqInt width, TqInt height)
{
	m_header.set<Attr::Width>(width);
	m_header.set<Attr::Height>(height);

	m_fileHandle->writeDirectory();

	// Write header data to this directory.
	/// \todo The header may be trimmed for directories after the first.
	CqTiffDirHandle dirHandle(m_fileHandle);
	dirHandle.writeHeader(m_header);
}

void CqTiffOutputFile::writeScanlinePixels(const CqMixedImageBuffer& buffer)
{
	CqTiffDirHandle dirHandle(m_fileHandle);
	// Simplest possible implementation using scanline TIFF I/O.  Could use
	// Strip-based IO if performance is ever a problem here.
	const TqUint8* rawBuf = buffer.rawData();
	const TqInt rowStride = buffer.channelList().bytesPerPixel()*buffer.width();
	const TqInt endLine = m_currentLine + buffer.height();
	for(TqInt line = m_currentLine; line < endLine; ++line)
	{
		TIFFWriteScanline( dirHandle.tiffPtr(),
				reinterpret_cast<tdata_t>(const_cast<TqUint8*>(rawBuf)),
				static_cast<uint32>(line) );
		rawBuf += rowStride;
	}
	m_currentLine = endLine;
}


namespace {

/** Strided memory copy.
 *
 * Copies numElems data elements from src to dest.  Each data element (eg,
 * contiguous group of pixels) has size given by elemSize bytes.  The stride
 * between one data element and the next is given in bytes.
 */
void stridedCopy(TqUint8* dest, TqInt destStride, const TqUint8* src, TqInt srcStride,
		TqInt numElems, TqInt elemSize)
{
	for(TqInt i = 0; i < numElems; ++i)
	{
		std::memcpy(dest, src, elemSize);
		dest += destStride;
		src += srcStride;
	}
}

} // unnamed namespace


void CqTiffOutputFile::writeTiledPixels(const CqMixedImageBuffer& buffer)
{
	SqTileInfo tileInfo = m_header.find<Attr::TileInfo>();
	// Check that the buffer has a height that is a multiple of the tile height.
	if( buffer.height() % tileInfo.height != 0
		&& m_currentLine + buffer.height() != m_header.height() )
	{
		AQSIS_THROW(XqInternal, "pixel buffer with height = " << buffer.height()
				<< " must be a multiple of requested tile height (= " << tileInfo.height
				<< ") or run exactly to the full image height (= " << m_header.height()
				<< ").");
	}

	CqTiffDirHandle dirHandle(m_fileHandle);
	const TqUint8* rawBuf = buffer.rawData();
	const TqInt bytesPerPixel = buffer.channelList().bytesPerPixel();
	boost::scoped_array<TqUint8> tileBuf(
			new TqUint8[bytesPerPixel*tileInfo.width*tileInfo.height]);
	const TqInt rowStride = bytesPerPixel*buffer.width();
	const TqInt endLine = m_currentLine + buffer.height();
	const TqInt tileCols = buffer.width()/tileInfo.width;
	for(TqInt line = m_currentLine; line < endLine; line += tileInfo.height)
	{
		// srcBuf will point to the beginning of the memory region which will
		// become the tile.
		const TqUint8* srcBuf = rawBuf;
		for(TqInt tileCol = 0; tileCol < tileCols; ++tileCol)
		{
			const TqInt tileRowStride = min(bytesPerPixel*tileInfo.width,
					rowStride - tileCol*bytesPerPixel*tileInfo.width);
			// Copy parts of the scanlines into the tile buffer.
			stridedCopy(tileBuf.get(), tileRowStride, srcBuf, rowStride,
					tileInfo.height, tileRowStride);

			TIFFWriteTile(dirHandle.tiffPtr(),
					reinterpret_cast<tdata_t>(const_cast<TqUint8*>(tileBuf.get())),
					tileCol*tileInfo.width, line, 0, 0);
			srcBuf += tileRowStride;
		}
		rawBuf += rowStride*tileInfo.height;
	}
	m_currentLine = endLine;
}

void CqTiffOutputFile::writePixelsImpl(const CqMixedImageBuffer& buffer)
{
	if(!buffer.channelList().channelTypesMatch(m_header.channelList()))
		AQSIS_THROW(XqInternal, "Buffer and file channels don't match");
	if(m_header.findPtr<Attr::TileInfo>())
		writeTiledPixels(buffer);
	else
		writeScanlinePixels(buffer);
}

} // namespace Aqsis
