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
 * \brief Cube face environment map sampling machinary
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef CUBEENVIRONMENTSAMPLER_H_INCLUDED
#define CUBEENVIRONMENTSAMPLER_H_INCLUDED

#include <aqsis/aqsis.h>

#include <boost/shared_ptr.hpp>

#include <aqsis/math/math.h>
#include "ewafilter.h"
#include <aqsis/tex/filtering/ienvironmentsampler.h>
#include "mipmap.h"
#include <aqsis/tex/io/texfileattributes.h>

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief Sampler for cube face environment maps
 *
 * A cube face environment map consists of the six faces of a cube as viewed
 * from the cube centre.  The cube environment sampler maps directions to
 * associated cube face texture coordinates and then samples the appropriate
 * mipmap level at that position.
 *
 * The faces are concatenated together into a single texture as follows:
 *
 * \verbatim
 *
 *   +----+----+----+
 *   | +x | +y | +z |
 *   |    |    |    |
 *   +----+----+----+
 *   | -x | -y | -z |
 *   |    |    |    |
 *   +----+----+----+
 *
 * \endverbatim
 *
 * Orientation of the cube faces is described in the RISpec.
 */
template<typename LevelCacheT>
class AQSIS_TEX_SHARE CqCubeEnvironmentSampler : public IqEnvironmentSampler
{
	public:
		/** \brief Construct a cube face environment sampler
		 *
		 * \param levels - Set of mipmap levels of the cubic environment faces.
		 */
		CqCubeEnvironmentSampler(const boost::shared_ptr<LevelCacheT>& levels);

