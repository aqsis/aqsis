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
 * \brief Unit tests for the RIB parser.
 * \author Chris Foster  [chris42f (at) gmail (dot) com]
 */

#include <aqsis/aqsis.h>

#ifndef	AQSIS_SYSTEM_WIN32
#define BOOST_TEST_DYN_LINK
#endif //AQSIS_SYSTEM_WIN32

#include <sstream>
#include <cctype>

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <boost/any.hpp>

#include "ribparser_impl.h"
#include <aqsis/util/smartptr.h>
#include <aqsis/riutil/primvartoken.h>


using namespace Aqsis;

//------------------------------------------------------------------------------
// Tests for getting data from the parser.

struct NullRibRequestHandler : public IqRibRequestHandler
{
	virtual void handleRequest(const std::string& requestName,
			IqRibParser& parser)
	{
		assert(0 && "shouldn't call NullRibRequestHandler::handleRequest()");
	}
};

struct NullRequestFixture
{
	std::istringstream in;
	NullRibRequestHandler nullHandler;
	CqRibParser parser;

	NullRequestFixture(const std::string& stringToParse)
		: in(stringToParse),
		nullHandler(),
		parser(boost::shared_ptr<NullRibRequestHandler>(&nullHandler, nullDeleter))
	{
		parser.pushInput(in, "test_stream");
	}
};

BOOST_AUTO_TEST_CASE(CqRibParser_get_scalar_tests)
{
	NullRequestFixture f("42 3.141592 3 \"some_string\" ");

	BOOST_CHECK_EQUAL(f.parser.getInt(), 42);
	BOOST_CHECK_CLOSE(f.parser.getFloat(), 3.141592f, 0.0001f);
	BOOST_CHECK_CLOSE(f.parser.getFloat(), 3.0f, 0.0001f);
	BOOST_CHECK_EQUAL(f.parser.getString(), "some_string");
}

BOOST_AUTO_TEST_CASE(CqRibParser_getIntArray_test)
{
	NullRequestFixture f("[1 2 3 4] [] [1 1.1]");

	// check that we can read int arrays
	const IqRibParser::TqIntArray& a1 = f.parser.getIntArray();
	BOOST_REQUIRE_EQUAL(a1.size(), 4U);
	BOOST_CHECK_EQUAL(a1[0], 1);
	BOOST_CHECK_EQUAL(a1[1], 2);
	BOOST_CHECK_EQUAL(a1[2], 3);
	BOOST_CHECK_EQUAL(a1[3], 4);

	// check that we can read empty arrays
	const IqRibParser::TqIntArray& a3 = f.parser.getIntArray();
	BOOST_CHECK_EQUAL(a3.size(), 0U);

	// check throw on reading a non-int array
	BOOST_CHECK_THROW(f.parser.getIntArray(), XqParseError);
}

BOOST_AUTO_TEST_CASE(CqRibParser_getFloatArray_test)
{
	NullRequestFixture f("[1 2.1 3.2 4] [-2.0e1 -4.1]  1 2.0  [1 2.0] [\"asdf\"]");

	const TqFloat eps = 0.0001;

	// check that mixed ints and floats can be read as a float array.
	const IqRibParser::TqFloatArray& a1 = f.parser.getFloatArray();
	BOOST_REQUIRE_EQUAL(a1.size(), 4U);
	BOOST_CHECK_CLOSE(a1[0], 1.0f, eps);
	BOOST_CHECK_CLOSE(a1[1], 2.1f, eps);
	BOOST_CHECK_CLOSE(a1[2], 3.2f, eps);
	BOOST_CHECK_CLOSE(a1[3], 4.0f, eps);

	// check that we can read a float array
	const IqRibParser::TqFloatArray& a2 = f.parser.getFloatArray();
	BOOST_REQUIRE_EQUAL(a2.size(), 2U);
	BOOST_CHECK_CLOSE(a2[0], -2.0e1f, eps);
	BOOST_CHECK_CLOSE(a2[1], -4.1f, eps);

	// check that we can read a float array with fixed length in either format.
	{
		const IqRibParser::TqFloatArray& a = f.parser.getFloatArray(2);
		BOOST_REQUIRE_EQUAL(a.size(), 2U);
		BOOST_CHECK_CLOSE(a[0], 1.0f, eps);
		BOOST_CHECK_CLOSE(a[1], 2.0f, eps);
	}
	{
		const IqRibParser::TqFloatArray& a = f.parser.getFloatArray(2);
		BOOST_REQUIRE_EQUAL(a.size(), 2U);
		BOOST_CHECK_CLOSE(a[0], 1.0f, eps);
		BOOST_CHECK_CLOSE(a[1], 2.0f, eps);
	}

	// check throw on reading a non-float array
	BOOST_CHECK_THROW(f.parser.getFloatArray(), XqParseError);
}

