// Aqsis
// Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the software's owners nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// (This is the New BSD license)

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
