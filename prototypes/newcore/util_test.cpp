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

#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include "util.h"

BOOST_AUTO_TEST_CASE(radicalInverse_test)
{
    const float e = 1e-5; // percentage tolerance.

    BOOST_CHECK_CLOSE(radicalInverse(0, 2), 0.0f, e);
    BOOST_CHECK_CLOSE(radicalInverse(1, 2), 0.5f, e);
    BOOST_CHECK_CLOSE(radicalInverse(2, 2), 0.25f, e);
    BOOST_CHECK_CLOSE(radicalInverse(3, 2), 0.75f, e);

    BOOST_CHECK_CLOSE(radicalInverse(1, 3), 1.0f/3, e);
    BOOST_CHECK_CLOSE(radicalInverse(4, 3), 1.0f/3+1.0f/9, e);
    BOOST_CHECK_CLOSE(radicalInverse(5, 3), 2.0f/3+1.0f/9, e);
}
