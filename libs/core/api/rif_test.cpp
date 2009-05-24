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
 * \brief Tests for the Rif interface
 * \author Chris Foster
 */

#include <aqsis/ri/rif.h>
#include <aqsis/ri/ri.h>

#ifndef	AQSIS_SYSTEM_WIN32
#define BOOST_TEST_DYN_LINK
#endif //AQSIS_SYSTEM_WIN32

#include <boost/test/auto_unit_test.hpp>

BOOST_AUTO_TEST_CASE(RifGetDeclaration_test)
{
	RifTokenType tokType;
	RifTokenDetail tokDetail;
	RtInt arrayLen;
	RtInt ret;

	// Check that inline declarations are parsed correctly in the absence of a
	// context.
	ret = RifGetDeclaration("constant color[3] myCol",
			&tokType, &tokDetail, &arrayLen);
	BOOST_CHECK_EQUAL(ret, 0);
	BOOST_CHECK_EQUAL(tokType, k_RifColor);
	BOOST_CHECK_EQUAL(tokDetail, k_RifConstant);
	BOOST_CHECK_EQUAL(arrayLen, 3);

	// Check that without a context, we can't look up "P"
	ret = RifGetDeclaration("P", &tokType, &tokDetail, &arrayLen);
	BOOST_CHECK_EQUAL(ret, 1);

	// Now initialise a context.
	RiBegin(RI_NULL);
	// Check that declaring and then looking up the decl works.
	RiDeclare("myMat", "facevertex matrix");
	ret = RifGetDeclaration("myMat", &tokType, &tokDetail, &arrayLen);
	BOOST_CHECK_EQUAL(ret, 0);
	BOOST_CHECK_EQUAL(tokType, k_RifMatrix);
	BOOST_CHECK_EQUAL(tokDetail, k_RifFaceVertex);
	BOOST_CHECK_EQUAL(arrayLen, 1);

	// Check that inbuilt decls can be found.
	ret = RifGetDeclaration("P", &tokType, &tokDetail, &arrayLen);
	BOOST_CHECK_EQUAL(ret, 0);
	BOOST_CHECK_EQUAL(tokType, k_RifPoint);
	BOOST_CHECK_EQUAL(tokDetail, k_RifVertex);
	BOOST_CHECK_EQUAL(arrayLen, 1);

	// Check that non-declared primvars can't be found.
	ret = RifGetDeclaration("nonexistant", &tokType, &tokDetail, &arrayLen);
	BOOST_CHECK_EQUAL(ret, 1);

	RiEnd();
}

