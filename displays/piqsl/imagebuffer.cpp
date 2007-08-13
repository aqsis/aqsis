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
 * \brief Implementation of CqImageBuffer and related stuff.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 * \author Paul C. Gregory (pgregory@aqsis.org)
 *
 */

#include "imagebuffer.h"

#include <boost/format.hpp>

#include "exception.h"
#include "logging.h"
#include "ndspy.h"

namespace Aqsis {


//------------------------------------------------------------------------------
// CqChannelInfoList implementation
CqChannelInfoList CqChannelInfoList::displayChannels()
{
    CqChannelInfoList displayChannels;
    displayChannels.addChannel(SqChannelInfo("r", Format_Unsigned8));
    displayChannels.addChannel(SqChannelInfo("g", Format_Unsigned8));
    displayChannels.addChannel(SqChannelInfo("b", Format_Unsigned8));
	return displayChannels;
}

void CqChannelInfoList::addChannel(const SqChannelInfo& newChan)
{
	m_channels.push_back(newChan);
	m_offsets.push_back(m_bytesPerPixel);
	m_bytesPerPixel += newChan.bytesPerPixel();
}

void CqChannelInfoList::reorderChannels()
{
	// If there are "r", "g", "b" and "a" channels, ensure they
	// are in the expected order.
	const char* elements[] = { "r", "g", "b", "a" };
	TqInt numElements = sizeof(elements) / sizeof(elements[0]);
	for(int elementIndex = 0; elementIndex < numElements; ++elementIndex)
	{
		for(TqListType::iterator channel = m_channels.begin(); channel != m_channels.end(); ++channel)
		{
			// If this entry in the channel list matches one in the expected list, 
			// move it to the right point in the channel list.
			if(channel->name == elements[elementIndex])
			{
				const SqChannelInfo temp = m_channels[elementIndex];
				m_channels[elementIndex] = *channel;
				*channel = temp;
				break;
			}
		}
	}
	recomputeByteOffsets();
}

CqChannelInfoList CqChannelInfoList::cloneAs8Bit() const
{
	CqChannelInfoList newChannels;
	for(const_iterator chInfo = m_channels.begin();
			chInfo != m_channels.end(); ++chInfo)
		newChannels.addChannel(SqChannelInfo(chInfo->name, Format_Unsigned8));
	return newChannels;
}

TqInt CqChannelInfoList::findChannelIndexImpl(const std::string& name) const
{
	TqUint index = 0;
	for(index = 0; index < m_channels.size(); ++index)
	{
		if(m_channels[index].name == name)
			break;
	}
	if(index < m_channels.size())
		return index;
	else
		return -1;
}

void CqChannelInfoList::recomputeByteOffsets()
{
	m_offsets.clear();
	TqUint offset = 0;
	for(TqListType::const_iterator chInfo = m_channels.begin();
			chInfo != m_channels.end(); ++chInfo)
	{
		m_offsets.push_back(offset);
		offset += chInfo->bytesPerPixel();
	}
	m_bytesPerPixel = offset;
}


//------------------------------------------------------------------------------
// CqImageBuffer implementation
CqImageBuffer::CqImageBuffer(const CqChannelInfoList& channels, TqUint width, TqUint height)
	: m_channelsInfo(channels),
	m_width(width),
	m_height(height),
	m_data(new TqUchar[width*height*channels.bytesPerPixel()])
{ }

CqImageBuffer::CqImageBuffer(const CqChannelInfoList& channels,
		boost::shared_array<TqUchar> data, TqUint width, TqUint height)
	: m_channelsInfo(channels),
	m_width(width),
	m_height(height),
	m_data(data)
{ }


// Simple image loading, uses TIFFReadRGBAImage to hide format details. Use the
// lower level reading facilities in the future.
boost::shared_ptr<CqImageBuffer> CqImageBuffer::loadFromTiff(TIFF* tif)
{
	if(!tif)
		return boost::shared_ptr<CqImageBuffer>();

	uint32 width, height;
	uint16 nChannels = 1;
	uint16 bitsPerSample = 1;
	uint16 sampleFormat = SAMPLEFORMAT_UINT;
	EqChannelFormat internalFormat = Format_Unsigned8;

	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &nChannels);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
	TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &sampleFormat);
	bool sampleFormatValid = true;
	switch(sampleFormat)
	{
		case SAMPLEFORMAT_UINT:
		{
			switch(bitsPerSample)
			{
				case 8:
					internalFormat = Format_Unsigned8;
					break;
				case 16:
					internalFormat = Format_Unsigned16;
					break;
				case 32:
					internalFormat = Format_Unsigned32;
					break;
				default:
					Aqsis::log() << Aqsis::error << "Unrecognised bit depth for unsigned int format " << bitsPerSample << std::endl;
					sampleFormatValid = false;
			}
		}
		break;

		case SAMPLEFORMAT_INT:
		{
			switch(bitsPerSample)
			{
				case 8:
					internalFormat = Format_Signed8;
					break;
				case 16:
					internalFormat = Format_Signed16;
					break;
				case 32:
					internalFormat = Format_Signed32;
					break;
				default:
					Aqsis::log() << Aqsis::error << "Unrecognised bit depth for signed int format " << bitsPerSample << std::endl;
					sampleFormatValid = false;
			}
		}
		break;

		case SAMPLEFORMAT_IEEEFP:
		{
			if(bitsPerSample != 32)
			{
				Aqsis::log() << Aqsis::error << "Unrecognised bit depth for ieeefp format " << bitsPerSample << std::endl;
				sampleFormatValid = false;
			}
			internalFormat = Format_Float32;
		}
		break;

		default:
			Aqsis::log() << Aqsis::error << "Unrecognised format " << sampleFormat << std::endl;
			sampleFormatValid = false;
	}
	if(!sampleFormatValid)
		return boost::shared_ptr<CqImageBuffer>();

	// Construct a vector of channel information for the new image buffer.
	const char* defaultChannelNames[] = { "r", "g", "b", "a" };
	CqChannelInfoList newChans;
	for(TqUint chanNum = 0; chanNum < nChannels; ++chanNum)
	{
		newChans.addChannel(SqChannelInfo(
				defaultChannelNames[Aqsis::min<TqUint>(chanNum,3)],
				internalFormat));
	}

	boost::shared_ptr<CqImageBuffer> newImBuf;

	if(!TIFFIsTiled(tif))
	{
		uint32 row;
		uint16 config;

		TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &config);
		if (config == PLANARCONFIG_CONTIG) 
		{
			newImBuf = boost::shared_ptr<CqImageBuffer>(
					new CqImageBuffer(newChans, width, height));
			TqUchar* data = newImBuf->rawData().get();
			TqUint rowLength = newImBuf->bytesPerPixel() * newImBuf->m_width;
			for (row = 0; row < height; row++)
			{
				TIFFReadScanline(tif, data, row);
				data += rowLength;
			}
		}
		else if (config == PLANARCONFIG_SEPARATE) 
		{
			Aqsis::log() << Aqsis::error << "Images with separate planar config not supported." << std::endl;
		}
	}
	else
	{
		Aqsis::log() << Aqsis::error << "Cannot currently load images in tiled format." << std::endl;
//		TIFFReadRGBAImage(tif, width, height, (uint32*)m_data, 0);
	}
	return newImBuf;
}

