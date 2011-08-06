// Aqsis
// Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the software's owners nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// (This is the New BSD license)

/// \file
///
/// \brief ErrorHandler object for C++ version of RI
/// \author Chris Foster [chris42f (at) g mail (d0t) com]
///

#ifndef AQSIS_ERRORHANDLER_H_INCLUDED
#define AQSIS_ERRORHANDLER_H_INCLUDED

#include <sstream>
#include <string>

#include <aqsis/util/tinyformat.h>

namespace Aqsis {

namespace Ri {

namespace detail { class ErrorFormatter; }

/// A simple error formatting and handler class.
///
/// This class is designed for simple formatting and dispatch of error
/// (code,string) pairs.  All formatting operations are evaluated lazily,
/// depending on the error logging verbosity.  This means we can almost
/// completely avoid a performance hit for log messages that we're going to
/// discard anyway.
///
/// Error codes are constructed from the combination of an error category in
/// the high eight bits, with the low 24 bits free for arbitrary
/// application-defined errors.
///
/// Filtering verbosity should be specified using setVerbosity().
///
/// Example usage:
/// \code
///     ErrorHandler& handler = /* ... */;
///
///     handler(ErrorHandler::Error, "a simple string error");
///
///     int some_int = 42;
///     handler.error(0, "some integer = %d", some_int);
///
///     const int badfileCode = 10;
///     if(!findFile(fileName))
///         handler.warning(badFileCode, "could not find file \"%s\"", fileName);
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

        /// Get the error reporting verbosity.
        inline ErrorCategory verbosity() const;
        inline void setVerbosity(ErrorCategory verbosity);

        // The following mess defines the formatting functions.  It would be
        // possible to do this without the macros if we assumed c++0x variadic
        // templates support.

        // Extra args for TINYFORMAT_WRAP_FORMAT macro
#       define TINYFORMAT_WRAP_FORMAT_EXTRA_ARGS int code,

#       define AQSIS_DEFINE_LOGGING_FUNC(category, funcName)            \
        TINYFORMAT_WRAP_FORMAT(void, funcName,                          \
            if(category < m_verbosity) return; std::ostringstream oss;, \
            oss,                                                        \
            dispatch(category | code, oss.str());                       \
        )

        /// Format message and report with a given category
        ///
        /// There's functions called debug(), info(), warning(), error(),
        /// severe(), and message().  Each have the same prototype:
        ///
        /// template<typename T1, ...>
        /// void error(int code, const char* fmt, const T1& value1, ...);
        ///
        /// where code is the detailed error code, fmt is a format string in
        /// printf notation, and the list of types is formatted in a type safe
        /// way according to the format directives given in fmt.
        AQSIS_DEFINE_LOGGING_FUNC(Debug, debug)
        AQSIS_DEFINE_LOGGING_FUNC(Info, info)
        AQSIS_DEFINE_LOGGING_FUNC(Warning, warning)
        AQSIS_DEFINE_LOGGING_FUNC(Error, error)
        AQSIS_DEFINE_LOGGING_FUNC(Severe, severe)
        AQSIS_DEFINE_LOGGING_FUNC(Message, message)

        /// The log() function is like error() and friends, but expects you to
        /// define the ErrorCategory as part of the code.
        TINYFORMAT_WRAP_FORMAT(void, log,
            if(code < m_verbosity) return; std::ostringstream oss;,
            oss,
            dispatch(code, oss.str());
        )

#       undef AQSIS_DEFINE_LOGGING_FUNC
#       undef TINYFORMAT_WRAP_FORMAT_EXTRA_ARGS

    protected:
        /// Send the error to the implementation backend.
        ///
        /// This function needs to be overridden by child classes to dispose
        /// of the actual error message somehow.
        virtual void dispatch(int code, const std::string& message) = 0;

        /// Get error category from an error code
        static ErrorCategory errorCategory(int code);
        /// Get error code detail from an error code.
        static int errorDetail(int code);

    private:
        friend class detail::ErrorFormatter;

        ErrorCategory m_verbosity;
};


//==============================================================================
// Implementation details
inline ErrorHandler::ErrorHandler(ErrorCategory verbosity)
    : m_verbosity(verbosity)
{ }

inline ErrorHandler::ErrorCategory ErrorHandler::verbosity() const
{
    return m_verbosity;
}

inline void ErrorHandler::setVerbosity(ErrorCategory verbosity)
{
    m_verbosity = verbosity;
}

inline ErrorHandler::ErrorCategory ErrorHandler::errorCategory(int code)
{
    return static_cast<ErrorCategory>(0xFF000000 & code);
}

inline int ErrorHandler::errorDetail(int code)
{
    return 0xFFFFFF & code;
}

} // namespace Ri

using Ri::ErrorHandler;

} // namespace Aqsis

#endif // AQSIS_ERRORHANDLER_H_INCLUDED
// vi: set et:
