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

#ifndef IRIBPARSER_H_INCLUDED
#define IRIBPARSER_H_INCLUDED

#include <aqsis/aqsis.h>

#include <iostream>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#ifdef AQSIS_SYSTEM_WIN32
#	ifdef AQSIS_STATIC_LINK
#		define AQSIS_RIBPARSER_SHARE
#	else
#		ifdef AQSIS_RIBPARSER_EXPORTS
#			define AQSIS_RIBPARSER_SHARE __declspec(dllexport)
#		else
#			define AQSIS_RIBPARSER_SHARE __declspec(dllimport)
#		endif
#	endif
#else
#	define AQSIS_RIBPARSER_SHARE
#endif

namespace Aqsis {

/// A holder for RIB position and name.
struct SqRibPos
{
	TqInt line;
	TqInt col;
	std::string name;

	/// Trivial constructor
	SqRibPos(TqInt line, TqInt col, const std::string& name);
};

/// pretty print source file position
std::ostream& operator<<(std::ostream& out, const SqRibPos& pos);


// forward declarations
class IqRibRequestHandler;
class IqRibParamListHandler;

//------------------------------------------------------------------------------
/** \brief A flexible parser for RIB-like file formats.
 *
 *
 * At a high level, the renderman interface bytestream (RIB) is a sequence of
 * renderman interface call requests.  Each request has the format
 *
 * \verbatim
 *   request  =>  SomeRequestName [required_params] [param_list]
 * \endverbatim
 *
 * the required parameters is list of parameters of type string, int, float, or
 * arrays of these basic types.  The optional parameter list is a set of
 * (string,array) paris;
 *
 * \verbatim
 *   required_params  =>  [ param ... ]
 *   param_list  =>  [ string param ... ]
 *   param  =>  ( int | float | string | int_array | float_array | string_array )
 * \endverbatim
 *
 *
 * There are a standard set of requests defined by the renderman interface, but
 * most RenderMan-compliant renderers also define implementation-specific
 * requests.  In addition, there are cases where a user may only want to parse
 * a subset of the standard RIB.
 *
 * IqRibParser is designed with both these situations in mind.  The user
 * provides a request handler of type IqRibRequestHandler to the parser at
 * runtime.  When the parser reads a request from the input stream, it sends
 * that request name to the handler object.  The handler should then call back
 * to the parser in order to get any required parameters for the request.
 *
 * The parser contains methods getInt(), getFloat(), getString(),
 * getIntArray(), getFloatArray() and getStringArray() which may be
 * called by the handler object to obtain the required parameters for the
 * request.  In addition, the getParamList() function allows variable argument
 * parameter lists of token value pairs to be parsed.
 *
 * The parser itself is concerned purely with processing the syntax of the RIB
 * format; it is completely ignorant of the semantics of any requests or
 * request parameter lists.  All semantic analysis is offloaded to the
 * IqRibRequestHandler class.
 *
 *
 * The input stream connected to the parser may be changed on the fly using the
 * pushInput() and popInput() functions which control an internal stack of
 * input streams.  This functionality supports the needs of standard interface
 * functions like ReadArchive and RiProcRunProgram which require the input
 * context to be changed between requests.
 */
class AQSIS_RIBPARSER_SHARE IqRibParser
{
	public:
		// Array types which are passed to RI request handlers from the RIB parser.
		/// RIB int array type
		typedef std::vector<TqInt> TqIntArray;
		/// RIB float array type
		typedef std::vector<TqFloat> TqFloatArray;
		/// RIB string array type
		typedef std::vector<std::string> TqStringArray;

		/// Callback function type for passing comments.
		typedef boost::function<void (const std::string&)> TqCommentCallback;

		/// RIB token types used by the peekNextType() function
		enum EqRibToken
		{
			Tok_Int,
			Tok_Float,
			Tok_String,
			Tok_Array,
			Tok_RequestEnd
		};

		//--------------------------------------------------
		/** \brief Construct and return a CqRibParser instance
		 *
		 * The initial input stream is a null stream so parseNextRequest() has
		 * no effect until pushInput() is called.
		 *
		 * \param handler - Object which will be called by the parser to handle
		 * the semantics of the various RIB requests.
		 */
		static boost::shared_ptr<IqRibParser> create(
				const boost::shared_ptr<IqRibRequestHandler>& handler);

		//--------------------------------------------------
		/** \brief Parse the next RIB request
		 *
		 * \throw XqParseError if an error is encountered
		 *
		 * Returns false if the end of the input has been reached, true
		 * otherwise.  If there are no input streams on the input stack return
		 * false immediately.
		 */
		virtual bool parseNextRequest() = 0;

