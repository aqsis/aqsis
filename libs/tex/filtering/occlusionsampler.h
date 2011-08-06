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
 * \brief Occlusion texture sampler.
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef OCCLUSIONSAMPLER_H_INCLUDED
#define OCCLUSIONSAMPLER_H_INCLUDED

#include <aqsis/aqsis.h>

#include <vector>

#include <boost/shared_ptr.hpp>

#include <aqsis/tex/filtering/iocclusionsampler.h>
#include <aqsis/math/matrix.h>
#include <aqsis/math/random.h>
#include <aqsis/tex/filtering/texturesampleoptions.h>

namespace Aqsis
{

template<typename>
class CqTileArray;

//------------------------------------------------------------------------------
/** \brief A sampler for ambient occlusion maps
 *
 * In aqsis, an occlusion map is a file containing a set of shadow maps
 * rendered from many viewpoints surrounding the scene.  A map rendered with
 * view direction V allows the question "is a point P occluded from direction
 * -D?" to be answered.  In this way, we can compute the ambient occlusion
 *  without resorting to raytracing.
 */
class AQSIS_TEX_SHARE CqOcclusionSampler : public IqOcclusionSampler
{
	public:
		/** \brief Create an occlusion sampler, sampling data from the provided file.
		 *
		 * \param file - file to obtain the occlusion map data from.
		 * \param currToWorld - a matrix transforming the "current" coordinate
		 *                      system to the world coordinate system.  Sample
		 *                      regions are assumed to be passed to the sample()
		 *                      function represented in the "current"
		 *                      coordinate system.
		 */
		CqOcclusionSampler(const boost::shared_ptr<IqTiledTexInputFile>& file,
				const CqMatrix& currToWorld);

		// inherited
		virtual void sample(const Sq3DSamplePllgram& samplePllgram,
				const CqVector3D& normal, const CqShadowSampleOptions& sampleOpts,
				TqFloat* outSamps) const;
		virtual const CqShadowSampleOptions& defaultSampleOptions() const;
	private:
		class CqOccView;
		typedef std::vector<boost::shared_ptr<CqOccView> > TqViewVec;

		/// List of all shadow maps making up the occlusion map.
		TqViewVec m_maps;
		/// Default occlusion sampling options.
		CqShadowSampleOptions m_defaultSampleOptions;
		/// Random number stream for importance sampling.
		mutable CqRandom m_random;
};


} // namespace Aqsis

#endif // OCCLUSIONSAMPLER_H_INCLUDED
