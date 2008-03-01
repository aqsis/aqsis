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
 * \brief Texture file types.
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef IMAGEFILETYPE_H_INCLUDED
#define IMAGEFILETYPE_H_INCLUDED

namespace Aqsis {

/** \brief Enumerate all image file types which aqsistex knows about.
 */
enum EqImageFileType
{
	ImageFile_Tiff,
	ImageFile_Exr,
	ImageFile_Jpg,
	ImageFile_Png
};

/** \brief Convert an image file type to a string.
 */
const char* imageFileTypeToString(EqImageFileType type);


//==============================================================================
// Implementation details
//==============================================================================

inline const char* imageFileTypeToString(EqImageFileType type)
{
	switch(type)
	{
		case ImageFile_Tiff:
			return "tiff";
		case ImageFile_Exr:
			return "OpenExr";
		case ImageFile_Jpg:
			return "jpeg";
		case ImageFile_Png:
			return "png";
	}
	return "unknown";
}

} // namespace Aqsis

#endif // IMAGEFILETYPE_H_INCLUDED
