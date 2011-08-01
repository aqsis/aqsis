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
