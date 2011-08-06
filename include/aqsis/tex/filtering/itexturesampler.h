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
 * \brief Interface to texture buffer sampling machinery.
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef ITEXTURESAMPLER_H_INCLUDED
#define ITEXTURESAMPLER_H_INCLUDED

#include <aqsis/aqsis.h>

#include <boost/shared_ptr.hpp>

#include <aqsis/tex/filtering/samplequad.h>
#include <aqsis/tex/filtering/texturesampleoptions.h>

namespace Aqsis {

class IqTiledTexInputFile;
class IqMultiTexInputFile;
class CqTexFileHeader;

//------------------------------------------------------------------------------
/** \brief An interface for sampling texture buffers.
 *
 * Sampling consists of filtering the texture over some appropriate filter
 * region; filtering can be performed with either of the two sample()
 * functions.  These provide alternative interfaces to specify the filter
 * region.
 *
 * Classes which implement the interface may attempt to use the sampling method
 * as specified by a CqTexutureSampleOptions, passed to the sample() interface
 * function.  Since this class is designed to be part of the renderman shading
 * language texture sampling pipeline, the sample method should default to
 * something else quietly if it's not supported.
 */
class AQSIS_TEX_SHARE IqTextureSampler
{
	public:
		/** \brief Filter the texture over the given quadrilateral region.
		 *
		 * This function has a default implementation which calls through to
		 * the other version of the sample() function, but can be overridden to
		 * deal with sampling methods which can make use of the full
		 * information in the sampling quadrilateral.
		 *
		 * \param sampleQuad - quadrilateral region to filter over
		 * \param sampleOpts - options to the sampler, including filter widths etc.
		 * \param outSamps - results of sampling will be placed here.
		 */
		virtual void sample(const SqSampleQuad& sampleQuad,
				const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const;

		/** \brief Filter the texture over the given parallelogram region.
		 *
		 * \param samplePllgram - parallelogram to sample over
		 * \param sampleOpts - options to the sampler, including filter widths etc.
		 * \param outSamps - the outSamps samples will be placed here.  
		 */
		virtual void sample(const SqSamplePllgram& samplePllgram,
				const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const = 0;

		/** \brief Get the default sample options for this texture.
		 *
		 * The default implementation returns texture sample options
		 * initialized by the default CqTextureSampleOptions constructor.
		 *
		 * \return The default sample options - in the case that the texture
		 * originates from an underlying file, these should include options
		 * which were used when the texture was created (things like the
		 * texutre wrap mode).
		 */
		virtual const CqTextureSampleOptions& defaultSampleOptions() const;

		//--------------------------------------------------
		/// \name Factory functions
		//@{
		/** \brief Create and return a IqTextureSampler derived class
		 *
		 * \param file - texture file which the sampler should be connected to.
		 */
		static boost::shared_ptr<IqTextureSampler> create(
				const boost::shared_ptr<IqTiledTexInputFile>& file);
		static boost::shared_ptr<IqTextureSampler> create(
				const boost::shared_ptr<IqMultiTexInputFile>& file);
		/** \brief Create a dummy texture sampler.
		 *
		 * Dummy samplers are useful when a texture file cannot be found but
		 * the render should go on regardless.
		 */
		static boost::shared_ptr<IqTextureSampler> createDummy();
		//@}

		virtual ~IqTextureSampler() {}
};

} // namespace Aqsis

#endif // ITEXTURESAMPLER_H_INCLUDED
