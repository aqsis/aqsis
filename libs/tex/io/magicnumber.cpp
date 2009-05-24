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
 * \brief Functions to get magic numbers and match them against the possible
 * file types.
 *
 * \author Chris Foster
 */

#include "magicnumber.h"

#include <algorithm>
#include <fstream>
#include <vector>

#include <aqsis/tex/texexception.h>

namespace Aqsis {

namespace {

/// Magic number type
typedef std::vector<char> TqMagicNumber;

/// Maximum number of bytes to read for a magic number
const TqInt magicNumberMaxBytes = 50;

TqMagicNumber getMagicNumber(std::istream& inStream)
{
	TqMagicNumber magicNumber(magicNumberMaxBytes,0);
	inStream.read(&magicNumber[0], magicNumberMaxBytes);
	// make sure we actually got the the requested number of bytes.
	TqInt numRead = inStream.gcount();
	if(numRead < magicNumberMaxBytes)
		magicNumber.resize(numRead);
	return magicNumber;
}

} // unnamed namespace


EqImageFileType guessFileType(const boostfs::path& fileName)
{
	std::ifstream inFile(fileName.file_string().c_str());
	if(!inFile)
	{
		AQSIS_THROW_XQERROR(XqInvalidFile, EqE_NoFile, "Cannot open file \""
				<< fileName << "\" for reading");
	}
	return guessFileType(inFile);
}

EqImageFileType guessFileType(std::istream& inStream)
{
	// load magic number
	TqMagicNumber magicNum = getMagicNumber(inStream);
	// Test magic number against various patterns to determine the file type.
	if( magicNum.size() >= 4
		&& (std::equal(magicNum.begin(), magicNum.begin()+4, "II\x2A\x00")
			|| std::equal(magicNum.begin(), magicNum.begin()+4, "MM\x00\x2A")) )
	{
		return ImageFile_Tiff;
	}
	else if( magicNum.size() >= 4
		&& std::equal(magicNum.begin(), magicNum.begin()+4, "v/1\x01") )
	{
		return ImageFile_Exr;
	}
	else if( magicNum.size() >= 16
		&& std::equal(magicNum.begin(), magicNum.begin()+15, "Aqsis bake file") )
	{
		return ImageFile_AqsisBake;
	}
	else if( magicNum.size() >= 11
		&& std::equal(magicNum.begin(), magicNum.begin()+11, "Aqsis ZFile") )
	{
		return ImageFile_AqsisZfile;
	}
	// Add further magic number matches here
	else
	{
		return ImageFile_Unknown;
	}
}

} // namespace Aqsis
