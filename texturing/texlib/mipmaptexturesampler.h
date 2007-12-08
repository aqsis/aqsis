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
 * \brief Declare a multiresolution texture sampler
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef MIPMAPTEXTURESAMPLER_H_INCLUDED
#define MIPMAPTEXTURESAMPLER_H_INCLUDED

#include "aqsis.h"

#include <boost/shared_ptr.hpp>

#include "itexturesampler.h"
#include "levelsamplercache.h"

namespace Aqsis {

/** \brief A mulitresolution (mipmapped) texture sampler class
 *
 * This class uses a set of power-of-two downsampled images (a typical
 * mipmap) to make texture filtering efficient.  When filtering, a mipmap level
 * is chosen based on the extent of the filter quadrilateral.  Here we make a
 * choice which eliminates aliasing and blurring: we choose a mipmap level such
 * that the thinnest direction of the sampling quad falls across more than one
 * pixel.
 *
 * \todo: Investigate optimal mipmap level selection.
 */
class CqMipmapTextureSampler : public IqTextureSampler
{
	public:
		CqMipmapTextureSampler(const boost::shared_ptr<CqLevelSamplerCache>& levels);

		// from IqTextureSampler
		virtual void filter(const SqSampleQuad& sampleQuad,
				const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const;
		virtual const CqTextureSampleOptions& defaultSampleOptions() const;
	private:
		boost::shared_ptr<CqLevelSamplerCache> m_levels;
};

} // namespace Aqsis

#endif // MIPMAPTEXTURESAMPLER_H_INCLUDED