BOOST_AUTO_TEST_CASE(CqRibParser_getStringArray_test)
{
	NullRequestFixture f("[\"asdf\" \"1234\" \"!@#$\"] 123");

	// Check that we can read string arrays
	const IqRibParser::TqStringArray& a = f.parser.getStringArray();
	BOOST_REQUIRE_EQUAL(a.size(), 3U);
	BOOST_CHECK_EQUAL(a[0], "asdf");
	BOOST_CHECK_EQUAL(a[1], "1234");
	BOOST_CHECK_EQUAL(a[2], "!@#$");

	// check throw on reading a non-string array
	BOOST_CHECK_THROW(f.parser.getStringArray(), XqParseError);
}

BOOST_AUTO_TEST_CASE(CqRibParser_getIntParam_test)
{
	NullRequestFixture f("1 [2 3]");

	const IqRibParser::TqIntArray& a1 = f.parser.getIntParam();
	BOOST_REQUIRE_EQUAL(a1.size(), 1U);
	BOOST_CHECK_EQUAL(a1[0], 1);

	const IqRibParser::TqIntArray& a2 = f.parser.getIntParam();
	BOOST_REQUIRE_EQUAL(a2.size(), 2U);
	BOOST_CHECK_EQUAL(a2[0], 2);
	BOOST_CHECK_EQUAL(a2[1], 3);
}

BOOST_AUTO_TEST_CASE(CqRibParser_getFloatParam_test)
{
	NullRequestFixture f("1 2.0 [3.0 4]");

	const IqRibParser::TqFloatArray& a1 = f.parser.getFloatParam();
	BOOST_REQUIRE_EQUAL(a1.size(), 1U);
	BOOST_CHECK_CLOSE(a1[0], 1.0f, 0.00001);

	const IqRibParser::TqFloatArray& a2 = f.parser.getFloatParam();
	BOOST_REQUIRE_EQUAL(a2.size(), 1U);
	BOOST_CHECK_CLOSE(a2[0], 2.0f, 0.00001);

	const IqRibParser::TqFloatArray& a3 = f.parser.getFloatParam();
	BOOST_REQUIRE_EQUAL(a3.size(), 2U);
	BOOST_CHECK_CLOSE(a3[0], 3.0f, 0.00001);
	BOOST_CHECK_CLOSE(a3[1], 4.0f, 0.00001);
}

BOOST_AUTO_TEST_CASE(CqRibParser_getStringParam_test)
{
	NullRequestFixture f("\"aa\" [\"bb\" \"cc\"]");

	const IqRibParser::TqStringArray& a1 = f.parser.getStringParam();
	BOOST_REQUIRE_EQUAL(a1.size(), 1U);
	BOOST_CHECK_EQUAL(a1[0], "aa");

	const IqRibParser::TqStringArray& a2 = f.parser.getStringParam();
	BOOST_REQUIRE_EQUAL(a2.size(), 2U);
	BOOST_CHECK_EQUAL(a2[0], "bb");
	BOOST_CHECK_EQUAL(a2[1], "cc");
}


// Dummy param list which just accumulates all parameters into a list of
// (token, value) pairs.
struct DummyParamListHandler : IqRibParamListHandler
{
	std::vector<std::pair<std::string, boost::any> > tokValPairs;
	virtual void readParameter(const std::string& name, IqRibParser& parser)
	{
		CqPrimvarToken token(name.c_str());
		switch(token.storageType())
		{
			case type_integer:
				tokValPairs.push_back(std::pair<std::string, boost::any>(name,
							parser.getIntParam()));
				break;
			case type_float:
				tokValPairs.push_back(std::pair<std::string, boost::any>(name,
							parser.getFloatParam()));
				break;
			case type_string:
				tokValPairs.push_back(std::pair<std::string, boost::any>(name,
							parser.getStringParam()));
				break;
			default:
				assert(0 && "type not recognized.");
				break;
		}
	}
};

