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

#ifndef RIBINPUTBUFFER_H_INCLUDED
#define RIBINPUTBUFFER_H_INCLUDED

#include <aqsis/aqsis.h>

#include <iostream>

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

#include <aqsis/ribparser.h>

namespace Aqsis
{

/// A holder for source code positions.
struct SqSourcePos
{
	TqInt line;
	TqInt col;
	SqSourcePos(TqInt line, TqInt col) : line(line), col(col) {}
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
class AQSIS_RIBPARSER_SHARE CqRibInputBuffer : boost::noncopyable
{
	public:
		/// "Character" type returned from the get() method.
		typedef std::istream::int_type TqOutputType;

		/** \brief Construct an input buffer.
		 *
		 * \param inStream - input stream from which to read.  The stream is
		 *                   allowed to be null in which case the buffer
		 *                   returns EOFs.
		 * \param streamName - name of the stream used in error messages.
		 */
		CqRibInputBuffer(std::istream& inStream,
				const std::string& streamName = "unknown");

		/// Get the next character from the input stream
		TqOutputType get();
		/// Put the last character back into the input stream
		void unget();

		/// Return the position of the previous character obtained with get()
		SqSourcePos pos() const;
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
		static const TqInt m_bufSize = 256;
		/// Internal buffer of characters.
		TqOutputType m_buffer[m_bufSize];
		/// Position of current character [ie, last char returned with get() ]
		TqInt m_bufPos;
		/// Position of last valid character in input buffer.
		TqInt m_bufEnd;

		/// Current source location
		SqSourcePos m_currPos;
		/// Previous source location
		SqSourcePos m_prevPos;
};


//==============================================================================
// Implementation details.
//==============================================================================
// CqRibInputBuffer implementation
inline CqRibInputBuffer::TqOutputType CqRibInputBuffer::get()
{
	// Get next character.
	++m_bufPos;
	if(m_bufPos >= m_bufEnd)
		bufferNextChars();
	TqOutputType c = m_buffer[m_bufPos];

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

inline void CqRibInputBuffer::unget()
{
	// Precondition: current buffer position is at least two chars into the
	// buffer so that lookback can work.
	assert(m_bufPos >= 1);
	--m_bufPos;
	m_currPos = m_prevPos;
}

inline SqSourcePos CqRibInputBuffer::pos() const
{
	return m_currPos;
}

inline const std::string& CqRibInputBuffer::streamName() const
{
	return m_streamName;
}

} // namespace Aqsis

#endif // RIBINPUTBUFFER_H_INCLUDED
