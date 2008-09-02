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
 * \brief Unit tests for enum <--> string conversion class
 *
 * \author Chris Foster [ chris42f (at) g mail (dot) com ]
 */

#include "enum.h"

#include <sstream>

#include <boost/test/auto_unit_test.hpp>

// Days of the week test enum.
enum Day
{
	Sunday,
	Monday,
	Tuesday,
	Wednesday,
	Thursday,
	Friday,
	Saturday
};

namespace Aqsis {
// Names for days of the week test enum.  Unfortunately the current
// implementation requires that this be in the aqsis namespace for things to
// work correctly :-/
AQSIS_ENUM_INFO_BEGIN(Day, Monday)
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday"
AQSIS_ENUM_INFO_END
}

using namespace Aqsis;


BOOST_AUTO_TEST_CASE(CqEnum_default_constructor_test)
{
	CqEnum<Day> d;
	BOOST_CHECK_EQUAL(d.value(), Monday);
	BOOST_CHECK_EQUAL(d.name(), std::string("Monday"));
}

BOOST_AUTO_TEST_CASE(CqEnum_string_constructor_test)
{
	{
		CqEnum<Day> d("Thursday");
		BOOST_CHECK_EQUAL(d.value(), Thursday);
		BOOST_CHECK_EQUAL(d.name(), std::string("Thursday"));
	}

	{
		CqEnum<Day> d("Invalid!!");
		BOOST_CHECK_EQUAL(d.value(), Monday);
	}
}

BOOST_AUTO_TEST_CASE(CqEnum_value_constructor_test)
{
	CqEnum<Day> d(Saturday);
	BOOST_CHECK_EQUAL(d.value(), Saturday);
	BOOST_CHECK_EQUAL(d.name(), std::string("Saturday"));
}

BOOST_AUTO_TEST_CASE(CqEnum_stream_insert_test)
{
	std::ostringstream out;
	out << CqEnum<Day>(Friday);
	BOOST_CHECK_EQUAL(out.str(), "Friday");
}

BOOST_AUTO_TEST_CASE(CqEnum_stream_extract_test)
{
	std::istringstream in("Saturday Thursday");
	CqEnum<Day> d;
	in >> d;
	BOOST_CHECK_EQUAL(d.value(), Saturday);
	in >> d;
	BOOST_CHECK_EQUAL(d.value(), Thursday);
}

