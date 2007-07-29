// Aqsis
// Copyright (C) 1997 - 2007, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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
 *
 */

#include "imagebuffer.h"

#include "ndspy.h"

namespace Aqsis {

TqUint CqImageChannel::channelSize(TqInt type)
{
	switch(type)
	{
		case PkDspyUnsigned32:
		case PkDspySigned32:
		case PkDspyFloat32:
			return 4;
			break;
		case PkDspyUnsigned16:
		case PkDspySigned16:
			return 2;
		case PkDspySigned8:
		case PkDspyUnsigned8:
		default:
			return 1;
	}
}

TqUint CqImageBuffer::bytesPerPixel(const TqChannelList& channels)
{
	TqUint size = 0;
	for(TqChannelList::const_iterator channel = channels.begin();
			channel != channels.end(); ++channel)
		size += channel->numBytes();
	return size;
}

void CqImageBuffer::quantizeForDisplay(const TqUchar* src, TqUchar* dest,
		const TqChannelList& srcChannels, TqUint width, TqUint height)
{
	const TqUint destStride = srcChannels.size();
	const TqUint numPixels = width*height;

	TqUint srcStride = bytesPerPixel(srcChannels);
	TqUint srcOffset = 0;

	TqUint channelNum = 0;
	for(TqChannelList::const_iterator channel = srcChannels.begin();
			channel != srcChannels.end(); ++channel)
	{
		switch(channel->type())
		{
			case PkDspyUnsigned16:
				quantize8bitChannelStrided<TqUshort>(src + srcOffset, 
						dest + channelNum, srcStride, destStride, numPixels);
				break;
			case PkDspySigned16:
				quantize8bitChannelStrided<TqShort>(src + srcOffset, 
						dest + channelNum, srcStride, destStride, numPixels);
				break;
			case PkDspyUnsigned32:
				quantize8bitChannelStrided<TqUint>(src + srcOffset, 
						dest + channelNum, srcStride, destStride, numPixels);
				break;
			case PkDspySigned32:
				quantize8bitChannelStrided<TqInt>(src + srcOffset, 
						dest + channelNum, srcStride, destStride, numPixels);
				break;
			case PkDspyFloat32:
				quantize8bitChannelStrided<TqFloat>(src + srcOffset, 
						dest + channelNum, srcStride, destStride, numPixels);
				break;
			case PkDspySigned8:
				quantize8bitChannelStrided<TqChar>(src + srcOffset, 
						dest + channelNum, srcStride, destStride, numPixels);
				break;
			case PkDspyUnsigned8:
			default:
				quantize8bitChannelStrided<TqUchar>(src + srcOffset, 
						dest + channelNum, srcStride, destStride, numPixels);
				break;
		}
		srcOffset += channel->numBytes();
		++channelNum;
	}
}

} // namespace Aqsis
