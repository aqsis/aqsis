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
 * \brief Interface wrapper functions and factory functions for texture file
 * interface.
 *
 * \author Chris Foster
 */

#include "itexinputfile.h"

#include "tiffinputfile.h"
#include "exrinputfile.h"
#include "magicnumber.h"
#include "logging.h"

namespace Aqsis {

namespace {

// Helper functions

// Open multi-image input file
boost::shared_ptr<IqMultiTexInputFile> openMultiInputFile(
		const TqMagicNumber& magicNum, const std::string& fileName)
{
	boost::shared_ptr<IqMultiTexInputFile> file;
	if(isTiffMagicNumber(magicNum))
		file.reset(new CqTiffInputFile(fileName));
	// else if(Add new formats here!)
	return file;
}

// Open a simple input file
boost::shared_ptr<IqTexInputFile> openInputFile(
		const TqMagicNumber& magicNum, const std::string& fileName)
{
	boost::shared_ptr<IqTexInputFile> file = openMultiInputFile(magicNum, fileName);
	if(!file)
	{
		if(isOpenExrMagicNumber(magicNum))
		{
#		ifdef USE_OPENEXR
			file.reset(new CqExrInputFile(fileName));
#		else
			AQSIS_THROW(XqInvalidFile, "Cannot open file \"" << fileName
					<< "\": Aqsis was compiled without OpenEXR support");
#		endif
		}
		// else if(Add new formats here!)
	}
	return file;
}

} // unnamed namespace 

//------------------------------------------------------------------------------
// IqTexInputFile

boost::shared_ptr<IqTexInputFile> IqTexInputFile::open(const std::string& fileName)
{
	boost::shared_ptr<IqTexInputFile> newFile;
	TqMagicNumberPtr magicNum = getMagicNumber(fileName);
	boost::shared_ptr<IqTexInputFile> file = openInputFile(*magicNum, fileName);
	if(file)
		return file;
	else
		AQSIS_THROW(XqInvalidFile, "Unknown file type for \"" << fileName << "\"");

	return newFile;
}

//------------------------------------------------------------------------------
// IqMultiTexInputFile

boost::shared_ptr<IqMultiTexInputFile> IqMultiTexInputFile::open(const std::string& fileName)
{
	boost::shared_ptr<IqMultiTexInputFile> newFile;
	TqMagicNumberPtr magicNum = getMagicNumber(fileName);

	boost::shared_ptr<IqMultiTexInputFile> file = openMultiInputFile(*magicNum, fileName);
	if(file)
		return file;
	else
		AQSIS_THROW(XqInvalidFile, "Unknown file type for \"" << fileName << "\"");

	return newFile;
}

} // namespace Aqsis
