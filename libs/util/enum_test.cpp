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

#include <aqsis/util/enum.h>

#ifndef	AQSIS_SYSTEM_WIN32
#define BOOST_TEST_DYN_LINK
#endif //AQSIS_SYSTEM_WIN32

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
	Saturday,
	Unknown
};

namespace Aqsis {
// Names for days of the week test enum.  Unfortunately the current
// implementation requires that this be in the aqsis namespace for things to
// work correctly :-/
AQSIS_ENUM_INFO_BEGIN(Day, Unknown)
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday",
	"Unknown"
AQSIS_ENUM_INFO_END
}

using namespace Aqsis;


BOOST_AUTO_TEST_CASE(enumCast_test)
{
	BOOST_CHECK_EQUAL(enumCast<Day>("Sunday"), Sunday);
	BOOST_CHECK_EQUAL(enumCast<Day>("Wednesday"), Wednesday);
	BOOST_CHECK_EQUAL(enumCast<Day>("invalid_day"), Unknown);
	BOOST_CHECK_EQUAL(enumCast<Day>("Unknown"), Unknown);
}

BOOST_AUTO_TEST_CASE(enumString_test)
{
	BOOST_CHECK_EQUAL(enumString(Sunday), "Sunday");
	BOOST_CHECK_EQUAL(enumString(Wednesday), "Wednesday");
	BOOST_CHECK_EQUAL(enumString(Unknown), "Unknown");
}

BOOST_AUTO_TEST_CASE(CqEnum_stream_insert_test)
{
	std::ostringstream out;
	out << Friday;
	BOOST_CHECK_EQUAL(out.str(), "Friday");
}

BOOST_AUTO_TEST_CASE(CqEnum_stream_extract_test)
{
	std::istringstream in("Saturday Thursday");
	Day d;
	in >> d;
	BOOST_CHECK_EQUAL(d, Saturday);
	in >> d;
	BOOST_CHECK_EQUAL(d, Thursday);
}

