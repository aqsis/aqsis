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
 * \brief Scanline-oriented pixel access for OpenEXR input - implementation.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 *
 */

#include "exrinputfile.h"

#include <algorithm>
#include <cctype>

#include <OpenEXR/ImfInputFile.h>
#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfFrameBuffer.h>
#include <OpenEXR/Iex.h>

#include <aqsis/util/logging.h>
#include <aqsis/tex/texexception.h>

namespace Aqsis {

//------------------------------------------------------------------------------
/// Implementation of some helper functions

/// Get the aqsistex channel type corresponding to an OpenEXR channel type
EqChannelType channelTypeFromExr(Imf::PixelType exrType)
{
	switch(exrType)
	{
		case Imf::UINT:
			return Channel_Unsigned32;
		case Imf::FLOAT:
			return Channel_Float32;
		case Imf::HALF:
			return Channel_Float16;
		default:
			AQSIS_THROW_XQERROR(XqInternal, EqE_Bug, "Unknown OpenEXR pixel type");
	}
}

Imf::PixelType exrChannelType(EqChannelType type)
{
	switch(type)
	{
		case Channel_Unsigned32:
			return Imf::UINT;
		case Channel_Float32:
			return Imf::FLOAT;
		case Channel_Float16:
			return Imf::HALF;
		default:
				AQSIS_THROW_XQERROR(XqInternal, EqE_Bug,
						"Unsupported output pixel type for OpenEXR");
	}
}

/** \brief Get a compression string from an OpenEXR compression type
 *
 * \param compression - OpenEXR compression enum
 * \return short descriptive string describing the compression scheme.
 */
const char* exrCompressionToString(Imf::Compression compression)
{
	/// \todo Try to adjust these names to correspond better with TIFF?
	switch(compression)
	{
		case Imf::RLE_COMPRESSION:
			// run length encoding
			return "rle";
		case Imf::ZIPS_COMPRESSION:
			// zlib compression, one scan line at a time
			return "zips";
		case Imf::ZIP_COMPRESSION:
			// zlib compression, in blocks of 16 scan lines
			return "zip";
		case Imf::PIZ_COMPRESSION:
			// piz-based wavelet compression
			return "piz";
		case Imf::PXR24_COMPRESSION:
			// lossy 24-bit float compression
			return "pixar24";
		case Imf::NO_COMPRESSION:
			// no compression
			return "none";
		default:
			return "unknown";
	}
}

/** \brief Convert an OpenEXR header to our own header representation.
 *
 * \param exrHeader - input header
 * \param header - output header
 */
void convertHeader(const Imf::Header& exrHeader, CqTexFileHeader& header)
{
	// Set width, height
	const Imath::Box2i& dataBox = exrHeader.dataWindow();
	header.setWidth(dataBox.max.x - dataBox.min.x+1);
	header.setHeight(dataBox.max.y - dataBox.min.y+1);
	// display window
	const Imath::Box2i& displayBox = exrHeader.displayWindow();
	header.set<Attr::DisplayWindow>( SqImageRegion(
				displayBox.max.x - displayBox.min.x,
				displayBox.max.y - displayBox.min.y,
				displayBox.min.x - dataBox.min.x,
				displayBox.min.y - dataBox.min.y) );

	// Set tiling information ?

	// Aspect ratio
	header.set<Attr::PixelAspectRatio>(exrHeader.pixelAspectRatio());

	TqChannelNameMap channelNameMap;
	// Convert channel representation
	const Imf::ChannelList& exrChannels = exrHeader.channels();
	CqChannelList& channels = header.channelList();
	for(Imf::ChannelList::ConstIterator i = exrChannels.begin();
			i != exrChannels.end(); ++i)
	{
		// use lower case names for channels; OpenEXR uses upper case.
		std::string chanName = i.name();
		std::transform(chanName.begin(), chanName.end(), chanName.begin(),
				::tolower);
		channelNameMap[chanName] = i.name();
		channels.addChannel( SqChannelInfo(chanName,
				channelTypeFromExr(i.channel().type)) );
	}
	header.set<Attr::ExrChannelNameMap>(channelNameMap);
	channels.reorderChannels();

	// Set compresssion type
	header.set<Attr::Compression>(exrCompressionToString(exrHeader.compression()));
}

//------------------------------------------------------------------------------
// CqExrInputFile - implementation

CqExrInputFile::CqExrInputFile(const boostfs::path& fileName)
	: m_header(),
	m_exrFile()
{
	try
	{
		m_exrFile.reset(new Imf::InputFile(native(fileName).c_str()));
	}
	catch(Iex::BaseExc &e)
	{
		AQSIS_THROW_XQERROR(XqBadTexture, EqE_BadFile, e.what());
	}
	convertHeader(m_exrFile->header(), m_header);
}

/*
CqExrInputFile::CqExrInputFile(std::istream& inStream)
	: m_header(),
	m_exrFile()
{
	initialize();
}
*/

boostfs::path CqExrInputFile::fileName() const
{
	return m_exrFile->fileName();
}

void CqExrInputFile::readPixelsImpl(TqUint8* buffer,
		TqInt startLine, TqInt numScanlines) const
{
	// correct the start line for OpenEXR conventions
	const Imath::Box2i& dataWindow = m_exrFile->header().dataWindow();
	startLine += dataWindow.min.y;
	// Set up an OpenEXR framebuffer
	Imf::FrameBuffer frameBuffer;
	const CqChannelList& channels = m_header.channelList();
	const TqChannelNameMap& nameMap = m_header.find<Attr::ExrChannelNameMap>();
	const TqInt xStride = channels.bytesPerPixel();
	const TqInt yStride = m_header.width()*xStride;
	// In OpenEXR, the buffer base pointer is assumed to point at the
	// coordinates of the (0,0) pixel.  We need to correct our buffer pointer
	// by subtracting the offset to (0,0) from the start of the data.
	buffer -= dataWindow.min.x*xStride + dataWindow.min.y*yStride;
	for(TqInt i = 0; i < channels.numChannels(); ++i)
	{
		frameBuffer.insert(nameMap.find(channels[i].name)->second.c_str(),
				Imf::Slice(
					exrChannelType(channels[i].type),
					reinterpret_cast<char*>(buffer + channels.channelByteOffset(i)),
					xStride,
					yStride
					)
				);
	}
	m_exrFile->setFrameBuffer(frameBuffer);
	// Read in the pixels
	m_exrFile->readPixels(startLine, startLine + numScanlines - 1);
}

} // namespace Aqsis
