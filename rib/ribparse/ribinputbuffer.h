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

namespace ribparse
{

/** Input buffer for RIB parsing.
 *
 * The buffer supports three main actions:
 *   * get a single character
 *   * peek at the next character of input
 *   * put back the last character read (unget)
 *
 * These actions are sufficient for correctly constructing tokens from the RIB
 * stream.  The RISpec explicitly states that RIB should be thought of as a
 * stream of single-byte commands.  Because we might be reading from a pipe via
 * stdin, the "end" of the rib stream may be encountered at any time.  This
 * class therefore makes sure that any input buffering of a requested number of
 * characters is non-blocking.
 */
class CqRibInputBuffer
{
	public:
		/// "Character" type returned from the get() and peek() methods.
		typedef std::istream::int_type TqOutputType;

		/// Construct an input buffer.
		CqRibInputBuffer(std::istream& inStream);

		/// Get the next character from the input stream
		TqOutputType get();
		/// Put the last character back into the input stream
		void unget();
		/// Peek (get without removing) at the next character in the input stream
		TqOutputType peek();

		/// Return the current line number
		TqInt lineNum() const;
		/// Return the current column number
		TqInt colNum() const;

	private:
		/// Stream we are reading from.
		std::istream& m_inStream;
		/// Character which was put back into the input.
		TqOutputType m_putbackChar;
		/// Flag to indicate current character has already been read from inStream.
		bool m_havePutbackChar;
		/// Current line number.
		TqInt m_lineNum;
		/// Current column number.
		TqInt m_colNum;
};


//==============================================================================
// Implementation details.
//==============================================================================
CqRibInputBuffer::CqRibInputBuffer(std::istream& inStream)
	: m_inStream(inStream),
	m_putbackChar(0),
	m_havePutbackChar(false),
	m_lineNum(0),
	m_colNum(0)
{ }

inline CqRibInputBuffer::TqOutputType CqRibInputBuffer::get()
{
	if(m_havePutbackChar)
	{
		m_havePutbackChar = false;
		return m_putbackChar;
	}
	if(m_putbackChar == '\n')
	{
		++m_lineNum;
		m_colNum = 0;
	}
	TqOutputType c = m_inStream.get();
	++m_colNum;
	if(c == '\r')
	{
		// translate all '\r' and '\r\n' characters to a single '\n'
		if(m_inStream.peek() == '\n')
			m_inStream.get();
		c = '\n';
	}
	m_putbackChar = c;
	return c;
}

inline CqRibInputBuffer::TqOutputType CqRibInputBuffer::peek()
{
	TqOutputType c = get();
	m_havePutbackChar = true;
	return c;
}

inline void CqRibInputBuffer::unget()
{
	m_havePutbackChar = true;
}

inline TqInt CqRibInputBuffer::lineNum() const
{
	return m_lineNum;
}

inline TqInt CqRibInputBuffer::colNum() const
{
	return m_colNum;
}

} // namespace ribparse

#endif // RIBINPUTBUFFER_H_INCLUDED
