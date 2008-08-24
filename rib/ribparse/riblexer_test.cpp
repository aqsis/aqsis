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
 * \brief Unit tests for the RIB lexer.
 * \author Chris Foster  [chris42f (at) gmail (dot) com]
 */

#include <sstream>

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include "riblexer.h"

#define ADD_ESCAPES(x) #x

#define CHECK_EOF(lex) BOOST_CHECK_EQUAL(lex.getToken(), \
		CqRibToken(CqRibToken::ENDOFFILE))

using namespace ribparse;

BOOST_AUTO_TEST_CASE(CqRibLexer_strings_test)
{
	{
		// Test multiple and adjacent strings
		std::istringstream in("\"xx\"  \"\"\"a\"");
		CqRibLexer lex(in);
		BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::STRING, "xx"));
		BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::STRING, ""));
		BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::STRING, "a"));
		CHECK_EOF(lex);
	}
	{
		// Test escape characters; these should be the same as escaping in C++.
		std::istringstream in(ADD_ESCAPES("_\n_\r_\t_\b_\f_\\_\"_\z_"));
		CqRibLexer lex(in);
		BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::STRING,
					"_\n_\r_\t_\b_\f_\\_\"_\\z_"));
		CHECK_EOF(lex);
	}
	{
		// Test octal escape characters; these should be the same as escaping
		// in C++ as well.
		std::istringstream in(ADD_ESCAPES("\101_\12_\7"));
		CqRibLexer lex(in);
		BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::STRING,
					"\101_\12_\7"));
		CHECK_EOF(lex);
	}
	{
		// Test embedded newlines and tabs
		std::istringstream in("\"xx\t\nXX\"");
		CqRibLexer lex(in);
		BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::STRING,
					"xx\t\nXX"));
		CHECK_EOF(lex);
	}
	{
		// escaped line break (should discard newline)
		std::istringstream in("\"jo\\\nined\"");
		CqRibLexer lex(in);
		BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::STRING, "joined"));
		CHECK_EOF(lex);
	}
	{
		// Test what happens when the stream ends before a string end character
		std::istringstream in("\"xx");
		CqRibLexer lex(in);
		BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::ERROR));
		CHECK_EOF(lex);
	}
}

BOOST_AUTO_TEST_CASE(CqRibLexer_int_test)
{
	std::istringstream in("42 +42 -42");
	CqRibLexer lex(in);
	TqInt ints[] = {42, +42, -42};
	for(TqInt i = 0; i < static_cast<TqInt>(sizeof(ints)/sizeof(ints[0])); ++i)
		BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(ints[i]));
	CHECK_EOF(lex);
}

BOOST_AUTO_TEST_CASE(CqRibLexer_float_test)
{
	std::istringstream in(
		"12.  .34  56.78 \
		+12.  +.34 +56.78 \
		-12.  -.34 -56.78 \
		-42e10 -12.e10 -.34e10 -56.78e10 \
		+42E10 +12.E10 +.34E10 +56.78E10 \
		42e-10 12.e-10 .34e-10 56.78e-10 \
		42E+10 12.E+10 .34E+10 56.78E+10 \
		-42E+10 -12.E+10 -.34E+10 -56.78E+10"
	);
	CqRibLexer lex(in);
	float floats[] = {
		12., .34, 56.78,
		+12., +.34, +56.78,
		-12., -.34, -56.78,
		-42e10, -12.e10, -.34e10, -56.78e10,
		+42E10, +12.E10, +.34E10, +56.78E10,
		42e-10, 12.e-10, .34e-10, 56.78e-10,
		42E+10, 12.E+10, .34E+10, 56.78E+10,
		-42E+10, -12.E+10, -.34E+10, -56.78E+10
	};
	for(TqInt i = 0; i < static_cast<TqInt>(sizeof(floats)/sizeof(floats[0])); ++i)
		BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(floats[i]));
	CHECK_EOF(lex);
}

BOOST_AUTO_TEST_CASE(CqRibLexer_comments_test)
{
	std::istringstream in("# I'm commented out\n##And a structrual comment\n");
	CqRibLexer lex(in);
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::COMMENT, 
				" I'm commented out"));
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::COMMENT, 
				"#And a structrual comment"));
	CHECK_EOF(lex);
}

BOOST_AUTO_TEST_CASE(CqRibLexer_array_test)
{
	std::istringstream in("[ 1.0 -1 ]");
	CqRibLexer lex(in);
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::BEGIN_ARRAY));
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(1.0f));
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(-1));
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::END_ARRAY));
	CHECK_EOF(lex);
}

BOOST_AUTO_TEST_CASE(CqRibLexer_request_test)
{
	std::istringstream in("WorldBegin version SomethingElse");
	CqRibLexer lex(in);
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::REQUEST, "WorldBegin"));
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::REQUEST, "version"));
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::REQUEST, "SomethingElse"));
	CHECK_EOF(lex);
}