BOOST_AUTO_TEST_CASE(CqRibParser_getParamList_test)
{
	NullRequestFixture f("\"uniform vector P\" [1 2 3] \"constant integer a\" 42\n"
			              "\"constant string texname\" \"somefile.map\"");

	// Grab the parameter list from the parser.
	DummyParamListHandler pList;
	f.parser.getParamList(pList);
	BOOST_REQUIRE_EQUAL(pList.tokValPairs.size(), 3U);

	// Check first parameter
	BOOST_CHECK_EQUAL(pList.tokValPairs[0].first, "uniform vector P");
	const IqRibParser::TqFloatArray& P =
		boost::any_cast<const IqRibParser::TqFloatArray&>(pList.tokValPairs[0].second);
	BOOST_REQUIRE_EQUAL(P.size(), 3U);
	BOOST_CHECK_CLOSE(P[0], 1.0f, 0.00001);
	BOOST_CHECK_CLOSE(P[1], 2.0f, 0.00001);
	BOOST_CHECK_CLOSE(P[2], 3.0f, 0.00001);
	// Check second parameter
	BOOST_CHECK_EQUAL(pList.tokValPairs[1].first, "constant integer a");
	const IqRibParser::TqIntArray& a =
		boost::any_cast<const IqRibParser::TqIntArray&>(pList.tokValPairs[1].second);
	BOOST_REQUIRE_EQUAL(a.size(), 1U);
	BOOST_CHECK_EQUAL(a[0], 42);
	// Check third parameter
	BOOST_CHECK_EQUAL(pList.tokValPairs[2].first, "constant string texname");
	const IqRibParser::TqStringArray& texname =
		boost::any_cast<const IqRibParser::TqStringArray&>(pList.tokValPairs[2].second);
	BOOST_CHECK_EQUAL(texname[0], "somefile.map");
}

//------------------------------------------------------------------------------
// Request handler invocation tests.

// Test the rib parser with a simple handler which takes a string and two array
// arguments.
struct MockRequestHandler : public IqRibRequestHandler
{
	std::string name;
	std::string s;
	TqInt i;
	IqRibParser::TqFloatArray a;

	virtual void handleRequest(const std::string& requestName, IqRibParser& parser)
	{
		name = requestName;
		if(name == "SomeRequest")
		{
			s = parser.getString();
			i = parser.getInt();
			a = parser.getFloatArray();
		}
		else if(name == "SimpleRequest")
		{
			i = parser.getInt();
		}
		else
		{
			AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax, "unknown request " << name);
		}
	}
};

struct MockHandlerFixture
{
	std::istringstream in;
	MockRequestHandler handler;
	CqRibParser parser;

	MockHandlerFixture(const char* str)
		: in(str),
		handler(),
		parser(boost::shared_ptr<MockRequestHandler>(&handler, nullDeleter))
	{
		parser.pushInput(in, "test_stream");
	}
};

BOOST_AUTO_TEST_CASE(CqRibParser_simple_req_test)
{
	MockHandlerFixture f("SomeRequest \"blah\" #removed_comment\n 42 [1.1 1.2]\n");

	BOOST_CHECK_EQUAL(f.parser.parseNextRequest(), true);

	BOOST_CHECK_EQUAL(f.handler.name, "SomeRequest");
	BOOST_CHECK_EQUAL(f.handler.s, "blah");
	BOOST_CHECK_EQUAL(f.handler.i, 42);
	BOOST_REQUIRE_EQUAL(f.handler.a.size(), 2U);
	BOOST_CHECK_CLOSE(f.handler.a[0], 1.1f, 0.0001f);
	BOOST_CHECK_CLOSE(f.handler.a[1], 1.2f, 0.0001f);

	BOOST_CHECK_EQUAL(f.parser.parseNextRequest(), false);
}


