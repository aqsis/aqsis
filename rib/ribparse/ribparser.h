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

#include "aqsis.h"

#include <string>
#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include "exception.h"
#include "primvartoken.h"
#include "riblexer.h"
#include "iribrequest.h"

namespace Aqsis {

class CqRequestMap;

//------------------------------------------------------------------------------
/** \brief A flexible parser for RIB-like file formats.
 *
 *
 * At a high level, the renderman interface bytestream (RIB) is a sequence of
 * renderman interface call requests.  Each request has the format
 *
 *   request  =>  SomeRequestName [required_params] [param_list]
 *
 * the required parameters is list of parameters of type string, int, float, or
 * array of these basic types.  The optional parameter list is a set of
 * (string,array) paris;
 *
 *   required_params  =>  [ param ... ]
 *   param_list  =>  [ string param ... ]
 *   param  =>  ( int | float | string | int_array | float_array | string_array )
 *
 *
 * There are a standard set of requests defined by the renderman interface, but
 * most RenderMan-compliant renderers also define implementation-specific
 * requests.  In addition, there are cases where a user may only want to parse
 * a subset of the standard RIB.
 *
 * CqRibParser is designed with both these situations in mind.  In the current
 * implementation, the user provides a set of request handlers - subclasses of
 * IqRibRequest - to the parser at runtime.  When the parser reads a request
 * from the input stream, it looks for a handler with the appropriate name and
 * calls the IqRibRequest::handleRequest() method.  With this setup, the user
 * can define arbitrary actions to be performed on reading a given request.
 *
 */
RIBPARSE_SHARE class CqRibParser : boost::noncopyable
{
	public:
		/** Construct a RIB parser, connected to the given lexer.
		 *
		 * \param lexer - lexical analyzer for a RIB input stream.
		 * \param requests - collection of request handlers which will
		 *        recognized by the parser.
		 * \param ignoreUnrecognized - If true, ignore unrecognized RIB
		 *        requests rather than throwing an exception.  This allows
		 *        selective RIB parsers to be built without too much hassle.
		 */
		CqRibParser(const boost::shared_ptr<CqRibLexer>& lexer,
				const boost::shared_ptr<CqRequestMap>& requests,
				bool ignoreUnrecognized = false);

		/** \brief Parse the next RIB request
		 *
		 * \throw XqParseError if an error is encountered
		 *
		 * \return false if the end of the input stream has been reached.
		 */
		bool parseNextRequest();

		//--------------------------------------------------
		/// \name callbacks for request parameter parsing.
		//@{
		/// Read an integer from the input
		TqInt getInt();
		/// Read a float from the input
		TqFloat getFloat();
		/// Read a string from the input
		std::string getString();

		/** \brief Read an array of integers from the input.
		 *
		 * A reference to the array is valid until parsing the next request
		 * commences.
		 */
		const TqRiIntArray& getIntArray();
		/** \brief Read an array of floats from the input
		 *
		 * A reference to the array is valid until parsing the next request
		 * commences.
		 */
		const TqRiFloatArray& getFloatArray();
		/** \brief Read an array of strings from the input
		 *
		 * A reference to the array is valid until parsing the next request
		 * commences.
		 */
		const TqRiStringArray& getStringArray();

		/** \brief Read a list of (token,value) pairs from the input stream.
		 *
		 * Most RI requests allow extra data to be passed in the form of
		 * parameter lists of (token, value) pairs.  A token is a string giving
		 * the type and name of the value data.  The value data is an array of
		 * floats strings or integers.  Value data consisting of a single float
		 * string or integer is parsed as an array of length one.
		 *
		 * References to the _value_ data inserted into paramList are valid
		 * until parsing the next request commences.
		 * 
		 * \param paramList - destination for (token, value) pairs as read from
		 *            the input stream.
		 */
		const void getParamList(IqRibParamList& paramList);
		//@}

	private:
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
				/// Release the previous buffer obtained with getBuf.
				void releasePrev();
				/// Mark all buffers in the pool as unused.
				void markUnused();
		};

		/// RIB lexer
		boost::shared_ptr<CqRibLexer> m_lex;
		/// Lookup table of RIB requests indexed on request names.
		boost::shared_ptr<CqRequestMap> m_requests;
		/// Ignore unrecognized requests rather than throwing an error.
		bool m_ignoreUnrecognized;
		/// pool of parsed float arrays for the current request.
		CqBufferPool<TqFloat> m_floatArrayPool;
		/// pool of parsed int arrays for the current request.
		CqBufferPool<TqInt> m_intArrayPool;
		/// pool of parsed string arrays for the current request.
		CqBufferPool<std::string> m_stringArrayPool;
};


/** \brief A container for RIB request handlers.
 */
class CqRequestMap
{
	public:
		/** \brief Add a RIB request to the list of valid requests.
		 *
		 * \param request - A pointer to the request object.  This takes
		 *                  ownership of the request, and will delete it
		 *                  appropriately.
		 */
		void add(const std::string& name, IqRibRequest* request);

		/** \brief Find the request with the given name.
		 *
		 * \return The request with the given name, or null no such request
		 *         exists.
		 */
		IqRibRequest* find(const std::string& name);
	private:
		typedef std::map<std::string, boost::shared_ptr<IqRibRequest> > TqRqstMap;
		// TODO: Decide on whether there might be a better container to use
		// here - a hash map for instance.
		TqRqstMap m_requests;
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
inline void CqRibParser::CqBufferPool<T>::releasePrev()
{
	--m_next;
}

template<typename T>
inline void CqRibParser::CqBufferPool<T>::markUnused()
{
	m_next = 0;
}


//------------------------------------------------------------------------------

inline IqRibRequest* CqRequestMap::find(const std::string& name)
{
	TqRqstMap::iterator i = m_requests.find(name);
	if(i == m_requests.end())
		return 0;
	else
		return i->second.get();
}

inline void CqRequestMap::add(const std::string& name, IqRibRequest* request)
{
	m_requests[name].reset(request);
}

} // namespace Aqsis

#endif // RIBPARSER_H_INCLUDED
