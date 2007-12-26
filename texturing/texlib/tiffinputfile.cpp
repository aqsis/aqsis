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

#include "tiffdirhandle.h"

namespace Aqsis {

//------------------------------------------------------------------------------
// CqTiffInputFile - implementation

CqTiffInputFile::CqTiffInputFile(const std::string& fileName)
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

const char* CqTiffInputFile::fileName() const
{
	return m_fileHandle->fileName().c_str();
}

void CqTiffInputFile::setImageIndex(TqInt newIndex)
{
	if(newIndex < 0)
		throw XqInternal((boost::format("Received invalid image index %d")
				% newIndex).str(), __FILE__, __LINE__);
	setDirectory(newIndex);
}

// Warning: don't use this function from another member which already has a
// lock on the underlying file handle.
TqInt CqTiffInputFile::numSubImages() const
{
	return TIFFNumberOfDirectories(
			CqTiffDirHandle(m_fileHandle,m_imageIndex).tiffPtr());
}

void CqTiffInputFile::readPixelsImpl(TqUchar* buffer,
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
		if(m_header.find<Attr::IsTiled>())
			readPixelsTiled(buffer, startLine, numScanlines);
		else
			readPixelsStripped(buffer, startLine, numScanlines);
	}
}

void CqTiffInputFile::readPixelsStripped(TqUchar* buffer, TqInt startLine,
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

void CqTiffInputFile::readPixelsTiled(TqUchar* buffer, TqInt startLine,
		TqInt numScanlines) const
{
	// Load in relevant tiles; discard the excess.  At this stage, we don't do
	// any caching, under the assumption that the user will want to load a
	// bunch of scanlines at once.  If they load a single scanline at a time,
	// this will be very inefficient.
	CqTiffDirHandle dirHandle(m_fileHandle, m_imageIndex);

	// lines of interest will be startLine to endLine (exclusive)
	TqInt endLine = startLine + numScanlines;

	TqInt tileWidth = m_header.find<Attr::TileWidth>();
	TqInt tileHeight = m_header.find<Attr::TileHeight>();
	// Compute the boundaries of the smallest tiled region containing the
	// scanlines we're interested in.
	TqInt startTileLine = (startLine/tileHeight) * tileHeight;
	// endTileLine is the start line of the row of tiles coming *after* the
	// last row we want to load.
	TqInt endTileLine = ((endLine-1)/tileHeight + 1) * tileHeight;

	TqInt width = m_header.width();
	TqInt bytesPerPixel = m_header.channelList().bytesPerPixel();
	TqInt lineSize = bytesPerPixel*width;
	TqInt tileLineSize = bytesPerPixel*tileWidth;
	TqInt tileSize = tileLineSize*tileHeight;

	// Buffer to hold tiles read using libtiff.
	boost::shared_array<TqUchar> tempTileBuf(
			static_cast<TqUchar*>(_TIFFmalloc(tileSize)),
			_TIFFfree);

	for(TqInt y = startTileLine; y < endTileLine; y += tileHeight)
	{
		// Determine how much of the beginning and end of the current strip
		// should be skipped due to the tiles falling off the range of
		// relevant scanlines.
		TqInt outStripSkipStart =
			(y == startTileLine) ? startLine - startTileLine : 0;
		TqInt outStripSkipEnd =
			(y + tileHeight == endTileLine) ? endTileLine - endLine : 0;
		// The output "Strip height" for the current line of tiles might be
		// smaller than the tile height; take this into account here.
		TqInt stripHeightOut = tileHeight - outStripSkipStart - outStripSkipEnd;

		for(TqInt x = 0; x < width; x += tileWidth)
		{
			// Grab the tile using libtiff
			TIFFReadTile(dirHandle.tiffPtr(),
					static_cast<tdata_t>(tempTileBuf.get()), x, y, 0, 0);
			// The width of the tile in the output image is smaller when it
			// falls of the RHS of the image.
			TqInt tileLineSizeOut = min(tileLineSize, bytesPerPixel*(width-x));
			// Copy the tile into the output buffer.
			TqUchar* lineBuf = buffer + x*bytesPerPixel;
			TqUchar* tileLineBuf = static_cast<TqUchar*>(tempTileBuf.get())
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

void CqTiffInputFile::readPixelsRGBA(TqUchar* buffer, TqInt startLine,
		TqInt numScanlines) const
{
	throw XqInternal("readPixelsRGBA not implemented", __FILE__, __LINE__);
}

void CqTiffInputFile::setDirectory(tdir_t newDir)
{
	const tdir_t numDirs = numSubImages();
	if(newDir >= numDirs)
		throw XqInternal((boost::format("TIFF directory %d out of range [0,%d]")
				% newDir % (numDirs-1)).str(), __FILE__, __LINE__);
	m_imageIndex = newDir;

	CqTiffDirHandle dirHandle(m_fileHandle, m_imageIndex);
	dirHandle.fillHeader(m_header);
	/// \todo We'll need to add something to deal with generic RGBA tiff
	/// reading at some stage.
}

} // namespace Aqsis
