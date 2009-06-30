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
 *
 * \brief RIB parser interface
 * \author Chris Foster [chris42f (at) g mail (d0t) com]
 */

#ifndef RIBPARSER_H_INCLUDED
#define RIBPARSER_H_INCLUDED

#include <aqsis/aqsis.h>

#include <map>
#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <aqsis/util/exception.h>
#include <aqsis/ribparser.h>
#include "riblexer.h"

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief An implementation of the IqRibParser interface.
 */
#ifdef AQSIS_SYSTEM_WIN32
class AQSIS_RIBPARSER_SHARE boost::noncopyable_::noncopyable;
#endif
class AQSIS_RIBPARSER_SHARE CqRibParser : public IqRibParser, private boost::noncopyable
{
	public:
		/** \brief Construct a RIB parser
		 *
		 * \param requestHandler - Handler for requests which will be
		 *                         recognized by the parser.
		 */
		CqRibParser(const boost::shared_ptr<IqRibRequestHandler>& requestHandler);

		// The following are inherited; see docs in IqRibParser.

		// Parser driver method
		virtual bool parseNextRequest();

		// stream stack management
		virtual void pushInput(std::istream& inStream, const std::string& streamName,
				const TqCommentCallback& callback = TqCommentCallback());
		virtual void popInput();
		virtual SqRibPos streamPos();

		// Callbacks for request required parameters.
		virtual TqInt getInt();
		virtual TqFloat getFloat();
		virtual std::string getString();
		virtual const TqIntArray& getIntArray();
		virtual const TqFloatArray& getFloatArray(TqInt length = -1);
		virtual const TqStringArray& getStringArray();
		virtual void getParamList(IqRibParamListHandler& paramHandler);
		virtual EqRibToken peekNextType();

		// Callbacks for parameter list parsing (used by IqRibParamListHandler)
		virtual const TqIntArray& getIntParam();
		virtual const TqFloatArray& getFloatParam();
		virtual const TqStringArray& getStringParam();

	private:
		void tokenError(const char* expected, const CqRibToken& badTok); 
		/** \brief A pool of buffers into which RIB arrays will be read.
		 *
		 * The pool serves two purposes:
		 *   * Maintains ownership of the allocated buffers.
		 *   * Attempts to avoid a lot of dynamic allocation.
		 *
		 * A potential problem with the current implementation is that the memory for
		 * individual buffers isn't shared, so the size of the buffers might become
		 * quite large and unbalaced over time.
		 */
		template<typename T>
		class CqBufferPool
		{
			private:
				/// List of buffers
				boost::ptr_vector<std::vector<T> > m_buffers;
				/// Index of next available buffer.
				TqInt m_next;
			public:
				CqBufferPool();
				/// Get the next available buffer from the pool.
				std::vector<T>& getBuf();
				/// Mark all buffers in the pool as unused.
				void markUnused();
		};

		/// RIB lexer
		CqRibLexer m_lex;
		/// RIB requests handler
		boost::shared_ptr<IqRibRequestHandler> m_requestHandler;
		/// pool of parsed float arrays for the current request.
		CqBufferPool<TqFloat> m_floatArrayPool;
		/// pool of parsed int arrays for the current request.
		CqBufferPool<TqInt> m_intArrayPool;
		/// pool of parsed string arrays for the current request.
		CqBufferPool<std::string> m_stringArrayPool;
};

//==============================================================================
// Implementation details
//==============================================================================
// CqBufferPool implementation
template<typename T>
CqRibParser::CqBufferPool<T>::CqBufferPool()
	: m_buffers(),
	m_next(0)
{ }

template<typename T>
std::vector<T>& CqRibParser::CqBufferPool<T>::getBuf()
{
	if(m_next >= static_cast<TqInt>(m_buffers.size()))
		m_buffers.push_back(new std::vector<T>());
	std::vector<T>& buf = m_buffers[m_next];
	++m_next;
	buf.clear();
	return buf;
}

template<typename T>
inline void CqRibParser::CqBufferPool<T>::markUnused()
{
	m_next = 0;
}

} // namespace Aqsis

#endif // RIBPARSER_H_INCLUDED
