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

#include "aqsis.h"

#include <boost/shared_ptr.hpp>

#include "aqsismath.h"
#include "ewafilter.h"
#include "ienvironmentsampler.h"
#include "mipmaplevelcache.h"

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
class AQSISTEX_SHARE CqLatLongEnvironmentSampler : public IqEnvironmentSampler
{
	public:
		/** \brief Construct a latlong environment sampler from the provided
		 * set of mipmap levels.
		 */
		CqLatLongEnvironmentSampler(const boost::shared_ptr<LevelCacheT>& levels);

		// from IqEnvironmentSampler
		virtual void sample(const Sq3DSampleQuad& sampleQuad,
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

/** \brief Map direction v into spherical angular coordinates.
 */
CqVector2D directionToSpherical(CqVector3D v)
{
	TqFloat phi = 0.5 + std::atan2(v.y(), v.x())*(1.0/(2*M_PI));
	TqFloat theta = 0;
	TqFloat r = v.Magnitude();
	if(r != 0)
		theta = std::acos(v.z()/r)*(1.0/M_PI);
	return CqVector2D(phi, theta);
}

}

template<typename LevelCacheT>
void CqLatLongEnvironmentSampler<LevelCacheT>::sample(const Sq3DSampleQuad& sampleQuad,
		const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	// Map the corners of the sampling quadrialateral into 2D texture coordinates.
	SqSampleQuad quad2d(
			detail::directionToSpherical(sampleQuad.v1),
			detail::directionToSpherical(sampleQuad.v2),
			detail::directionToSpherical(sampleQuad.v3),
			detail::directionToSpherical(sampleQuad.v4)
			);
	// TODO: Make blur work properly
	// Construct EWA filter factory
	CqEwaFilterFactory ewaFactory(quad2d, m_levels->width0(),
			m_levels->height0(), sampleOpts.sBlur(), sampleOpts.tBlur());

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
