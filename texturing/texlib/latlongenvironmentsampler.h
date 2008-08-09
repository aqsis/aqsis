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
class AQSISTEX_SHARE CqLatLongEnvironmentSampler : public IqEnvironmentSampler
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
 */
SqSamplePllgram directionToLatLong(const Sq3DSamplePllgram& region)
{
	CqVector3D R = region.c;
	// First compute the position of the parallelogram centre.  This is
	// relatively expensive, since it involves calling atan2(), sqrt() and
	// acos().
	TqFloat phi = 0.5 + std::atan2(R.y(), R.x())*(1.0/(2*M_PI));
	TqFloat theta = 0;
	TqFloat R2 = R.Magnitude2();
	if(R2 != 0)
		theta = std::acos(R.z()/std::sqrt(R2))*(1.0/M_PI);
	// (s,t) coordinates of parallelogram center
	CqVector2D st(phi, theta);

	// Next compute the coefficients of the tangent map.  That is, the
	// coefficients of the linear function which take the sides of the 3D
	// sampling parallelogram into the sides of the 2D one.
	TqFloat Rxy2 = R.x()*R.x() + R.y()*R.y();

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
			TqFloat mult2 = 1/(M_PI*R2*std::sqrt(Rxy2));
			tMap21 = R.x()*R.z() * mult2;
			tMap22 = R.y()*R.z() * mult2;
			tMap23 = (R.z()*R.z() - R2) * mult2;
		}
	}

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
	// Map sampling parallelogram into latlong texture coords.
	SqSamplePllgram pllgram2D = detail::directionToLatLong(samplePllgram);
	// TODO: Make blur and width work!

	// Construct EWA filter factory
	CqEwaFilterFactory ewaFactory(pllgram2D,
			m_levels->width0(), m_levels->height0(),
			sampleOpts.sBlur(), sampleOpts.tBlur());

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
