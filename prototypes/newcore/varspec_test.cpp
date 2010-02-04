// Aqsis
// Copyright (C) 1997 - 2010, Paul C. Gregory
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

#include "varspec.h"

#include <boost/range.hpp> // for begin() and end()

#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/auto_unit_test.hpp>

BOOST_AUTO_TEST_CASE(VarSpec_cmp)
{
    BOOST_CHECK_LT(Stdvar::Cs, Stdvar::P);

    BOOST_CHECK_EQUAL(VarSpec(VarSpec::Point, 1, ustring("P")), Stdvar::P);
}


BOOST_AUTO_TEST_CASE(VarSet_test)
{
    VarSpec myVar(VarSpec::Point, 2, ustring("asdf"));
    VarSpec varsInit[] = {
        Stdvar::I,
        Stdvar::P,
        Stdvar::Cs,
        Stdvar::Ci,
        myVar
    };
    std::sort(boost::begin(varsInit), boost::end(varsInit));
    VarSet vars(boost::begin(varsInit), boost::end(varsInit));

    BOOST_CHECK_EQUAL(vars.size(), 5);

    // Check the the expected variables are present.
    // 1) Check the call with StdIndices::Id id's
    BOOST_CHECK(vars.contains(StdIndices::P));
    BOOST_CHECK(vars.contains(StdIndices::I));
    // 2) Check the call with VarSpec id's
    BOOST_CHECK(vars.contains(Stdvar::Cs));
    BOOST_CHECK(vars.contains(Stdvar::Ci));
    BOOST_CHECK(vars.contains(myVar));

    // Check that the indices of standard vars are correctly cached.
    BOOST_CHECK_EQUAL(vars.find(Stdvar::P), vars.stdIndices().get(StdIndices::P));
    BOOST_CHECK_EQUAL(vars.find(Stdvar::I), vars.stdIndices().get(StdIndices::I));
    BOOST_CHECK_EQUAL(vars.find(Stdvar::Cs), vars.find(StdIndices::Cs));
    BOOST_CHECK_EQUAL(vars.find(Stdvar::Ci), vars.find(StdIndices::Ci));

    // Check that the myVar index is present
    BOOST_CHECK_NE(vars.find(myVar), VarSet::npos);
    // Check that some random var is reported as not-found.
    BOOST_CHECK_EQUAL(vars.find(VarSpec(VarSpec::Point, 2, ustring("blah"))),
                      VarSet::npos);
}

