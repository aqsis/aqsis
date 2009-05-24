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
 * \brief Declare some testing fixtures for binary TIFF data.
 *
 * Here we declare some inline binary data for use in TIFF-related unit tests
 * rather than relying on external files.  Two different sets of data are
 * provided - one in strips and the other in tiles.
 *
 * See the generating code in tifftest_examples.cpp for the layout and
 * content of the tiffs.
 *
 * \author Chris Foster
 */

#ifndef TIFFFILE_TEST_H_INCLUDED
#define TIFFFILE_TEST_H_INCLUDED

#include <sstream>
#include <string>

#include "boost/shared_ptr.hpp"

#include <aqsis/util/smartptr.h>
#include "tiffdirhandle.h"

//------------------------------------------------------------------------------
// Fixtures which wrap up some binary tiff data.

// Tiff data stored in strips
const unsigned char stripTiffData[] =
{
73,73,42,0,38,0,0,0,120,156,251,207,128,14,254,99,136,0,0,53,238,1,255,120,156,
99,96,248,207,128,14,208,69,0,49,242,1,255,14,0,0,1,3,0,1,0,0,0,6,0,0,0,1,1,
3,0,1,0,0,0,4,0,0,0,2,1,3,0,3,0,0,0,212,0,0,0,3,1,3,0,1,0,0,0,178,128,0,0,6,
1,3,0,1,0,0,0,2,0,0,0,14,1,2,0,36,0,0,0,218,0,0,0,17,1,4,0,2,0,0,0,254,0,0,0,
18,1,3,0,1,0,0,0,1,0,0,0,21,1,3,0,1,0,0,0,3,0,0,0,22,1,3,0,1,0,0,0,2,0,0,0,23,
1,4,0,2,0,0,0,6,1,0,0,28,1,3,0,1,0,0,0,1,0,0,0,61,1,3,0,1,0,0,0,2,0,0,0,83,1,
3,0,3,0,0,0,14,1,0,0,48,1,0,0,8,0,8,0,8,0,83,116,114,105,112,45,97,108,108,111,
99,97,116,101,100,32,116,105,102,102,32,102,111,114,32,117,110,105,116,32,116,
101,115,116,115,0,8,0,0,0,23,0,0,0,15,0,0,0,15,0,0,0,1,0,1,0,1,0,120,156,251,
255,255,63,3,42,248,143,33,2,0,155,136,5,251,120,156,99,96,32,12,0,0,36,0,1,
13,0,0,1,3,0,1,0,0,0,6,0,0,0,1,1,3,0,1,0,0,0,4,0,0,0,2,1,3,0,3,0,0,0,210,1,0,
0,3,1,3,0,1,0,0,0,178,128,0,0,6,1,3,0,1,0,0,0,2,0,0,0,17,1,4,0,2,0,0,0,216,1,
0,0,18,1,3,0,1,0,0,0,1,0,0,0,21,1,3,0,1,0,0,0,3,0,0,0,22,1,3,0,1,0,0,0,2,0,0,
0,23,1,4,0,2,0,0,0,224,1,0,0,28,1,3,0,1,0,0,0,1,0,0,0,61,1,3,0,1,0,0,0,2,0,0,
0,83,1,3,0,3,0,0,0,232,1,0,0,0,0,0,0,8,0,8,0,8,0,20,1,0,0,37,1,0,0,17,0,0,0,
11,0,0,0,1,0,1,0,1,0
};
const std::string stripTiffString(stripTiffData, stripTiffData+sizeof(stripTiffData));