void CqImageBuffer::saveToTiff(TIFF* pOut) const
{
	if ( !pOut )
		return;

	TIFFSetField( pOut, TIFFTAG_PLANARCONFIG, uint16(PLANARCONFIG_CONTIG) );
	TIFFSetField( pOut, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB );
	if ( numChannels() == 4 )
	{
		short ExtraSamplesTypes[ 1 ] = {EXTRASAMPLE_ASSOCALPHA};
		TIFFSetField( pOut, TIFFTAG_EXTRASAMPLES, 1, ExtraSamplesTypes );
	}

	TIFFSetField( pOut, TIFFTAG_IMAGEWIDTH, uint32(m_width) );
	TIFFSetField( pOut, TIFFTAG_IMAGELENGTH, uint32(m_height) );
	TIFFSetField( pOut, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
	TIFFSetField( pOut, TIFFTAG_SAMPLESPERPIXEL, numChannels() );
	TIFFSetField( pOut, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize( pOut, 0 ) );

	// Work out the format of the image to write.
	EqChannelFormat widestType = Format_Unsigned8;
	for(CqChannelInfoList::const_iterator ichan = m_channelsInfo.begin();
			ichan != m_channelsInfo.end() ; ++ichan)
		widestType = Aqsis::min(widestType, ichan->type);

	// Write out an 8 bits per pixel integer image.
	if ( widestType == Format_Unsigned8 || widestType == Format_Signed8 )
	{
		TIFFSetField( pOut, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_INT );
		TIFFSetField( pOut, TIFFTAG_BITSPERSAMPLE, 8 );
	}
	else if(widestType == Format_Float32)
	{
		/* use uncompressed IEEEFP pixels */
		TIFFSetField( pOut, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP );
		TIFFSetField( pOut, TIFFTAG_BITSPERSAMPLE, 32 );
	}
	else if(widestType == Format_Signed16)
	{
		TIFFSetField( pOut, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_INT );
		TIFFSetField( pOut, TIFFTAG_BITSPERSAMPLE, 16 );
	}
	else if(widestType == Format_Unsigned16)
	{
		TIFFSetField( pOut, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT );
		TIFFSetField( pOut, TIFFTAG_BITSPERSAMPLE, 16 );
	}
	else if(widestType == Format_Signed32)
	{
		TIFFSetField( pOut, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_INT );
		TIFFSetField( pOut, TIFFTAG_BITSPERSAMPLE, 32 );
	}
	else if(widestType == Format_Unsigned32)
	{
		TIFFSetField( pOut, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT );
		TIFFSetField( pOut, TIFFTAG_BITSPERSAMPLE, 32 );
	}
	// Write out the actual pixel data.
	TqInt lineLength = m_channelsInfo.bytesPerPixel() * m_width;
	TqUchar* dataPtr = m_data.get();
	for (TqUint row = 0; row < m_height; row++ )
	{
		if ( TIFFWriteScanline( pOut, reinterpret_cast<void*>(dataPtr) , row, 0 ) < 0 )
			break;
		dataPtr += lineLength;
	}
}


boost::shared_ptr<CqImageBuffer> CqImageBuffer::quantizeForDisplay() const
{
	// Make a new image buffer with the same channel names, but with all
	// channel types set to unsigned 8 bit.
	boost::shared_ptr<CqImageBuffer> newBuf(
			new CqImageBuffer(m_channelsInfo.cloneAs8Bit(), m_width, m_height));

	// Copy across the data.
	for(TqUint chanNum = 0; chanNum < m_channelsInfo.numChannels(); ++chanNum)
		newBuf->channel(chanNum)->copyFrom(*channel(chanNum));
	return newBuf;
}

void CqImageBuffer::clearBuffer(TqFloat f)
{
	// Fill all channels with the given constant.
	CqImageChannelConstant constChan(f);
	for(TqUint chanNum = 0; chanNum < m_channelsInfo.numChannels(); ++chanNum)
		channel(chanNum)->copyFrom(constChan);
}

void CqImageBuffer::initToCheckerboard(TqUint tileSize)
{
	// Fill all channels with checker pattern
	CqImageChannelCheckered checkerChan(tileSize);
	for(TqUint chanNum = 0; chanNum < m_channelsInfo.numChannels(); ++chanNum)
		channel(chanNum)->copyFrom(checkerChan);
}

void CqImageBuffer::copyFrom(const CqImageBuffer& source, TqUint topLeftX, TqUint topLeftY)
{
	if(source.m_width + topLeftX > m_width || source.m_height + topLeftY > m_height)
		throw XqInternal("Source region too big for destination", __FILE__, __LINE__);
	if(source.m_channelsInfo.numChannels() != m_channelsInfo.numChannels())
		throw XqInternal("Number of source and destination channels do not match", __FILE__, __LINE__);

	for(TqUint i = 0; i < m_channelsInfo.numChannels(); ++i)
	{
		channel(i, topLeftX, topLeftY, source.m_width, source.m_height)
			->copyFrom(*source.channel(i));
	}
}

void CqImageBuffer::copyFrom(const CqImageBuffer& source, const TqChannelNameMap& nameMap,
		TqUint topLeftX, TqUint topLeftY)
{
	if(source.m_width + topLeftX > m_width || source.m_height + topLeftY > m_height)
		throw XqInternal("Source region too big for destination", __FILE__, __LINE__);

	for(TqChannelNameMap::const_iterator i = nameMap.begin(), e = nameMap.end();
			i != e; ++i)
	{
		channel(i->first, topLeftX, topLeftY, source.m_width, source.m_height)
			->copyFrom(*source.channel(i->second));
	}
}

void CqImageBuffer::compositeOver(const CqImageBuffer& source,
		const TqChannelNameMap& nameMap, TqUint topLeftX, TqUint topLeftY,
		const std::string alphaName)
{
	if(source.m_width + topLeftX > m_width || source.m_height + topLeftY > m_height)
		throw XqInternal("Source region too big for destination", __FILE__, __LINE__);

	for(TqChannelNameMap::const_iterator i = nameMap.begin(), e = nameMap.end();
			i != e; ++i)
	{
		channel(i->first, topLeftX, topLeftY, source.m_width, source.m_height)
			->compositeOver(*source.channel(i->second), *source.channel(alphaName));
	}
}

inline boost::shared_ptr<CqImageChannel> CqImageBuffer::channel(const std::string& name,
		TqUint topLeftX, TqUint topLeftY, TqUint width, TqUint height)
{
	return channelImpl(m_channelsInfo.findChannelIndex(name),
			topLeftX, topLeftY, width, height);
}

inline boost::shared_ptr<CqImageChannel> CqImageBuffer::channel(TqUint index, TqUint topLeftX,
		TqUint topLeftY, TqUint width, TqUint height)
{
	return channelImpl(index, topLeftX, topLeftY, width, height);
}

inline boost::shared_ptr<const CqImageChannel> CqImageBuffer::channel(const std::string& name,
		TqUint topLeftX, TqUint topLeftY, TqUint width, TqUint height) const
{
	return channelImpl(m_channelsInfo.findChannelIndex(name),
			topLeftX, topLeftY, width, height);
}

inline boost::shared_ptr<const CqImageChannel> CqImageBuffer::channel(TqUint index,
		TqUint topLeftX, TqUint topLeftY, TqUint width, TqUint height) const
{
	return channelImpl(index, topLeftX, topLeftY, width, height);
}

boost::shared_ptr<CqImageChannel> CqImageBuffer::channelImpl(TqUint index,
		TqUint topLeftX, TqUint topLeftY, TqUint width, TqUint height) const
{
	if(width == 0)
		width = m_width;
	if(height == 0)
		height = m_height;
	assert(topLeftX + width <= m_width);
	assert(topLeftY + height <= m_height);
	TqUint stride = m_channelsInfo.bytesPerPixel();
	// Start offset for the channel
	TqUchar* startPtr = m_data.get()
			+ (topLeftY*m_width + topLeftX)*stride
			+ m_channelsInfo.channelByteOffset(index);
	TqUint rowSkip = m_width - width;
	switch(m_channelsInfo[index].type)
	{
		case Format_Float32:
			return boost::shared_ptr<CqImageChannel>(
					new CqImageChannelTyped<PtDspyFloat32>(m_channelsInfo[index],
						startPtr, width, height, stride, rowSkip));
		case Format_Unsigned32:
			return boost::shared_ptr<CqImageChannel>(
					new CqImageChannelTyped<PtDspyUnsigned32>(m_channelsInfo[index],
						startPtr, width, height, stride, rowSkip));
		case Format_Signed32:
			return boost::shared_ptr<CqImageChannel>(
					new CqImageChannelTyped<PtDspySigned32>(m_channelsInfo[index],
						startPtr, width, height, stride, rowSkip));
		case Format_Unsigned16:
			return boost::shared_ptr<CqImageChannel>(
					new CqImageChannelTyped<PtDspyUnsigned16>(m_channelsInfo[index],
						startPtr, width, height, stride, rowSkip));
		case Format_Signed16:
			return boost::shared_ptr<CqImageChannel>(
					new CqImageChannelTyped<PtDspySigned16>(m_channelsInfo[index],
						startPtr, width, height, stride, rowSkip));
		case Format_Signed8:
			return boost::shared_ptr<CqImageChannel>(
					new CqImageChannelTyped<PtDspySigned8>(m_channelsInfo[index],
						startPtr, width, height, stride, rowSkip));
		case Format_Unsigned8:
		default:
			return boost::shared_ptr<CqImageChannel>(
					new CqImageChannelTyped<PtDspyUnsigned8>(m_channelsInfo[index],
						startPtr, width, height, stride, rowSkip));
	}
}

} // namespace Aqsis
