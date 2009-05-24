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
 *
 * \brief Unit tests for primvar token class.
 *
 * \author Chris Foster [ chris42f (at) g mail (dot) com ]
 */

#include <aqsis/riutil/primvartoken.h>

#ifndef	AQSIS_SYSTEM_WIN32
#define BOOST_TEST_DYN_LINK
#endif //AQSIS_SYSTEM_WIN32

#include <boost/test/auto_unit_test.hpp>

#include <aqsis/util/exception.h>

using namespace Aqsis;

BOOST_AUTO_TEST_CASE(CqPrimvarToken_parse_test)
{
	{
		// Check basic behaviour
		CqPrimvarToken tok("vertex float[2] st");
		BOOST_CHECK_EQUAL(tok.Class(), class_vertex);
		BOOST_CHECK_EQUAL(tok.type(), type_float);
		BOOST_CHECK_EQUAL(tok.count(), 2);
		BOOST_CHECK_EQUAL(tok.name(), "st");
	}

	{
		// Check whitespace handling.
		CqPrimvarToken tok("\n  vertex \tfloat [ 2 ] st    ");
		BOOST_CHECK_EQUAL(tok.Class(), class_vertex);
		BOOST_CHECK_EQUAL(tok.type(), type_float);
		BOOST_CHECK_EQUAL(tok.count(), 2);
		BOOST_CHECK_EQUAL(tok.name(), "st");
	}

	{
		// Check capitalization handling
		CqPrimvarToken tok("VERTEX veCtor P");
		BOOST_CHECK_EQUAL(tok.Class(), class_vertex);
		BOOST_CHECK_EQUAL(tok.type(), type_vector);
		BOOST_CHECK_EQUAL(tok.count(), 1);
		BOOST_CHECK_EQUAL(tok.name(), "P");
	}

	{
		// Check a special case kludge - integer types can be specified either
		// with "int" or with "integer"
		CqPrimvarToken tok("uniform int i");
		BOOST_CHECK_EQUAL(tok.type(), type_integer);
	}
}

BOOST_AUTO_TEST_CASE(CqPrimvarToken_parse_defaults_test)
{
	{
		CqPrimvarToken tok("matrix some_mat");
		BOOST_CHECK_EQUAL(tok.Class(), class_uniform);
		BOOST_CHECK_EQUAL(tok.type(), type_matrix);
		BOOST_CHECK_EQUAL(tok.count(), 1);
		BOOST_CHECK_EQUAL(tok.name(), "some_mat");
	}

	{
		CqPrimvarToken tok("matrix", "some_mat");
		BOOST_CHECK_EQUAL(tok.Class(), class_uniform);
		BOOST_CHECK_EQUAL(tok.type(), type_matrix);
		BOOST_CHECK_EQUAL(tok.count(), 1);
		BOOST_CHECK_EQUAL(tok.name(), "some_mat");
	}
}

// tests for parsing invalid tokens.
BOOST_AUTO_TEST_CASE(CqPrimvarToken_invalid_parse_test)
{
	// invalid tokens
	BOOST_CHECK_THROW(CqPrimvarToken("#u"), XqParseError);
	BOOST_CHECK_THROW(CqPrimvarToken("\"u"), XqParseError);

	// invalid/incomplete token sequences
	BOOST_CHECK_THROW(CqPrimvarToken("] u"), XqParseError);
	BOOST_CHECK_THROW(CqPrimvarToken("[ u"), XqParseError);
	BOOST_CHECK_THROW(CqPrimvarToken("[ 2"), XqParseError);
	BOOST_CHECK_THROW(CqPrimvarToken("["), XqParseError);
	BOOST_CHECK_THROW(CqPrimvarToken("float varying u"), XqParseError);
	BOOST_CHECK_THROW(CqPrimvarToken("varying u"), XqParseError);
	BOOST_CHECK_THROW(CqPrimvarToken("[2] u"), XqParseError);
	BOOST_CHECK_THROW(CqPrimvarToken("[2] float st"), XqParseError);
	BOOST_CHECK_THROW(CqPrimvarToken("P Cs"), XqParseError);
	BOOST_CHECK_THROW(CqPrimvarToken("P", "Cs"), XqParseError);
}
