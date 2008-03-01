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
 * \brief Unit tests for functions to get magic numbers and match them against
 * the possible file types.
 *
 * \author Chris Foster
 */

#include "magicnumber.h"

#include <boost/test/auto_unit_test.hpp>
#include <sstream>

#include "tifffile_test.h"

BOOST_AUTO_TEST_CASE(getMagicNumber_test)
{
	std::istringstream inStream(stripTiffString);
	Aqsis::TqMagicNumberPtr magicNum = Aqsis::getMagicNumber(inStream);

	BOOST_CHECK_EQUAL(magicNum->size(), TqUint(Aqsis::magicNumberMaxBytes));
}


BOOST_AUTO_TEST_CASE(isTiffMagicNumber_test)
{
	std::istringstream inStream(stripTiffString);
	Aqsis::TqMagicNumberPtr magicNum = Aqsis::getMagicNumber(inStream);

	// Check that what should be a tiff magic number actually checks out as
	// one.
	BOOST_CHECK(Aqsis::isTiffMagicNumber(*magicNum));

	// Two checks for non-tiff magic numbers
	Aqsis::TqMagicNumber notTiffNum;
	notTiffNum.resize(1,0);
	BOOST_CHECK(!Aqsis::isTiffMagicNumber(notTiffNum));

	notTiffNum.resize(100,0);
	notTiffNum[0] = 'I';
	notTiffNum[1] = 'I';
	notTiffNum[2] = 0;
	notTiffNum[3] = 26;
	BOOST_CHECK(!Aqsis::isTiffMagicNumber(notTiffNum));
}

