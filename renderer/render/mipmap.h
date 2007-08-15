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
 * \brief Classes and functions for creating mipmaps
 *
 * \author Chris Foster
 */

#ifndef MIPMAP_H_INCLUDED
#define MIPMAP_H_INCLUDED

#include "aqsis.h"

#include <vector>

namespace Aqsis
{

//------------------------------------------------------------------------------
/** \brief Determine the mipmap level sizes for a given image
 *
 * A mipmap consists of successive downscalings of the original file by
 * a factor of two.  The smallest mipmap level consists of a single
 * pixel (this is the convention taken in the OpenExr spec.)
 *
 * \param width  - image width
 * \param height - image height
 * \param levelWidths  - output vector of mipmap level widths
 * \param levelHeights - output vector of mipmap level heights
 */
void mipmapLevelSizes(TqUint width, TqUint height,
		std::vector<TqUint>& levelWidths, std::vector<TqUint>& levelHeights);

//------------------------------------------------------------------------------
} // namespace Aqsis

#endif // MIPMAP_H_INCLUDED
