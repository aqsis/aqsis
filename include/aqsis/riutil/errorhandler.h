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

/// \file
///
/// \brief ErrorHandler object for C++ version of RI
/// \author Chris Foster [chris42f (at) g mail (d0t) com]
///

#ifndef AQSIS_ERRORHANDLER_H_INCLUDED
#define AQSIS_ERRORHANDLER_H_INCLUDED

#include <sstream>
#include <string>

namespace Aqsis {

namespace Ri {

namespace detail { class ErrorFormatter; }

/// A simple error formatting and handler class.
///
/// This class and associated macros are designed for simple formatting and
/// dispatch of error (code,string) pairs.  operator() should be used to
/// report simple string error messages.  For more complicated error message
/// formatting, the family of macros AQSIS_LOG_* (AQSIS_LOG_ERROR, etc) are
/// provided.  These allow the formatting operations to be evaluated lazily,
/// depending on the error logging verbosity.  This means we can almost
/// completely avoid a performance hit for log messages that we're going to
/// discard anyway.
///
/// Error codes are constructed from the combination of an error category in
/// the high eight bits, with the low 24 bits free for arbitrary
/// application-defined errors.
///
/// Filtering verbosity should be specified using
///
/// Example usage:
/// \code
///     ErrorHandler& handler = /* ... */;
///
///     handler(ErrorHandler::Error, "a simple string error");
///
///     int some_int = 42;
///     AQSIS_LOG_ERROR(handler,0) << "some integer = " << some_int;
///
///     const int badfileCode = 10;
///     if(!findFile(fileName))
///     {
///         AQSIS_LOG_WARNING(handler,badFileCode)
///             << "could not find file \"" << fileName << "\"";
///     }
///
///     // Lazy formatting even works with boost::format, if you want to do
///     // some heavyweight formatting:
///     AQSIS_LOG_WARNING(handler,0)
///         << boost::format("complicated format: %0.10f:%s") % 1.234 % "str";
/// \endcode
///
class ErrorHandler
{
    public:
        enum ErrorCategory
        {
            Debug    = 1<<24,
            Info     = 2<<24,
            Warning  = 3<<24,
            Error    = 4<<24,
            Severe   = 5<<24,
            Message  = 6<<24,
        };

        ErrorHandler(ErrorCategory verbosity);
        virtual ~ErrorHandler() {}

        /// Report a simple string error with the given code
        ///
        /// For efficient lazy formatting of complicated error messages, use
        /// the AQSIS_LOG_* macros.
        inline void operator()(int code, const std::string& message);

        /// Get the error reporting verbosity.
        inline ErrorCategory verbosity() const;
        inline void setVerbosity(ErrorCategory verbosity) const;

    protected:
        /// Send the error to the implementation backend.
        ///
        /// This function needs to be overridden by child classes to dispose
        /// of the actual error message somehow.
        virtual void sendError(int code, const std::string& message) = 0;

        /// Get error category from an error code
        static ErrorCategory errorCategory(int code);
        /// Get error code detail from an error code.
        static int errorDetail(int code);

    private:
        friend class detail::ErrorFormatter;

        ErrorCategory m_verbosity;
};


///\name Lazy error formatter macros
//@{
/// Log to the error handler with the Info message category
/// \param handler - handler instance to log message to
/// \param detail  - application defined error code
#define AQSIS_LOG_INFO(handler, detail)    AQSIS_FORMAT_AND_LOG_ERROR(handler, Aqsis::Ri::ErrorHandler::Info | detail)
/// \see AQSIS_LOG_INFO
#define AQSIS_LOG_WARNING(handler, detail) AQSIS_FORMAT_AND_LOG_ERROR(handler, Aqsis::Ri::ErrorHandler::Warning | detail)
/// \see AQSIS_LOG_INFO
#define AQSIS_LOG_ERROR(handler, detail)   AQSIS_FORMAT_AND_LOG_ERROR(handler, Aqsis::Ri::ErrorHandler::Error | detail)
/// \see AQSIS_LOG_INFO
#define AQSIS_LOG_SEVERE(handler, detail)  AQSIS_FORMAT_AND_LOG_ERROR(handler, Aqsis::Ri::ErrorHandler::Severe | detail)
/// \see AQSIS_LOG_INFO
#define AQSIS_LOG_MESSAGE(handler, detail) AQSIS_FORMAT_AND_LOG_ERROR(handler, Aqsis::Ri::ErrorHandler::Message | detail)

#ifdef AQSIS_DEBUG
#    define AQSIS_LOG_DEBUG(handler, detail) AQSIS_FORMAT_AND_LOG_ERROR(handler, Aqsis::Ri::ErrorHandler::Debug | detail)
#else
/// \see AQSIS_LOG_INFO
#    define AQSIS_LOG_DEBUG(handler, detail)
#endif

#define AQSIS_LOG(handler, code) AQSIS_FORMAT_AND_LOG_ERROR(handler, code)
//@}


//==============================================================================
// Implementation details
inline ErrorHandler::ErrorHandler(ErrorCategory verbosity)
    : m_verbosity(verbosity)
{ }

inline ErrorHandler::ErrorCategory ErrorHandler::verbosity() const
{
    return m_verbosity;
}

inline void ErrorHandler::operator()(int code, const std::string& message)
{
    if(m_verbosity <= code)
        sendError(code, message);
}

inline ErrorHandler::ErrorCategory ErrorHandler::errorCategory(int code)
{
    return static_cast<ErrorCategory>(0xFF000000 & code);
}

inline int ErrorHandler::errorDetail(int code)
{
    return 0xFFFFFF & code;
}

#define AQSIS_FORMAT_AND_LOG_ERROR(handler,code)   \
if((handler).verbosity() > (code))                 \
    /*avoid formatting if verbosity is low*/;      \
else                                               \
    Aqsis::Ri::detail::ErrorFormatter(handler, code)

namespace detail
{

/// Formatting shim for custom ErrorHandler formatting
///
/// Override this function for your type to provide custom ErrorHandler
/// formatting; the default uses operator<<.
template<typename T>
inline void formatForRiErrorHandler(std::ostream& out, const T& value)
{
    out << value;
}

/// Formatter helper class for ErrorHandler.
class ErrorFormatter
{
    private:
        std::ostringstream m_stream;
        ErrorHandler& m_handler;
        int m_code;

    public:
        ErrorFormatter(ErrorHandler& handler, int code)
            : m_stream(),
            m_handler(handler),
            m_code(code)
        {}

        template<typename T>
        ErrorFormatter& operator<<(const T& val)
        {
            formatForRiErrorHandler(m_stream, val);
            return *this;
        }

        ~ErrorFormatter()
        {
            m_handler.sendError(m_code, m_stream.str());
        }
};

}

} // namespace Ri

using Ri::ErrorHandler;

} // namespace Aqsis

#endif // AQSIS_ERRORHANDLER_H_INCLUDED
// vi: set et:
