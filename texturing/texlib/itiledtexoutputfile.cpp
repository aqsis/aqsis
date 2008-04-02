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

#include "itiledtexoutputfile.h"

//#include "tiledtiffoutputfile.h"

namespace Aqsis {

//------------------------------------------------------------------------------
boost::shared_ptr<IqTiledTexOutputFile>
IqTiledTexOutputFile::open(const std::string& fileName,
		EqImageFileType fileType, const CqTexFileHeader& header)
{
	// Check some of the header data to make sure it's minimally sane...
	if(header.width() <= 0 || header.height() <= 0)
	{
		AQSIS_THROW(XqInternal, "Cannot open \"" << fileName
				<< "\": Image width and height cannot be negative or zero.");
	}
	if(header.channelList().numChannels() == 0)
	{
		AQSIS_THROW(XqInternal, "Cannot open \"" << fileName
				<< "\": No data channels present.");
	}

	// Create the new file object
	boost::shared_ptr<IqTiledTexOutputFile> newFile;

	switch(fileType)
	{
//		case ImageFile_Tiff:
//			newFile.reset(new CqTiledTiffOutputFile(fileName, header));
//			break;
		// case ...:  // Add new output formats here!
		default:
			AQSIS_THROW(XqInternal, "Cannot open \"" << fileName
					<< "\": Unknown file type \""
					<< imageFileTypeToString(fileType) << "\"");
	}

	return newFile;
}

} // namespace Aqsis
