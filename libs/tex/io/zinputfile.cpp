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
 * \brief Scanline-oriented pixel access for Aqsis z-file input.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 *
 */

#include "zinputfile.h"

#include <algorithm>
#include <vector>

#include <aqsis/tex/texexception.h>
#include <aqsis/version.h>

namespace Aqsis {

//------------------------------------------------------------------------------
// CqZInputFile Implementation

CqZInputFile::CqZInputFile(const boostfs::path& fileName)
	: m_header(),
	m_fileName(fileName),
	m_fileStream(fileName.file_string().c_str(), std::ios::in | std::ios::binary),
	m_dataBegin(0)
{
	if(!m_fileStream.is_open())
	{
		AQSIS_THROW_XQERROR(XqInvalidFile, EqE_NoFile,
				"Could not open z-file \"" << fileName << "\" for reading");
	}
	readHeader(m_fileStream, m_header);
	m_dataBegin = m_fileStream.tellg();
}

boostfs::path CqZInputFile::fileName() const
{
	return m_fileName;
}

EqImageFileType CqZInputFile::fileType() const
{
	return ImageFile_AqsisZfile;
}

const CqTexFileHeader& CqZInputFile::header() const
{
	return m_header;
}

void CqZInputFile::readPixelsImpl(TqUint8* buffer, TqInt startLine,
		TqInt numScanlines) const
{
	// Seek to beginning of data.
	std::istream::pos_type chunkStart = m_dataBegin
		+ std::istream::off_type(sizeof(TqFloat)*m_header.width()*startLine);
	m_fileStream.seekg(chunkStart);

	// Read in scanlines of data.
	m_fileStream.read(reinterpret_cast<char*>(buffer),
			sizeof(TqFloat)*m_header.width()*numScanlines);
}

void CqZInputFile::readHeader(std::istream& inStream, CqTexFileHeader& header)
{
	const char zFileMagicNum[] = "Aqsis ZFile";
	const TqInt magicNumSize = sizeof(zFileMagicNum)-1;
	const TqInt versionNumSize = sizeof(AQSIS_VERSION_STR)-1;
	std::vector<char> buf(max(magicNumSize, versionNumSize));

	// Read in magic number
	inStream.read(&buf[0], magicNumSize);
	if(!std::equal(buf.begin(), buf.begin() + magicNumSize, zFileMagicNum)
		|| inStream.gcount() != magicNumSize)
	{
		AQSIS_THROW_XQERROR(XqBadTexture, EqE_BadFile,
				"Magic number missmatch in zfile");
	}

	// Read in Aqsis version.  We require this to match the current aqsis version.
	inStream.read(&buf[0], versionNumSize);
	if(!std::equal(buf.begin(), buf.begin() + versionNumSize, AQSIS_VERSION_STR)
		|| inStream.gcount() != versionNumSize)
	{
		AQSIS_THROW_XQERROR(XqBadTexture, EqE_Version,
				"zfile was created with a different aqsis version");
	}

	// Read in map width
	TqUint width = 0;
	inStream.read(reinterpret_cast<char*>(&width), sizeof(width));
	if(inStream.gcount() != sizeof(width))
		AQSIS_THROW_XQERROR(XqBadTexture, EqE_BadFile,
			"cannot read width from aqsis z-file");
	// Read in map height
	TqUint height = 0;
	inStream.read(reinterpret_cast<char*>(&height), sizeof(height));
	if(inStream.gcount() != sizeof(height))
		AQSIS_THROW_XQERROR(XqBadTexture, EqE_BadFile,
			"cannot read height from aqsis z-file");

	// Read world to camera transformation matrix
	CqMatrix worldToCamera;
	worldToCamera.SetfIdentity(false);
	inStream.read(reinterpret_cast<char*>(worldToCamera.pElements()), 16*sizeof(TqFloat));
	if(inStream.gcount() != 16*sizeof(TqFloat))
		AQSIS_THROW_XQERROR(XqBadTexture,EqE_BadFile,
			"could not read world to camera matrix from aqsis z-file");

	// Read world to screen transformation matrix
	CqMatrix worldToScreen;
	worldToScreen.SetfIdentity(false);
	inStream.read(reinterpret_cast<char*>(worldToScreen.pElements()), 16*sizeof(TqFloat));
	if(inStream.gcount() != 16*sizeof(TqFloat))
		AQSIS_THROW_XQERROR(XqBadTexture, EqE_BadFile,
			"could not read world to screen matrix from aqsis z-file");

	// Save the read header attributes into the file header.
	header.setWidth(width);
	header.setHeight(height);
	header.set<Attr::WorldToScreenMatrix>(worldToScreen);
	header.set<Attr::WorldToCameraMatrix>(worldToCamera);
	// Complete the header with some other attributes implied by the file format
	header.set<Attr::TextureFormat>(TextureFormat_Shadow);
	header.channelList().addChannel(SqChannelInfo("z", Channel_Float32));
}

} // namespace Aqsis
