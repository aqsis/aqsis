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

#include "riblexer.h"

#ifndef	AQSIS_SYSTEM_WIN32
#define BOOST_TEST_DYN_LINK
#endif //AQSIS_SYSTEM_WIN32

#include <boost/test/auto_unit_test.hpp>

#define ADD_ESCAPES(x) #x

#define CHECK_EOF(lex) BOOST_CHECK_EQUAL(lex.get(), \
		CqRibToken(CqRibToken::ENDOFFILE))

using namespace Aqsis;


struct LexerFixture
{
	std::istringstream input;
	CqRibLexer lex;

	template<typename StrT> LexerFixture(const StrT& str)
		: input(str),
		lex()
	{
		lex.pushInput(input, "test_stream");
	}
};


//------------------------------------------------------------------------------
// Test cases for ASCII RIB parsing
//------------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(CqRibLexer_strings_test)
{
	{
		// Test multiple and adjacent strings
		LexerFixture f("\"xx\"  \"\"\"a\"");
		BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::STRING, "xx"));
		BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::STRING, ""));
		BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::STRING, "a"));
		CHECK_EOF(f.lex);
	}
	{
		// Test escape characters; these should be the same as escaping in C++.
		LexerFixture f(ADD_ESCAPES("_\n_\r_\t_\b_\f_\\_\"_\z_"));
		BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::STRING,
					"_\n_\r_\t_\b_\f_\\_\"_z_"));
		CHECK_EOF(f.lex);
	}
	{
		// Test octal escape characters; these should be the same as escaping
		// in C++ as well.
		LexerFixture f(ADD_ESCAPES("\101_\12_\7"));
		BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::STRING,
					"\101_\12_\7"));
		CHECK_EOF(f.lex);
	}
	{
		// Test embedded newlines and tabs
		LexerFixture f("\"xx\t\nXX\"  \"a\r\nb\rc\nd\"");
		BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::STRING,
					"xx\t\nXX"));
		BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::STRING,
					"a\nb\nc\nd"));
		CHECK_EOF(f.lex);
	}
	{
		// escaped line break (should discard newline)
		LexerFixture f("\"jo\\\nined\"");
		BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::STRING, "joined"));
		CHECK_EOF(f.lex);
	}
	{
		// Test what happens when the stream ends before a string end character
		LexerFixture f("\"xx");
		BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::ERROR));
		CHECK_EOF(f.lex);
	}
}

BOOST_AUTO_TEST_CASE(CqRibLexer_int_test)
{
	LexerFixture f("42 +42 -42");
	TqInt ints[] = {42, +42, -42};
	for(TqInt i = 0; i < static_cast<TqInt>(sizeof(ints)/sizeof(ints[0])); ++i)
		BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(ints[i]));
	CHECK_EOF(f.lex);
}

BOOST_AUTO_TEST_CASE(CqRibLexer_float_test)
{
	LexerFixture f(
		"12.  .34  56.78 \
		+12.  +.34 +56.78 \
		-12.  -.34 -56.78 \
		-42e10 -12.e10 -.34e10 -56.78e10 \
		+42E10 +12.E10 +.34E10 +56.78E10 \
		42e-10 12.e-10 .34e-10 56.78e-10 \
		42E+10 12.E+10 .34E+10 56.78E+10 \
		-42E+10 -12.E+10 -.34E+10 -56.78E+10"
	);
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
		BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(floats[i]));
	CHECK_EOF(f.lex);
}

BOOST_AUTO_TEST_CASE(CqRibLexer_array_test)
{
	LexerFixture f("[ 1.0 -1 ]");
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::ARRAY_BEGIN));
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(1.0f));
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(-1));
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::ARRAY_END));
	CHECK_EOF(f.lex);
}

BOOST_AUTO_TEST_CASE(CqRibLexer_request_test)
{
	LexerFixture f("WorldBegin version SomethingElse");
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::REQUEST, "WorldBegin"));
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::REQUEST, "version"));
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::REQUEST, "SomethingElse"));
	CHECK_EOF(f.lex);
}


//------------------------------------------------------------------------------
// Test cases for binary RIB decoding
//------------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(CqRibLexer_integer_decode)
{
	LexerFixture f("\200a\201ab\202abc\203abcd");
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(TqInt('a')));
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken((TqInt('a') << 8) + TqInt('b')) );
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(
			(TqInt('a') << 16) + (TqInt('b') << 8) + TqInt('c')) );
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(
			(TqInt('a') << 24) + (TqInt('b') << 16) + (TqInt('c') << 8) + TqInt('d')) );
	CHECK_EOF(f.lex);
}

