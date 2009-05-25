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
 * \brief Interface to shadow texture sampling machinery.
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef ISHADOWSAMPLER_H_INCLUDED
#define ISHADOWSAMPLER_H_INCLUDED

#include <aqsis/aqsis.h>

#include <boost/shared_ptr.hpp>

#include <aqsis/math/matrix.h>
#include <aqsis/tex/filtering/samplequad.h>
#include <aqsis/tex/filtering/texturesampleoptions.h>

namespace Aqsis {

class IqTiledTexInputFile;

//------------------------------------------------------------------------------
/** \brief An interface for sampling shadow texture buffers.
 *
 * This interface provides shadow sampling facilities independently of the
 * sampling options.  Classes which implement the interface should attempt to
 * use the sampling method as specified by a CqShadowSampleOptions, passed to
 * the sample() interface function.
 *
 */
class AQSIS_TEX_SHARE IqShadowSampler
{
	public:
		/** \brief Sample the texture over a quadrilateral region
		 *
		 * \param sampleQuad - quadrilateral region to sample over
		 * \param sampleOpts - options to the sampler, including filter widths etc.
		 * \param outSamps - the outSamps samples will be placed here.  
		 */
		virtual void sample(const Sq3DSampleQuad& sampleQuad,
				const CqShadowSampleOptions& sampleOpts, TqFloat* outSamps) const = 0;

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
		/** \brief Create and return a IqShadowSampler derived class
		 *
		 * \param file - texture file which the sampler should be connected
		 *               with.
		 */
		static boost::shared_ptr<IqShadowSampler> create(
				const boost::shared_ptr<IqTiledTexInputFile>& file,
				const CqMatrix& camToWorld);
		/** \brief Create a dummy shadow texture sampler.
		 *
		 * Dummy samplers are useful when a texture file cannot be found but
		 * the render should go on regardless.
		 */
		static boost::shared_ptr<IqShadowSampler> createDummy();
		//@}

		virtual ~IqShadowSampler() {}
};

} // namespace Aqsis

#endif // ISHADOWSAMPLER_H_INCLUDED
