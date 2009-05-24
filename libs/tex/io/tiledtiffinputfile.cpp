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
 * \author Chris Foster [chris42f (at) gmail (d0t) com]
 */

#include "tiledtiffinputfile.h"

#include <boost/scoped_array.hpp>

#include <aqsis/tex/texexception.h>

namespace Aqsis {

CqTiledTiffInputFile::CqTiledTiffInputFile(const boostfs::path& fileName)
	: m_headers(),
	m_fileHandle(new CqTiffFileHandle(fileName, "r")),
	m_numDirs(m_fileHandle->numDirectories()),
	m_tileInfo(0,0),
	m_widths(),
	m_heights()
{
	m_headers.reserve(m_numDirs);
	m_widths.reserve(m_numDirs);
	m_heights.reserve(m_numDirs);
	// Iterate through all subimages and check some conditions which will
	// become assumptions in the rest of the code.
	for(TqInt i = 0; i < m_numDirs; ++i)
	{
		CqTiffDirHandle dirHandle(m_fileHandle, i);
		boost::shared_ptr<CqTexFileHeader> tmpHeader(new CqTexFileHeader);
		dirHandle.fillHeader(*tmpHeader);
		// Check that the subimage is tiled.
		SqTileInfo* tileInfo= tmpHeader->findPtr<Attr::TileInfo>();
		if(!tileInfo)
		{
			AQSIS_THROW_XQERROR(XqBadTexture, EqE_BadFile, "TIFF file \""
				<< fileName << "\" has non-tiled sub-image " << i);
		}
		// Check that we can natively read the pixel format held in the TIFF.
		if(tmpHeader->find<Attr::TiffUseGenericRGBA>())
		{
			AQSIS_THROW_XQERROR(XqBadTexture, EqE_BadFile,
				"Unsupported TIFF pixel format");
		}
		if(i == 0)
		{
			// Store the tile info from the first sub-image only.  We force the
			// tile sizes for other sub-images to match this.
			m_tileInfo = *tileInfo;
		}
		else
		{
			// Check that the tile size is the same as the first image.
			if(m_tileInfo.width != tileInfo->width
					|| m_tileInfo.height != tileInfo->height)
			{
				AQSIS_THROW_XQERROR(XqBadTexture, EqE_BadFile, "TIFF file \""
						<< fileName << "\" has unequal tile sizes for sub-images");
			}
		}
		// Grab store the width and height of the current subimage.
		m_widths.push_back(tmpHeader->width());
		m_heights.push_back(tmpHeader->height());
		// Store the header itself.  Sometimes we really do need access to the
		// extra attributes (eg, matrices for occlusion sampling).  We could
		// define special extra methods for these, but it bloats up the
		// interface a bit.
		m_headers.push_back(tmpHeader);
	}
}

boostfs::path CqTiledTiffInputFile::fileName() const
{
	return m_fileHandle->fileName();
}

EqImageFileType CqTiledTiffInputFile::fileType() const
{
	return ImageFile_Tiff;
}

const CqTexFileHeader& CqTiledTiffInputFile::header(TqInt index) const
{
	if(index >= 0 && index < m_numDirs)
		return *m_headers[index];
	else
		return *m_headers[0];
}

SqTileInfo CqTiledTiffInputFile::tileInfo() const
{
	return m_tileInfo;
}

TqInt CqTiledTiffInputFile::numSubImages() const
{
	return m_numDirs;
}

TqInt CqTiledTiffInputFile::width(TqInt index) const
{
	assert(index < m_numDirs);
	return m_widths[index];
}

TqInt CqTiledTiffInputFile::height(TqInt index) const
{
	assert(index < m_numDirs);
	return m_heights[index];
}

void CqTiledTiffInputFile::readTileImpl(TqUint8* buffer, TqInt x, TqInt y,
		TqInt subImageIdx, const SqTileInfo tileSize) const
{
	CqTiffDirHandle dirHandle(m_fileHandle, subImageIdx);
	if((x+1)*m_tileInfo.width > m_widths[subImageIdx]
			|| (y+1)*m_tileInfo.height > m_heights[subImageIdx])
	{
		// Here we handle a special case where the tile overlaps either the
		// right or bottom edge of the image.  In this case, libtiff reads in
		// the tile as the same size as all other tiles, not touching the parts
		// of buffer outside the image.  We want to truncate the tile instead.
		boost::scoped_array<TqUint8> tmpBuf(
				new TqUint8[TIFFTileSize(dirHandle.tiffPtr())]);
		TIFFReadTile(dirHandle.tiffPtr(), static_cast<tdata_t>(tmpBuf.get()),
				x*m_tileInfo.width, y*m_tileInfo.height, 0, 0);
		TqInt bytesPerPixel = m_headers[subImageIdx]->channelList().bytesPerPixel();
		stridedCopy(buffer, tileSize.width*bytesPerPixel, tmpBuf.get(),
				m_tileInfo.width*bytesPerPixel, tileSize.height,
				tileSize.width*bytesPerPixel);
	}
	else
	{
		// Simple case for wholly contained buffers - the provided buffer is
		// the correct size, and we get libtiff to read directly into it.
		TIFFReadTile(dirHandle.tiffPtr(), static_cast<tdata_t>(buffer),
				x*m_tileInfo.width, y*m_tileInfo.height, 0, 0);
	}
}

} // namespace Aqsis
