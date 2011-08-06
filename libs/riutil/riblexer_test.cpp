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

/** \file
 * \brief Unit tests for the RIB lexer.
 * \author Chris Foster  [chris42f (at) gmail (dot) com]
 */

#include <aqsis/aqsis.h>

#define BOOST_TEST_DYN_LINK

#include <sstream>
#include <cctype>

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include "riblexer_impl.h"
#include <aqsis/util/exception.h>

using namespace Aqsis;

//------------------------------------------------------------------------------
// Tests for getting data from the lexer.

struct Fixture
{
    std::istringstream in;
    RibLexerImpl lex;

    Fixture(const std::string& stringToParse)
        : in(stringToParse),
        lex()
    {
        lex.pushInput(in, "test_stream");
    }
};

BOOST_AUTO_TEST_SUITE(rib_lexer_tests)

BOOST_AUTO_TEST_CASE(RibLexerImpl_get_scalar_tests)
{
    Fixture f("42 3.141592 3 \"some_string\" ");

    BOOST_CHECK_EQUAL(f.lex.getInt(), 42);
    BOOST_CHECK_CLOSE(f.lex.getFloat(), 3.141592f, 0.0001f);
    BOOST_CHECK_CLOSE(f.lex.getFloat(), 3.0f, 0.0001f);
    BOOST_CHECK_EQUAL(f.lex.getString(), "some_string");
}

BOOST_AUTO_TEST_CASE(RibLexerImpl_getIntArray_test)
{
    Fixture f("[1 2 3 4] [] [1 1.1]");

    // check that we can read int arrays
    RibLexer::IntArray a1 = f.lex.getIntArray();
    BOOST_REQUIRE_EQUAL(a1.size(), 4U);
    BOOST_CHECK_EQUAL(a1[0], 1);
    BOOST_CHECK_EQUAL(a1[1], 2);
    BOOST_CHECK_EQUAL(a1[2], 3);
    BOOST_CHECK_EQUAL(a1[3], 4);

    // check that we can read empty arrays
    RibLexer::IntArray a3 = f.lex.getIntArray();
    BOOST_CHECK_EQUAL(a3.size(), 0U);

    // check throw on reading a non-int array
    BOOST_CHECK_THROW(f.lex.getIntArray(), XqParseError);
}

BOOST_AUTO_TEST_CASE(RibLexerImpl_getFloatArray_test)
{
    Fixture f("[1 2.1 3.2 4] [-2.0e1 -4.1]  1 2.0  [1 2.0] [\"asdf\"]");

    const float eps = 0.0001;

    // check that mixed ints and floats can be read as a float array.
    RibLexer::FloatArray a1 = f.lex.getFloatArray();
    BOOST_REQUIRE_EQUAL(a1.size(), 4U);
    BOOST_CHECK_CLOSE(a1[0], 1.0f, eps);
    BOOST_CHECK_CLOSE(a1[1], 2.1f, eps);
    BOOST_CHECK_CLOSE(a1[2], 3.2f, eps);
    BOOST_CHECK_CLOSE(a1[3], 4.0f, eps);

    // check that we can read a float array
    RibLexer::FloatArray a2 = f.lex.getFloatArray();
    BOOST_REQUIRE_EQUAL(a2.size(), 2U);
    BOOST_CHECK_CLOSE(a2[0], -2.0e1f, eps);
    BOOST_CHECK_CLOSE(a2[1], -4.1f, eps);

    // check that we can read a float array with fixed length in either format.
    {
        RibLexer::FloatArray a = f.lex.getFloatArray(2);
        BOOST_REQUIRE_EQUAL(a.size(), 2U);
        BOOST_CHECK_CLOSE(a[0], 1.0f, eps);
        BOOST_CHECK_CLOSE(a[1], 2.0f, eps);
    }
    {
        RibLexer::FloatArray a = f.lex.getFloatArray(2);
        BOOST_REQUIRE_EQUAL(a.size(), 2U);
        BOOST_CHECK_CLOSE(a[0], 1.0f, eps);
        BOOST_CHECK_CLOSE(a[1], 2.0f, eps);
    }

    // check throw on reading a non-float array
    BOOST_CHECK_THROW(f.lex.getFloatArray(), XqParseError);
}

