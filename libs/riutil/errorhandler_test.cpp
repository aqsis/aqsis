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
/// \brief ErrorHandler object - tests
/// \author Chris Foster [chris42f (at) g mail (d0t) com]
///

#include <aqsis/riutil/errorhandler.h>

#define BOOST_TEST_DYN_LINK
#include <boost/test/auto_unit_test.hpp>

BOOST_AUTO_TEST_SUITE(errorhandler_tests)

namespace {

class ErrorHandlerTestImpl : public Aqsis::Ri::ErrorHandler
{
    public:
        ErrorHandlerTestImpl(ErrorCategory verbosity)
            : ErrorHandler(verbosity)
        { }

        const std::string& lastError() { return m_lastError; }

    protected:
        virtual void dispatch(int code, const std::string& message)
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

    handler.log(Aqsis::Ri::ErrorHandler::Warning, "blah blah");

    BOOST_CHECK_EQUAL(handler.lastError(), "WARNING: blah blah");

    int i = 42;
    handler.error(0, "an integer = %d %d", i, i);
    BOOST_CHECK_EQUAL(handler.lastError(), "ERROR: an integer = 42 42");
}


BOOST_AUTO_TEST_CASE(ErrorHandler_lazy_formatting_test)
{
    ErrorHandlerTestImpl handler(Aqsis::Ri::ErrorHandler::Error);
    {
        FormatLogger l;
        handler.error(0, "%s", l);
        BOOST_CHECK(l.wasFormatted);
        BOOST_CHECK_EQUAL(handler.lastError(), "ERROR: ");
    }
    {
        // Check that when a message is below the verbosity threshold, it
        // doesn't even get formatted.
        FormatLogger l;
        handler.warning(0, "%s", l);
        BOOST_CHECK(!l.wasFormatted);
        BOOST_CHECK_EQUAL(handler.lastError(), "ERROR: ");
    }
}


BOOST_AUTO_TEST_SUITE_END()

// vi: set et:
