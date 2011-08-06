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
