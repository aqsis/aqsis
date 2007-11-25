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
 * \brief Interface to texture buffer sampling machinery.
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef ITEXTURESAMPLER_H_INCLUDED
#define ITEXTURESAMPLER_H_INCLUDED

#include "aqsis.h"

#include <valarray>

#include "tilearray.h"
#include "aqsismath.h"
#include "samplequad.h"
#include "texturesampleoptions.h"
#include "random.h"

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief A class which knows how to sample texture buffers.
 *
 */
class IqTextureSampler
{
	public:
		/** \brief Sample the texture with the provided sample options.
		 *
		 * \param sampleQuad - quadrilateral region to sample over
		 * \param sampleOpts - options to the sampler, including filter widths etc.
		 * \param outSamps - the outSamps samples will be placed here.  
		 */
		virtual void filter(const SqSampleQuad& sampleQuad,
				const SqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const = 0;

		/** \brief Create and return a IqTextureSampler derived class
		 *
		 * The returned class is a CqTextureSamplerImpl<T> where T is a type
		 * appropriate to the pixel type held in the file.
		 *
		 * \param file - tiled texture file which the sampler should be connected to.
		 */
		static boost::shared_ptr<IqTextureSampler> create(
				const boost::shared_ptr<IqTiledTexInputFile>& file);
		virtual ~IqTextureSampler() {}
};

} // namespace Aqsis

#endif // ITEXTURESAMPLER_H_INCLUDED