BOOST_AUTO_TEST_CASE(CqRibLexer_fixedpoint_decode)
{
	// Only a selection of the possibilites for fixed-point numbers are tested
	// here...
	LexerFixture f("\204a\205bc\212def\213ghij\214k");
	// 0204:  .b
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(TqFloat('a')/256));
	// 0205:  b.b
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken('b' + TqFloat('c')/256));
	// 0212:  b.bb
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken('d' + ('e' + TqFloat('f')/256)/256));
	// 0213:  bb.bb
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(
				(TqInt('g') << 8) + 'h' + (TqFloat('i') + TqFloat('j')/256)/256));
	// 0214:  .__b
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(TqFloat('k')/256/256/256));
	CHECK_EOF(f.lex);
}

BOOST_AUTO_TEST_CASE(CqRibLexer_short_string_decode)
{
	LexerFixture f("\220\221a\230bcdefghi\237jklm\\nopqrstuvw");
	// min. length short string.
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::STRING, ""));
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::STRING, "a"));
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::STRING, "bcdefghi"));
	// Test max. length short string and lack of escaping in encoded strings.
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::STRING, "jklm\\nopqrstuvw"));
	CHECK_EOF(f.lex);
}

// Do a funny dance with char* for string literals which contains null
// "terminators" which we don't want to actually terminate the string.
#define STRING_FROM_CHAR_ARRAY(stringName, array) \
const char aq_strPtr[] = array; std::string stringName(aq_strPtr, aq_strPtr + sizeof(aq_strPtr)-1)

BOOST_AUTO_TEST_CASE(CqRibLexer_long_string_decode)
{
	STRING_FROM_CHAR_ARRAY(str, "\240\000\240\012aaaaAaaaaA\243\000\000\000\002aX");
	LexerFixture f(str);
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::STRING, ""));
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::STRING, "aaaaAaaaaA"));
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::STRING, "aX"));
	CHECK_EOF(f.lex);
}

BOOST_AUTO_TEST_CASE(CqRibLexer_float_decode)
{
	{
		// 32-bit float tests
		STRING_FROM_CHAR_ARRAY(str, "\244\277\200\000\000a"
				"\244\100\000\000\000b\244\077\201\200\100");
		LexerFixture f(str);
		BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(-1.0f));
		BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::REQUEST, "a"));
		BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(2.0f));
		BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::REQUEST, "b"));
		BOOST_CHECK_EQUAL(f.lex.get(),
				CqRibToken(1.0f + 1.0f/(1<<7) + 1.0f/(1<<8) + 1.0f/(1<<17)));
		CHECK_EOF(f.lex);
	}
	{
		// 64-bit float tests
		STRING_FROM_CHAR_ARRAY(str, "\245\277\360\000\000\000\000\000\000"
				"a"
				"\245\100\004\000\000\000\000\000\000");
		LexerFixture f(str);
		BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(-1.0f));
		BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::REQUEST, "a"));
		BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(2.5f));
		CHECK_EOF(f.lex);
	}
	{
		// 32-bit float array tests
		STRING_FROM_CHAR_ARRAY(str, "\310\002\277\200\000\000\100\000\000\000a");
		LexerFixture f(str);
		BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::ARRAY_BEGIN));
		BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(-1.0f));
		BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(2.0f));
		BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::ARRAY_END));
		BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::REQUEST, "a"));
		CHECK_EOF(f.lex);
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
	LexerFixture f(str);
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::REQUEST, "DefinedRequest377"));
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::REQUEST, "DefinedRequest000"));
	CHECK_EOF(f.lex);
}

BOOST_AUTO_TEST_CASE(CqRibLexer_defined_string_test)
{
	STRING_FROM_CHAR_ARRAY(str,
			"\315\000\"DefinedString000\"" // define string at code 0.
			"\316\100\100\"DefinedString100100\"" // define string at code 0100100.
			"\320\100\100"                 // reference string at 0100100
			"\317\000"                     // reference string at 0
			);
	LexerFixture f(str);
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::STRING, "DefinedString100100"));
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::STRING, "DefinedString000"));
	CHECK_EOF(f.lex);
}

BOOST_AUTO_TEST_CASE(CqRibLexer_binarymode_newlines)
{
	// Test that the lexer correctly avoids translating newline characters when
	// in binary mode.
	//
	// For simplicity, we perform this test with the integer binary encoding
	LexerFixture f("\203a\rcd\203a\ncd\203a\r\nd");
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(
		(TqInt('a') << 24) + (TqInt('\r') << 16) + (TqInt('c') << 8) + TqInt('d')) );
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(
		(TqInt('a') << 24) + (TqInt('\n') << 16) + (TqInt('c') << 8) + TqInt('d')) );
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(
		(TqInt('a') << 24) + (TqInt('\r') << 16) + (TqInt('\n') << 8) + TqInt('d')) );
	CHECK_EOF(f.lex);
}

