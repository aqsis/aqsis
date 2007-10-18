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

#include "aqsis.h"

#include <istream>
#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>

namespace Aqsis {

//------------------------------------------------------------------------------
/// Magic number type
typedef std::vector<char> TqMagicNumber;
/// Pointer to magic number
typedef boost::shared_ptr<TqMagicNumber> TqMagicNumberPtr;

/// Maximum number of bytes to read for a magic number
const TqInt magicNumberMaxBytes = 50;

//------------------------------------------------------------------------------
/// \name Functions to get magic numbers
//@{
/** \brief Get a magic number
 *
 * Attempt to read in the first few bytes of a file to serve as a "magic
 * number" which can be used for file type detection.
 *
 * \throw XqEnvironment if the file cannot be opened.
 *
 * \param fileName - file name to read from.
 * \return a vector containing the magic number as a sequence of bytes.
 */
TqMagicNumberPtr getMagicNumber(const std::string& fileName);

/** \brief Get a magic number from a stream
 *
 * Attempt to read in the first few bytes of a stream to serve as a "magic
 * number" which can be used for file type detection.
 *
 * \param inStream - stream to read from
 * \return a vector containing the magic number as a sequence of bytes.
 */
TqMagicNumberPtr getMagicNumber(std::istream& inStream);

//@}


//------------------------------------------------------------------------------
/// \name Functions to match magic numbers
//@{
/** Determine if the magic number indicates a tiff file.
 *
 * \return true if magicNum is a TIFF magic number.
 */
bool isTiffMagicNumber(const TqMagicNumber& magicNum);

/** Determine if the magic number indicates an OpenEXR file.
 *
 * \return true if magicNum is an OpenEXR magic number.
 */
bool isOpenExrMagicNumber(const TqMagicNumber& magicNum);

// --> Add further file type detection functions here <--

//@}

//------------------------------------------------------------------------------
} // namespace Aqsis

#endif // MAGICNUMBER_H_INCLUDED
