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
 * \brief Scanline-oriented pixel access for OpenEXR input - implementation.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 *
 */

#include "exrinputfile.h"

#include <algorithm>
#include <cctype>

#include <ImfInputFile.h>
#include <ImfChannelList.h>
#include <ImfFrameBuffer.h>
#include <Iex.h>

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
		m_exrFile.reset(new Imf::InputFile(fileName.file_string().c_str()));
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
