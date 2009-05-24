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
 * \brief Conversion utilities from 2D point clouds to a baked texture
 *
 * \author Michel Joron (?)
 * \author Chris Foster [chris42f (at) gmail (d0t) com]
 */

#ifndef BAKE_H_INCLUDED
#define BAKE_H_INCLUDED

#include <aqsis/aqsis.h>

namespace Aqsis {

/** \brief Convert a bake file to a TIFF file.
 *
 * \todo Modify this to return a baked texture buffer rather than saving to file.
 *
 * Main function to convert any bake to tif format
 * It used the standard standard definition of bake file defined in bake() from
 * Siggraph 2001.
 *
 * It created texture file of only 64x64 pixels not a lot but which some
 * changes in teqser we should be able to defined as much as the user needs.
 * Surprising 64x64 is good enough for small tests; I won't be surprised we
 * need more and agressive filtering (between subsamples s/t)
 */
void bakeToTiff(const char* inFileName, const char* outFileName, int bake);

} // namespace Aqsis

#endif // BAKE_H_INCLUDED