		// from IqEnvironmentSampler
		virtual void sample(const Sq3DSamplePllgram& samplePllgram,
				const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const;
		virtual const CqTextureSampleOptions& defaultSampleOptions() const;
	private:
		// mipmap levels.
		boost::shared_ptr<LevelCacheT> m_levels;
		// Scale factor for cube face environment map coordinates = 1/tan(fov/2)
		TqFloat m_fovCotan;
};


//==============================================================================
// Implementation details
//==============================================================================
// CqCubeEnvironmentSampler implementation
template<typename LevelCacheT>
CqCubeEnvironmentSampler<LevelCacheT>::CqCubeEnvironmentSampler(
		const boost::shared_ptr<LevelCacheT>& levels)
	: m_levels(levels),
	m_fovCotan(levels->header().template find<Attr::FieldOfViewCot>(1))
{ }

template<typename LevelCacheT>
void CqCubeEnvironmentSampler<LevelCacheT>::sample(
		const Sq3DSamplePllgram& region,
		const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	// The tricky task here is to map the 3D sample parallelogram into a
	// 2D one on the appropriate cube face.  This involves mapping both the
	// centre point, along with the parallelogram sides.

	// (s,t) will end up in the range [-1,1] for the appropriate face.
	TqFloat s = 0; TqFloat t = 0;
	// Scaling and shifting of the range will be computed in the next step
	// using the following parameters which are adjusted per-face.
	TqFloat sScale = 1;
	TqFloat tScale = -1;
	TqFloat sOffset = 0;
	TqFloat tOffset = 0.25;

	// Coefficients of the linear tangent map which translate the 3D
	// parallogram sides into the 2D ones.
	TqFloat tMap11 = 0, tMap12 = 0, tMap13 = 0; 
	TqFloat tMap21 = 0, tMap22 = 0, tMap23 = 0; 

	const CqVector3D R = region.c;
	const TqFloat absRx = std::fabs(R.x());
	const TqFloat absRy = std::fabs(R.y());
	const TqFloat absRz = std::fabs(R.z());
	// Determine which axis we're pointing along, and map (s,t) onto the
	// appropriate cube face with the range [-1,1].  Also compute the
	// appropriate tangent map coefficients.  The orientation of faces is
	// specified in the RISpec, in the section detailing the
	// RiMakeCubeFaceEnvironment interface call.
	if(absRx >= absRy && absRx >= absRz)
	{
		// x-axis
		// cube face map parameters
		TqFloat invRx = 1/R.x();
		s = R.z()*invRx;
		t = R.y()*invRx;
		sScale *= -1;
		sOffset = 1.0/6;
		if(R.x() < 0)
		{
			tScale *= -1;
			tOffset = 0.75;
		}
		// Tangent map parameters
		tMap11 = -s*invRx;
		tMap13 = invRx; 
		tMap21 = -t*invRx;
		tMap22 = invRx;
	}
	else if(absRy >= absRx && absRy >= absRz)
	{
		// y-axis
		TqFloat invRy = 1/R.y();
		s = R.x()*invRy;
		t = R.z()*invRy;
		sOffset = 0.5;
		if(R.y() < 0)
		{
			sScale *= -1;
			tOffset = 0.75;
		}
		tScale *= -1;
		// Tangent map parameters
		tMap11 = invRy;
		tMap12 = -s*invRy;
		tMap22 = -t*invRy;
		tMap23 = invRy; 
	}
	else
	{
		// z-axis
		TqFloat invRz = 1/R.z();
		s = R.x()*invRz;
		t = R.y()*invRz;
		sOffset = 5.0/6;
		if(R.z() < 0)
		{
			tScale *= -1;
			tOffset = 0.75;
		}
		// Tangent map parameters
		tMap11 = invRz;
		tMap13 = -s*invRz; 
		tMap22 = invRz;
		tMap23 = -t*invRz; 
	}

	// (s1,t1) are the correctly-oriented coordinates on the cube face
	// [-1,1]x[-1,1] used for computing the blur variance matrix.
	TqFloat s1 = s*sScale;
	TqFloat t1 = t*tScale;

	sScale *= 0.5/3*m_fovCotan;
	tScale *= 0.5/2*m_fovCotan;

	tMap11 *= sScale; tMap12 *= sScale; tMap13 *= sScale;
	tMap21 *= tScale; tMap22 *= tScale; tMap23 *= tScale;

	// Compute center of new parallelogram
	// centre of the face.
	CqVector2D st(s*sScale + sOffset, t*tScale + tOffset);
	// Compute sides of new parallelogram using the tangent map.
	CqVector2D side1(
		tMap11*region.s1.x() + tMap12*region.s1.y() + tMap13*region.s1.z(),
		tMap21*region.s1.x() + tMap22*region.s1.y() + tMap23*region.s1.z()
	);
	CqVector2D side2(
		tMap11*region.s2.x() + tMap12*region.s2.y() + tMap13*region.s2.z(),
		tMap21*region.s2.x() + tMap22*region.s2.y() + tMap23*region.s2.z()
	);
	// Construct the mapped 2D sample parallelogram.
	SqSamplePllgram region2d(st, side1, side2);
	region2d.scaleWidth(sampleOpts.sWidth(), sampleOpts.tWidth());

	// Compute the blur matrix if necessary.
	SqMatrix2D blurVariance(0);
	// The blurAmp here is chosen to approximately give the same amount of
	// blurring as latlong environment maps with the same user input.
	const TqFloat blurAmp = (sampleOpts.sBlur() + sampleOpts.tBlur())*0.25;
	if(blurAmp > 0)
	{
		// A fixed angle maps to a larger region at the edges of a face than at
		// the middle.  The blur needs to be adjusted accordingly - we require
		// that the blur is constant in angular space.
		//
		// To compute the adjustment, it's necessary to compute the tangent map
		// (jacobian) of the texture to environment sphere mapping, call this
		// J.  The blur variance is then inverse(J.transpose * J), scaled by
		// the user-requested blur variance.
		//
		// The factor of m_fovCotan is added so that changing the cube face fov
		// doesn't change the amount of blur.  The scaling factors of 1.5=3/2
		// and 2.25=(3/2)*(3/2) are needed to undo the stretching that happens
		// when the blur is scaled by the non-square base texture resolution
		// (which has a ratio of 3:2)
		blurVariance = blurAmp*blurAmp * m_fovCotan*m_fovCotan * (s1*s1 + t1*t1 + 1)
			* SqMatrix2D(s1*s1+1, 1.5*t1*s1, 1.5*t1*s1,  2.25*(t1*t1+1));
	}

	// Construct the filter factory
	CqEwaFilterFactory ewaFactory(region2d, m_levels->width0(),
			m_levels->height0(), blurVariance);
	// Apply the filter to the mipmap levels
	m_levels->applyFilter(ewaFactory, sampleOpts, outSamps);
}

template<typename LevelCacheT>
const CqTextureSampleOptions&
CqCubeEnvironmentSampler<LevelCacheT>::defaultSampleOptions() const
{
	return m_levels->defaultSampleOptions();
}


} // namespace Aqsis

#endif // CUBEENVIRONMENTSAMPLER_H_INCLUDED
