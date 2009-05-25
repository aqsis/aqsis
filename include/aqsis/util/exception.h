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
 * \brief Declares classes for the aqsis exception heiarchy
 */

#ifndef EXCEPTION_H_INCLUDED
#define EXCEPTION_H_INCLUDED

#include <aqsis/aqsis.h>
#include <iosfwd>
#include <stdexcept>
#include <utility>
#include <string>
#include <sstream>

namespace Aqsis
{

//-----------------------------------------------------------------------

/** General message based exception. Specific exceptions are derived from this.
 */
class XqException : public std::runtime_error
{
	public:

		/** Simple exception constructor
		 * \param error code (0-61) of the exception (refer to RIE_* error codes in ri.h)
		 * \param reason for the exception
		 */
		XqException (const TqInt code, const std::string& reason);

		/** Basic exception constructor
		 * \param error code (0-61) of the exception (refer to RIE_* error codes in ri.h)
		 * \param reason for the exception
		 * \param file name where the exception was thrown
		 * \param line number where the exception was thrown
		 */
		XqException (const TqInt code, const std::string& reason,
		             const std::string& file, const unsigned int line);

		/** Error code access
		 * \return the error code (0-61) of the exception (refer to RIE_* error codes in ri.h)
		 */
		const TqInt& code() const;

		/** File location access
		 * \return a pair <file name, line number> where the exceptions was thrown
		 */
		std::pair<std::string, unsigned int> where() const;

		/** Description access
		 * \return "General error"
		 */
		virtual const char* description() const;

		/** Exception destructor
		 * @note C++ Standard,  [except.spec] 15.4.14, Example a function that overrides
		 * a virtual function from a base class shall have an exception specification at
		 * least as restrictive as that in the base class.
		 */
		virtual ~XqException() throw() { }

	private:
		const TqInt         m_code;     //< The error code of the exception
		const std::string   m_file;     //< The file name where the exception was thrown
		const unsigned int  m_line;     //< The line number where the exception was thrown
};

/// Stream insertion operator for the aqsis exception base type.
AQSIS_UTIL_SHARE std::ostream& operator<<(std::ostream& o, const XqException& e);

//------------------------------------------------------------------------------

/** \brief Macro to ease the declaration of additional exception types.
 *
 * \note In any try/catch block, it's important to avoid catching exceptions which
 * cannot be resonably handled within the current context.  In general, new
 * exception types should be declared whenever there is a need to catch a
 * something specific and no other exception class fits the bill.
 *
 * \param ExceptionName - name for the new exception class
 * \param ExceptionBase - base class for the new exception
 */
#define AQSIS_DECLARE_XQEXCEPTION(ExceptionName, ExceptionBase)                \
class ExceptionName : public ExceptionBase                                     \
{                                                                              \
	public:                                                                    \
		                                                                       \
		ExceptionName (const TqInt code, const std::string& reason):           \
				ExceptionBase(code, reason)                                    \
		{ }                                                                    \
		                                                                       \
		ExceptionName (const TqInt code, const std::string& reason,            \
		               const std::string& file, const unsigned int line):      \
				ExceptionBase(code, reason, file, line)                        \
		{ }                                                                    \
		virtual const char* description () const                               \
		{                                                                      \
			return #ExceptionName " error";                                    \
		}                                                                      \
		                                                                       \
		virtual ~ExceptionName () throw () { }                                 \
}

//------------------------------------------------------------------------------

/** \class XqInternal
 * \brief Exception base class for all errors internal to aqsis
 */
AQSIS_DECLARE_XQEXCEPTION(XqInternal, XqException);

/** \class XqInvalidFile
 * \brief Errors related to file IO
 * Errors which should be signalled by XqInvalidFile include trying to open
 * non-existant files, and trying to open files with the wrong format.
 */
AQSIS_DECLARE_XQEXCEPTION(XqInvalidFile, XqInternal);

//------------------------------------------------------------------------------

/** \class XqValidation
 * \brief Class for signifying errors in validation of API calls
 */
AQSIS_DECLARE_XQEXCEPTION(XqValidation, XqException);

/** \class XqParseError
 * \brief An exception class for parsing errors
 * Eg: errors in parsing a RIB stream or parsing Ri primvar definition tokens.
 */
AQSIS_DECLARE_XQEXCEPTION(XqParseError, XqValidation);

//------------------------------------------------------------------------------

/** \class XqEnvironment
 * \brief Base class for external and OS-level exceptions
 */
AQSIS_DECLARE_XQEXCEPTION(XqEnvironment, XqException);

//==============================================================================
// Error Codes and Severity Level enums
//==============================================================================

/** Aqsis Error Code Enum
 * \brief mirrors the RIE_* error codes in ri.h and are to be used for tagging
 * exceptions and log messages with a suitable code.
 */

enum EqErrorCode
{
	EqE_NoError     = 0,  // RIE_NOERROR
	
