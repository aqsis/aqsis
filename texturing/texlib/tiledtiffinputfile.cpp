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
 * \brief Tiled TIFF input interface.
 *
 * \author Chris Foster
 */

#include "tiledtiffinputfile.h"

#include <boost/scoped_array.hpp>

#include "texexception.h"

namespace Aqsis {

CqTiledTiffInputFile::CqTiledTiffInputFile(const std::string& fileName)
	: m_header(),
	m_fileHandle(new CqTiffFileHandle(fileName, "r")),
	m_imageIndex(0),
	m_numDirs(m_fileHandle->numDirectories()),
	m_tileInfo(0,0),
	m_width(0),
	m_height(0)
{
	setDirectory(m_imageIndex);
}

const char* CqTiledTiffInputFile::fileName() const
{
	return m_fileHandle->fileName().c_str();
}

EqImageFileType CqTiledTiffInputFile::fileType() const
{
	return ImageFile_Tiff;
}

const CqTexFileHeader& CqTiledTiffInputFile::header() const
{
	return m_header;
}

SqTileInfo CqTiledTiffInputFile::tileInfo() const
{
	return m_tileInfo;
}

void CqTiledTiffInputFile::setImageIndex(TqInt newIndex)
{
	assert(newIndex >= 0);
	setDirectory(newIndex);
}

TqInt CqTiledTiffInputFile::imageIndex() const
{
	return m_imageIndex;
}

TqInt CqTiledTiffInputFile::numSubImages() const
{
	return m_numDirs;
}

void CqTiledTiffInputFile::readTileImpl(TqUint8* buffer, TqInt x, TqInt y,
		const SqTileInfo tileSize) const
{
	CqTiffDirHandle dirHandle(m_fileHandle);
	if((x+1)*m_tileInfo.width > m_header.width()
			|| (y+1)*m_tileInfo.height > m_header.height())
	{
		// Here we handle a special case where the tile overlaps either the
		// right or bottom edge of the image.  In this case, libtiff reads in
		// the tile as the same size as all other tiles, not touching the parts
		// of buffer outside the image.  We want to truncate the tile instead.
		boost::scoped_array<TqUint8> tmpBuf(
				new TqUint8[TIFFTileSize(dirHandle.tiffPtr())]);
		TIFFReadTile(dirHandle.tiffPtr(), static_cast<tdata_t>(tmpBuf.get()), x, y, 0, 0);
		TqInt bytesPerPixel = m_header.channelList().bytesPerPixel();
		stridedCopy(buffer, tileSize.width*bytesPerPixel, tmpBuf.get(),
				m_tileInfo.width*bytesPerPixel, tileSize.height,
				tileSize.width*bytesPerPixel);
	}
	else
	{
		// Simple case for wholly contained buffers - the provided buffer is
		// the correct size, and we get libtiff to read directly into it.
		TIFFReadTile(dirHandle.tiffPtr(), static_cast<tdata_t>(buffer), x, y, 0, 0);
	}
}

void CqTiledTiffInputFile::setDirectory(tdir_t newDir)
{
	if(newDir >= m_numDirs)
	{
		AQSIS_THROW(XqInternal, "TIFF directory " << newDir
				<< " out of range [0," << m_numDirs-1 << "]");
	}
	m_imageIndex = newDir;

	CqTiffDirHandle dirHandle(m_fileHandle);
	dirHandle.fillHeader(m_header);
	if(m_header.find<Attr::TiffUseGenericRGBA>())
	{
		AQSIS_THROW(XqBadTexture, "Usupported TIFF pixel format");
	}

	// Load the tile information, or complain if it's not present.
	SqTileInfo* tileInfoPtr = m_header.findPtr<Attr::TileInfo>();
	if(!tileInfoPtr)
	{
		AQSIS_THROW(XqBadTexture, "TIFF file \"" << fileName()
				<< "\" is not tiled");
	}
	m_tileInfo = *tileInfoPtr;
	m_width = m_header.width();
	m_height = m_header.height();
}

} // namespace Aqsis
