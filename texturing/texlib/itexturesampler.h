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

#include <boost/shared_ptr.hpp>

#include "samplequad.h"

namespace Aqsis {

class CqTextureSampleOptions;
class IqTiledTexInputFile;
class IqTexInputFile;
class CqTexFileHeader;

//------------------------------------------------------------------------------
/** \brief An interface for sampling texture buffers.
 *
 * This intent of this interface is to provide texture sampling facilities
 * independently of the sampling options.  Classes which implement the
 * interface should attempt to use the sampling method as specified by a
 * CqTexutureSampleOptions, passed to the sample() interface function.
 *
 */
class AQSISTEX_SHARE IqTextureSampler
{
	public:
		/** \brief Sample the texture with the provided sample options.
		 *
		 * \param sampleQuad - quadrilateral region to sample over
		 * \param sampleOpts - options to the sampler, including filter widths etc.
		 * \param outSamps - the outSamps samples will be placed here.  
		 */
		virtual void sample(const SqSampleQuad& sampleQuad,
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
		 * The returned class is a CqTextureSamplerImpl<T> where T is a type
		 * appropriate to the pixel type held in the file.
		 *
		 * \param file - tiled texture file which the sampler should be connected to.
		 */
		static boost::shared_ptr<IqTextureSampler> create(
				const boost::shared_ptr<IqTiledTexInputFile>& file);
		static boost::shared_ptr<IqTextureSampler> create(
				const boost::shared_ptr<IqTexInputFile>& file);
		static boost::shared_ptr<IqTextureSampler> create(const char* fileName);
		//@}

		virtual ~IqTextureSampler() {}
};

} // namespace Aqsis

#endif // ITEXTURESAMPLER_H_INCLUDED
