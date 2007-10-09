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
	m_fileHandle(new CqTiffFileHandle(fileName, "r"))
{
	initialize();
}

CqTiffInputFile::CqTiffInputFile(std::istream& inStream)
	: m_header(),
	m_fileHandle(new CqTiffFileHandle(inStream))
{
	initialize();
}

const std::string& CqTiffInputFile::fileName() const
{
	return m_fileHandle->fileName();
}

void CqTiffInputFile::readPixelsImpl(TqUchar* buffer,
		TqInt startLine, TqInt numScanlines) const
{
	CqTiffDirHandle dirHandle(m_fileHandle);
	tsize_t bytesPerRow = TIFFScanlineSize(dirHandle.tiffPtr());
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

void CqTiffInputFile::initialize()
{
	CqTiffDirHandle dirHandle(m_fileHandle);
	dirHandle.fillHeader(m_header);
	if(m_header.findAttribute<bool>("isTiled"))
		throw XqInternal("Can't read tiled tiff files", __FILE__, __LINE__);
	/// \todo We'll need to add something to deal with generic RGBA tiff
	/// reading at some stage.
}

} // namespace Aqsis
