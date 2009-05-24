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

#include <aqsis/util/exception.h>

namespace Aqsis {

CqRibInputBuffer::CqRibInputBuffer(std::istream& inStream, const std::string& streamName)
	: m_inStream(&inStream),
	m_streamName(streamName),
	m_gzipStream(),
	m_bufPos(1),
	m_bufEnd(2),
	m_currPos(1,0),
	m_prevPos(-1,-1)
{
	// Zero the putback chars
	m_buffer[0] = 0;
	m_buffer[1] = 0;
	if(isGzippedStream(inStream))
	{
#		ifdef USE_GZIPPED_RIB
		// Initialise gzip decompressor
		namespace io = boost::iostreams;
		io::filtering_stream<io::input>* zipStream = 0;
		m_gzipStream.reset(zipStream = new io::filtering_stream<io::input>());
		zipStream->push(io::gzip_decompressor());
		zipStream->push(inStream);
		m_inStream = m_gzipStream.get();
#		else
		AQSIS_THROW_XQERROR(XqParseError, EqE_Unimplement,
			"gzipped RIB detected, but aqsis compiled without gzip support.");
#		endif // USE_GZIPPED_RIB
	}
}

/** \brief Fill the internal buffer with as many characters as possible
 * (guarenteed >= 1)
 *
 * This function reads in as many characters as possible using a non-blocking
 * read on the istream.  If no characters are returned from the non-blocking
 * read, a single character is read using the blocking std::istream::get()
 * function.
 *
 * Postconditions: The m_bufPos index is one before the next character in the
 * input stream.  The m_bufEnd index points to one after the last valid
 * character.  m_bufPos < m_bufEnd
 */
void CqRibInputBuffer::bufferNextChars()
{
	// Precondition: m_bufPos is pointing to a one off the end of the valid
	// characters in the buffer.
	assert(m_bufPos == m_bufEnd);
	// first make sure that we're not at the maximum extent of the buffer; if
	// so we need to wrap around to the beginning.
	if(m_bufEnd == m_bufSize)
	{
		// Copy over some chars so that we can always unget() at least one and
		// still look back into the buffer an additional char for line ending
		// detection.
		m_buffer[0] = m_buffer[m_bufSize-2];
		m_buffer[1] = m_buffer[m_bufSize-1];
		// Reset buffer position
		m_bufPos = 1;
	}
	// Now fill the buffer with as many characters as possible using a
	// non-blocking read with readsome().
	char auxBuf[m_bufSize];
	int numRead = m_inStream->readsome(auxBuf, m_bufSize - m_bufPos);
	if(numRead > 0)
	{
		// copy the chars over as unsigned.  Ugly, but necessary if our
		// buffer is going to accomodate holding EOFs.
		for(int i = 0; i < numRead; ++i)
			m_buffer[m_bufPos+i] = static_cast<unsigned char>(auxBuf[i]);
		m_bufEnd = m_bufPos + numRead;
	}
	else
	{
		// Else ugh: We failed to read a group of characters with the
		// non-blocking read so we have to make an inefficient read of a single
		// charater.  (Reading a single char may block, but that's acceptable.)
		m_buffer[m_bufPos] = m_inStream->get();
		m_bufEnd = m_bufPos + 1;
	}
}

/// Determine whether the given stream is gzipped.
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
