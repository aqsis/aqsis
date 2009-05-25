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
 * \brief Interface to occlusion map sampling machinery.
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef IOCCLUSIONSAMPLER_H_INCLUDED
#define IOCCLUSIONSAMPLER_H_INCLUDED

#include <aqsis/aqsis.h>

#include <boost/shared_ptr.hpp>

#include <aqsis/math/matrix.h>
#include <aqsis/tex/filtering/samplequad.h>
#include <aqsis/tex/filtering/texturesampleoptions.h>

namespace Aqsis {

class IqTiledTexInputFile;

//------------------------------------------------------------------------------
/** \brief An interface for sampling occlusion texture buffers.
 */
class AQSIS_TEX_SHARE IqOcclusionSampler
{
	public:
		/** \brief Sample the texture over a parallelogram region
		 *
		 * \param samplePllgram - parallelogram region to sample over
		 * \param sampleOpts - options to the sampler, including filter widths etc.
		 * \param outSamps - the outSamps samples will be placed here.  
		 */
		virtual void sample(const Sq3DSamplePllgram& samplePllgram,
				const CqVector3D& normal, const CqShadowSampleOptions& sampleOpts,
				TqFloat* outSamps) const = 0;

		/** \brief Get the default sample options for this texture.
		 *
		 * The default implementation returns texture sample options
		 * initialized by the default CqShadowSampleOptions constructor.
		 *
		 * \return The default sample options - in the case that the texture
		 * originates from an underlying file, these should include options
		 * which were used when the texture was created.
		 */
		virtual const CqShadowSampleOptions& defaultSampleOptions() const;

		//--------------------------------------------------
		/// \name Factory functions
		//@{
		/** \brief Create and return an appropriate IqOcclusionSampler derived class
		 *
		 * \param file - texture file which the sampler should be connected
		 *               with.
		 */
		static boost::shared_ptr<IqOcclusionSampler> create(
				const boost::shared_ptr<IqTiledTexInputFile>& file,
				const CqMatrix& camToWorld);
		/** \brief Create a dummy occlusion sampler.
		 *
		 * Dummy samplers are useful when an occlusion map cannot be found but
		 * the render should go on regardless.
		 */
		static boost::shared_ptr<IqOcclusionSampler> createDummy();
		//@}

		virtual ~IqOcclusionSampler() {}
};

} // namespace Aqsis

#endif // IOCCLUSIONSAMPLER_H_INCLUDED
