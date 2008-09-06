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

#include <sstream>

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include "ribparser.h"
#include "smartptr.h"

using namespace Aqsis;

//------------------------------------------------------------------------------
// Tests for getting data from the parser.

BOOST_AUTO_TEST_CASE(CqRibParser_get_scalar_tests)
{
	std::istringstream in("42 3.141592 \"some_string\" ");
	CqRibLexer lex(in);
	CqRequestMap map;
	CqRibParser parser( boost::shared_ptr<CqRibLexer>(&lex, nullDeleter),
			boost::shared_ptr<CqRequestMap>(&map, nullDeleter));

	BOOST_CHECK_EQUAL(parser.getInt(), 42);
	BOOST_CHECK_CLOSE(parser.getFloat(), 3.141592f, 0.0001f);
	BOOST_CHECK_EQUAL(parser.getString(), "some_string");
}

BOOST_AUTO_TEST_CASE(CqRibParser_getIntArray_test)
{
	std::istringstream in("[1 2 3 4] [] [1 1.1]");
	CqRibLexer lex(in);
	CqRequestMap map;
	CqRibParser parser( boost::shared_ptr<CqRibLexer>(&lex, nullDeleter),
			boost::shared_ptr<CqRequestMap>(&map, nullDeleter));

	// check that we can read int arrays
	const TqRiIntArray& a1 = parser.getIntArray();
	BOOST_REQUIRE_EQUAL(a1.size(), 4U);
	BOOST_CHECK_EQUAL(a1[0], 1);
	BOOST_CHECK_EQUAL(a1[1], 2);
	BOOST_CHECK_EQUAL(a1[2], 3);
	BOOST_CHECK_EQUAL(a1[3], 4);

	// check that we can read empty arrays
	const TqRiIntArray& a3 = parser.getIntArray();
	BOOST_CHECK_EQUAL(a3.size(), 0U);

	// check throw on reading a non-int array
	BOOST_CHECK_THROW(parser.getIntArray(), XqParseError);
}

BOOST_AUTO_TEST_CASE(CqRibParser_getFloatArray_test)
{
	std::istringstream in("[1 2.1 3.2 4] [-2.0e1 -4.1] [\"asdf\"]");
	CqRibLexer lex(in);
	CqRequestMap map;
	CqRibParser parser( boost::shared_ptr<CqRibLexer>(&lex, nullDeleter),
			boost::shared_ptr<CqRequestMap>(&map, nullDeleter));

	const TqFloat eps = 0.0001;

	// check that mixed ints and floats can be read as a float array.
	const TqRiFloatArray& a1 = parser.getFloatArray();
	BOOST_REQUIRE_EQUAL(a1.size(), 4U);
	BOOST_CHECK_CLOSE(a1[0], 1.0f, eps);
	BOOST_CHECK_CLOSE(a1[1], 2.1f, eps);
	BOOST_CHECK_CLOSE(a1[2], 3.2f, eps);
	BOOST_CHECK_CLOSE(a1[3], 4.0f, eps);

	// check that we can read a float array
	const TqRiFloatArray& a2 = parser.getFloatArray();
	BOOST_REQUIRE_EQUAL(a2.size(), 2U);
	BOOST_CHECK_CLOSE(a2[0], -2.0e1f, eps);
	BOOST_CHECK_CLOSE(a2[1], -4.1f, eps);

	// check throw on reading a non-float array
	BOOST_CHECK_THROW(parser.getFloatArray(), XqParseError);
}

BOOST_AUTO_TEST_CASE(CqRibParser_getStringArray_test)
{
	std::istringstream in("[\"asdf\" \"1234\" \"!@#$\"] 123");
	CqRibLexer lex(in);
	CqRequestMap map;
	CqRibParser parser( boost::shared_ptr<CqRibLexer>(&lex, nullDeleter),
			boost::shared_ptr<CqRequestMap>(&map, nullDeleter));

	// Check that we can read string arrays
	const TqRiStringArray& a = parser.getStringArray();
	BOOST_REQUIRE_EQUAL(a.size(), 3U);
	BOOST_CHECK_EQUAL(a[0], "asdf");
	BOOST_CHECK_EQUAL(a[1], "1234");
	BOOST_CHECK_EQUAL(a[2], "!@#$");

	// check throw on reading a non-string array
	BOOST_CHECK_THROW(parser.getStringArray(), XqParseError);
}

BOOST_AUTO_TEST_CASE(CqRibParser_getParamList_test)
{
	std::istringstream in("\"uniform vector P\" [1 2 3] \"constant integer a\" 42");
	CqRibLexer lex(in);
	CqRequestMap map;
	CqRibParser parser(boost::shared_ptr<CqRibLexer>(&lex, nullDeleter),
			boost::shared_ptr<CqRequestMap>(&map, nullDeleter));

	const TqRiParamList pList = parser.getParamList();
	BOOST_REQUIRE_EQUAL(pList.size(), 2U);
	// Check first parameter
	BOOST_CHECK_EQUAL(pList[0].token().name(), "P");
	const TqRiFloatArray& P = pList[0].value<ParamType_FloatArray>();
	BOOST_REQUIRE_EQUAL(P.size(), 3U);
	BOOST_CHECK_CLOSE(P[0], 1.0f, 0.00001);
	BOOST_CHECK_CLOSE(P[1], 2.0f, 0.00001);
	BOOST_CHECK_CLOSE(P[2], 3.0f, 0.00001);
	// Check second parameter
	BOOST_CHECK_EQUAL(pList[1].token().name(), "a");
	BOOST_CHECK_EQUAL(pList[1].value<ParamType_Int>(), 42);
}

//------------------------------------------------------------------------------
// Request handler invocation tests.

// Test the rib parser with 
struct TestRequestHandler : public IqRibRequest
{
	std::string s;
	TqRiIntArray a1;
	TqRiFloatArray a2;

	TestRequestHandler(const std::string& name)
		: IqRibRequest(name)
	{ }

	virtual void handleRequest(CqRibParser& parser)
	{
		s = parser.getString();
		a1 = parser.getIntArray();
		a2 = parser.getFloatArray();
	}
};

BOOST_AUTO_TEST_CASE(CqRibParser_simple_req_test)
{
	std::istringstream in("SomeRequest \"blah\" [1 2] [1.1 1.2]\n");
	CqRibLexer lex(in);
	CqRequestMap map;
	TestRequestHandler* rqst = new TestRequestHandler("SomeRequest");
	map.add(rqst);
	CqRibParser parser( boost::shared_ptr<CqRibLexer>(&lex, nullDeleter),
			boost::shared_ptr<CqRequestMap>(&map, nullDeleter));

	BOOST_CHECK_EQUAL(parser.parseNextRequest(), true);
	BOOST_CHECK_EQUAL(rqst->s, "blah");
	BOOST_REQUIRE_EQUAL(rqst->a1.size(), 2U);
	BOOST_CHECK_EQUAL(rqst->a1[0], 1);
	BOOST_CHECK_EQUAL(rqst->a1[1], 2);
	BOOST_REQUIRE_EQUAL(rqst->a2.size(), 2U);
	BOOST_CHECK_CLOSE(rqst->a2[0], 1.1f, 0.0001f);
	BOOST_CHECK_CLOSE(rqst->a2[1], 1.2f, 0.0001f);
}

