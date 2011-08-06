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

#include <sstream>

#include "ribtokenizer.h"

#define BOOST_TEST_DYN_LINK

#include <boost/test/auto_unit_test.hpp>

#define ADD_ESCAPES(x) #x

#define CHECK_EOF(tokenizer) BOOST_CHECK_EQUAL(tokenizer.get(), \
		RibToken(RibToken::ENDOFFILE))

using namespace Aqsis;


struct TokenizerFixture
{
	std::istringstream input;
	RibTokenizer t;

	template<typename StrT> TokenizerFixture(const StrT& str)
		: input(str),
		t()
	{
		t.pushInput(input, "test_stream");
	}
};


//------------------------------------------------------------------------------
// Test cases for ASCII RIB parsing
//------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(rib_tokenizer_tests)

BOOST_AUTO_TEST_CASE(RibTokenizer_strings_test)
{
	{
		// Test multiple and adjacent strings
		TokenizerFixture f("\"xx\"  \"\"\"a\"");
		BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::STRING, "xx"));
		BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::STRING, ""));
		BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::STRING, "a"));
		CHECK_EOF(f.t);
	}
	{
		// Test escape characters; these should be the same as escaping in C++.
		TokenizerFixture f(ADD_ESCAPES("_\n_\r_\t_\b_\f_\\_\"_\z_"));
		BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::STRING,
					"_\n_\r_\t_\b_\f_\\_\"_z_"));
		CHECK_EOF(f.t);
	}
	{
		// Test octal escape characters; these should be the same as escaping
		// in C++ as well.
		TokenizerFixture f(ADD_ESCAPES("\101_\12_\7"));
		BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::STRING,
					"\101_\12_\7"));
		CHECK_EOF(f.t);
	}
	{
		// Test embedded newlines and tabs
		TokenizerFixture f("\"xx\t\nXX\"  \"a\r\nb\rc\nd\"");
		BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::STRING,
					"xx\t\nXX"));
		BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::STRING,
					"a\nb\nc\nd"));
		CHECK_EOF(f.t);
	}
	{
		// escaped line break (should discard newline)
		TokenizerFixture f("\"jo\\\nined\"");
		BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::STRING, "joined"));
		CHECK_EOF(f.t);
	}
	{
		// Test what happens when the stream ends before a string end character
		TokenizerFixture f("\"xx");
		BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::ERROR));
		CHECK_EOF(f.t);
	}
}

BOOST_AUTO_TEST_CASE(RibTokenizer_int_test)
{
	TokenizerFixture f("42 +42 -42");
	int ints[] = {42, +42, -42};
	for(int i = 0; i < static_cast<int>(sizeof(ints)/sizeof(ints[0])); ++i)
		BOOST_CHECK_EQUAL(f.t.get(), RibToken(ints[i]));
	CHECK_EOF(f.t);
}

BOOST_AUTO_TEST_CASE(RibTokenizer_float_test)
{
	TokenizerFixture f(
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
	for(int i = 0; i < static_cast<int>(sizeof(floats)/sizeof(floats[0])); ++i)
		BOOST_CHECK_EQUAL(f.t.get(), RibToken(floats[i]));
	CHECK_EOF(f.t);
}

BOOST_AUTO_TEST_CASE(RibTokenizer_array_test)
{
	TokenizerFixture f("[ 1.0 -1 ]");
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::ARRAY_BEGIN));
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(1.0f));
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(-1));
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::ARRAY_END));
	CHECK_EOF(f.t);
}

BOOST_AUTO_TEST_CASE(RibTokenizer_request_test)
{
	TokenizerFixture f("WorldBegin version SomethingElse");
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::REQUEST, "WorldBegin"));
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::REQUEST, "version"));
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::REQUEST, "SomethingElse"));
	CHECK_EOF(f.t);
}


//------------------------------------------------------------------------------
// Test cases for binary RIB decoding
//------------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(RibTokenizer_integer_decode)
{
	TokenizerFixture f("\200a\201ab\202abc\203abcd");
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(int('a')));
	BOOST_CHECK_EQUAL(f.t.get(), RibToken((int('a') << 8) + int('b')) );
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(
			(int('a') << 16) + (int('b') << 8) + int('c')) );
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(
			(int('a') << 24) + (int('b') << 16) + (int('c') << 8) + int('d')) );
	CHECK_EOF(f.t);
}

BOOST_AUTO_TEST_CASE(RibTokenizer_fixedpoint_decode)
{
	// Only a selection of the possibilites for fixed-point numbers are tested
	// here...
	TokenizerFixture f("\204a\205bc\212def\213ghij\214k");
	// 0204:  .b
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(float('a')/256));
	// 0205:  b.b
	BOOST_CHECK_EQUAL(f.t.get(), RibToken('b' + float('c')/256));
	// 0212:  b.bb
	BOOST_CHECK_EQUAL(f.t.get(), RibToken('d' + ('e' + float('f')/256)/256));
	// 0213:  bb.bb
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(
				(int('g') << 8) + 'h' + (float('i') + float('j')/256)/256));
	// 0214:  .__b
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(float('k')/256/256/256));
	CHECK_EOF(f.t);
}

