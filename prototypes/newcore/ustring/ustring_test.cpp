/*
  Copyright 2008 Larry Gritz and the other authors and contributors.
  All Rights Reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:
  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
  * Neither the name of the software's owners nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  (This is the Modified BSD License)
*/

#include "ustring.h"

#define BOOST_TEST_MAIN
#ifndef _WIN32
#define BOOST_TEST_DYN_LINK
#endif
#include <boost/test/auto_unit_test.hpp>

BOOST_AUTO_TEST_CASE(ustring_cmp_test)
{
    // Test comparisons between strings
    ustring blah("blah");
    ustring asdf("asdf");
    BOOST_CHECK_EQUAL(blah, "blah");
    BOOST_CHECK_LT(asdf, blah);
    BOOST_CHECK_NE(asdf, blah);
}

BOOST_AUTO_TEST_CASE(ustring_container_test)
{
    // Check correctness of container-like operations
    ustring s("blah");
    BOOST_CHECK_EQUAL(s[0], 'b');
    BOOST_CHECK_EQUAL(s[3], 'h');
    BOOST_CHECK_EQUAL(s.size(), size_t(4));
    // check iterators
    {
        std::string extract(s.begin(), s.end());
        BOOST_CHECK_EQUAL(extract, "blah");
    }
    {
        std::string extract(s.rbegin(), s.rend());
        BOOST_CHECK_EQUAL(extract, "halb");
    }
}

BOOST_AUTO_TEST_CASE(ustring_empty_test)
{
    ustring emptystr;
    BOOST_CHECK_EQUAL(emptystr, "");
    BOOST_CHECK(emptystr.empty());
}

BOOST_AUTO_TEST_CASE(ustring_unique_test)
{
    // Use two copies to make sure the compiler can't coalesce the identicle
    // string constants
    char s1[] = "a string";
    char s2[] = "a string";
    // For good measure, prove to ourselves that the pointers really are
    // different
    BOOST_CHECK_NE((void*)s1, (void*)s2);
    ustring us1(s1);
    ustring us2(s2);
    // Now check that the strings have been correctly uniqueified
    BOOST_CHECK_EQUAL(us1, us2);
}


// FIXME: When threading is enabled, import some ustring threading tests from
// OIIO.

