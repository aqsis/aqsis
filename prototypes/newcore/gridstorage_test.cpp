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

#include "gridstorage.h"

#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/auto_unit_test.hpp>

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
