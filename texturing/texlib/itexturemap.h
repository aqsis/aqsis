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
 * \brief Declare a filtered texture mapping class
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef ITEXTUREMAP2_H_INCLUDED
#define ITEXTUREMAP2_H_INCLUDED

#include "aqsis.h"

#include "samplequad.h"
#include "texturesampleoptions.h"

namespace Aqsis {

// Forward declarations
class CqTexFileHeader;

//------------------------------------------------------------------------------
/* \brief Texture map sampling interface.
 *
 * This interface is designed to support the sampling of "plain" 2D textures
 * using sample options which are relevant to the renderman shading language.
 * A "plain" texture is a simple mapping from 2D space to a colour or other
 * quantity representable by a short vector of components.
 *
 * The sample options allow for control over the way an implementation performs
 * filtering, and correspond closely to the extra parameters which may be
 * expected in calls to the the RSL texture() function.
 *
 * Direct access to attributes of the underlying texture file is provided via
 * the fileAttributes() method.
 *
 */
class AQSISTEX_SHARE IqTextureMap
{
	public:
		/** \brief Get the texture attributes of the underlying file.
		 *
		 * This function allows access to the texture file attributes such as
		 * transformation matrices, image resolution etc.
		 *
		 * \return Underlying file attributes, or 0 if there isn't an
		 * underlying file.
		 */
		virtual inline const CqTexFileHeader* fileAttributes() const = 0;

		/** \brief Sample the texture map.
		 *
		 * \param sampleQuad - quadrilateral region to sample over
		 * \param outSamples - the output samples will be placed here.
		 */
		virtual inline void sampleMap(const SqSampleQuad& sampleQuad,
				TqFloat* outSamples) const = 0;

		/** \brief Get the current sample options
		 */
		virtual inline CqTextureSampleOptions& sampleOptions() = 0;
		/** \brief Get the current sample options (const version)
		 */
		virtual inline const CqTextureSampleOptions& sampleOptions() const = 0;

		virtual ~IqTextureMap() {}
};

} // namespace Aqsis

#endif // ITEXTUREMAP2_H_INCLUDED
