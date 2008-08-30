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

#include "riblexer.h"


#define ADD_ESCAPES(x) #x

#define CHECK_EOF(lex) BOOST_CHECK_EQUAL(lex.getToken(), \
		CqRibToken(CqRibToken::ENDOFFILE))

using namespace Aqsis;

//------------------------------------------------------------------------------
// Test cases for ASCII RIB parsing
//------------------------------------------------------------------------------

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
					"_\n_\r_\t_\b_\f_\\_\"_z_"));
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

BOOST_AUTO_TEST_CASE(CqRibLexer_array_test)
{
	std::istringstream in("[ 1.0 -1 ]");
	CqRibLexer lex(in);
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::ARRAY_BEGIN));
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(1.0f));
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(-1));
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::ARRAY_END));
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


//------------------------------------------------------------------------------
// Test cases for binary RIB decoding
//------------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(CqRibLexer_integer_decode)
{
	std::istringstream in("\200a\201ab\202abc\203abcd");
	CqRibLexer lex(in);
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(TqInt('a')));
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken((TqInt('a') << 8) + TqInt('b')) );
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(
			(TqInt('a') << 16) + (TqInt('b') << 8) + TqInt('c')) );
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(
			(TqInt('a') << 24) + (TqInt('b') << 16) + (TqInt('c') << 8) + TqInt('d')) );
	CHECK_EOF(lex);
}

BOOST_AUTO_TEST_CASE(CqRibLexer_fixedpoint_decode)
{
	// Only a selection of the possibilites for fixed-point numbers are tested
	// here...
	std::istringstream in("\204a\205bc\212def\213ghij\214k");
	CqRibLexer lex(in);
	// 0204:  .b
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(TqFloat('a')/256));
	// 0205:  b.b
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken('b' + TqFloat('c')/256));
	// 0212:  b.bb
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken('d' + ('e' + TqFloat('f')/256)/256));
	// 0213:  bb.bb
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(
				(TqInt('g') << 8) + 'h' + (TqFloat('i') + TqFloat('j')/256)/256));
	// 0214:  .__b
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(TqFloat('k')/256/256/256));
	CHECK_EOF(lex);
}

BOOST_AUTO_TEST_CASE(CqRibLexer_short_string_decode)
{
	std::istringstream in("\220\221a\230bcdefghi\237jklm\\nopqrstuvw");
	CqRibLexer lex(in);
	// min. length short string.
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::STRING, ""));
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::STRING, "a"));
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::STRING, "bcdefghi"));
	// Test max. length short string and lack of escaping in encoded strings.
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::STRING, "jklm\\nopqrstuvw"));
	CHECK_EOF(lex);
}

// Do a funny dance with char* for string literals which contains null
// "terminators" which we don't want to actually terminate the string.
#define STRING_FROM_CHAR_ARRAY(stringName, array) \
const char aq_strPtr[] = array; std::string stringName(aq_strPtr, aq_strPtr + sizeof(aq_strPtr)-1)

BOOST_AUTO_TEST_CASE(CqRibLexer_long_string_decode)
{
	STRING_FROM_CHAR_ARRAY(str, "\240\000\240\012aaaaAaaaaA\243\000\000\000\002aX");
	std::istringstream in(str);
	CqRibLexer lex(in);
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::STRING, ""));
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::STRING, "aaaaAaaaaA"));
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::STRING, "aX"));
	CHECK_EOF(lex);
}

BOOST_AUTO_TEST_CASE(CqRibLexer_float_decode)
{
	{
		// 32-bit float tests
		STRING_FROM_CHAR_ARRAY(str, "\244\277\200\000\000a"
				"\244\100\000\000\000b\244\077\201\200\100");
		std::istringstream in(str);
		CqRibLexer lex(in);
		BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(-1.0f));
		BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::REQUEST, "a"));
		BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(2.0f));
		BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::REQUEST, "b"));
		BOOST_CHECK_EQUAL(lex.getToken(),
				CqRibToken(1.0f + 1.0f/(1<<7) + 1.0f/(1<<8) + 1.0f/(1<<17)));
		CHECK_EOF(lex);
	}
	{
		// 64-bit float tests
		STRING_FROM_CHAR_ARRAY(str, "\245\277\360\000\000\000\000\000\000"
				"a"
				"\245\100\004\000\000\000\000\000\000");
		std::istringstream in(str);
		CqRibLexer lex(in);
		BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(-1.0f));
		BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::REQUEST, "a"));
		BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(2.5f));
		CHECK_EOF(lex);
	}
	{
		// 32-bit float array tests
		STRING_FROM_CHAR_ARRAY(str, "\310\002\277\200\000\000\100\000\000\000a");
		std::istringstream in(str);
		CqRibLexer lex(in);
		BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::ARRAY_BEGIN));
		BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(-1.0f));
		BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(2.0f));
		BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::ARRAY_END));
		BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::REQUEST, "a"));
		CHECK_EOF(lex);
	}
}

BOOST_AUTO_TEST_CASE(CqRibLexer_defined_request_test)
{
	STRING_FROM_CHAR_ARRAY(str,
			"\314\000\240\021DefinedRequest000" // define request at code 0.
			"\314\377\"DefinedRequest377\"" // define request at code 0377.
			"\246\377"                  // reference request 0377
			"\246\000"                  // reference request 0
			);
	std::istringstream in(str);
	CqRibLexer lex(in);
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::REQUEST, "DefinedRequest377"));
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::REQUEST, "DefinedRequest000"));
	CHECK_EOF(lex);
}

BOOST_AUTO_TEST_CASE(CqRibLexer_defined_string_test)
{
	STRING_FROM_CHAR_ARRAY(str,
			"\315\000\"DefinedString000\"" // define string at code 0.
			"\316\100\100\"DefinedString100100\"" // define string at code 0100100.
			"\320\100\100"                 // reference string at 0100100
			"\317\000"                     // reference string at 0
			);
	std::istringstream in(str);
	CqRibLexer lex(in);
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::STRING, "DefinedString100100"));
	BOOST_CHECK_EQUAL(lex.getToken(), CqRibToken(CqRibToken::STRING, "DefinedString000"));
	CHECK_EOF(lex);
}

//------------------------------------------------------------------------------
// Test misc. public methods
//------------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(CqRibLexer_position_test)
{
	std::istringstream in("Rqst [1.0\n 1] \"asdf\" ");
	CqRibLexer lex(in);
	lex.getToken();
	lex.getToken();
	lex.getToken();
	SqSourcePos pos = lex.pos();
	BOOST_CHECK_EQUAL(pos.line, 1);
	BOOST_CHECK_EQUAL(pos.col, 7);
	lex.getToken();
	lex.getToken();
	lex.getToken();
	pos = lex.pos();
	BOOST_CHECK_EQUAL(pos.line, 2);
	BOOST_CHECK_EQUAL(pos.col, 4);
}

