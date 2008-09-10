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

#include "aqsis.h"

#include <iostream>

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

#include "ribparse_share.h"

namespace Aqsis
{

/// A holder for source code positions.
struct SqSourcePos
{
	TqInt line;
	TqInt col;
	SqSourcePos(TqInt line, TqInt col) : line(line), col(col) {}
};

/// pretty print SqSourcePos
std::ostream& operator<<(std::ostream& out, const SqSourcePos& pos);


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
class RIBPARSE_SHARE CqRibInputBuffer : boost::noncopyable
{
	public:
		/// "Character" type returned from the get() method.
		typedef std::istream::int_type TqOutputType;

		/// Construct an input buffer.
		CqRibInputBuffer(std::istream& inStream);

		/// Get the next character from the input stream
		TqOutputType get();
		/// Put the last character back into the input stream
		void unget();

		/// Return the position of the previous character obtained with get()
		SqSourcePos pos() const;

	private:
		/// Determine whether the given stream is gzipped.
		static bool isGzippedStream(std::istream& in);

		/// Stream we are reading from.
		std::istream* m_inStream;
		/// gzip decompressor for compressed input
		boost::scoped_ptr<std::istream> m_gzipStream;
		/// Character which was put back into the input.
		TqOutputType m_putbackChar;
		/// Flag to indicate current character has already been read from inStream.
		bool m_havePutbackChar;
		/// Current source location
		SqSourcePos m_currPos;
		/// Previous source location
		SqSourcePos m_prevPos;
};


//==============================================================================
// Implementation details.
//==============================================================================

// SqSourcePos functions
inline std::ostream& operator<<(std::ostream& out, const SqSourcePos& pos)
{
	out << "line " << pos.line << ", col " << pos.col;
	return out;
}

//------------------------------------------------------------------------------
// CqRibInputBuffer implementation
inline CqRibInputBuffer::TqOutputType CqRibInputBuffer::get()
{
	TqOutputType c = 0;
	if(m_havePutbackChar)
	{
		m_havePutbackChar = false;
		c = m_putbackChar;
	}
	else
	{
		c = m_inStream->get();
		// TODO: Make sure this is actually necessary and that istream::get()
		// doesn't do any undesirable translation.
		if(c == '\r')
		{
			// translate all '\r' and '\r\n' characters to a single '\n'
			if(m_inStream->peek() == '\n')
				m_inStream->get();
			c = '\n';
		}
	}
	// Save character in case of unget()
	m_putbackChar = c;
	// Update current line and column position
	m_prevPos = m_currPos;
	++m_currPos.col;
	if(c == '\n')
	{
		++m_currPos.line;
		m_currPos.col = 0;
	}
	return c;
}

inline void CqRibInputBuffer::unget()
{
	m_currPos = m_prevPos;
	m_havePutbackChar = true;
}

inline SqSourcePos CqRibInputBuffer::pos() const
{
	return m_currPos;
}

} // namespace Aqsis

#endif // RIBINPUTBUFFER_H_INCLUDED
