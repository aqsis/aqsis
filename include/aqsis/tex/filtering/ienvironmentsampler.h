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
 * \brief Interface to environment map sampling machinery.
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef IENVIRONMENTSAMPLER_H_INCLUDED
#define IENVIRONMENTSAMPLER_H_INCLUDED

#include <aqsis/aqsis.h>

#include <boost/shared_ptr.hpp>

#include <aqsis/tex/filtering/samplequad.h>
#include <aqsis/tex/filtering/texturesampleoptions.h>

namespace Aqsis {

class IqTiledTexInputFile;

//------------------------------------------------------------------------------
/** \brief An interface for sampling environment texture buffers.
 *
 * This interface provides environment sampling facilities independently of the
 * sampling options.  Classes which implement the interface should attempt to
 * use the sampling method as specified by a CqTextureSampleOptions, passed to
 * the sample() interface function.
 *
 */
class AQSIS_TEX_SHARE IqEnvironmentSampler
{
	public:
		/** \brief Sample the texture with the provided sample options.
		 *
		 * This function has a default implementation which calls through to
		 * the other version of the sample() function, but can be overridden to
		 * deal with sampling methods which can make use of the full
		 * information in the sampling quadrilateral.
		 *
		 * \param sampleQuad - quadrilateral region to sample over
		 * \param sampleOpts - options to the sampler, including filter widths etc.
		 * \param outSamps - the outSamps samples will be placed here.  
		 */
		virtual void sample(const Sq3DSampleQuad& sampleQuad,
				const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const;

		/** \brief Filter the texture over the given parallelogram region.
		 *
		 * \param samplePllgram - parallelogram to sample over
		 * \param sampleOpts - options to the sampler, including filter widths etc.
		 * \param outSamps - the outSamps samples will be placed here.  
		 */
		virtual void sample(const Sq3DSamplePllgram& samplePllgram,
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
		/** \brief Create and return a IqEnvironmentSampler derived class
		 *
		 * \param file - texture file which the sampler should be connected
		 *               with.
		 */
		static boost::shared_ptr<IqEnvironmentSampler> create(
				const boost::shared_ptr<IqTiledTexInputFile>& file);
		/** \brief Create a dummy environment sampler.
		 *
		 * Dummy samplers are useful when a texture file cannot be found but
		 * the render should go on regardless.
		 */
		static boost::shared_ptr<IqEnvironmentSampler> createDummy();
		//@}

		virtual ~IqEnvironmentSampler() {}
};

} // namespace Aqsis

#endif // IENVIRONMENTSAMPLER_H_INCLUDED