	//// 1 - 10 System and File Errors ////
	EqE_NoMem       = 1,  // RIE_NOMEM       /* Out of memory */
	EqE_System      = 2,  // RIE_SYSTEM      /* Miscellaneous system error */
	EqE_NoFile      = 3,  // RIE_NOFILE      /* File nonexistent */
	EqE_BadFile     = 4,  // RIE_BADFILE     /* Bad file format */
	EqE_Version     = 5,  // RIE_VERSION     /* File version mismatch */
	EqE_DiskFull    = 6,  // RIE_DISKFULL    /* Target disk is full */

	//// 11 - 20 Program Limitations ////
	EqE_Incapable   = 11, // RIE_INCAPABLE   /* Optional RI feature */
	EqE_Unimplement = 12, // RIE_UNIMPLEMENT /* Unimplemented feature */
	EqE_Limit       = 13, // RIE_LIMIT       /* Arbitrary program limit */
	EqE_Bug         = 14, // RIE_BUG         /* Probably a bug in renderer */

	//// 21 - 40 State Errors ////
	EqE_NotStarted  = 23, // RIE_NOTSTARTED  /* RiBegin not called */
	EqE_Nesting     = 24, // RIE_NESTING     /* Bad begin-end nesting */
	EqE_NotOptions  = 25, // RIE_NOTOPTIONS  /* Invalid state for options */
	EqE_NotAttribs  = 26, // RIE_NOTATTRIBS  /* Invalid state for attribs */
	EqE_NotPrims    = 27, // RIE_NOTPRIMS    /* Invalid state for primitives */
	EqE_IllState    = 28, // RIE_ILLSTATE    /* Other invalid state */
	EqE_BadMotion   = 29, // RIE_BADMOTION   /* Badly formed motion block */
	EqE_BadSolid    = 30, // RIE_BADSOLID    /* Badly formed solid block */

	//// 41 - 60 Parameter and Protocol Errors ////
	EqE_BadToken    = 41, // RIE_BADTOKEN    /* Invalid token for request */
	EqE_Range       = 42, // RIE_RANGE       /* Parameter out of range */
	EqE_Consistency = 43, // RIE_CONSISTENCY /* Parameters inconsistent */
	EqE_BadHandle   = 44, // RIE_BADHANDLE   /* Bad object/light handle */
	EqE_NoShader    = 45, // RIE_NOSHADER    /* Can't load requested shader */
	EqE_MissingData = 46, // RIE_MISSINGDATA /* Required parameters not provided */
	EqE_Syntax      = 47, // RIE_SYNTAX      /* Declare type syntax error */

	//// 61 - 80 Execution Errors ////
	EqE_Math        = 61  // RIE_MATH        /* Zerodivide, noninvert matrix, etc. */
};

/** Aqsis Severity Level Enum
 * \brief mirrors the RIE_* severity levels in ri.h and are to be used for tagging
 * exceptions and log messages with a suitable level.
 */

enum EqSeverityLevel
{
	EqE_Info        = 0, // RIE_INFO         /* Rendering stats and other info */
	EqE_Warning     = 1, // RIE_WARNING      /* Something seems wrong, maybe okay */
	EqE_Error       = 2, // RIE_ERROR        /* Problem. Results may be wrong */
	EqE_Severe      = 3  // RIE_SEVERE       /* So bad you should probably abort */
};

//==============================================================================
// Convenience Macros
//==============================================================================

/** Throw a basic XqException
 *
 * \brief this macro eases the use of XqExceptions
 *
 * \code
 * AQSIS_THROW_XQERROR(XqInternal, EqE_NoFile, "Could not find file \"" << fileName << "\"");
 * \endcode
 *
 * \param ExceptionClass of the exception to throw
 * \param error code (0-61) of the exception (refer to RIE_* error codes in ri.h)
 * \param reason for the exception (any expression which can be ostream-inserted)
 */

// note the weird usage of do {...} while (0) is to allow the usual if/else syntax to
// work as expected when the macro is treated syntatcitcally as a function. For
// details, see http://kernelnewbies.org/FAQ/DoWhile0

#define AQSIS_THROW_XQERROR(ExceptionClass, code, reason)                      \
	do {                                                                       \
		std::ostringstream os;                                                 \
		os << reason;                                                          \
		throw ExceptionClass(code, os.str(), __FILE__, __LINE__);              \
	} while(0)

//==============================================================================
// Implementation details
//==============================================================================

inline XqException::XqException (const TqInt code, const std::string& reason):
								std::runtime_error(reason),
								m_code(code),
								m_file("Unspecified"),
								m_line(0)
{
}

inline XqException::XqException (const TqInt code, const std::string& reason,
								 const std::string& file, const unsigned int line):
								std::runtime_error(reason),
								m_code(code),
								m_file(file),
								m_line(line)
{
}

inline const TqInt& XqException::code() const
{
	return m_code;
}

inline std::pair<std::string, unsigned int> XqException::where() const
{
	return std::make_pair (m_file, m_line);
}

inline const char* XqException::description() const
{
	return "General error";
}

} // namespace Aqsis

#endif // EXCEPTION_H_INCLUDED
