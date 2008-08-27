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
 * \brief A RIB lexer class.
 * \author Chris Foster  [chris42f (at) gmail (dot) com]
 */

#ifndef RIBLEXER_H_INCLUDED
#define RIBLEXER_H_INCLUDED

#include "aqsis.h"

#include <iosfwd>
#include <vector>
#include <map>

#include <boost/intrusive_ptr.hpp>
#include <boost/noncopyable.hpp>

#include "ribinputbuffer.h"
#include "ribtoken.h"

namespace ribparse
{

//------------------------------------------------------------------------------
/** \brief A lexical analyser for the Renderman Interface Byetstream.
 *
 * The lexer translates input characters into a sequence of tokens of type
 * CqRibToken.  The token values are computed in the lexer rather than being
 * returned as strings.
 */
class CqRibLexer : boost::noncopyable
{
	public:
		/** \brief Create new lexer using a given stream.
		 * \param inStream - stream to connect to
		 */
		CqRibLexer(std::istream& inStream);
		/** \brief Get the next token.
		 * \return The next token from the input stream.
		 */
		CqRibToken getToken();
		/** \brief Put a token back into the input stream
		 *
		 * This only works for a *single* token, since the unget buffer is of
		 * length one.
		 *
		 * \param tok - token to put back.
		 */
		void ungetToken(const CqRibToken& tok);

		/** Return the position in the input file
		 *
		 * \return The position in the input file for the previous token
		 *         obtained with getToken().  Correctly accounts for the use of
		 *         ungetToken();
		 */
		SqSourcePos pos() const;

	private:
		/// Read in a number (integer or real)
		CqRibToken readNumber();
		/** \brief Read in a string
		 * Assumes that the leading '"' has already been read.
		 */
		CqRibToken readString();
		/// Read in a RIB request
		CqRibToken readRequest();
		/// Read in and discard a comment.
		void readComment();
		/// Signal an error condition
		CqRibToken error(std::string message);

		/// Input buffer from which characters are read.
		CqRibInputBuffer m_inBuf;
		/// source position of previous token in input stream
		SqSourcePos m_currPos;
		/// source position of previous previous token in input stream
		SqSourcePos m_prevPos;
		/// next token, if already read.
		CqRibToken m_nextTok;
		/// flag indicating whether we've already got the next token.
		bool m_haveNext;

		/// Array of encoded requests for binary RIB decoding
		std::vector<std::string> m_encodedRequests;
		/// Map of encoded strings for binary RIB decoding
		typedef std::map<TqInt, std::string> TqEncodedStringMap;
		TqEncodedStringMap m_encodedStrings;

		/// Number of array elements remaining in current encoded float array.
		TqInt m_arrayElementsRemaining;
};



//==============================================================================
// Implementation details
//==============================================================================
inline SqSourcePos CqRibLexer::pos() const
{
	return m_currPos;
}

} // namespace ribparse

#endif // RIBLEXER_H_INCLUDED
