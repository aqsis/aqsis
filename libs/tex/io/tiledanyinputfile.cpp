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

