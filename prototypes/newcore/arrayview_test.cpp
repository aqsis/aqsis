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

#include "arrayview.h"
#include "util.h"

#define BOOST_TEST_MAIN
#ifndef _WIN32
#define BOOST_TEST_DYN_LINK
#endif
#include <boost/test/auto_unit_test.hpp>

using namespace Aqsis;

BOOST_AUTO_TEST_CASE(FvecView_copy_test)
{
    const int nvals = 4;
    const int elSize = 2;
    float aData[elSize*nvals];
    float bData[elSize*nvals];

    FvecView a(aData, elSize);
    FvecView b(bData, elSize);

    // initialize b
    for(int i = 0; i < nvals; ++i)
    {
        b[i][0] = 1;
        b[i][1] = 2;
    }
    b[nvals-1][0] = 2;
    b[nvals-1][0] = 1;

    // Copy b into a.
    copy(a, b, nvals);

    // check that the copy worked
    BOOST_CHECK_EQUAL(a[0][0], b[0][0]);
    BOOST_CHECK_EQUAL(a[0][1], b[0][1]);
    BOOST_CHECK_EQUAL(a[1][0], b[1][0]);
    BOOST_CHECK_EQUAL(a[1][1], b[1][1]);
    BOOST_CHECK_EQUAL(a[2][0], b[2][0]);
    BOOST_CHECK_EQUAL(a[2][1], b[2][1]);
    BOOST_CHECK_EQUAL(a[3][0], b[3][0]);
    BOOST_CHECK_EQUAL(a[3][1], b[3][1]);

    // Create c & initialize to something else
    const int skip = 2;
    float cData[elSize*nvals/skip];
    FvecView c(cData, elSize);
    for(int i = 0; i < nvals/skip; ++i)
    {
        c[i][0] = -1;
        c[i][1] = -1;
    }

    // Now make a view onto a that only includes every second element
    FvecView a2(aData, elSize, skip*elSize);
    // Copy c into every second element of a.
    copy(a2, c, nvals/skip);

    // Check that a now contains 1) some stuff from c
    BOOST_CHECK_EQUAL(a[0][0], c[0][0]);
    BOOST_CHECK_EQUAL(a[0][1], c[0][1]);
    BOOST_CHECK_EQUAL(a[2][0], c[1][0]);
    BOOST_CHECK_EQUAL(a[2][1], c[1][1]);

    // 2) some stuff from a
    BOOST_CHECK_EQUAL(a[1][0], b[1][0]);
    BOOST_CHECK_EQUAL(a[1][1], b[1][1]);
    BOOST_CHECK_EQUAL(a[3][0], b[3][0]);
    BOOST_CHECK_EQUAL(a[3][1], b[3][1]);
}


BOOST_AUTO_TEST_CASE(DataView_copy_test)
{
    const int nvals = 4;
    const int v3Size = DataView<V3f>::elSize();
    std::vector<float> aData(v3Size*nvals);
    std::vector<float> bData(v3Size*nvals);

    DataView<V3f> a(cbegin(aData));
    DataView<V3f> b(cbegin(bData));

    // initialize b
    for(int i = 0; i < nvals; ++i)
        b[i] = V3f(1,2,3);
    b[nvals-1] = V3f(3,2,1);

    // Copy b into a.
    copy(a, b, nvals);

    // check that the copy worked
    BOOST_CHECK_EQUAL(a[0], b[0]);
    BOOST_CHECK_EQUAL(a[1], b[1]);
    BOOST_CHECK_EQUAL(a[2], b[2]);
    BOOST_CHECK_EQUAL(a[3], b[3]);

    // Create c & initialize to something else
    const int skip = 2;
    std::vector<float> cData(v3Size*nvals/skip);
	DataView<V3f> c(cbegin(cData));
    for(int i = 0; i < nvals/skip; ++i)
        c[i] = V3f(-1,-1,-1);

    // Now make a view onto a that only includes every second element
    DataView<V3f> a2(cbegin(aData), skip*v3Size);
    // Copy c into every second element of a.
    copy(a2, c, nvals/skip);

    // Check that a now contains a mixture of stuff from b & c.
    BOOST_CHECK_EQUAL(a[0], c[0]);
    BOOST_CHECK_EQUAL(a[1], b[1]);
    BOOST_CHECK_EQUAL(a[2], c[1]);
    BOOST_CHECK_EQUAL(a[3], b[3]);
}

BOOST_AUTO_TEST_CASE(DataView_diff_test)
{
    float data[] = {
        // ---> "u-direction"
        1,1,1, 2,2,2, 3,3,3, 0,0,0,  //  |
        2,2,2, 3,3,3, 4,4,4, 0,0,0,  //  |  "v-direction"
        0,0,0, 2,2,2, 0,0,0, 0,0,0   //  v
    };
    ConstDataView<V3f> P(data);

    // Derivatives in u-direction
    BOOST_CHECK_EQUAL(diff(P+1, 1, 4), V3f(1,1,1));
    BOOST_CHECK_EQUAL(diff(P+1, 1, 2), V3f(1,1,1));
    BOOST_CHECK_EQUAL(diff(P, 0, 2), V3f(1,1,1));

    // Derivatives in v-direction using a slice
    ConstDataView<V3f> Pv = slice(P+1, 4);
    BOOST_CHECK_EQUAL(diff(Pv+1, 1, 3), V3f(0,0,0));
    BOOST_CHECK_EQUAL(diff(Pv+1, 1, 2), V3f(1,1,1));
    BOOST_CHECK_EQUAL(diff(Pv, 0, 2), V3f(1,1,1));
}
