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
 * \brief Shadow texture sampler.
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef SHADOWSAMPLER_H_INCLUDED
#define SHADOWSAMPLER_H_INCLUDED

#include <aqsis/aqsis.h>

#include <aqsis/tex/filtering/ishadowsampler.h>
#include <aqsis/math/matrix.h>
#include <aqsis/tex/filtering/texturesampleoptions.h>

namespace Aqsis
{

template<typename>
class CqTileArray;

//------------------------------------------------------------------------------
/** \brief A sampler for shadow maps, implementing percentage closer filtering.
 */
class AQSIS_TEX_SHARE CqShadowSampler : public IqShadowSampler
{
	public:
		/** \brief Construct a shadow sampler with data from the provided file.
		 *
		 * \param file - file to obtain the shadow map data from.
		 * \param currToWorld - a matrix transforming the "current" coordinate
		 *                      system to the world coordinate system.  Sample
		 *                      quads are assumed to be passed to the sample()
		 *                      function represented in the "current"
		 *                      coordinate system.
		 */
		CqShadowSampler(const boost::shared_ptr<IqTiledTexInputFile>& file,
				const CqMatrix& currToWorld);

		// inherited
		virtual void sample(const Sq3DSampleQuad& sampleQuad,
				const CqShadowSampleOptions& sampleOpts, TqFloat* outSamps) const;
		virtual const CqShadowSampleOptions& defaultSampleOptions() const;
	private:
		class CqShadowView;
		typedef std::vector<boost::shared_ptr<CqShadowView> > TqViewVec;

		/// List of map views, used for point shadows, which have 6 subimages.
		TqViewVec m_maps;
		/// Default shadow sampling options.
		CqShadowSampleOptions m_defaultSampleOptions;
};


} // namespace Aqsis

#endif // SHADOWSAMPLER_H_INCLUDED
