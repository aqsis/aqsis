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

#include "vartable.h"

#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/auto_unit_test.hpp>

BOOST_AUTO_TEST_CASE(VarTable_test)
{
    VarTable table;
    VarSpec Ci(VarSpec::Color, 1, ustring("Ci"));
    VarSpec s(VarSpec::Float, 1, ustring("s"));
    VarSpec t(VarSpec::Float, 1, ustring("t"));
    VarSpec st(VarSpec::Float, 2, ustring("st"));
    VarId Ci_id = table.getId(Ci);
    VarId s_id = table.getId(s);
    VarId t_id = table.getId(t);
    VarId st_id = table.getId(st);

    BOOST_CHECK_EQUAL(table.getVar(Ci_id), Ci);
    BOOST_CHECK_EQUAL(table.getVar(s_id), s);
    BOOST_CHECK_EQUAL(table.getVar(t_id), t);
    BOOST_CHECK_EQUAL(table.getVar(st_id), st);

    BOOST_CHECK_EQUAL(table.getId(Ci), Ci_id);
}

BOOST_AUTO_TEST_CASE(UVarSpec_test)
{
    VarSpec Ci_spec(VarSpec::Color, 1, ustring("Ci"));
    VarSpec s_spec(VarSpec::Float, 1, ustring("s"));
    VarSpec t_spec(VarSpec::Float, 1, ustring("t"));
    VarSpec st_spec(VarSpec::Float, 2, ustring("st"));

    UVarSpec Ci(Ci_spec);
    UVarSpec s(s_spec);
    UVarSpec t(t_spec);
    UVarSpec st(st_spec);

    BOOST_CHECK_EQUAL(*Ci, Ci_spec);
    BOOST_CHECK_EQUAL(Ci->type, VarSpec::Color);
    BOOST_CHECK_EQUAL(Ci->arraySize, 1);
    BOOST_CHECK_EQUAL(Ci->name, "Ci");

    BOOST_CHECK_NE(Ci, s);
}
