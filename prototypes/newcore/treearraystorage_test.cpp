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

#include "treearraystorage.h"

#define BOOST_TEST_MAIN
#ifndef _WIN32
#define BOOST_TEST_DYN_LINK
#endif
#include <boost/test/auto_unit_test.hpp>

using namespace Aqsis;

BOOST_AUTO_TEST_SUITE(treearraystorage_tests)

BOOST_AUTO_TEST_CASE(bspTreeNodeIndex_test)
{
    BOOST_CHECK_EQUAL(0, bspTreeNodeIndex(0, 0, 0));

    // depth 1
    BOOST_CHECK_EQUAL(1, bspTreeNodeIndex(0, 0, 1));
    BOOST_CHECK_EQUAL(2, bspTreeNodeIndex(1, 0, 1));

    // depth 2
    BOOST_CHECK_EQUAL(3, bspTreeNodeIndex(0, 0, 2));
    BOOST_CHECK_EQUAL(4, bspTreeNodeIndex(0, 1, 2));
    BOOST_CHECK_EQUAL(5, bspTreeNodeIndex(1, 0, 2));
    BOOST_CHECK_EQUAL(6, bspTreeNodeIndex(1, 1, 2));

    // depth 3
    BOOST_CHECK_EQUAL(7,  bspTreeNodeIndex(0, 0, 3));
    BOOST_CHECK_EQUAL(8,  bspTreeNodeIndex(1, 0, 3));
    BOOST_CHECK_EQUAL(9,  bspTreeNodeIndex(0, 1, 3));
    BOOST_CHECK_EQUAL(10, bspTreeNodeIndex(1, 1, 3));
    BOOST_CHECK_EQUAL(11, bspTreeNodeIndex(2, 0, 3));
    BOOST_CHECK_EQUAL(12, bspTreeNodeIndex(3, 0, 3));
    BOOST_CHECK_EQUAL(13, bspTreeNodeIndex(2, 1, 3));
    BOOST_CHECK_EQUAL(14, bspTreeNodeIndex(3, 1, 3));
}

BOOST_AUTO_TEST_CASE(quadTreeNodeIndex_test)
{
    // depth 0.
    BOOST_CHECK_EQUAL(0, quadTreeNodeIndex(0, 0, 0));

    // depth 1
    BOOST_CHECK_EQUAL(1, quadTreeNodeIndex(0, 0, 1));
    BOOST_CHECK_EQUAL(2, quadTreeNodeIndex(1, 0, 1));
    BOOST_CHECK_EQUAL(3, quadTreeNodeIndex(0, 1, 1));
    BOOST_CHECK_EQUAL(4, quadTreeNodeIndex(1, 1, 1));

    // depth 2
    BOOST_CHECK_EQUAL(5, quadTreeNodeIndex(0, 0, 2));
    BOOST_CHECK_EQUAL(6, quadTreeNodeIndex(1, 0, 2));
    BOOST_CHECK_EQUAL(7, quadTreeNodeIndex(0, 1, 2));
    BOOST_CHECK_EQUAL(8, quadTreeNodeIndex(1, 1, 2));

    BOOST_CHECK_EQUAL(9 , quadTreeNodeIndex(2, 0, 2));
    BOOST_CHECK_EQUAL(10, quadTreeNodeIndex(3, 0, 2));
    BOOST_CHECK_EQUAL(11, quadTreeNodeIndex(2, 1, 2));
    BOOST_CHECK_EQUAL(12, quadTreeNodeIndex(3, 1, 2));

    BOOST_CHECK_EQUAL(13, quadTreeNodeIndex(0, 2, 2));
    BOOST_CHECK_EQUAL(14, quadTreeNodeIndex(1, 2, 2));
    BOOST_CHECK_EQUAL(15, quadTreeNodeIndex(0, 3, 2));
    BOOST_CHECK_EQUAL(16, quadTreeNodeIndex(1, 3, 2));

    BOOST_CHECK_EQUAL(17, quadTreeNodeIndex(2, 2, 2));
    BOOST_CHECK_EQUAL(18, quadTreeNodeIndex(3, 2, 2));
    BOOST_CHECK_EQUAL(19, quadTreeNodeIndex(2, 3, 2));
    BOOST_CHECK_EQUAL(20, quadTreeNodeIndex(3, 3, 2));
}


BOOST_AUTO_TEST_SUITE_END()
