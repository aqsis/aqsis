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
