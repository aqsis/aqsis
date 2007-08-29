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
 * \brief Tests for mipmap utilities
 * \author Chris Foster
 */

#include <boost/test/auto_unit_test.hpp>

#include "mipmap.h"

using namespace Aqsis;

//------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(mipmapLevelSizes_test)
{
	typedef std::vector<TqUint> MipLengthVec;

	{
		MipLengthVec widths;
		MipLengthVec heights;
		mipmapLevelSizes(4, 5, widths, heights);
		TqUint desiredWidths[] = {4, 2, 1, 1};
		TqUint desiredHeights[] = {5, 3, 2, 1};
		MipLengthVec::size_type len = sizeof(desiredWidths)/sizeof(TqUint);
		BOOST_CHECK(widths == MipLengthVec(desiredWidths, desiredWidths+len));
		BOOST_CHECK(heights == MipLengthVec(desiredHeights, desiredHeights+len));
	}

//	{
//		MipLengthVec widths;
//		MipLengthVec heights;
//		mipmapLevelSizes(2, 1, widths, heights);
//		TqUint desiredWidths[] = {2, 1};
//		TqUint desiredHeights[] = {5, 3, 2, 1};
//		MipLengthVec::size_type len = sizeof(desiredWidths)/sizeof(TqUint);
//		BOOST_CHECK(widths == MipLengthVec(desiredWidths, desiredWidths+len));
//		BOOST_CHECK(heights == MipLengthVec(desiredHeights, desiredHeights+len));
//	}
}

