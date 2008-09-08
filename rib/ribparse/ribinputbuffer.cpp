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
 * \brief A buffer class for RIB input streams.
 * \author Chris Foster  [chris42f (at) gmail (dot) com]
 */

#include "ribinputbuffer.h"

#ifdef USE_GZIPPED_RIB
#	include <boost/iostreams/filtering_stream.hpp>
#	include <boost/iostreams/filter/gzip.hpp>
#endif

#include "exception.h"

namespace Aqsis {

CqRibInputBuffer::CqRibInputBuffer(std::istream& inStream)
	: m_inStream(&inStream),
	m_gzipStream(),
	m_putbackChar(0),
	m_havePutbackChar(false),
	m_currPos(1,0),
	m_prevPos(-1,-1)
{
	if(isGzippedStream(inStream))
	{
#ifdef USE_GZIPPED_RIB
		namespace io = boost::iostreams;
		io::filtering_stream<io::input>* zipStream = 0;
		m_gzipStream.reset(zipStream = new io::filtering_stream<io::input>());
		zipStream->push(io::gzip_decompressor());
		zipStream->push(inStream);
		m_inStream = m_gzipStream.get();
#else
		AQSIS_THROW(XqParseError, "gzipped RIB detected, but aqsis compiled without gzip support.");
#endif // USE_GZIPPED_RIB
	}
}

bool CqRibInputBuffer::isGzippedStream(std::istream& in)
{
	bool isZipped = false;
	TqOutputType c = in.get();
	// Check whether the magic number matches that for a gzip stream
	const TqOutputType gzipMagic[] = {0x1f, 0x8b};
	if(c == gzipMagic[0])
	{
		if(in.peek() == gzipMagic[1])
			isZipped = true;
	}
	in.unget();
	return isZipped;
}

} // namespace Aqsis
