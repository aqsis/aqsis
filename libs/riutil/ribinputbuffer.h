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

#ifndef RIBINPUTBUFFER_H_INCLUDED
#define RIBINPUTBUFFER_H_INCLUDED

#include <aqsis/aqsis.h>

#include <iostream>

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

namespace Aqsis
{

/// A holder for source code positions.
struct SourcePos
{
	int line;
	int col;
	SourcePos(int line, int col) : line(line), col(col) {}
};


//------------------------------------------------------------------------------
/** Input buffer for RIB parsing.
 *
 * The buffer supports three main actions:
 *   * get a single character
 *   * put back the last character read (unget)
 *
 * These actions are sufficient for correctly constructing tokens from the RIB
 * stream.  The RISpec explicitly states that RIB should be thought of as a
 * stream of single-byte commands.  Because we might be reading from a pipe via
 * stdin, the "end" of the rib stream may be encountered at any time.  This
 * class therefore makes sure that any input buffering of a requested number of
 * characters is non-blocking.
 */
class RibInputBuffer : boost::noncopyable
{
	public:
		/// Character type returned from the get() method.
		typedef unsigned char CharType;
		/// 255 is our helpful uchar-sized standin for EOF.  It is used as a
		/// de-facto EOF in RIB streams, so might as well make the most of it.
		static const CharType eof = 255;

		/** \brief Construct an input buffer.
		 *
		 * \param inStream - input stream from which to read.
		 * \param streamName - name of the stream used in error messages.
		 */
		RibInputBuffer(std::istream& inStream,
				const std::string& streamName = "unknown");

		/// Get the next character from the input stream
		CharType get();
		/// Put the last character back into the input stream
		void unget();

		/// Return the position of the previous character obtained with get()
		SourcePos pos() const;
		/// Return the name of the input stream
		const std::string& streamName() const;

	private:
		static bool isGzippedStream(std::istream& in);
		void bufferNextChars();

		/// Stream we are reading from.
		std::istream* m_inStream;
		/// Stream name
		const std::string m_streamName;
		/// gzip decompressor for compressed input
		boost::scoped_ptr<std::istream> m_gzipStream;

		/// Internal buffer size.
		static const int m_bufSize = 256;
		/// Internal buffer of characters.
		CharType m_buffer[m_bufSize];
		/// Position of current character [ie, last char returned with get() ]
		int m_bufPos;
		/// Position of last valid character in input buffer.
		int m_bufEnd;

		/// Current source location
		SourcePos m_currPos;
		/// Previous source location
		SourcePos m_prevPos;
};


//==============================================================================
// Implementation details.
//==============================================================================
// RibInputBuffer implementation
inline RibInputBuffer::CharType RibInputBuffer::get()
{
	// Get next character.
	++m_bufPos;
	if(m_bufPos >= m_bufEnd)
		bufferNextChars();
	CharType c = m_buffer[m_bufPos];

	// Keep line and column numbers up to date.
	m_prevPos = m_currPos;
	++m_currPos.col;
	if(c == '\r' || (c == '\n' && m_buffer[m_bufPos-1] != '\r'))
	{
		++m_currPos.line;
		m_currPos.col = 0;
	}
	else if(c == '\n')
		m_currPos.col = 0;
	return c;
}

inline void RibInputBuffer::unget()
{
	// Precondition: current buffer position is at least two chars into the
	// buffer so that lookback can work.
	assert(m_bufPos >= 1);
	--m_bufPos;
	m_currPos = m_prevPos;
}

inline SourcePos RibInputBuffer::pos() const
{
	return m_currPos;
}

inline const std::string& RibInputBuffer::streamName() const
{
	return m_streamName;
}

} // namespace Aqsis

#endif // RIBINPUTBUFFER_H_INCLUDED