		//--------------------------------------------------
		/// \name Stream management
		//@{
		/** \brief Push a stream onto the input stack
		 *
		 * The stream will remain as the current input stream until a
		 * corresponding call to popInput() is encountered.
		 *
		 * Note that the stream should be opened in binary mode so that no
		 * translation of newline characters is performed.  If not, any binary
		 * encoded RIB will be read incorrectly.
		 *
		 * According to the RISpec, it's possible to install a callback
		 * function to perform a custom action when the parser encounters a
		 * comment (see the section describing RiReadArchive).  The comment
		 * callback function is called immediately whenever a comment is
		 * encountered in the RIB stream.  The argument to the callback is the
		 * comment string from one character after the leading # up to and not
		 * including the next end of line character.
		 *
		 * \param inStream - stream from which RIB will be read
		 * \param streamName - name of the input stream
		 * \param commentCallback - callback function for handling comments in
		 *                          the RIB stream.  Defaults to an empty
		 *                          callback which corresponds to discarding
		 *                          all comments.
		 */
		virtual void pushInput(std::istream& inStream,
				const std::string& streamName,
				const TqCommentCallback& callback = TqCommentCallback()) = 0;
		/** \brief Pop a stream off the input stack
		 *
		 * If the stream is the last on the input stack, the parser reverts to
		 * null input for which parseNextRequest() has no effect.
		 */
		virtual void popInput() = 0;
		/** \brief Get the position in the currently connected RIB
		 *
		 * The current position and name of the RIB on top of the stream stack
		 * is returned.  This should be used for error reporting.
		 */
		virtual SqRibPos streamPos() = 0;
		//@}

		//--------------------------------------------------
		/// \name Callbacks for request parameter parsing.
		//@{
		/// Read an integer from the input
		virtual TqInt getInt() = 0;
		/// Read a float from the input
		virtual TqFloat getFloat() = 0;
		/// Read a string from the input
		virtual std::string getString() = 0;

		/** \brief Read an array of integers from the input.
		 *
		 * \return a reference to the array which is valid until parsing the
		 * next request commences.
		 */
		virtual const TqIntArray& getIntArray() = 0;
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
		virtual const TqFloatArray& getFloatArray(TqInt length = -1) = 0;
		/** \brief Read an array of strings from the input
		 *
		 * \return a reference to the array which is valid until parsing the
		 * next request commences.
		 */
		virtual const TqStringArray& getStringArray() = 0;

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
		virtual void getParamList(IqRibParamListHandler& paramHandler) = 0;

		/** \brief Return the type of the next RIB token in the stream.
		 *
		 * Some requests such as SubdivisionMesh or Basis have multiple
		 * allowable forms so the request handler may need to decide which type
		 * to read next from the parser.
		 *
		 * \return The type of the next request parameter in the input.  If the
		 * next token is not a valid request parameter Tok_RequestEnd is
		 * returned.
		 */
		virtual EqRibToken peekNextType() = 0;
		//@}

		//--------------------------------------------------
		/// \name Callbacks used by IqRibParamListHandler in parameter list parsing.
		//@{
		/// Read an integer or integer array from the input as an array
		virtual const TqIntArray& getIntParam() = 0;
		/// Read an float or float array from the input as an array
		virtual const TqFloatArray& getFloatParam() = 0;
		/// Read an string or string array from the input as an array
		virtual const TqStringArray& getStringParam() = 0;
		//@}

		/// Overridable destructor.
		virtual ~IqRibParser() {}
};


//------------------------------------------------------------------------------
/** \brief RIB request handler interface
 *
 * Code expecting to handle RIB requests should inherit from this class.  This
 * allows it to be attached to and called by the RIB parser at runtime.
 */
class AQSIS_RIBPARSER_SHARE IqRibRequestHandler
{
	public:
		/** \brief Handle a RIB request by reading from the parser
		 *
		 * The request handler is expected to call the IqRibParser::get*
		 * functions of the parser object in order to read the required
		 * parameters for the request.
		 *
		 * \param requestName - name of the request to handle.
		 * \param parser - source from which request parameters should be read.
		 */
		virtual void handleRequest(const std::string& requestName,
				IqRibParser& parser) = 0;

		virtual ~IqRibRequestHandler() {}
};

//------------------------------------------------------------------------------
/// RIB parameter list handler.
class IqRibParamListHandler
{
	public:
		/** \brief Read a RIB parameter with the supplied name from the parser.
		 *
		 * The handler should be prepared to determine the type of parameter to
		 * be read by inspecting the parameter name.  It should then read the
		 * parameter using IqRibParser::getIntParam(), getFloatParam() or
		 * getStringParam().
		 *
		 * \param name - raw parameter name as read from the input stream
		 * \param parser - source from which the parameter should be read.
		 */
		virtual void readParameter(const std::string& name, IqRibParser& parser) = 0;

		virtual ~IqRibParamListHandler() {}
};



//==============================================================================
// Implementation details
//==============================================================================
// SqRibPos functions

inline SqRibPos::SqRibPos(TqInt line, TqInt col, const std::string& name)
	: line(line),
	col(col),
	name(name)
{ }

inline std::ostream& operator<<(std::ostream& out, const SqRibPos& pos)
{
	out << pos.name << ": " << pos.line << " (col " << pos.col << ")";
	return out;
}

} // namespace Aqsis

#endif // IRIBPARSER_H_INCLUDED
