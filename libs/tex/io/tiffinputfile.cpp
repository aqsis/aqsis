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
 * \brief Scanline-oriented pixel access for TIFF input - implementation.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 *
 */

#include "tiffinputfile.h"

#include "boost/scoped_array.hpp"

#include "tiffdirhandle.h"

namespace Aqsis {

//------------------------------------------------------------------------------
// CqTiffInputFile - implementation

CqTiffInputFile::CqTiffInputFile(const boostfs::path& fileName)
	: m_header(),
	m_fileHandle(new CqTiffFileHandle(fileName, "r")),
	m_imageIndex(0)
{
	setDirectory(m_imageIndex);
}

CqTiffInputFile::CqTiffInputFile(std::istream& inStream)
	: m_header(),
	m_fileHandle(new CqTiffFileHandle(inStream)),
	m_imageIndex(0)
{
	setDirectory(m_imageIndex);
}

boostfs::path CqTiffInputFile::fileName() const
{
	return m_fileHandle->fileName();
}

void CqTiffInputFile::setImageIndex(TqInt newIndex)
{
	if(newIndex < 0)
		AQSIS_THROW_XQERROR(XqInternal, EqE_Bug, "Cannot set negative image index.");
	setDirectory(newIndex);
}

// Warning: don't use this function from another member which already has a
// lock on the underlying file handle.
TqInt CqTiffInputFile::numSubImages() const
{
	return m_fileHandle->numDirectories();
}

void CqTiffInputFile::readPixelsImpl(TqUint8* buffer,
		TqInt startLine, TqInt numScanlines) const
{
	if(m_header.find<Attr::TiffUseGenericRGBA>())
	{
		// Use generic libtiff RGBA when we encounter unusual TIFF formats.
		readPixelsRGBA(buffer, startLine, numScanlines);
	}
	else
	{
		// The usual case - use aqsistex native pixel input
		if(m_header.findPtr<Attr::TileInfo>())
			readPixelsTiled(buffer, startLine, numScanlines);
		else
			readPixelsStripped(buffer, startLine, numScanlines);
	}
}

void CqTiffInputFile::readPixelsStripped(TqUint8* buffer, TqInt startLine,
		TqInt numScanlines) const
{
	CqTiffDirHandle dirHandle(m_fileHandle, m_imageIndex);
	const tsize_t bytesPerRow = TIFFScanlineSize(dirHandle.tiffPtr());
	// Implement simplest possible version for now - read in scanlines
	// sequentially...  Looking at the source code for libtiff, this should be
	// reasonably efficient.
	//
	// In the unlikely event that it turns out to be a problem, we should look
	// at using the strip-based interface via TIFFReadEncodedStrip & friends.
	for(TqInt line = startLine; line < startLine + numScanlines; ++line)
	{
		TIFFReadScanline(dirHandle.tiffPtr(), reinterpret_cast<tdata_t>(buffer),
				static_cast<uint32>(line));
		buffer += bytesPerRow;
	}
}

void CqTiffInputFile::readPixelsTiled(TqUint8* buffer, TqInt startLine,
		TqInt numScanlines) const
{
	// Load in relevant tiles; discard the excess.  At this stage, we don't do
	// any caching, under the assumption that the user will want to load a
	// bunch of scanlines at once.  If they load a single scanline at a time,
	// this will be very inefficient.
	CqTiffDirHandle dirHandle(m_fileHandle, m_imageIndex);

	// lines of interest will be startLine to endLine (exclusive)
	TqInt endLine = startLine + numScanlines;

	SqTileInfo tileInfo = m_header.find<Attr::TileInfo>();
	// Compute the boundaries of the smallest tiled region containing the
	// scanlines we're interested in.
	TqInt startTileLine = (startLine/tileInfo.height) * tileInfo.height;
	// endTileLine is the start line of the row of tiles coming *after* the
	// last row we want to load.
	TqInt endTileLine = ((endLine-1)/tileInfo.height + 1) * tileInfo.height;

	TqInt width = m_header.width();
	TqInt bytesPerPixel = m_header.channelList().bytesPerPixel();
	TqInt lineSize = bytesPerPixel*width;
	TqInt tileLineSize = bytesPerPixel*tileInfo.width;
	TqInt tileSize = tileLineSize*tileInfo.height;

	// Buffer to hold tiles read using libtiff.
	boost::shared_array<TqUint8> tempTileBuf(
			static_cast<TqUint8*>(_TIFFmalloc(tileSize)),
			_TIFFfree);

	for(TqInt y = startTileLine; y < endTileLine; y += tileInfo.height)
	{
		// Determine how much of the beginning and end of the current strip
		// should be skipped due to the tiles falling off the range of
		// relevant scanlines.
		TqInt outStripSkipStart =
			(y == startTileLine) ? startLine - startTileLine : 0;
		TqInt outStripSkipEnd =
			(y + tileInfo.height == endTileLine) ? endTileLine - endLine : 0;
		// The output "Strip height" for the current line of tiles might be
		// smaller than the tile height; take this into account here.
		TqInt stripHeightOut = tileInfo.height - outStripSkipStart - outStripSkipEnd;

		for(TqInt x = 0; x < width; x += tileInfo.width)
		{
			// Grab the tile using libtiff
			TIFFReadTile(dirHandle.tiffPtr(),
					static_cast<tdata_t>(tempTileBuf.get()), x, y, 0, 0);
			// The width of the tile in the output image is smaller when it
			// falls of the RHS of the image.
			TqInt tileLineSizeOut = min(tileLineSize, bytesPerPixel*(width-x));
			// Copy the tile into the output buffer.
			TqUint8* lineBuf = buffer + x*bytesPerPixel;
			TqUint8* tileLineBuf = static_cast<TqUint8*>(tempTileBuf.get())
				+ outStripSkipStart*tileLineSize;
			for(TqInt tileLine = 0; tileLine < stripHeightOut; ++tileLine)
			{
				_TIFFmemcpy(lineBuf, tileLineBuf, tileLineSizeOut);
				tileLineBuf += tileLineSize;
				lineBuf += lineSize;
			}
		}
		buffer += lineSize*stripHeightOut;
	}
}

void CqTiffInputFile::readPixelsRGBA(TqUint8* buffer, TqInt startLine,
		TqInt numScanlines) const
{
	CqTiffDirHandle dirHandle(m_fileHandle, m_imageIndex);
	// Support odd tiff formats using the generic RGBA tiff functionality.
	//
	// We read in the entire image each time readPixelsRGBA() is called; this
	// *will* cause a performance bottleneck if the user calls readPixels() on
	// only a small number of scanlines at a time.
	boost::scoped_array<uint32> inBuf(
			new uint32[m_header.width()*m_header.height()]);
	TIFFReadRGBAImageOriented(dirHandle.tiffPtr(), m_header.width(),
			m_header.height(), inBuf.get(), ORIENTATION_TOPLEFT, 0);

	// Unfortunately, the RGBA format which libtiff uses puts the RGBA bytes in
	// the opposite order which we'd like, so we need to swap them.  It's
	// possible to get around this using TIFFRGBAImageGet() but it's extra work
	// which somewhat defeats the purpose of using the RGBA functionality as a
	// catchall fallback anyway.
	TqInt width = m_header.width();
	TqInt bytesPerPixel = m_header.channelList().bytesPerPixel();
	assert(bytesPerPixel == 4);
	const uint32* inPtr = inBuf.get() + width*startLine;
	for(TqInt line = 0; line < numScanlines; ++line)
	{
		for(TqInt col = 0; col < width; ++col)
		{
			buffer[col*bytesPerPixel] = TIFFGetR(inPtr[col]);
			buffer[col*bytesPerPixel + 1] = TIFFGetG(inPtr[col]);
			buffer[col*bytesPerPixel + 2] = TIFFGetB(inPtr[col]);
			buffer[col*bytesPerPixel + 3] = TIFFGetA(inPtr[col]);
		}
		buffer += width*bytesPerPixel;
		inPtr += width;
	}
}

void CqTiffInputFile::setDirectory(tdir_t newDir)
{
	const tdir_t numDirs = numSubImages();
	if(newDir >= numDirs)
	{
		AQSIS_THROW_XQERROR(XqInternal, EqE_Bug, "TIFF directory "
			<< newDir << " out of range [0," << numDirs-1 << "]");
	}
	m_imageIndex = newDir;

	CqTiffDirHandle dirHandle(m_fileHandle, m_imageIndex);
	dirHandle.fillHeader(m_header);
}

} // namespace Aqsis
