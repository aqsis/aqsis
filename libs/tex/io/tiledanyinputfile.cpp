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
 * \brief Tiled input wrapper for files which don't really hold tiled data.
 *
 * \author Chris Foster
 */

#include "tiledanyinputfile.h"

#include <aqsis/tex/io/itexinputfile.h>
#include <aqsis/util/smartptr.h>

namespace Aqsis {

CqTiledAnyInputFile::CqTiledAnyInputFile(const boostfs::path& fileName)
	: m_texFile(IqTexInputFile::open(fileName)),
	m_tileInfo(m_texFile->header().width(), m_texFile->header().height())
{ }

boostfs::path CqTiledAnyInputFile::fileName() const
{
	return m_texFile->fileName();
}

EqImageFileType CqTiledAnyInputFile::fileType() const
{
	return m_texFile->fileType();
}

const CqTexFileHeader& CqTiledAnyInputFile::header(TqInt index) const
{
	return m_texFile->header();
}

SqTileInfo CqTiledAnyInputFile::tileInfo() const
{
	return m_tileInfo;
}

TqInt CqTiledAnyInputFile::numSubImages() const
{
	return 1;
}

TqInt CqTiledAnyInputFile::width(TqInt index) const
{
	assert(index == 0);
	return m_tileInfo.width;
}

TqInt CqTiledAnyInputFile::height(TqInt index) const
{
	assert(index == 0);
	return m_tileInfo.height;
}

void CqTiledAnyInputFile::readTileImpl(TqUint8* buffer, TqInt tileX, TqInt tileY,
		TqInt subImageIdx, const SqTileInfo tileSize) const
{
	assert(tileX == 0);
	assert(tileY == 0);
	assert(m_tileInfo.width == tileSize.width);
	assert(m_tileInfo.height == tileSize.height);
	m_texFile->readPixelsImpl(buffer, 0, tileSize.height);
}

} // namespace Aqsis