BOOST_AUTO_TEST_CASE(CqRibParser_error_recovery_immediate_request)
{
	// Test recovery after an invalid request.  According to the RISpec, the
	// parser should recover by discarding tokens until the next request name
	// is read.
	MockHandlerFixture f("SomeInvalidRequest SimpleRequest 42\n");

	BOOST_CHECK_THROW(f.parser.parseNextRequest(), XqParseError);

	BOOST_CHECK_EQUAL(f.parser.parseNextRequest(), true);
	BOOST_CHECK_EQUAL(f.handler.name, "SimpleRequest");
	BOOST_CHECK_EQUAL(f.handler.i, 42);

	BOOST_CHECK_EQUAL(f.parser.parseNextRequest(), false);
}

BOOST_AUTO_TEST_CASE(CqRibParser_error_recovery_extra_tokens)
{
	// Test throw on extra tokens occurring after a request and before the
	// start of the next request.
	MockHandlerFixture f(
		"SimpleRequest -1 \"some junk\" \"tokens\" SimpleRequest 42\n"
	);

	BOOST_CHECK_EQUAL(f.parser.parseNextRequest(), true);
	BOOST_CHECK_EQUAL(f.handler.name, "SimpleRequest");
	BOOST_CHECK_EQUAL(f.handler.i, -1);

	BOOST_CHECK_THROW(f.parser.parseNextRequest(), XqParseError);

	BOOST_CHECK_EQUAL(f.parser.parseNextRequest(), true);
	BOOST_CHECK_EQUAL(f.handler.name, "SimpleRequest");
	BOOST_CHECK_EQUAL(f.handler.i, 42);

	BOOST_CHECK_EQUAL(f.parser.parseNextRequest(), false);
}

BOOST_AUTO_TEST_CASE(CqRibParser_error_recovery_initial_junk)
{
	// Test throw on initial junk tokens.
	MockHandlerFixture f(
		" \"some junk\" \"tokens\" SimpleRequest 42\n"
	);

	BOOST_CHECK_THROW(f.parser.parseNextRequest(), XqParseError);

	BOOST_CHECK_EQUAL(f.parser.parseNextRequest(), true);
	BOOST_CHECK_EQUAL(f.handler.name, "SimpleRequest");
	BOOST_CHECK_EQUAL(f.handler.i, 42);

	BOOST_CHECK_EQUAL(f.parser.parseNextRequest(), false);
}

BOOST_AUTO_TEST_CASE(CqRibParser_error_message_position)
{
	MockHandlerFixture f(
		"SimpleRequest \"some junk\""
	);

	bool didThrow = false;
	try
	{
		f.parser.parseNextRequest();
	}
	catch(XqParseError& e)
	{
		didThrow = true;
		std::string msg = e.what();
		std::string::size_type pos = msg.find("(col ");
		std::string::iterator p = msg.begin() + pos + 5;
		// strip out the column number from the string
		TqInt colNum = 0;
		while(std::isdigit(*p))
		{
			colNum = 10*colNum + (*p - '0');
			++p;
		}
		BOOST_CHECK_EQUAL(colNum, 15);
	}
	BOOST_CHECK(didThrow);
}

BOOST_AUTO_TEST_CASE(CqRibParser_EOF_immediate_stop)
{
	// Here we try to check that the RIB parser will stop immediately on
	// reading the EOF character \377.  If it continues past this, the lexer
	// will read the \321 character which is an illegal reserved byte and
	// should throw an error.
	MockHandlerFixture f(
		"SimpleRequest 42\377\321"
	);

	BOOST_CHECK(f.parser.parseNextRequest());
	BOOST_CHECK(!f.parser.parseNextRequest());
	// ^^ At this point, we would normally stop reading the stream.

	// Check that the next read fails with an exception.
	BOOST_CHECK_THROW(f.parser.parseNextRequest(), XqParseError);
}

BOOST_AUTO_TEST_CASE(CqRibParser_EOF_immediate_stop_error)
{
	// Check that the RIB parser will stop reading immediately if a request is
	// terminated incorrectly with an EOF.
	MockHandlerFixture f(
		"SimpleRequest \377\321"
	);

	BOOST_CHECK_THROW(f.parser.parseNextRequest(), XqParseError);
	BOOST_CHECK(!f.parser.parseNextRequest());
	// ^^ At this point, we would normally stop reading the stream.

	// Check that the next read fails from reading the reserved byte
	BOOST_CHECK_THROW(f.parser.parseNextRequest(), XqParseError);
}
