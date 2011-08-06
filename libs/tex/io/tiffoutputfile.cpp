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

#include "tiffoutputfile.h"

#include <cstring>  // for memcpy()

#include <boost/scoped_array.hpp>

#include <aqsis/math/math.h>
#include "tiffdirhandle.h"

namespace Aqsis {

CqTiffOutputFile::CqTiffOutputFile(const boostfs::path& fileName,
		const CqTexFileHeader& header)
	: m_header(header),
	m_currentLine(0),
	m_fileHandle(new CqTiffFileHandle(fileName, "w"))
{
	initialize();
}

CqTiffOutputFile::CqTiffOutputFile(std::ostream& outStream,
		const CqTexFileHeader& header)
	: m_header(header),
	m_currentLine(0),
	m_fileHandle(new CqTiffFileHandle(outStream))
{
	initialize();
}

boostfs::path CqTiffOutputFile::fileName() const
{
	return m_fileHandle->fileName();
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
		AQSIS_THROW_XQERROR(XqInternal, EqE_Limit,
			"tiff cannot store multiple pixel types in the same image");

	// Use lzw compression if the compression hasn't been specified.
	if(!m_header.findPtr<Attr::Compression>())
		m_header.set<Attr::Compression>("lzw");

	// Timestamp the file.
	m_header.setTimestamp();

	/// \todo more checking & validation of the header.

	// Now load the initial settings into the TIFF.
	CqTiffDirHandle dirHandle(m_fileHandle);
	dirHandle.writeHeader(m_header);
}

void CqTiffOutputFile::newSubImage(TqInt width, TqInt height)
{
	m_header.setWidth(width);
	m_header.setHeight(height);

	// It might be reasonable to trim the header for directories after the
	// first (?)  Any trimming should consider the effect on CqTiffInputFile...
	nextSubImage(m_header);
}

void CqTiffOutputFile::newSubImage(const CqTexFileHeader& header)
{
	m_header = header;
	nextSubImage(m_header);
}

void CqTiffOutputFile::nextSubImage(const CqTexFileHeader& header)
{
	m_fileHandle->writeDirectory();
	m_currentLine = 0;

	// Write header data to this directory.
	CqTiffDirHandle dirHandle(m_fileHandle);
	dirHandle.writeHeader(header);
}

void CqTiffOutputFile::writeScanlinePixels(const CqMixedImageBuffer& buffer)
{
	CqTiffDirHandle dirHandle(m_fileHandle);
	// Simplest possible implementation using scanline TIFF I/O.  Could use
	// Strip-based IO if performance is ever a problem here.
	const TqUint8* rawBuf = buffer.rawData();
	const TqInt rowStride = buffer.channelList().bytesPerPixel()*buffer.width();
	const TqInt endLine = m_currentLine + buffer.height();
	// Temporary buffer for scanlines.  We need to copy the data into here
	// since libtiff trashes the buffer when encoding is turned on.  (The TIFF
	// docs don't seem to mention this though, ugh.)
	boost::scoped_array<TqUint8> lineBuf(new TqUint8[rowStride]);
	for(TqInt line = m_currentLine; line < endLine; ++line)
	{
		// copy the data into temp buffer.
		std::memcpy(lineBuf.get(), rawBuf, rowStride);
		// write data
		TIFFWriteScanline( dirHandle.tiffPtr(), reinterpret_cast<tdata_t>(lineBuf.get()),
				static_cast<uint32>(line) );
		rawBuf += rowStride;
	}
	m_currentLine = endLine;
}

void CqTiffOutputFile::writeTiledPixels(const CqMixedImageBuffer& buffer)
{
	SqTileInfo tileInfo = m_header.find<Attr::TileInfo>();
	// Check that the buffer has a height that is a multiple of the tile height.
	if( buffer.height() % tileInfo.height != 0
		&& m_currentLine + buffer.height() != m_header.height() )
	{
		AQSIS_THROW_XQERROR(XqInternal, EqE_Bug,
				"pixel buffer with height = " << buffer.height() << " must be a multiple "
				"of requested tile height (= " << tileInfo.height << ") or run exactly to "
				"the full image height (= " << m_header.height() << ").");
	}

	CqTiffDirHandle dirHandle(m_fileHandle);
	const TqUint8* rawBuf = buffer.rawData();
	const TqInt bytesPerPixel = buffer.channelList().bytesPerPixel();
	boost::scoped_array<TqUint8> tileBuf(
			new TqUint8[bytesPerPixel*tileInfo.width*tileInfo.height]);
	const TqInt rowStride = bytesPerPixel*buffer.width();
	const TqInt tileRowStride = bytesPerPixel*tileInfo.width;
	const TqInt endLine = m_currentLine + buffer.height();
	const TqInt numTileCols = (buffer.width()-1)/tileInfo.width + 1;
	for(TqInt line = m_currentLine; line < endLine; line += tileInfo.height)
	{
		// srcBuf will point to the beginning of the memory region which will
		// become the tile.
		const TqUint8* srcBuf = rawBuf;
		for(TqInt tileCol = 0; tileCol < numTileCols; ++tileCol)
		{
			const TqInt tileDataLen = min(tileRowStride,
					rowStride - tileCol*tileRowStride);
			const TqInt tileDataHeight = min(tileInfo.height, buffer.height() - line);
			// Copy parts of the scanlines into the tile buffer.
			stridedCopy(tileBuf.get(), tileRowStride, srcBuf, rowStride,
					tileDataHeight, tileDataLen);

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
	{
		AQSIS_THROW_XQERROR(XqInternal, EqE_Bug,
				"Buffer and file channels don't match");
	}
	if(m_header.findPtr<Attr::TileInfo>())
		writeTiledPixels(buffer);
	else
		writeScanlinePixels(buffer);
}

} // namespace Aqsis
