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

RibInputBuffer::RibInputBuffer(std::istream& inStream, const std::string& streamName)
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
void RibInputBuffer::bufferNextChars()
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
	int numRead = m_inStream->readsome((char*)m_buffer + m_bufPos,
									   m_bufSize - m_bufPos);
	if(numRead > 0)
	{
		m_bufEnd = m_bufPos + numRead;
	}
	else
	{
		// Else ugh: We failed to read a group of characters with the
		// non-blocking read so we have to make an inefficient read of a single
		// charater.  (Reading a single char may block, but that's acceptable.)
		std::istream::int_type c = m_inStream->get();
		// translate EOFs
		m_buffer[m_bufPos] = (c == EOF) ? eof : c;
		m_bufEnd = m_bufPos + 1;
	}
}

/// Determine whether the given stream is gzipped.
bool RibInputBuffer::isGzippedStream(std::istream& in)
{
	bool isZipped = false;
	std::istream::int_type c = in.get();
	// Check whether the magic number matches that for a gzip stream
	const std::istream::int_type gzipMagic[] = {0x1f, 0x8b};
	if(c == gzipMagic[0])
	{
		if(in.peek() == gzipMagic[1])
			isZipped = true;
	}
	in.unget();
	return isZipped;
}

} // namespace Aqsis
