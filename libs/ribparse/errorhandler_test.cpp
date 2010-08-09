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
/// \brief ErrorHandler object - tests
/// \author Chris Foster [chris42f (at) g mail (d0t) com]
///

#include "errorhandler.h"

#ifndef AQSIS_SYSTEM_WIN32
#define BOOST_TEST_DYN_LINK
#endif //AQSIS_SYSTEM_WIN32

#include <boost/test/auto_unit_test.hpp>

namespace {

class ErrorHandlerTestImpl : public Aqsis::Ri::ErrorHandler
{
    public:
        ErrorHandlerTestImpl(ErrorCategory verbosity)
            : ErrorHandler(verbosity)
        { }

        const std::string& lastError() { return m_lastError; }

    protected:
        virtual void sendError(int code, const std::string& message)
        {
            std::ostringstream out;
            switch(errorCategory(code))
            {
                case Debug:   out << "DEBUG: ";   break;
                case Info:    out << "INFO: ";    break;
                case Warning: out << "WARNING: "; break;
                case Error:   out << "ERROR: ";   break;
                case Severe:  out << "SEVERE: ";  break;
                case Message: break;
            }
            out << message;
            m_lastError = out.str();
        }

    private:
        std::string m_lastError;
};

struct FormatLogger
{
    mutable bool wasFormatted;
    FormatLogger() : wasFormatted(false) {}
};

std::ostream& operator<<(std::ostream& out, const FormatLogger& l)
{
    l.wasFormatted = true;
    return out;
}

} // anon. namespace

BOOST_AUTO_TEST_CASE(ErrorHandler_test)
{
    ErrorHandlerTestImpl handler(Aqsis::Ri::ErrorHandler::Debug);

    handler(Aqsis::Ri::ErrorHandler::Warning, "blah blah");

    BOOST_CHECK_EQUAL(handler.lastError(), "WARNING: blah blah");

    int i = 42;
    AQSIS_LOG_ERROR(handler,0) << "an integer = " << i << " " << i;
    BOOST_CHECK_EQUAL(handler.lastError(), "ERROR: an integer = 42 42");
}


BOOST_AUTO_TEST_CASE(ErrorHandler_lazy_formatting_test)
{
    ErrorHandlerTestImpl handler(Aqsis::Ri::ErrorHandler::Error);
    {
        FormatLogger l;
        AQSIS_LOG_ERROR(handler,0) << l;
        BOOST_CHECK(l.wasFormatted);
        BOOST_CHECK_EQUAL(handler.lastError(), "ERROR: ");
    }
    {
        // Check that when a message is below the verbosity threshold, it
        // doesn't even get formatted.
        FormatLogger l;
        AQSIS_LOG_WARNING(handler,0) << l;
        BOOST_CHECK(!l.wasFormatted);
        BOOST_CHECK_EQUAL(handler.lastError(), "ERROR: ");
    }
}

// vi: set et:
