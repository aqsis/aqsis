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
	m_fileStream(native(fileName).c_str(), std::ios::in | std::ios::binary),
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
