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

#ifndef MAGICNUMBER_H_INCLUDED
#define MAGICNUMBER_H_INCLUDED

#include <aqsis/aqsis.h>

#include <iosfwd>

#include <aqsis/util/file.h>
#include <aqsis/tex/io/imagefiletype.h>

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief Determine the type of a file using a "magic number"
 *
 * Many file formats have a distinctive pattern in the first few bytes which
 * can be matched to guess the file type.  This is rather more reliable than a
 * file suffix naming convention, but can still become confused.
 *
 * \throw XqInvalidFile if the file cannot be opened.
 *
 * \param fileName - file name to read from.
 * \return a guess at the file type based on the first few bytes.
 */
AQSIS_TEX_SHARE EqImageFileType guessFileType(const boostfs::path& fileName);

/** \brief Determine the type of an open file stream by using a "magic number"
 *
 * \see guessFileType(const boostfs::path& fileName)
 *
 * \param inStream - The first few bytes of this stream are read
 */
AQSIS_TEX_SHARE EqImageFileType guessFileType(std::istream& inStream);


//------------------------------------------------------------------------------
} // namespace Aqsis

#endif // MAGICNUMBER_H_INCLUDED
