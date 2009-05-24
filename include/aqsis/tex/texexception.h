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
 * \brief Exception classes for use with textures.
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef TEXEXCEPTIONS_H_INCLUDED
#define TEXEXCEPTIONS_H_INCLUDED

#include <aqsis/aqsis.h>

#include <aqsis/util/exception.h>

namespace Aqsis {

/** \class XqBadTexture
 * \brief An exception indicating that something is broken about a texture.
 *
 * Examples:
 *   * Trying to load a texture file as a mipmap when it's got the wrong
 *     number/shape of subtextures.
 *   * Trying to use a non-floating point texture as a shadow map.
 */
AQSIS_DECLARE_XQEXCEPTION(XqBadTexture, XqInternal);

} // namespace Aqsis

#endif // TEXEXCEPTIONS_H_INCLUDED
