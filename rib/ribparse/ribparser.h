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
 * \author Chris Foster [chris42f (at) gmail (d0t) com]
 */

#ifndef RIBPARSER_H_INCLUDED
#define RIBPARSER_H_INCLUDED

#include "aqsis.h"

#include <string>
#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/noncopyable.hpp>
#include <boost/any.hpp>

#include "riblexer.h"
#include "requestparam.h"
#include "exception.h"

namespace ribparse {

class CqRibParser;


//------------------------------------------------------------------------------
/** \brief RIB request handler interface
 */
class IqRibRequest
{
	public:
		/// Construct rib request handler with the given name
		IqRibRequest(const std::string& name);
		/// overridable destructor
		virtual ~IqRibRequest() {}

		/** \brief Handle a RIB request by reading from the parser
		 *
		 * The request handler is expected to call the CqRibParser::get*
		 * functions in order to read appropriate parameters from the parser.
		 */
		virtual void handleRequest(CqRibParser& parser) = 0;

		/** Get the name of the request.
		 *
		 * The name cannot change over the object lifetime.
		 */
		const std::string& name() const;
	private:
		const std::string m_name;
};


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
 * implementation, the user adds request handlers - subclasses of IqRibRequest
 * - to the parser at runtime.  When the parser reads a request from the input
 * stream, it looks for a handler with the appropriate name and calls the
 * IqRibRequest::handleRequest() method.  With this setup, the user can define
 * arbitrary actions to be performed on reading a given request.
 *
 */
class CqRibParser : boost::noncopyable
{
	public:
		/** Construct a RIB parser, connected to the given lexer.
		 *
		 * \param lexer - lexical analyzer for a RIB input stream.
		 * \param ignoreUnrecognized - If true, ignore unrecognized RIB
		 *        requests rather than throwing an exception.  This allows
		 *        selective RIB parsers to be built without too much hassle.
		 */
		CqRibParser(const boost::shared_ptr<CqRibLexer>& lexer,
				bool ignoreUnrecognized = false);

		//--------------------------------------------------
		/// Add a RIB request to the list of valid requests.
		void addRequest(const boost::shared_ptr<IqRibRequest>& request);

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

		/// Read an array of integers from the input
		const TqRibIntArray& getIntArray();
		/// Read an array of floats from the input
		const TqRibFloatArray& getFloatArray();
		/// Read an array of strings from the input
		const TqRibStringArray& getStringArray();

		/// Read a parameter list from the input
		const TqRibParamList& getParamList();
		//@}

	private:
		/// TODO: investigate whether this would be faster as a hash table.
		typedef std::map<std::string,
				boost::shared_ptr<IqRibRequest> > TqRequestMap;
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
		TqRequestMap m_requests;
		/// Ignore unrecognized requests rather than throwing an error.
		bool m_ignoreUnrecognized;
		/// pool of parsed float arrays for the current request.
		CqBufferPool<TqFloat> m_floatArrayPool;
		/// pool of parsed int arrays for the current request.
		CqBufferPool<TqInt> m_intArrayPool;
		/// pool of parsed string arrays for the current request.
		CqBufferPool<std::string> m_stringArrayPool;
		/// Storage for the parameter list of the current request.
		TqRibParamList m_currParamList;
};


//------------------------------------------------------------------------------
/** \class XqParseError
 * \brief An exception class for parser errors.
 */
AQSIS_DECLARE_EXCEPTION(XqParseError, Aqsis::XqValidation);



//==============================================================================
// Implementation details
//==============================================================================

// IqRibRequest Implementation
inline IqRibRequest::IqRibRequest(const std::string& name)
	: m_name(name)
{}

inline const std::string& IqRibRequest::name() const
{
	return m_name;
}

//------------------------------------------------------------------------------
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


} // namespace ribparse

#endif // RIBPARSER_H_INCLUDED
