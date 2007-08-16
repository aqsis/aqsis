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
 * \brief Implementations of classes for manipulating image channels.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 *
 */

#include "imagechannel.h"

#include <boost/format.hpp>

#include "exception.h"
#include "ndspy.h"

namespace Aqsis {
//------------------------------------------------------------------------------

EqChannelFormat chanFormatFromPkDspy(TqInt dspyFormat)
{
	switch(dspyFormat)
	{
		case PkDspyFloat32:
			return Format_Float32;
		case PkDspyUnsigned32:
			return Format_Unsigned32;
		case PkDspySigned32:
			return Format_Signed32;
		case PkDspyUnsigned16:
			return Format_Unsigned16;
		case PkDspySigned16:
			return Format_Signed16;
		case PkDspyUnsigned8:
			return Format_Unsigned8;
		case PkDspySigned8:
			return Format_Signed8;
		default:
			throw XqInternal("Unknown PkDspy data format", __FILE__, __LINE__);
	}
}

TqInt pkDspyFromChanFormat(EqChannelFormat format)
{
	switch(format)
	{
		case Format_Float32:
			return PkDspyFloat32;
		case Format_Unsigned32:
			return PkDspyUnsigned32;
		case Format_Signed32:
			return PkDspySigned32;
		case Format_Unsigned16:
			return PkDspyUnsigned16;
		case Format_Signed16:
			return PkDspySigned16;
		case Format_Unsigned8:
			return PkDspyUnsigned8;
		default:
		case Format_Signed8:
			return PkDspySigned8;
	}
}

//------------------------------------------------------------------------------
// CqImageChannelConstant implementation
CqImageChannelConstant::CqImageChannelConstant(TqFloat value)
	: m_value(value),
	m_rowBuf()
{ }

void CqImageChannelConstant::requireSize(TqUint width, TqUint height) const
{
	if(m_rowBuf.size() != width)
		m_rowBuf.assign(width, m_value);
}

const TqFloatConv* CqImageChannelConstant::getRow(TqUint row) const
{
	return &m_rowBuf[0];
}

//------------------------------------------------------------------------------
// CqImageChannelCheckered implementation

CqImageChannelCheckered::CqImageChannelCheckered(TqUint tileSize)
	: m_tileSize(tileSize),
	m_checkerRow0(),
	m_checkerRow1()
{ }

void CqImageChannelCheckered::requireSize(TqUint width, TqUint height) const
{
	if(m_checkerRow0.size() != width)
	{
		m_checkerRow0.resize(width);
		m_checkerRow1.resize(width);
		// Make two buffers, one holding the first row of the checker pattern
		// and one holding the second.
		for(TqUint col = 0; col < width; ++col)
		{
			TqInt whichTile = (col % (m_tileSize*2)) / m_tileSize;
			m_checkerRow0[col] = (whichTile+1)*0.5f;
			m_checkerRow1[col] = (2-whichTile)*0.5f;
		}
	}
}

const TqFloatConv* CqImageChannelCheckered::getRow(TqUint row) const
{
	if( ((row % (m_tileSize*2)) / m_tileSize) == 0 )
		return &m_checkerRow0[0];
	else
		return &m_checkerRow1[0];
}


//------------------------------------------------------------------------------
// SqChannelInfo implementation
TqUint SqChannelInfo::bytesPerPixel() const
{
	switch(type)
	{
		case Format_Unsigned32:
		case Format_Signed32:
		case Format_Float32:
			return 4;
			break;
		case Format_Unsigned16:
		case Format_Signed16:
			return 2;
		case Format_Signed8:
		case Format_Unsigned8:
		default:
			return 1;
	}
}


//------------------------------------------------------------------------------
// CqImageChannel implementation
CqImageChannel::CqImageChannel(const SqChannelInfo& chanInfo, TqUchar* data,
		TqUint width, TqUint height, TqUint stride, TqUint rowSkip)
	: m_chanInfo(chanInfo),
	m_data(data),
	m_width(width),
	m_height(height),
	m_stride(stride),
	m_rowSkip(rowSkip),
	m_copyBuf(width)
{ }

void CqImageChannel::requireSize(TqUint width, TqUint height) const
{
	// Normal image channels cannot change size; just check that the sizes match.
	if(m_width != width || m_height != height)
	{
		throw XqInternal("Image channel cannot produce required size", 
				(boost::format("required size = %dx%d;  actual size = %dx%d")
				% width % height % m_width % m_height).str(),
				__FILE__, __LINE__);
	}
}

void CqImageChannel::copyFrom(const IqImageChannelSource& source)
{
	source.requireSize(m_width, m_height);

	/** \todo make copying from the same type efficient...  Need also to take
	 * into account the min and max quantization parameters when they are
	 * implemented.
	 */

	for(TqUint row = 0; row < m_height; ++row)
		replaceRow(row, source.getRow(row));
}

void CqImageChannel::compositeOver(const IqImageChannelSource& source,
		const IqImageChannelSource& sourceAlpha)
{
	source.requireSize(m_width, m_height);
	sourceAlpha.requireSize(m_width, m_height);

	for(TqUint row = 0; row < m_height; ++row)
		compositeRow(row, source.getRow(row), sourceAlpha.getRow(row));
}

//------------------------------------------------------------------------------
} // namespace Aqsis
