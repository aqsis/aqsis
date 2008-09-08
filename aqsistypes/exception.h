// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
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
 * \brief Declares classes for the aqsis exception heiarchy.
 * \author Paul C. Gregory (pgregory@aqsis.org)
 */

#ifndef EXCEPTION_H_INCLUDED
#define EXCEPTION_H_INCLUDED

#include "aqsis.h"
#include <iosfwd>
#include <stdexcept>
#include <utility>
#include <string>
#include <sstream>

namespace Aqsis {

//-----------------------------------------------------------------------
/** General message based exception.  specific exceptions are derived from this.
 */

class COMMON_SHARE XqException : public std::runtime_error
{
	public:
		/**
		* @deprecated Legacy constructor for backwards compatibility
		*/
		XqException (const std::string& reason);
		
		XqException (const std::string& reason, const std::string& detail,
		const std::string& file, const unsigned int line);
		
		XqException (const std::string& reason, const std::string& file,
			const unsigned int line);
		
		const std::string& detail () const;
		
		std::pair<std::string, unsigned int> where () const;
		
		virtual const char* description () const;
		
		/**
		* C++ Standard,  [except.spec] 15.4.14, Example
		* a function that overrides a virtual function from a base class shall have an exception specification
		* at least as restrictive as that in the base class.
		*/
		virtual ~XqException () throw ();
		
	private:
		const std::string 	m_detail; 	//< Optional, a detailed message
		const std::string 	m_file;		//< The file name where the exception was thrown
		const unsigned int	m_line;		//< The line numer where the exception was thrown
}
;

/// Stream insertion operator for the aqsis exception base type.
COMMON_SHARE std::ostream& operator<<(std::ostream& o, const XqException& e);

//------------------------------------------------------------------------------
/** \brief Macro to ease the declaration of additional exception types.
 *
 * In any try/catch block, it's important to avoid catching exceptions which
 * cannot be resonably handled within the current context.  In general, new
 * exception types should be declared whenever there is a need to catch a
 * something specific and no other exception class fits the bill.
 *
 * \param ExceptionName - name for the new exception class
 * \param ExceptionBase - base class for the new exception
 */
#define AQSIS_DECLARE_EXCEPTION(ExceptionName, ExceptionBase)                 \
class ExceptionName : public ExceptionBase                                    \
{                                                                             \
	public:                                                                   \
		ExceptionName (const std::string& reason, const std::string& detail,  \
			const std::string& file, const unsigned int line)                 \
			: ExceptionBase(reason, detail, file, line)                       \
		{ }                                                                   \
		ExceptionName (const std::string& reason, const std::string& file,    \
			const unsigned int line)                                          \
			: ExceptionBase(reason, file, line)                               \
		{ }                                                                   \
		virtual const char* description () const                              \
		{                                                                     \
			return #ExceptionName " error";                                   \
		}                                                                     \
		virtual ~ExceptionName () throw () { }                                \
}

//------------------------------------------------------------------------------
/** \class XqInternal
 *
 * \brief Exception base class for all errors internal to aqsis
 */
AQSIS_DECLARE_EXCEPTION(XqInternal, XqException);

/** \class XqInvalidFile
 *
 * \brief Errors related to file IO
 *
 * Errors which should be signalled by XqInvalidFile include trying to open
 * non-existant files, and trying to open files with the wrong format.
 */
AQSIS_DECLARE_EXCEPTION(XqInvalidFile, XqInternal);

//------------------------------------------------------------------------------
/** \class XqValidation
 * \brief Class for signifying errors in validation of API calls
 */
AQSIS_DECLARE_EXCEPTION(XqValidation, XqException);

/** \class XqParseError
 * \brief An exception class for parsing errors
 *
 * Eg: errors in parsing a RIB stream or parsing Ri primvar definition tokens.
 */
AQSIS_DECLARE_EXCEPTION(XqParseError, XqValidation);

//------------------------------------------------------------------------------
/** \class XqEnvironment
 * \brief Base class for external and OS-level exceptions
 */
AQSIS_DECLARE_EXCEPTION(XqEnvironment, XqException);


//------------------------------------------------------------------------------
/** \brief Macro to ease the formatting of exception messages.
 *
 * Example:
 *   AQ_THROW(XqInternal, "Could not find file \"" << fileName << "\"");
 *
 * \param ExceptionClass - class of the exception to throw.
 * \param message - any expression which can be ostream-inserted.
 */
// Implementation note: weird usage of do { } while (0) is to allow the usual
// if/else syntax to work as expected when the macro is treated syntatcitcally
// as a function.  For details, see http://kernelnewbies.org/FAQ/DoWhile0
#define AQSIS_THROW(ExceptionClass, message)                                  \
	do {                                                                      \
		std::ostringstream os;                                                \
		os << message;                                                        \
		throw ExceptionClass(os.str(), __FILE__, __LINE__);                   \
	} while(0)

/** \brief Macro to ease formatting of exception messages with a "detail" field.
 *
 * \see AQSIS_THROW
 */
#define AQSIS_THROW_DETAIL(ExceptionClass, message, detail)                   \
	do {                                                                      \
		std::ostringstream os;                                                \
		os << message;                                                        \
		std::ostringstream os2;                                               \
		os2 << detail;                                                        \
		throw ExceptionClass(os.str(), os2.str(), __FILE__, __LINE__);        \
	} while(0)


//------------------------------------------------------------------------------
} // namespace Aqsis

#endif // EXCEPTION_H_INCLUDED
