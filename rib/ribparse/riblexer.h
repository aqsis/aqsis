// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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
 * \brief A RIB lexer class.
 * \author Chris Foster  [chris42f (at) gmail (dot) com]
 */

/* TODO: error handling,
 *   aqsis types,
 *   non-blocking stream buffers,
 *   proper use of doxygen markup,
 *   integrate binary rib handling
 */

#ifndef RIBLEXER_H_INCLUDED
#define RIBLEXER_H_INCLUDED

#include "aqsis.h"

#include <istream>
#include <vector>

#include <boost/intrusive_ptr.hpp>

#include "ribinputbuffer.h"
#include "ribtoken.h"

namespace ribparse
{

//------------------------------------------------------------------------------
/** \brief A lexical analyser for the Renderman Interface Byetstream.
 */
class CqRibLexer
{
	public:
		/** \brief Create new lexer using a given stream.
		 * \param inStream - stream to connect to
		 */
		CqRibLexer(std::istream& inStream);
		/** \brief Get the next token.
		 * \returns The next token from the input stream.
		 */
		CqRibToken getToken();

		/// Return the current line number
		TqInt lineNum() const;
		/// Return the current column number
		TqInt colNum() const;

	private:
		/// Read in a number (integer or real)
		CqRibToken readNumber();
		/// Read in a string
		CqRibToken readString();
		/// Read in a RIB request
		CqRibToken readRequest();
		/// Read in a comment.
		CqRibToken readComment();
		/// Signal an error condition
		CqRibToken error(std::string message);

		CqRibInputBuffer m_inBuf;
};



//==============================================================================
// Implementation details
//==============================================================================
inline TqInt CqRibLexer::lineNum() const
{
	return m_inBuf.lineNum();
}

inline TqInt CqRibLexer::colNum() const
{
	return m_inBuf.colNum();
}

} // namespace ribparse

#endif // RIBLEXER_H_INCLUDED
