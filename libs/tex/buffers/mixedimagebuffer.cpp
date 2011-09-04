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
 * \brief Implementation of CqMixedImageBuffer and related stuff.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 * \author Paul C. Gregory (pgregory@aqsis.org)
 *
 */

#include <aqsis/tex/buffers/mixedimagebuffer.h>

#ifdef USE_OPENEXR
#include <OpenEXR/half.h>
#endif

#include <aqsis/util/exception.h>
#include <aqsis/util/logging.h>
#include <aqsis/ri/ndspy.h>

namespace Aqsis {

//------------------------------------------------------------------------------
// CqMixedImageBuffer implementation
CqMixedImageBuffer::CqMixedImageBuffer()
	: m_channelList(),
	m_width(0),
	m_height(0),
	m_data()
{ }

CqMixedImageBuffer::CqMixedImageBuffer(const CqChannelList& channelList, TqInt width, TqInt height)
	: m_channelList(channelList),
	m_width(width),
	m_height(height),
	m_data(new TqUint8[width*height*channelList.bytesPerPixel()])
{ }

CqMixedImageBuffer::CqMixedImageBuffer(const CqChannelList& channelList,
		boost::shared_array<TqUint8> data, TqInt width, TqInt height)
	: m_channelList(channelList),
	m_width(width),
	m_height(height),
	m_data(data)
{ }

void CqMixedImageBuffer::clearBuffer(TqFloat f)
{
	// Fill all channels with the given constant.
	CqImageChannelConstant constChan(f);
	for(TqInt chanNum = 0; chanNum < m_channelList.numChannels(); ++chanNum)
		channel(chanNum)->copyFrom(constChan);
}

void CqMixedImageBuffer::initToCheckerboard(TqInt tileSize)
{
	// Fill all channels with checker pattern
	CqImageChannelCheckered checkerChan(tileSize);
	for(TqInt chanNum = 0; chanNum < m_channelList.numChannels(); ++chanNum)
		channel(chanNum)->copyFrom(checkerChan);
}

void CqMixedImageBuffer::resize(TqInt width, TqInt height,
		const CqChannelList& channelList)
{
	if(width*height*channelList.bytesPerPixel()
			!= m_width*m_height*m_channelList.bytesPerPixel())
	{
		// resize the buffer if the new buffer 
		m_data.reset(new TqUint8[width*height*channelList.bytesPerPixel()]);
	}
	m_channelList = channelList;
	m_width = width;
	m_height = height;
}

void CqMixedImageBuffer::getCopyRegionSize(TqInt offset, TqInt srcWidth, TqInt destWidth,
		TqInt& srcOffset, TqInt& destOffset, TqInt& copyWidth)
{
	destOffset = max(offset, 0);
	srcOffset = max(-offset, 0);
	copyWidth = min(destWidth, offset+srcWidth) - destOffset;
}

void CqMixedImageBuffer::copyFrom(const CqMixedImageBuffer& source,
		TqInt topLeftX, TqInt topLeftY)
{
	if(source.m_channelList.numChannels() != m_channelList.numChannels())
		AQSIS_THROW_XQERROR(XqInternal, EqE_Limit,
			"Number of source and destination channels do not match");

	// compute size and top left coords of region to copy.
	TqInt copyWidth = 0;
	TqInt destTopLeftX = 0;
	TqInt srcTopLeftX = 0;
	getCopyRegionSize(topLeftX, source.m_width, m_width,
			srcTopLeftX, destTopLeftX, copyWidth);
	TqInt copyHeight = 0;
	TqInt destTopLeftY = 0;
	TqInt srcTopLeftY = 0;
	getCopyRegionSize(topLeftY, source.m_height, m_height,
			srcTopLeftY, destTopLeftY, copyHeight);
	// return if no overlap
	if(copyWidth <= 0 || copyHeight <= 0)
		return;

	for(TqInt i = 0; i < m_channelList.numChannels(); ++i)
	{
		channel(i, destTopLeftX, destTopLeftY, copyWidth, copyHeight)
			->copyFrom(*source.channel(i, srcTopLeftX, srcTopLeftY,
						copyWidth, copyHeight));
	}
}

void CqMixedImageBuffer::copyFrom(const CqMixedImageBuffer& source,
		const TqChannelNameMap& nameMap,
		TqInt topLeftX, TqInt topLeftY)
{
	// compute size and top left coords of region to copy.
	TqInt copyWidth = 0;
	TqInt destTopLeftX = 0;
	TqInt srcTopLeftX = 0;
	getCopyRegionSize(topLeftX, source.m_width, m_width,
			srcTopLeftX, destTopLeftX, copyWidth);
	TqInt copyHeight = 0;
	TqInt destTopLeftY = 0;
	TqInt srcTopLeftY = 0;
	getCopyRegionSize(topLeftY, source.m_height, m_height,
			srcTopLeftY, destTopLeftY, copyHeight);
	// return if no overlap
	if(copyWidth <= 0 || copyHeight <= 0)
		return;

	for(TqChannelNameMap::const_iterator i = nameMap.begin(), e = nameMap.end();
			i != e; ++i)
	{
		channel(i->first, destTopLeftX, destTopLeftY, copyWidth, copyHeight)
			->copyFrom(*source.channel(i->second, srcTopLeftX, srcTopLeftY,
						copyWidth, copyHeight));
	}
}

void CqMixedImageBuffer::compositeOver(const CqMixedImageBuffer& source,
		const TqChannelNameMap& nameMap, TqInt topLeftX, TqInt topLeftY,
		const std::string alphaName)
{
	if(!source.channelList().hasChannel(alphaName))
	{
		copyFrom(source, nameMap, topLeftX, topLeftY);
	}
	else
	{
		// compute size and top left coords of region to copy.
		TqInt copyWidth = 0;
		TqInt destTopLeftX = 0;
		TqInt srcTopLeftX = 0;
		getCopyRegionSize(topLeftX, source.m_width, m_width,
				srcTopLeftX, destTopLeftX, copyWidth);
		TqInt copyHeight = 0;
		TqInt destTopLeftY = 0;
		TqInt srcTopLeftY = 0;
		getCopyRegionSize(topLeftY, source.m_height, m_height,
				srcTopLeftY, destTopLeftY, copyHeight);
		// return if no overlap
		if(copyWidth <= 0 || copyHeight <= 0)
			return;

		for(TqChannelNameMap::const_iterator i = nameMap.begin(), e = nameMap.end();
				i != e; ++i)
		{
			channel(i->first, destTopLeftX, destTopLeftY, copyWidth, copyHeight)
				->compositeOver(*source.channel(i->second, srcTopLeftX, srcTopLeftY, copyWidth, copyHeight),
						*source.channel(alphaName, srcTopLeftX, srcTopLeftY, copyWidth, copyHeight));
		}
	}
}

boost::shared_ptr<CqImageChannel> CqMixedImageBuffer::channel(const std::string& name,
		TqInt topLeftX, TqInt topLeftY, TqInt width, TqInt height)
{
	return channelImpl(m_channelList.findChannelIndex(name),
			topLeftX, topLeftY, width, height);
}

boost::shared_ptr<CqImageChannel> CqMixedImageBuffer::channel(TqInt index, TqInt topLeftX,
		TqInt topLeftY, TqInt width, TqInt height)
{
	return channelImpl(index, topLeftX, topLeftY, width, height);
}

boost::shared_ptr<const CqImageChannel> CqMixedImageBuffer::channel(const std::string& name,
		TqInt topLeftX, TqInt topLeftY, TqInt width, TqInt height) const
{
	return channelImpl(m_channelList.findChannelIndex(name),
			topLeftX, topLeftY, width, height);
}

boost::shared_ptr<const CqImageChannel> CqMixedImageBuffer::channel(TqInt index,
		TqInt topLeftX, TqInt topLeftY, TqInt width, TqInt height) const
{
	return channelImpl(index, topLeftX, topLeftY, width, height);
}

boost::shared_ptr<CqImageChannel> CqMixedImageBuffer::channelImpl(TqInt index,
		TqInt topLeftX, TqInt topLeftY, TqInt width, TqInt height) const
{
	if(width == 0)
		width = m_width;
	if(height == 0)
		height = m_height;
	assert(topLeftX + width <= m_width);
	assert(topLeftY + height <= m_height);
	TqInt stride = m_channelList.bytesPerPixel();
	// Start offset for the channel
	TqUint8* startPtr = m_data.get()
			+ (topLeftY*m_width + topLeftX)*stride
			+ m_channelList.channelByteOffset(index);
	TqInt rowSkip = m_width - width;
	switch(m_channelList[index].type)
	{
		case Channel_Float32:
			return boost::shared_ptr<CqImageChannel>(
					new CqImageChannelTyped<TqFloat>(m_channelList[index],
						startPtr, width, height, stride, rowSkip));
		case Channel_Unsigned32:
			return boost::shared_ptr<CqImageChannel>(
					new CqImageChannelTyped<TqUint32>(m_channelList[index],
						startPtr, width, height, stride, rowSkip));
		case Channel_Signed32:
			return boost::shared_ptr<CqImageChannel>(
					new CqImageChannelTyped<TqInt32>(m_channelList[index],
						startPtr, width, height, stride, rowSkip));
#		ifdef USE_OPENEXR
		case Channel_Float16:
			return boost::shared_ptr<CqImageChannel>(
					new CqImageChannelTyped<half>(m_channelList[index],
						startPtr, width, height, stride, rowSkip));
#		endif
		case Channel_Unsigned16:
			return boost::shared_ptr<CqImageChannel>(
					new CqImageChannelTyped<TqUint16>(m_channelList[index],
						startPtr, width, height, stride, rowSkip));
		case Channel_Signed16:
			return boost::shared_ptr<CqImageChannel>(
					new CqImageChannelTyped<TqInt16>(m_channelList[index],
						startPtr, width, height, stride, rowSkip));
		case Channel_Signed8:
			return boost::shared_ptr<CqImageChannel>(
					new CqImageChannelTyped<TqInt8>(m_channelList[index],
						startPtr, width, height, stride, rowSkip));
		case Channel_Unsigned8:
			return boost::shared_ptr<CqImageChannel>(
					new CqImageChannelTyped<TqUint8>(m_channelList[index],
						startPtr, width, height, stride, rowSkip));
		default:
			AQSIS_THROW_XQERROR(XqInternal, EqE_Bug, "Unknown channel type");
	}
}

} // namespace Aqsis
