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
 * \brief A C++ wrapper around tiff files for the functions of interest in aqsis.
 *
 * \author Chris Foster
 */

#include "tiffdirhandle.h"

#include <tiffio.hxx>

#include "aqsismath.h"

namespace Aqsis
{

//------------------------------------------------------------------------------
// CqTiffDirHandle
//------------------------------------------------------------------------------

CqTiffDirHandle::CqTiffDirHandle(boost::shared_ptr<CqTiffFileHandle> fileHandle, const tdir_t dirIdx)
	: m_fileHandle(fileHandle)
{
	fileHandle->setDirectory(dirIdx);
}

tdir_t CqTiffDirHandle::dirIndex() const
{
	return m_fileHandle->m_currDir;
}

bool CqTiffDirHandle::isLastDirectory() const
{
	return static_cast<bool>(TIFFLastDirectory(tiffPtr()));
}


//------------------------------------------------------------------------------
// CqTiffFileHandle
//------------------------------------------------------------------------------

void safeTiffClose(TIFF* tif)
{
	if(tif)
		TIFFClose(tif);
}

CqTiffFileHandle::CqTiffFileHandle(const std::string& fileName, const char* openMode)
	: m_tiffPtr(TIFFOpen(fileName.c_str(), openMode), safeTiffClose),
	m_currDir(0)
{
	if(!m_tiffPtr)
	{
		throw XqEnvironment( boost::str(boost::format("Could not open tiff file '%s'")
					% fileName).c_str(), __FILE__, __LINE__);
	}
}

CqTiffFileHandle::CqTiffFileHandle(std::istream& inputStream)
	: m_tiffPtr(TIFFStreamOpen("stream", &inputStream), safeTiffClose),
	m_currDir(0)
{
	if(!m_tiffPtr)
	{
		throw XqInternal("Could not use input stream for tiff", __FILE__, __LINE__);
	}
}

const std::string& CqTiffFileHandle::fileName() const
{
	return m_fileName;
}

void CqTiffFileHandle::setDirectory(tdir_t dirIdx)
{
	if(dirIdx != m_currDir)
	{
		if(!TIFFSetDirectory(m_tiffPtr.get(), dirIdx))
			throw XqInternal("Invalid Tiff directory", __FILE__, __LINE__);
		m_currDir = dirIdx;
	}
}


//------------------------------------------------------------------------------
} // namespace Aqsis
