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
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include "exception.h"
#include "riblexer.h"
#include "iribrequest.h"

namespace Aqsis {

class IqStringToBasis;

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
 * CqRibParser is designed with both these situations in mind.  The user
 * provides a request handler of type IqRibRequestHandler to the parser at
 * runtime.  When the parser reads a request from the input stream, it sends
 * that request to the handler object.  The handler can then call back to the
 * parser in order to get any request arguments.
 *
 * The parser achieves the ideal of being completely ignorant of the
 * semantics of any requests or request parameter lists.  In this design, all
 * the semantics are offloaded to the IqRibRequestHandler class.
 */
class RIBPARSE_SHARE CqRibParser : boost::noncopyable
{
	public:
		// Array types which are passed to RI request handlers from the RIB parser.
		/// RIB int array type
		typedef std::vector<TqInt> TqIntArray;
		/// RIB float array type
		typedef std::vector<TqFloat> TqFloatArray;
		/// RIB string array type
		typedef std::vector<std::string> TqStringArray;

		typedef TqFloat TqBasis[4][4];

		//--------------------------------------------------
		/** \brief Construct a RIB parser, connected to the given lexer.
		 *
		 * \param lexer - lexical analyzer for a RIB input stream.
		 * \param requests - collection of request handlers which will
		 *        recognized by the parser.
		 * \param ignoreUnrecognized - If true, ignore unrecognized RIB
		 *        requests rather than throwing an exception.  This allows
		 *        selective RIB parsers to be built without too much hassle.
		 */
		CqRibParser(const boost::shared_ptr<CqRibLexer>& lexer,
				const boost::shared_ptr<IqRibRequestHandler>& requestHandler);

		/// Access to the underlying lexer object.
		const boost::shared_ptr<CqRibLexer>& lexer();

		/** \brief Parse the next RIB request
		 *
		 * \throw XqParseError if an error is encountered
		 *
		 * \return false if the end of the input stream has been reached.
		 */
		bool parseNextRequest();

		//--------------------------------------------------
		/// \name callbacks for request required parameters.
		//@{
		/// Read an integer from the input
		TqInt getInt();
		/// Read a float from the input
		TqFloat getFloat();
		/// Read a string from the input
		std::string getString();

		/** \brief Read an array of integers from the input.
		 *
		 * \return a reference to the array which is valid until parsing the
		 * next request commences.
		 */
		const TqIntArray& getIntArray();
		/** \brief Read an array of floats from the input
		 *
		 * The array can be in two formats.  The default is an array of
		 * indeterminate length, in the format
		 *
		 *   '[' num_1 num_2 ... num_n ']'
		 *
		 * However, if the length parameter is non-negative, the function also
		 * parses arrays without the delimiting brackets, that is, of the form
		 *
		 *   num_1 num_2 ... num_n
		 *
		 * \param length - number of elements expected in the array.  When
		 * non-negative, this also allows arrays without delimiting brackets to
		 * be parsed.
		 *
		 * \return a reference to the array which is valid until parsing the
		 * next request commences.
		 * \return a reference to the array which is valid until parsing the
		 * next request commences.
		 */
		const TqFloatArray& getFloatArray(TqInt length = -1);
		/** \brief Read an array of strings from the input
		 *
		 * \return a reference to the array which is valid until parsing the
		 * next request commences.
		 */
		const TqStringArray& getStringArray();

		/** \brief Read in a basis matrix.
		 *
		 * The basis can be represented either explicitly as an array of 16
		 * floats, or as a string providing the basis name.  When the string
		 * form is encountered, stringToBasis.getBasis() is invoked with the
		 * basis name and the result returned.
		 *
		 * \param stringToBasis - callback object mapping basis names to basis
		 *                        matrices.
		 * \return A pointer to the retrieved basis matrix.
		 */
		const TqBasis* getBasis(const IqStringToBasis& stringToBasis);
		//@}

		//--------------------------------------------------
		/// \name callbacks for parameter list parsing (used by IqRibParamListHandler)
		//@{
		/// Read an integer or integer array from the input as an array
		const TqIntArray& getIntParam();
		/// Read an float or float array from the input as an array
		const TqFloatArray& getFloatParam();
		/// Read an string or string array from the input as an array
		const TqStringArray& getStringParam();

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
		void getParamList(IqRibParamListHandler& paramHandler);
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
				/// Mark all buffers in the pool as unused.
				void markUnused();
		};

		/// RIB lexer
		boost::shared_ptr<CqRibLexer> m_lex;
		/// RIB requests handler
		boost::shared_ptr<IqRibRequestHandler> m_requestHandler;
		/// pool of parsed float arrays for the current request.
		CqBufferPool<TqFloat> m_floatArrayPool;
		/// pool of parsed int arrays for the current request.
		CqBufferPool<TqInt> m_intArrayPool;
		/// pool of parsed string arrays for the current request.
		CqBufferPool<std::string> m_stringArrayPool;
};

/// Callback interface to get a basis from a name string.
class IqStringToBasis
{
	public:
		/// Get the spline basis corresponding to a name string (throw if not found)
		virtual CqRibParser::TqBasis* getBasis(const std::string& name) const = 0;

		virtual ~IqStringToBasis() {};
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