//------------------------------------------------------------------------------
// Test misc. public methods
//------------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(CqRibLexer_position_test)
{
	LexerFixture f("Rqst [1.0\n 1] \"asdf\" ");
	f.lex.get();
	f.lex.get();
	f.lex.get();
	SqRibPos pos = f.lex.pos();
	BOOST_CHECK_EQUAL(pos.line, 1);
	BOOST_CHECK_EQUAL(pos.col, 7);
	f.lex.get();
	f.lex.get();
	f.lex.get();
	pos = f.lex.pos();
	BOOST_CHECK_EQUAL(pos.line, 2);
	BOOST_CHECK_EQUAL(pos.col, 5);
}

BOOST_AUTO_TEST_CASE(CqRibLexer_peek_test)
{
	LexerFixture f("Rqst [1.0 1]");
	BOOST_CHECK_EQUAL(f.lex.peek(), CqRibToken(CqRibToken::REQUEST, "Rqst"));

	f.lex.get();
	f.lex.get();
	f.lex.get();
	f.lex.peek();
	f.lex.peek();

	SqRibPos pos = f.lex.pos();
	BOOST_CHECK_EQUAL(pos.line, 1);
	BOOST_CHECK_EQUAL(pos.col, 7);
	BOOST_CHECK_EQUAL(f.lex.peek(), CqRibToken(1));
	f.lex.get();
	f.lex.get();
	CHECK_EOF(f.lex);
}

BOOST_AUTO_TEST_CASE(CqRibLexer_unget_test)
{
	LexerFixture f("Rqst [1.0 1]");

	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::REQUEST, "Rqst"));
	f.lex.unget();
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::REQUEST, "Rqst"));

	f.lex.get();
	f.lex.get();
	f.lex.get();
	f.lex.get();
	f.lex.unget();

	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::ARRAY_END));
	CHECK_EOF(f.lex);
}

BOOST_AUTO_TEST_CASE(CqRibLexer_streamstack_test)
{
	CqRibLexer lex;
	CHECK_EOF(lex);

	std::istringstream in1("Stream1Start Stream1End");
	lex.pushInput(in1, "stream1");
	BOOST_CHECK_EQUAL(lex.get(), CqRibToken(CqRibToken::REQUEST, "Stream1Start"));
	BOOST_CHECK_EQUAL(lex.pos().name, "stream1");

	lex.peek();
	std::istringstream in2("Stream2Start");
	lex.pushInput(in2, "stream2");
	BOOST_CHECK_EQUAL(lex.get(), CqRibToken(CqRibToken::REQUEST, "Stream2Start"));
	BOOST_CHECK_EQUAL(lex.pos().name, "stream2");
	CHECK_EOF(lex);

	lex.popInput();
	BOOST_CHECK_EQUAL(lex.get(), CqRibToken(CqRibToken::REQUEST, "Stream1End"));
	BOOST_CHECK_EQUAL(lex.pos().col, 14);
	BOOST_CHECK_EQUAL(lex.pos().name, "stream1");
	CHECK_EOF(lex);

	lex.popInput();
	CHECK_EOF(lex);
}

struct MockCommentCallback
{
	std::string str;
	void operator()(const std::string& comment)
	{
		str = comment;
	}
};

BOOST_AUTO_TEST_CASE(CqRibLexer_comment_callback_test)
{
	CqRibLexer lex;
	CHECK_EOF(lex);

	std::istringstream in1("Stream1Start ##some RIB structure comment here\n Stream1End");
	MockCommentCallback comment1;
	lex.pushInput(in1, "stream1", boost::ref(comment1));
	BOOST_CHECK_EQUAL(lex.get(), CqRibToken(CqRibToken::REQUEST, "Stream1Start"));

	std::istringstream in2("#A second comment\n  Stream2Start");
	MockCommentCallback comment2;
	lex.pushInput(in2, "stream2", boost::ref(comment2));

	BOOST_CHECK_EQUAL(comment2.str, "");
	BOOST_CHECK_EQUAL(lex.get(), CqRibToken(CqRibToken::REQUEST, "Stream2Start"));
	BOOST_CHECK_EQUAL(comment2.str, "A second comment");
	CHECK_EOF(lex);

	lex.popInput();
	BOOST_CHECK_EQUAL(comment1.str, "");
	BOOST_CHECK_EQUAL(lex.get(), CqRibToken(CqRibToken::REQUEST, "Stream1End"));
	BOOST_CHECK_EQUAL(comment1.str, "#some RIB structure comment here");
	CHECK_EOF(lex);
}

BOOST_AUTO_TEST_CASE(CqRibLexer_end_of_file_test)
{
	// Tests for characters which create EOFs
	LexerFixture f("Rqst \377 Rq2 # asdf \377\nRq3");

	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::REQUEST, "Rqst"));
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::ENDOFFILE));
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::REQUEST, "Rq2"));
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::ENDOFFILE));
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::REQUEST, "Rq3"));
	BOOST_CHECK_EQUAL(f.lex.get(), CqRibToken(CqRibToken::ENDOFFILE));
}