BOOST_AUTO_TEST_CASE(RibTokenizer_short_string_decode)
{
	TokenizerFixture f("\220\221a\230bcdefghi\237jklm\\nopqrstuvw");
	// min. length short string.
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::STRING, ""));
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::STRING, "a"));
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::STRING, "bcdefghi"));
	// Test max. length short string and lack of escaping in encoded strings.
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::STRING, "jklm\\nopqrstuvw"));
	CHECK_EOF(f.t);
}

// Do a funny dance with char* for string literals which contains null
// "terminators" which we don't want to actually terminate the string.
#define STRING_FROM_CHAR_ARRAY(stringName, array) \
const char aq_strPtr[] = array; std::string stringName(aq_strPtr, aq_strPtr + sizeof(aq_strPtr)-1)

BOOST_AUTO_TEST_CASE(RibTokenizer_long_string_decode)
{
	STRING_FROM_CHAR_ARRAY(str, "\240\000\240\012aaaaAaaaaA\243\000\000\000\002aX");
	TokenizerFixture f(str);
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::STRING, ""));
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::STRING, "aaaaAaaaaA"));
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::STRING, "aX"));
	CHECK_EOF(f.t);
}

BOOST_AUTO_TEST_CASE(RibTokenizer_float_decode)
{
	{
		// 32-bit float tests
		STRING_FROM_CHAR_ARRAY(str, "\244\277\200\000\000a"
				"\244\100\000\000\000b\244\077\201\200\100");
		TokenizerFixture f(str);
		BOOST_CHECK_EQUAL(f.t.get(), RibToken(-1.0f));
		BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::REQUEST, "a"));
		BOOST_CHECK_EQUAL(f.t.get(), RibToken(2.0f));
		BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::REQUEST, "b"));
		BOOST_CHECK_EQUAL(f.t.get(),
				RibToken(1.0f + 1.0f/(1<<7) + 1.0f/(1<<8) + 1.0f/(1<<17)));
		CHECK_EOF(f.t);
	}
	{
		// 64-bit float tests
		STRING_FROM_CHAR_ARRAY(str, "\245\277\360\000\000\000\000\000\000"
				"a"
				"\245\100\004\000\000\000\000\000\000");
		TokenizerFixture f(str);
		BOOST_CHECK_EQUAL(f.t.get(), RibToken(-1.0f));
		BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::REQUEST, "a"));
		BOOST_CHECK_EQUAL(f.t.get(), RibToken(2.5f));
		CHECK_EOF(f.t);
	}
	{
		// 32-bit float array tests
		STRING_FROM_CHAR_ARRAY(str, "\310\002\277\200\000\000\100\000\000\000a");
		TokenizerFixture f(str);
		BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::ARRAY_BEGIN));
		BOOST_CHECK_EQUAL(f.t.get(), RibToken(-1.0f));
		BOOST_CHECK_EQUAL(f.t.get(), RibToken(2.0f));
		BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::ARRAY_END));
		BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::REQUEST, "a"));
		CHECK_EOF(f.t);
	}
}

BOOST_AUTO_TEST_CASE(RibTokenizer_defined_request_test)
{
	STRING_FROM_CHAR_ARRAY(str,
			"\314\000\240\021DefinedRequest000" // define request at code 0.
			"\314\377\"DefinedRequest377\"" // define request at code 0377.
			"\246\377"                  // reference request 0377
			"\246\000"                  // reference request 0
			);
	TokenizerFixture f(str);
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::REQUEST, "DefinedRequest377"));
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::REQUEST, "DefinedRequest000"));
	CHECK_EOF(f.t);
}

BOOST_AUTO_TEST_CASE(RibTokenizer_defined_string_test)
{
	STRING_FROM_CHAR_ARRAY(str,
			"\315\000\"DefinedString000\"" // define string at code 0.
			"\316\100\100\"DefinedString100100\"" // define string at code 0100100.
			"\320\100\100"                 // reference string at 0100100
			"\317\000"                     // reference string at 0
			);
	TokenizerFixture f(str);
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::STRING, "DefinedString100100"));
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::STRING, "DefinedString000"));
	CHECK_EOF(f.t);
}

BOOST_AUTO_TEST_CASE(RibTokenizer_binarymode_newlines)
{
	// Test that the lexer correctly avoids translating newline characters when
	// in binary mode.
	//
	// For simplicity, we perform this test with the integer binary encoding
	TokenizerFixture f("\203a\rcd\203a\ncd\203a\r\nd");
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(
		(int('a') << 24) + (int('\r') << 16) + (int('c') << 8) + int('d')) );
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(
		(int('a') << 24) + (int('\n') << 16) + (int('c') << 8) + int('d')) );
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(
		(int('a') << 24) + (int('\r') << 16) + (int('\n') << 8) + int('d')) );
	CHECK_EOF(f.t);
}

