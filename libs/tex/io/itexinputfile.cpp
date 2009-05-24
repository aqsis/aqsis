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

#include <aqsis/tex/io/itexinputfile.h>

#include "exrinputfile.h"
#include <aqsis/util/logging.h>
#include "magicnumber.h"
#include "tiffinputfile.h"
#include "zinputfile.h"

namespace Aqsis {

namespace {

// Helper functions

// Open multi-image input file
boost::shared_ptr<IqMultiTexInputFile> openMultiInputFile(
		EqImageFileType type, const boostfs::path& fileName)
{
	boost::shared_ptr<IqMultiTexInputFile> file;
	switch(type)
	{
		case ImageFile_Tiff:
			file.reset(new CqTiffInputFile(fileName));
			break;
		default:
			break;
	}
	return file;
}

// Open a simple input file
boost::shared_ptr<IqTexInputFile> openInputFile(
		EqImageFileType type, const boostfs::path& fileName)
{
	boost::shared_ptr<IqTexInputFile> file = openMultiInputFile(type, fileName);
	if(!file)
	{
		switch(type)
		{
			case ImageFile_Exr:
#				ifdef USE_OPENEXR
				file.reset(new CqExrInputFile(fileName));
#				else
				AQSIS_THROW_XQERROR(XqInvalidFile, EqE_Unimplement,
						"Cannot open file \"" << fileName << "\""
						": Aqsis was compiled without OpenEXR support");
#				endif
				break;
			case ImageFile_AqsisZfile:
				file.reset(new CqZInputFile(fileName));
			default:
				break;
		}
	}
	return file;
}

} // unnamed namespace 

//------------------------------------------------------------------------------
// IqTexInputFile

boost::shared_ptr<IqTexInputFile> IqTexInputFile::open(const boostfs::path& fileName)
{
	boost::shared_ptr<IqTexInputFile> file = openInputFile(guessFileType(fileName), fileName);
	if(file)
		return file;
	else
		AQSIS_THROW_XQERROR(XqInvalidFile, EqE_BadFile, "Unknown file type for \""
				<< fileName << "\"");

	assert(0);
	return boost::shared_ptr<IqTexInputFile>();
}

//------------------------------------------------------------------------------
// IqMultiTexInputFile

boost::shared_ptr<IqMultiTexInputFile> IqMultiTexInputFile::open(
		const boostfs::path& fileName)
{
	EqImageFileType type = guessFileType(fileName);
	boost::shared_ptr<IqMultiTexInputFile> file
		= openMultiInputFile(type, fileName);
	if(file)
		return file;
	else
	{
		AQSIS_THROW_XQERROR(XqInvalidFile, EqE_BadFile,
			"File \"" << fileName << "\" of type " << type << " doesn't support "
			"multiple subimages.");
	}

	assert(0);
	return boost::shared_ptr<IqMultiTexInputFile>();
}

} // namespace Aqsis