BOOST_AUTO_TEST_CASE(RibLexerImpl_getStringArray_test)
{
    Fixture f("[\"asdf\" \"1234\" \"!@#$\"] 123");

    // Check that we can read string arrays
    RibLexer::StringArray a = f.lex.getStringArray();
    BOOST_REQUIRE_EQUAL(a.size(), 3U);
    BOOST_CHECK_EQUAL(a[0], "asdf");
    BOOST_CHECK_EQUAL(a[1], "1234");
    BOOST_CHECK_EQUAL(a[2], "!@#$");

    // check throw on reading a non-string array
    BOOST_CHECK_THROW(f.lex.getStringArray(), XqParseError);
}

BOOST_AUTO_TEST_CASE(RibLexerImpl_getIntParam_test)
{
    Fixture f("1 [2 3]");

    RibLexer::IntArray a1 = f.lex.getIntParam();
    BOOST_REQUIRE_EQUAL(a1.size(), 1U);
    BOOST_CHECK_EQUAL(a1[0], 1);

    RibLexer::IntArray a2 = f.lex.getIntParam();
    BOOST_REQUIRE_EQUAL(a2.size(), 2U);
    BOOST_CHECK_EQUAL(a2[0], 2);
    BOOST_CHECK_EQUAL(a2[1], 3);
}

BOOST_AUTO_TEST_CASE(RibLexerImpl_getFloatParam_test)
{
    Fixture f("1 2.0 [3.0 4]");

    RibLexer::FloatArray a1 = f.lex.getFloatParam();
    BOOST_REQUIRE_EQUAL(a1.size(), 1U);
    BOOST_CHECK_CLOSE(a1[0], 1.0f, 0.00001);

    RibLexer::FloatArray a2 = f.lex.getFloatParam();
    BOOST_REQUIRE_EQUAL(a2.size(), 1U);
    BOOST_CHECK_CLOSE(a2[0], 2.0f, 0.00001);

    RibLexer::FloatArray a3 = f.lex.getFloatParam();
    BOOST_REQUIRE_EQUAL(a3.size(), 2U);
    BOOST_CHECK_CLOSE(a3[0], 3.0f, 0.00001);
    BOOST_CHECK_CLOSE(a3[1], 4.0f, 0.00001);
}

BOOST_AUTO_TEST_CASE(RibLexerImpl_getStringParam_test)
{
    Fixture f("\"aa\" [\"bb\" \"cc\"]");

    RibLexer::StringArray a1 = f.lex.getStringParam();
    BOOST_REQUIRE_EQUAL(a1.size(), 1U);
    BOOST_CHECK_EQUAL(a1[0], "aa");

    RibLexer::StringArray a2 = f.lex.getStringParam();
    BOOST_REQUIRE_EQUAL(a2.size(), 2U);
    BOOST_CHECK_EQUAL(a2[0], "bb");
    BOOST_CHECK_EQUAL(a2[1], "cc");
}

BOOST_AUTO_TEST_CASE(RibLexerImpl_nextRequest_test)
{
    Fixture f("SomeRequest AnotherRequest");
    BOOST_CHECK_EQUAL(f.lex.nextRequest(), "SomeRequest");
    BOOST_CHECK_EQUAL(f.lex.nextRequest(), "AnotherRequest");
    BOOST_CHECK(!f.lex.nextRequest());
}

BOOST_AUTO_TEST_CASE(RibLexerImpl_error_recovery)
{
    Fixture f("\"oops a string\" 1 2 3 10.0 SomeRequest");

    BOOST_CHECK_THROW(f.lex.nextRequest(), XqParseError);
    // check that the error position correctly reflects the bad token
    BOOST_CHECK_EQUAL(f.lex.streamPos(), "test_stream:1 (col 1)");
    f.lex.discardUntilRequest();
    BOOST_CHECK_EQUAL(f.lex.nextRequest(), "SomeRequest");
}

BOOST_AUTO_TEST_CASE(RibLexerImpl_EOF_immediate_stop)
{
    // Here we try to check that the RIB parser will stop immediately on
    // reading the EOF character \377.  If it continues past this, the lexer
    // will read the \321 character which is an illegal reserved byte and
    // should throw an error.
    Fixture f("SimpleRequest 42\377\321");

    BOOST_CHECK_EQUAL(f.lex.nextRequest(), "SimpleRequest");
    BOOST_CHECK_EQUAL(f.lex.getInt(), 42);
    BOOST_CHECK(!f.lex.nextRequest());
    // ^^ At this point, we would normally stop reading the stream.

    // Check that the next read fails with an exception.
    BOOST_CHECK_THROW(f.lex.nextRequest(), XqParseError);
}

BOOST_AUTO_TEST_SUITE_END()

// vi: set et:
