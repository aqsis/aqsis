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
 * \brief Latitude-longitude environment map sampling machinary
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef LATLONGENVIRONMENTSAMPLER_H_INCLUDED
#define LATLONGENVIRONMENTSAMPLER_H_INCLUDED

#include <aqsis/aqsis.h>

#include <boost/shared_ptr.hpp>

#include <aqsis/math/math.h>
#include "ewafilter.h"
#include <aqsis/tex/filtering/ienvironmentsampler.h>
#include "mipmap.h"

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief Sampler for latitude-longitude environment maps
 *
 * Latitude-longitude environment maps represent all directions in the angles
 * of spherical coordinates.  From left to right, the longitude runs from 0 to
 * 360 degrees, while the latitude runs from -90 to 90 in the vertical
 * direction.
 */
template<typename LevelCacheT>
class AQSIS_TEX_SHARE CqLatLongEnvironmentSampler : public IqEnvironmentSampler
{
	public:
		/** \brief Construct a latlong environment sampler from the provided
		 * set of mipmap levels.
		 */
		CqLatLongEnvironmentSampler(const boost::shared_ptr<LevelCacheT>& levels);

		// from IqEnvironmentSampler
		virtual void sample(const Sq3DSamplePllgram& samplePllgram,
				const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const;
		virtual const CqTextureSampleOptions& defaultSampleOptions() const;
	private:
		// mipmap levels.
		boost::shared_ptr<LevelCacheT> m_levels;
};


//==============================================================================
// Implementation details
//==============================================================================
// CqLatLongEnvironmentSampler implementation
template<typename LevelCacheT>
CqLatLongEnvironmentSampler<LevelCacheT>::CqLatLongEnvironmentSampler(
		const boost::shared_ptr<LevelCacheT>& levels)
	: m_levels(levels)
{ }

namespace detail {

/** \brief Mapping from directions to coordinates for a latlong environment
 * texture.
 *
 * This function maps from directions in 3D space to texture coordinates (s,t)
 * such that s corresponds to the longitude, and t the latitude in spherical
 * angular coordinates.  s and t are rescaled such that they lie in the
 * interval [0,1].
 *
 * It also maps the edge vectors of the parallelogram region (which should be
 * considered to lie in the tangent space) using the tangent map to get
 * corresponding directions in the 2D texture space.
 *
 * Finally, it rescales the blur in the longitudinal direction so that any
 * blurring is isotropic in angular space.
 */
inline SqSamplePllgram directionToLatLong(const Sq3DSamplePllgram& region,
		TqFloat& sBlur)
{
	CqVector3D R = region.c;
	// First compute the position of the parallelogram centre.  This is
	// relatively expensive, since it involves calling atan2(), sqrt() and
	// acos().
	TqFloat phi = 0.5 + std::atan2(R.y(), R.x())*(1.0/(2*M_PI));
	TqFloat theta = 0;
	TqFloat R2 = R.Magnitude2();
	TqFloat RLen = std::sqrt(R2);
	if(R2 != 0)
		theta = std::acos(R.z()/RLen)*(1.0/M_PI);
	// (s,t) coordinates of parallelogram center
	CqVector2D st(phi, theta);

	// Next compute the coefficients of the tangent map.  That is, the
	// coefficients of the linear function which take the sides of the 3D
	// sampling parallelogram into the sides of the 2D one.
	TqFloat Rxy2 = R.x()*R.x() + R.y()*R.y();
	TqFloat RxyLen = std::sqrt(Rxy2);

	// Coefficents of the tangent map (note: tMap13 = 0 always)
	TqFloat tMap11 = 0;
	TqFloat tMap12 = 0;
	TqFloat tMap21 = 0;
	TqFloat tMap22 = 0;
	TqFloat tMap23 = 0;
	if(Rxy2 != 0)
	{
		TqFloat mult1 = 1/(2*M_PI*Rxy2);
		tMap11 = -R.y()*mult1;
		tMap12 = R.x()*mult1;

		if(R2 != 0)
		{
			TqFloat mult2 = 1/(M_PI*R2*RxyLen);
			tMap21 = R.x()*R.z() * mult2;
			tMap22 = R.y()*R.z() * mult2;
			tMap23 = (R.z()*R.z() - R2) * mult2;
		}
	}

	// Modify the sblur factor; the factor of 0.0001 is added to clamp the
	// maximum blur near the singularities at the poles.
	sBlur *= RLen / (RxyLen + 0.0001);

	// Apply the tangent map to the parallelogram sides
	CqVector2D side1(
		tMap11*region.s1.x() + tMap12*region.s1.y(),
		tMap21*region.s1.x() + tMap22*region.s1.y() + tMap23*region.s1.z()
	);
	CqVector2D side2(
		tMap11*region.s2.x() + tMap12*region.s2.y(),
		tMap21*region.s2.x() + tMap22*region.s2.y() + tMap23*region.s2.z()
	);
	return SqSamplePllgram(st, side1, side2);
}

} // namespace detail


template<typename LevelCacheT>
void CqLatLongEnvironmentSampler<LevelCacheT>::sample(
		const Sq3DSamplePllgram& samplePllgram,
		const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	TqFloat sBlur = sampleOpts.sBlur();
	// Map sampling parallelogram into latlong texture coords.
	SqSamplePllgram region2d = detail::directionToLatLong(samplePllgram, sBlur);
	region2d.scaleWidth(sampleOpts.sWidth(), sampleOpts.tWidth());

	// Compute the blur variance matrix.  tBlur is scaled by a factor of two so
	// that the blur will be isotropic in angular space.
	SqMatrix2D blurVariance = ewaBlurMatrix(sBlur, 2*sampleOpts.tBlur());

	// Construct EWA filter factory
	CqEwaFilterFactory ewaFactory(region2d, m_levels->width0(),
			m_levels->height0(), blurVariance);

	// Apply the filter to the mipmap levels
	m_levels->applyFilter(ewaFactory, sampleOpts, outSamps);
}

template<typename LevelCacheT>
const CqTextureSampleOptions&
CqLatLongEnvironmentSampler<LevelCacheT>::defaultSampleOptions() const
{
	return m_levels->defaultSampleOptions();
}


} // namespace Aqsis

#endif // LATLONGENVIRONMENTSAMPLER_H_INCLUDED
