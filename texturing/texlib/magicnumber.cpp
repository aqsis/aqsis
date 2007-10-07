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

#include <fstream>

#include "exception.h"

namespace Aqsis {

//------------------------------------------------------------------------------
// Functions to get magic numbers
TqMagicNumberPtr getMagicNumber(const std::string& fileName)
{
	std::ifstream inFile(fileName.c_str());
	if(!inFile)
		throw XqEnvironment("Cannot open file for reading", __FILE__, __LINE__);
	return getMagicNumber(inFile);
}

TqMagicNumberPtr getMagicNumber(std::istream& inStream)
{
	TqMagicNumberPtr magicNumber(new TqMagicNumber(magicNumberMaxBytes, 0) );
	inStream.read(&(*magicNumber)[0], magicNumberMaxBytes);
	// make sure we actually got the the requested number of bytes.
	TqInt numRead = inStream.gcount();
	if(numRead < magicNumberMaxBytes)
		magicNumber->resize(numRead);
	return magicNumber;
}


//------------------------------------------------------------------------------
// Functions to match magic numbers
bool isTiffMagicNumber(const TqMagicNumber& magicNum)
{
	if(magicNum.size() < 4)
		return false;
	// TIFF can have one of two possible magic numbers, depending on endianness.
	return (magicNum[0] == 'I' && magicNum[1] == 'I'
			&& magicNum[2] == 42 && magicNum[3] == 0)
		|| (magicNum[0] == 'M' && magicNum[1] == 'M'
			&& magicNum[2] == 0 && magicNum[3] == 42);
}

} // namespace Aqsis
