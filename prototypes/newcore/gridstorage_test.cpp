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

#include "gridstorage.h"

#define BOOST_TEST_MAIN
#ifndef _WIN32
#define BOOST_TEST_DYN_LINK
#endif
#include <boost/test/auto_unit_test.hpp>

using namespace Aqsis;

BOOST_AUTO_TEST_CASE(GridStorageBuilder_unique_test)
{
    GridStorageBuilder builder;
    builder.add(Stdvar::P, GridStorage::Uniform);
    builder.add(Stdvar::Cs, GridStorage::Uniform);
    builder.add(Stdvar::P, GridStorage::Uniform);
    builder.add(Stdvar::Ci, GridStorage::Uniform);
    builder.add(Stdvar::P, GridStorage::Uniform);

    GridStoragePtr stor = builder.build(10);
    BOOST_CHECK_EQUAL(stor->varSet().size(), 3);
    BOOST_CHECK(stor->varSet().contains(Stdvar::P));
    BOOST_CHECK(stor->varSet().contains(Stdvar::Cs));
    BOOST_CHECK(stor->varSet().contains(Stdvar::Ci));
}

BOOST_AUTO_TEST_CASE(GridStorageBuilder_precedence_test)
{
    {
        GridStorageBuilder builder;
        // Add P as uniform
        builder.add(Stdvar::P, GridStorage::Uniform);
        builder.setFromGeom();
        // then as varying, which should override the uniform case.
        builder.add(Stdvar::P, GridStorage::Varying);
        GridStoragePtr stor = builder.build(10);
        BOOST_REQUIRE_EQUAL(stor->varSet().size(), 1);
        BOOST_CHECK(!stor->get(0).uniform());
    }

    {
        GridStorageBuilder builder;
        // Add P as varying
        builder.add(Stdvar::P, GridStorage::Varying);
        builder.setFromGeom();
        // then as uniform, which should override the varying case.
        builder.add(Stdvar::P, GridStorage::Uniform);
        GridStoragePtr stor = builder.build(10);
        BOOST_REQUIRE_EQUAL(stor->varSet().size(), 1);
        BOOST_CHECK(stor->get(0).uniform());
    }
}

BOOST_AUTO_TEST_CASE(GridStorage_maxAggregateSize_test)
{
    GridStorageBuilder builder;
    builder.add(Stdvar::P, GridStorage::Varying);
    builder.add(VarSpec(VarSpec::Float, 42, ustring("f")), GridStorage::Varying);
    GridStoragePtr stor = builder.build(10);

    BOOST_CHECK_EQUAL(stor->maxAggregateSize(), 42);
}

BOOST_AUTO_TEST_CASE(GridStorage_P_test)
{
    GridStorageBuilder builder;
    builder.add(Stdvar::P, GridStorage::Varying);
    GridStoragePtr stor = builder.build(10);
    // Check that P is present.
    BOOST_CHECK(stor->P());
}

BOOST_AUTO_TEST_CASE(GridStorage_get_test)
{
    GridStorageBuilder builder;
    builder.add(Stdvar::P, GridStorage::Varying);
    VarSpec f(VarSpec::Float, 42, ustring("f"));
    builder.add(f, GridStorage::Varying);
    GridStoragePtr stor = builder.build(10);
    // Check that P and f are present
    BOOST_CHECK(stor->get(Stdvar::P));
    BOOST_CHECK(stor->get(f));
    // Check that a random other variable isn't present
    BOOST_CHECK(!stor->get(VarSpec(VarSpec::Float, 1, ustring("f"))));
}
