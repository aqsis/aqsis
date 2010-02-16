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

#include "arrayview.h"
#include "util.h"

#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/auto_unit_test.hpp>


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
    const int v3Size = DataView<Vec3>::elSize();
    float aData[v3Size*nvals];
    float bData[v3Size*nvals];

    DataView<Vec3> a(aData);
    DataView<Vec3> b(bData);

    // initialize b
    for(int i = 0; i < nvals; ++i)
        b[i] = Vec3(1,2,3);
    b[nvals-1] = Vec3(3,2,1);

    // Copy b into a.
    copy(a, b, nvals);

    // check that the copy worked
    BOOST_CHECK_EQUAL(a[0], b[0]);
    BOOST_CHECK_EQUAL(a[1], b[1]);
    BOOST_CHECK_EQUAL(a[2], b[2]);
    BOOST_CHECK_EQUAL(a[3], b[3]);

    // Create c & initialize to something else
    const int skip = 2;
    float cData[v3Size*nvals/skip];
    DataView<Vec3> c(cData);
    for(int i = 0; i < nvals/skip; ++i)
        c[i] = Vec3(-1,-1,-1);

    // Now make a view onto a that only includes every second element
    DataView<Vec3> a2(aData, skip*v3Size);
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
    ConstDataView<Vec3> P(data);

    // Derivatives in u-direction
    BOOST_CHECK_EQUAL(diff(P+1, 1, 4), Vec3(1,1,1));
    BOOST_CHECK_EQUAL(diff(P+1, 1, 2), Vec3(1,1,1));
    BOOST_CHECK_EQUAL(diff(P, 0, 2), Vec3(1,1,1));

    // Derivatives in v-direction using a slice
    ConstDataView<Vec3> Pv = slice(P+1, 4);
    BOOST_CHECK_EQUAL(diff(Pv+1, 1, 3), Vec3(0,0,0));
    BOOST_CHECK_EQUAL(diff(Pv+1, 1, 2), Vec3(1,1,1));
    BOOST_CHECK_EQUAL(diff(Pv, 0, 2), Vec3(1,1,1));
}
