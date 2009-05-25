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
 * \brief Dummy occlusion sampler for missing files
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef DUMMYOCCLUSIONSAMPLER_H_INCLUDED
#define DUMMYOCCLUSIONSAMPLER_H_INCLUDED

#include <aqsis/tex/filtering/iocclusionsampler.h>

namespace Aqsis {

/** \brief Placeholder occlusion sampler implementation
 *
 * Dummy sampler to be used in place of a real one when the requested file is
 * not found.
 */
class AQSIS_TEX_SHARE CqDummyOcclusionSampler : public IqOcclusionSampler
{
	public:
		/// Return not-occluded always.
		virtual void sample(const Sq3DSamplePllgram& samplePllgram,
			const CqVector3D& normal, const CqShadowSampleOptions& sampleOpts,
			TqFloat* outSamps) const;
};


//==============================================================================
// Implementation details.
//==============================================================================

inline void CqDummyOcclusionSampler::sample(
		const Sq3DSamplePllgram& samplePllgram,
		const CqVector3D& normal, const CqShadowSampleOptions& sampleOpts,
		TqFloat* outSamps) const
{
	for(TqInt i = 0; i < sampleOpts.numChannels(); ++i)
		outSamps[i] = 0;
}

} // namespace Aqsis

#endif // DUMMYOCCLUSIONSAMPLER_H_INCLUDED
