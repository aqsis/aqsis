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
 * \brief Input interface for tiled images, implementation.
 *
 * \author Chris Foster
 */

#include "itiledtexinputfile.h"

#include "magicnumber.h"
#include "tiledtiffinputfile.h"
#include "texexception.h"

namespace Aqsis {

boost::shared_ptr<IqTiledTexInputFile> IqTiledTexInputFile::open(
		const std::string& fileName)
{
	EqImageFileType type = guessFileType(fileName);
	switch(type)
	{
		case ImageFile_Tiff:
			return boost::shared_ptr<IqTiledTexInputFile>(new
					CqTiledTiffInputFile(fileName));
		case ImageFile_Unknown:
			AQSIS_THROW(XqInvalidFile, "File \"" << fileName
					<< "\" is not a recognised image type");
			break;
		default:
			AQSIS_THROW(XqBadTexture, "Cannot open file \"" << fileName
					<< "\" of type " << imageFileTypeToString(type)
					<< " for tiled image I/O");
			break;
	}
	assert(0);
	return boost::shared_ptr<IqTiledTexInputFile>();
}

} // namespace Aqsis