//------------------------------------------------------------------------------
// Test misc. public methods
//------------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(RibTokenizer_position_test)
{
	TokenizerFixture f("Rqst [1.0\n 1] \"asdf\" ");
	f.t.get();
	f.t.get();
	f.t.get();
	BOOST_CHECK_EQUAL("test_stream:1 (col 7)", f.t.streamPos());
	f.t.get();
	f.t.get();
	f.t.get();
	BOOST_CHECK_EQUAL("test_stream:2 (col 5)", f.t.streamPos());
}

BOOST_AUTO_TEST_CASE(RibTokenizer_peek_test)
{
	TokenizerFixture f("Rqst [1.0 1]");
	BOOST_CHECK_EQUAL(f.t.peek(), RibToken(RibToken::REQUEST, "Rqst"));

	f.t.get();
	f.t.get();
	f.t.get();
	f.t.peek();
	f.t.peek();

	BOOST_CHECK_EQUAL("test_stream:1 (col 7)", f.t.streamPos());
	BOOST_CHECK_EQUAL(f.t.peek(), RibToken(1));
	f.t.get();
	f.t.get();
	CHECK_EOF(f.t);
}

BOOST_AUTO_TEST_CASE(RibTokenizer_unget_test)
{
	TokenizerFixture f("Rqst [1.0 1]");

	BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::REQUEST, "Rqst"));
	f.t.unget();
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::REQUEST, "Rqst"));

	f.t.get();
	f.t.get();
	f.t.get();
	f.t.get();
	f.t.unget();

	BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::ARRAY_END));
	CHECK_EOF(f.t);
}

BOOST_AUTO_TEST_CASE(RibTokenizer_streamstack_test)
{
	RibTokenizer lex;
	CHECK_EOF(lex);

	std::istringstream in1("Stream1Start Stream1End");
	lex.pushInput(in1, "stream1");
	BOOST_CHECK_EQUAL(lex.get(), RibToken(RibToken::REQUEST, "Stream1Start"));
	BOOST_CHECK_EQUAL(lex.streamPos(), "stream1:1 (col 1)");

	lex.peek();
	std::istringstream in2("Stream2Start");
	lex.pushInput(in2, "stream2");
	BOOST_CHECK_EQUAL(lex.get(), RibToken(RibToken::REQUEST, "Stream2Start"));
	BOOST_CHECK_EQUAL(lex.streamPos(), "stream2:1 (col 1)");
	CHECK_EOF(lex);

	lex.popInput();
	BOOST_CHECK_EQUAL(lex.get(), RibToken(RibToken::REQUEST, "Stream1End"));
	BOOST_CHECK_EQUAL(lex.streamPos(), "stream1:1 (col 14)");
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

BOOST_AUTO_TEST_CASE(RibTokenizer_comment_callback_test)
{
	RibTokenizer lex;
	CHECK_EOF(lex);

	std::istringstream in1("Stream1Start ##some RIB structure comment here\n Stream1End");
	MockCommentCallback comment1;
	lex.pushInput(in1, "stream1", boost::ref(comment1));
	BOOST_CHECK_EQUAL(lex.get(), RibToken(RibToken::REQUEST, "Stream1Start"));

	std::istringstream in2("#A second comment\n  Stream2Start");
	MockCommentCallback comment2;
	lex.pushInput(in2, "stream2", boost::ref(comment2));

	BOOST_CHECK_EQUAL(comment2.str, "");
	BOOST_CHECK_EQUAL(lex.get(), RibToken(RibToken::REQUEST, "Stream2Start"));
	BOOST_CHECK_EQUAL(comment2.str, "A second comment");
	CHECK_EOF(lex);

	lex.popInput();
	BOOST_CHECK_EQUAL(comment1.str, "");
	BOOST_CHECK_EQUAL(lex.get(), RibToken(RibToken::REQUEST, "Stream1End"));
	BOOST_CHECK_EQUAL(comment1.str, "#some RIB structure comment here");
	CHECK_EOF(lex);
}

BOOST_AUTO_TEST_CASE(RibTokenizer_end_of_file_test)
{
	// Tests for characters which create EOFs
	TokenizerFixture f("Rqst \377 Rq2 # asdf \377\nRq3");

	BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::REQUEST, "Rqst"));
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::ENDOFFILE));
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::REQUEST, "Rq2"));
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::ENDOFFILE));
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::REQUEST, "Rq3"));
	BOOST_CHECK_EQUAL(f.t.get(), RibToken(RibToken::ENDOFFILE));
}

BOOST_AUTO_TEST_SUITE_END()
