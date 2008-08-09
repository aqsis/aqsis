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
 * \brief Cube face environment map sampling machinary
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef CUBEENVIRONMENTSAMPLER_H_INCLUDED
#define CUBEENVIRONMENTSAMPLER_H_INCLUDED

#include "aqsis.h"

#include <boost/shared_ptr.hpp>

#include "aqsismath.h"
#include "ewafilter.h"
#include "ienvironmentsampler.h"
#include "mipmap.h"
#include "texfileattributes.h"

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief Sampler for cube face environment maps
 *
 * Cube environment maps represent all directions with textures mapped onto the
 * six faces of a cube.  This sampler maps directions to associated cube face
 * texture coordinates and then samples underlying mipmap level at that
 * position.
 */
template<typename LevelCacheT>
class AQSISTEX_SHARE CqCubeEnvironmentSampler : public IqEnvironmentSampler
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

namespace detail {

class CqCubeFaceMapper
{
	private:
		TqInt m_sIndex;
		TqInt m_tIndex;
		TqInt m_denomIndex;
		TqFloat m_sOffset;
		TqFloat m_tOffset;
		TqFloat m_fovScale;
		TqFloat m_sScale;
		TqFloat m_tScale;
	public:
		/** \brief Create a cube face mapper onto a particular face.
		 *
		 * The face to map onto is chosen by determining which face the given
		 * reference direction maps onto.
		 *
		 * \param refDirection - Direction which determines which face will be
		 *                       mapped onto.
		 * \param fovCotan - 1/tan(FOV/2) where FOV is the field of view of an
		 *                   individual face.
		 */
		CqCubeFaceMapper(const CqVector3D& refDirection, TqFloat fovCotan)
			: m_sIndex(0),
			m_tIndex(0),
			m_denomIndex(0),
			m_sOffset(0),
			m_tOffset(0.25),
			m_fovScale(0.5*fovCotan),
			m_sScale(0.5/3*fovCotan),
			// use negative sign, since t increases _downward from the top_
			m_tScale(-0.5/2*fovCotan)
		{
			const TqFloat vx = refDirection.x();
			const TqFloat vy = refDirection.y();
			const TqFloat vz = refDirection.z();
			const TqFloat absVx = std::fabs(vx);
			const TqFloat absVy = std::fabs(vy);
			const TqFloat absVz = std::fabs(vz);

			const TqInt xIdx = 0;
			const TqInt yIdx = 1;
			const TqInt zIdx = 2;

			// Determine which axis we're pointing along, and set parameters
			// to map onto the correct cube face.  The orientation of faces is
			// specified in the RISpec, in the section detailing the
			// RiMakeCubeFaceEnvironment interface call.
			if(absVx >= absVy && absVx >= absVz)
			{
				// x-axis
				m_sIndex = zIdx;
				m_tIndex = yIdx;
				m_denomIndex = xIdx;
				m_sScale *= -1;
				m_sOffset = 1.0/6;
				if(vx < 0)
				{
					m_tScale *= -1;
					m_tOffset = 0.75;
				}
			}
			else if(absVy >= absVx && absVy >= absVz)
			{
				// y-axis
				m_sIndex = xIdx;
				m_tIndex = zIdx;
				m_denomIndex = yIdx;
				m_sOffset = 0.5;
				if(vy < 0)
				{
					m_sScale *= -1;
					m_tOffset = 0.75;
				}
				m_tScale *= -1;
			}
			else
			{
				// z-axis
				m_sIndex = xIdx;
				m_tIndex = yIdx;
				m_denomIndex = zIdx;
				m_sOffset = 5.0/6;
				if(vz < 0)
				{
					m_tScale *= -1;
					m_tOffset = 0.75;
				}
			}
		}

		/** \brief Map a direction into texture coordinates
		 *
		 * This function uses the parameters determined in the constructor to
		 * map the given point onto the corresponding cube face.
		 *
		 * \param v - direction
		 *
		 * \return texture coordinates associated with v
		 */
		CqVector2D operator()(const CqVector3D& v)
		{
			TqFloat s = m_sScale*v[m_sIndex]/v[m_denomIndex] + m_sOffset;
			TqFloat t = m_tScale*v[m_tIndex]/v[m_denomIndex] + m_tOffset;
			return CqVector2D(s,t);
		}
};


/** \brief Mapping from directions to coordinates of a cube face environment texture.
 *
 * Each mipmap level of a cube face environment maps consists of the six faces
 * of a cube.  These are concatenated together into the same texture as follows:
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
 * This function represents a mapping from directions in 3D space to texture
 * coordinates (s,t) consistent with the squares of the cube face environment,
 * as laid out above.  The tangent map is used to convert directions along the
 * sides of the 3D sampling parallelogram into side vectors for the 2D result.
 *
 * Orientation of the cube faces is described in the RISpec.
 */
SqSamplePllgram directionToCubeFaceCoords(const Sq3DSamplePllgram& region,
		TqFloat fovCotan)
{
	// Coefficients of the tangent map which translate one 
	TqFloat tMap11 = 0, tMap12 = 0, tMap13 = 0; 
	TqFloat tMap21 = 0, tMap22 = 0, tMap23 = 0; 
	// Determine which axis we're pointing along, and map (s,t) onto the
	// appropriate cube face with the range [-1,1].The orientation of faces
	// is specified in the RISpec, in the section detailing the
	// RiMakeCubeFaceEnvironment interface call.
	TqFloat s = 0; TqFloat t = 0;
	// Scaling and shifting of the range will be computed in the next step
	// using the following parameters which are adjusted per-face.
	TqFloat sScale = 0.5/3*fovCotan;
	TqFloat tScale = -0.5/2*fovCotan;
	TqFloat sOffset = 0;
	TqFloat tOffset = 0.25;

	const CqVector3D R = region.c;
	const TqFloat absRx = std::fabs(R.x());
	const TqFloat absRy = std::fabs(R.y());
	const TqFloat absRz = std::fabs(R.z());
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
	tMap11 *= sScale;
	tMap12 *= sScale;
	tMap13 *= sScale;
	tMap21 *= tScale;
	tMap22 *= tScale;
	tMap23 *= tScale;

	// Compute center of new parallelogram with the cube face map.
	CqVector2D st(sScale*s + sOffset, tScale*t + tOffset);

	// Compute sides of new parallelogram using the tangent map.
	CqVector2D side1(
		tMap11*region.s1.x() + tMap12*region.s1.y() + tMap13*region.s1.z(),
		tMap21*region.s1.x() + tMap22*region.s1.y() + tMap23*region.s1.z()
	);
	CqVector2D side2(
		tMap11*region.s2.x() + tMap12*region.s2.y() + tMap13*region.s2.z(),
		tMap21*region.s2.x() + tMap22*region.s2.y() + tMap23*region.s2.z()
	);

	return SqSamplePllgram(st, side1, side2);
}

} // namespace detail

template<typename LevelCacheT>
void CqCubeEnvironmentSampler<LevelCacheT>::sample(
		const Sq3DSamplePllgram& samplePllgram,
		const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	// Map the corners of the sampling quadrialateral into 2D texture coordinates.
	SqSamplePllgram region2D
		= detail::directionToCubeFaceCoords(samplePllgram, m_fovCotan);
	// TODO: Make blur and width work correctly...
	// Construct EWA filter factory
	CqEwaFilterFactory ewaFactory(region2D, m_levels->width0(),
			m_levels->height0(), sampleOpts.sBlur(), sampleOpts.tBlur());

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
