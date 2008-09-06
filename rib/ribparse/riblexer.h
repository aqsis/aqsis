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

namespace Aqsis
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
		const CqRibToken& get();
		/** \brief Look at but don't remove the next token from the input sequence
		 * \return the next token from the input stream
		 */
		const CqRibToken& peek();

		/** Return the position in the input file
		 *
		 * \return The position in the input file for the previous token
		 *         obtained with get().
		 */
		SqSourcePos pos() const;

	private:
		//--------------------------------------------------
		/// \name ASCII RIB decoding functions
		//@{
		static CqRibToken readNumber(CqRibInputBuffer& inBuf);
		static CqRibToken readString(CqRibInputBuffer& inBuf);
		static CqRibToken readRequest(CqRibInputBuffer& inBuf);
		static void readComment(CqRibInputBuffer& inBuf);
		//@}

		//--------------------------------------------------
		/// \name Binary RIB decoding functions
		//@{
		static TqUint32 decodeInt(CqRibInputBuffer& inBuf, TqInt numBytes);
		static TqFloat decodeFixedPoint(CqRibInputBuffer& inBuf,
				TqInt numBytes, TqInt radixPos);
		static TqFloat decodeFloat32(CqRibInputBuffer& inBuf);
		static TqDouble decodeFloat64(CqRibInputBuffer& inBuf);
		static std::string decodeString(CqRibInputBuffer& inBuf, TqInt numBytes);
		void defineEncodedRequest(TqUint8 code, const CqRibToken& requestNameTok);
		CqRibToken lookupEncodedRequest(TqUint8 code) const;
		void defineEncodedString(TqInt code, const CqRibToken& stringNameTok);
		CqRibToken lookupEncodedString(TqInt code) const;
		//@}

		// scan next token from underlying input stream.
		CqRibToken scanNext();

		//--------------------------------------------------
		// Member data
		/// Input buffer from which characters are read.
		CqRibInputBuffer m_inBuf;
		/// source position of previous token in input stream
		SqSourcePos m_currPos;
		/// source position of latest token read from the input stream
		SqSourcePos m_nextPos;
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

inline const CqRibToken& CqRibLexer::get()
{
	if(!m_haveNext)
		m_nextTok = scanNext();
	m_haveNext = false;
	m_currPos = m_nextPos;
	return m_nextTok;
}

inline const CqRibToken& CqRibLexer::peek()
{
	if(!m_haveNext)
	{
		m_nextTok = scanNext();
		m_haveNext = true;
	}
	return m_nextTok;
}

} // namespace Aqsis

#endif // RIBLEXER_H_INCLUDED
