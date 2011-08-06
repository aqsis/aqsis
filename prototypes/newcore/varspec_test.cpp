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

#include "varspec.h"

#include <boost/range.hpp> // for begin() and end()

#define BOOST_TEST_MAIN
#ifndef _WIN32
#define BOOST_TEST_DYN_LINK
#endif
#include <boost/test/auto_unit_test.hpp>

using namespace Aqsis;

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

