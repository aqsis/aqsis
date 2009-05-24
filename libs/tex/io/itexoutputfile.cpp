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
 * \brief Implementation of a scanline-based output interface for texture files.
 *
 * \author Chris Foster
 */

#include <aqsis/tex/io/itexoutputfile.h>

#include <aqsis/util/exception.h>
#include "tiffoutputfile.h"

namespace Aqsis {

//------------------------------------------------------------------------------
namespace {

// Helper functions.

/** Open a file with the IqMultiTexOutputFile interface
 *
 * \return an open file, or NULL if the file type is invalid.
 */
boost::shared_ptr<IqMultiTexOutputFile> openMultiOutputFile(
		const boostfs::path& fileName, EqImageFileType fileType,
		const CqTexFileHeader& header)
{
	switch(fileType)
	{
		case ImageFile_Tiff:
			return boost::shared_ptr<IqMultiTexOutputFile>(
					new CqTiffOutputFile(fileName, header));
		// case ...:  // Add new output formats here!
		default:
			return boost::shared_ptr<IqMultiTexOutputFile>();
	}
}

}  // unnamed namespace

//------------------------------------------------------------------------------
// IqTexOutputFile implementation
boost::shared_ptr<IqTexOutputFile> IqTexOutputFile::open(
		const boostfs::path& fileName, EqImageFileType fileType,
		const CqTexFileHeader& header)
{
	// Check some of the header data to make sure it's minimally sane...
	if(header.width() <= 0 || header.height() <= 0)
	{
		AQSIS_THROW_XQERROR(XqInternal, EqE_BadFile, "Cannot open \"" << fileName
				<< "\" - image width and height cannot be negative or zero.");
	}
	if(header.channelList().numChannels() == 0)
	{
		AQSIS_THROW_XQERROR(XqInternal, EqE_BadFile, "Cannot open \"" << fileName
				<< "\" - no data channels present.");
	}

	// Create the new file object
	boost::shared_ptr<IqTexOutputFile> newFile
		= openMultiOutputFile(fileName, fileType, header);
	if(newFile)
		return newFile;

	switch(fileType)
	{
		// case ...:  // Add new output formats here!
		case ImageFile_Exr:
		case ImageFile_Jpg:
		case ImageFile_Png:
			AQSIS_THROW_XQERROR(XqInternal, EqE_Unimplement, "Cannot open \""
					<< fileName << "\" - unimplemented file type \"" << fileType << "\"");
		default:
			AQSIS_THROW_XQERROR(XqInternal, EqE_BadFile, "Cannot open \""
					<< fileName << "\" - unknown file type \"" << fileType << "\"");
	}

	return newFile;
}


//------------------------------------------------------------------------------
// IqMultiTexOutputFile implementation
boost::shared_ptr<IqMultiTexOutputFile> IqMultiTexOutputFile::open(
		const boostfs::path& fileName, EqImageFileType fileType,
		const CqTexFileHeader& header)
{
	boost::shared_ptr<IqMultiTexOutputFile> newFile
		= openMultiOutputFile(fileName, fileType, header);
	if(newFile)
	{
		return newFile;
	}
	else
	{
		AQSIS_THROW_XQERROR(XqInternal, EqE_Incapable, "Cannot open \""
				<< fileName << "\" - file type \"" << fileType << "\""
				<< " doesn't support multiple subimages");
	}
}

} // namespace Aqsis
