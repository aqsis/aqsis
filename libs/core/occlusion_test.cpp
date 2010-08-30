// Aqsis
// Copyright (C) 1997 - 2007, Paul C. Gregory
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

/** \file
 *
 * \brief Unit tests for occlusion tree
 * \author Chris Foster
 */

#include "occlusion.h"

#define BOOST_TEST_DYN_LINK
#include <boost/test/auto_unit_test.hpp>

namespace Aqsis
{
// Expose private methods of CqOcclusionTree for testing (ugh!)
struct CqOcclusionTree::Test
{
	static TqInt treeIndexForPoint(TqInt treeDepth, bool splitXFirst,
			TqInt x, TqInt y)
	{
		return CqOcclusionTree::treeIndexForPoint(treeDepth, splitXFirst, x, y);
	}
};
}

BOOST_AUTO_TEST_SUITE(occlusion_tests)

using namespace Aqsis;

BOOST_AUTO_TEST_CASE(treeIndexForPoint_test)
{
	typedef CqOcclusionTree::Test Test;
	/*
	 * The following diagrams represent the levels of a binary tree used for
	 * spatial subdivision.  The indices are the storage locations for the
	 * tree nodes of the areas in which they sit.
	 *
	 *                            x
	 *  --------------------------->
	 *
	 * +---------------------------+    ^
	 * |                           |    : y
	 * |                           |    :
	 * |                           |    :
	 * |             0             |    :
	 * |                           |    :
	 * |                           |    :
	 * |                           |    :
	 * +---------------------------+    :
	 */
    BOOST_CHECK_EQUAL(Test::treeIndexForPoint(1, true, 0, 0), 0);

	/*
	 * +---------------------------+
	 * |             |             |
	 * |             |             |
	 * |             |             |
	 * |      1      |      2      |
	 * |             |             |
	 * |             |             |
	 * |             |             |
	 * +---------------------------+
	 */
    BOOST_CHECK_EQUAL(Test::treeIndexForPoint(2, true, 0, 0), 1);
    BOOST_CHECK_EQUAL(Test::treeIndexForPoint(2, true, 1, 0), 2);

	/*
	 * +---------------------------+
	 * |             |             |
	 * |      4      |      6      |
	 * |             |             |
	 * |-------------|-------------|
	 * |             |             |
	 * |      3      |      5      |
	 * |             |             |
	 * +---------------------------+
	 */
    BOOST_CHECK_EQUAL(Test::treeIndexForPoint(3, true, 0, 0), 3);
    BOOST_CHECK_EQUAL(Test::treeIndexForPoint(3, true, 0, 1), 4);
    BOOST_CHECK_EQUAL(Test::treeIndexForPoint(3, true, 1, 0), 5);
    BOOST_CHECK_EQUAL(Test::treeIndexForPoint(3, true, 1, 1), 6);

	/* +---------------------------+
	 * |      |      |      |      |
	 * |   9  |  10  |  13  |  14  |
	 * |      |      |      |      |
	 * |-------------|-------------|
	 * |      |      |      |      |
	 * |   7  |  8   |  11  |  12  |
	 * |      |      |      |      |
	 * +---------------------------+
	 */
    BOOST_CHECK_EQUAL(Test::treeIndexForPoint(4, true, 0, 0), 7);
    BOOST_CHECK_EQUAL(Test::treeIndexForPoint(4, true, 1, 0), 8);
    BOOST_CHECK_EQUAL(Test::treeIndexForPoint(4, true, 2, 0), 11);
    BOOST_CHECK_EQUAL(Test::treeIndexForPoint(4, true, 3, 0), 12);
    BOOST_CHECK_EQUAL(Test::treeIndexForPoint(4, true, 0, 1), 9);
    BOOST_CHECK_EQUAL(Test::treeIndexForPoint(4, true, 1, 1), 10);
    BOOST_CHECK_EQUAL(Test::treeIndexForPoint(4, true, 2, 1), 13);
    BOOST_CHECK_EQUAL(Test::treeIndexForPoint(4, true, 3, 1), 14);

	// Check case where we split in the y-direction first.
    BOOST_CHECK_EQUAL(Test::treeIndexForPoint(4, false, 0, 0), 7);
    BOOST_CHECK_EQUAL(Test::treeIndexForPoint(4, false, 1, 0), 9);
    BOOST_CHECK_EQUAL(Test::treeIndexForPoint(4, false, 0, 1), 8);
    BOOST_CHECK_EQUAL(Test::treeIndexForPoint(4, false, 1, 1), 10);
    BOOST_CHECK_EQUAL(Test::treeIndexForPoint(4, false, 0, 2), 11);
    BOOST_CHECK_EQUAL(Test::treeIndexForPoint(4, false, 1, 2), 13);
    BOOST_CHECK_EQUAL(Test::treeIndexForPoint(4, false, 0, 3), 12);
    BOOST_CHECK_EQUAL(Test::treeIndexForPoint(4, false, 1, 3), 14);
}

BOOST_AUTO_TEST_SUITE_END()