// Tiff data stored in tiles
const unsigned char tileTiffData[] =
{
73,73,42,0,24,0,0,0,120,156,99,96,24,5,163,96,228,2,0,3,0,0,1,0,14,0,0,1,3,0,
1,0,0,0,4,0,0,0,1,1,3,0,1,0,0,0,5,0,0,0,2,1,3,0,3,0,0,0,198,0,0,0,3,1,3,0,1,
0,0,0,178,128,0,0,6,1,3,0,1,0,0,0,2,0,0,0,18,1,3,0,1,0,0,0,1,0,0,0,21,1,3,0,
1,0,0,0,3,0,0,0,28,1,3,0,1,0,0,0,1,0,0,0,61,1,3,0,1,0,0,0,2,0,0,0,66,1,3,0,1,
0,0,0,16,0,0,0,67,1,3,0,1,0,0,0,16,0,0,0,68,1,4,0,1,0,0,0,8,0,0,0,69,1,4,0,1,
0,0,0,15,0,0,0,83,1,3,0,3,0,0,0,204,0,0,0,226,0,0,0,8,0,8,0,8,0,1,0,1,0,1,0,
120,156,99,96,24,5,163,96,228,2,0,3,0,0,1,0,14,0,0,1,3,0,1,0,0,0,2,0,0,0,1,1,
3,0,1,0,0,0,3,0,0,0,2,1,3,0,3,0,0,0,144,1,0,0,3,1,3,0,1,0,0,0,178,128,0,0,6,
1,3,0,1,0,0,0,2,0,0,0,18,1,3,0,1,0,0,0,1,0,0,0,21,1,3,0,1,0,0,0,3,0,0,0,28,1,
3,0,1,0,0,0,1,0,0,0,61,1,3,0,1,0,0,0,2,0,0,0,66,1,3,0,1,0,0,0,16,0,0,0,67,1,
3,0,1,0,0,0,16,0,0,0,68,1,4,0,1,0,0,0,210,0,0,0,69,1,4,0,1,0,0,0,15,0,0,0,83,
1,3,0,3,0,0,0,150,1,0,0,172,1,0,0,8,0,8,0,8,0,1,0,1,0,1,0,120,156,99,96,24,5,
163,96,228,2,0,3,0,0,1,0,14,0,0,1,3,0,1,0,0,0,1,0,0,0,1,1,3,0,1,0,0,0,2,0,0,
0,2,1,3,0,3,0,0,0,90,2,0,0,3,1,3,0,1,0,0,0,178,128,0,0,6,1,3,0,1,0,0,0,2,0,0,
0,18,1,3,0,1,0,0,0,1,0,0,0,21,1,3,0,1,0,0,0,3,0,0,0,28,1,3,0,1,0,0,0,1,0,0,0,
61,1,3,0,1,0,0,0,2,0,0,0,66,1,3,0,1,0,0,0,16,0,0,0,67,1,3,0,1,0,0,0,16,0,0,0,
68,1,4,0,1,0,0,0,156,1,0,0,69,1,4,0,1,0,0,0,15,0,0,0,83,1,3,0,3,0,0,0,96,2,0,
0,118,2,0,0,8,0,8,0,8,0,1,0,1,0,1,0,120,156,99,96,24,5,163,96,228,2,0,3,0,0,
1,0,14,0,0,1,3,0,1,0,0,0,1,0,0,0,1,1,3,0,1,0,0,0,1,0,0,0,2,1,3,0,3,0,0,0,36,
3,0,0,3,1,3,0,1,0,0,0,178,128,0,0,6,1,3,0,1,0,0,0,2,0,0,0,18,1,3,0,1,0,0,0,1,
0,0,0,21,1,3,0,1,0,0,0,3,0,0,0,28,1,3,0,1,0,0,0,1,0,0,0,61,1,3,0,1,0,0,0,2,0,
0,0,66,1,3,0,1,0,0,0,16,0,0,0,67,1,3,0,1,0,0,0,16,0,0,0,68,1,4,0,1,0,0,0,102,
2,0,0,69,1,4,0,1,0,0,0,15,0,0,0,83,1,3,0,3,0,0,0,42,3,0,0,0,0,0,0,8,0,8,0,8,
0,1,0,1,0,1,0
};
const std::string tileTiffString(tileTiffData, tileTiffData+sizeof(tileTiffData));

// Fixture which includes a CqTiffFileHandle allocated using a std::string as
// the "file" buffer
struct TiffFileHandleFixture
{
	std::istringstream tiffStream;
	Aqsis::CqTiffFileHandle fileHandle;
	boost::shared_ptr<Aqsis::CqTiffFileHandle> fileHandlePtr;
	
	TiffFileHandleFixture(const std::string& data)
		: tiffStream(data),
		fileHandle(tiffStream),
		fileHandlePtr(&fileHandle, Aqsis::nullDeleter)
	{ }
};

#endif // TIFFFILE_TEST_H_INCLUDED
