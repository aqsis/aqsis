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
	std::ifstream inFile(native(fileName).c_str());
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
	else if( magicNum.size() >= 4 &&
            magicNum[0] == (char)0x89 &&
            magicNum[1] == 'P' &&
            magicNum[2] == 'N' &&
            magicNum[3] == 'G')
    {
        return ImageFile_Png;
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
