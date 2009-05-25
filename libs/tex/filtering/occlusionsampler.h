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
