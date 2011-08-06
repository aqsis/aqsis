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
 *
 * \brief Unit tests for enum <--> string conversion class
 *
 * \author Chris Foster [ chris42f (at) g mail (dot) com ]
 */

#include <aqsis/util/enum.h>

#include <sstream>

#define BOOST_TEST_DYN_LINK
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


BOOST_AUTO_TEST_SUITE(enum_tests)
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

BOOST_AUTO_TEST_SUITE_END()
