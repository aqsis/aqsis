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
 * \brief Depth approximation functors for PCF filtering.
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#include <aqsis/aqsis.h>

#include <aqsis/tex/filtering/samplequad.h>

namespace Aqsis {

/** \brief A functor to determine the depth of an (x,y) point in a filter support
 *
 * When performing percentage closer filtering, the depths from the shadow map
 * at some points inside some filter support are compared against to the depth
 * of the surface at the same point.
 *
 * This functor approximates the surface depth by a linear function of the
 * raster coordinates, which is determined from the quadrilateral filter region.
 */
class CqSampleQuadDepthApprox
{
	private:
		/// Coefficients used to compute the depth at given (x,y) raster coords.
		TqFloat m_xMult;
		TqFloat m_yMult;
		TqFloat m_z0;
	public:
		/** Calculate and store linear approximation coefficints for the given
		 * sample quad.
		 *
		 * \param sampleQuad - quadrilateral in texture space (x,y) and depth
		 *    (z) over which to filter.
		 * \param baseTexWidth
		 * \param baseTexHeight - width and height of the base texture which
		 *    the texture coordinates will be scaled by
		 */
		CqSampleQuadDepthApprox(const Sq3DSampleQuad& sampleQuad,
				TqFloat baseTexWidth, TqFloat baseTexHeight)
			: m_xMult(0),
			m_yMult(0),
			m_z0(0)
		{
			// Compute an approximate normal for the sample quad
			CqVector3D quadNormal = (sampleQuad.v4 - sampleQuad.v1)
									% (sampleQuad.v3 - sampleQuad.v2);
			// Center of the sample quad.  We need the extra factor of 0.5
			// divided by the base texture dimensions so that pixel sample
			// positions are *centered* on the unit square.
			CqVector3D quadCenter = sampleQuad.center()
				+ CqVector3D(-0.5/baseTexWidth, -0.5/baseTexHeight, 0);

			// A normal and a point define a plane; here we use this fact to
			// compute the appropriate coefficients for the linear
			// approximation to the surface depth.
			if(quadNormal.z() != 0)
			{
				m_xMult = -quadNormal.x()/(quadNormal.z()*baseTexWidth);
				m_yMult = -quadNormal.y()/(quadNormal.z()*baseTexHeight);
				m_z0 = quadNormal*quadCenter/quadNormal.z();
			}
			else
			{
				m_z0 = quadCenter.z();
			}
		}
		/// Compute the depth of the surface at the given raster coordinates.
		TqFloat operator()(TqFloat x, TqFloat y) const
		{
			return m_z0 + m_xMult*x + m_yMult*y;
		}
};


/** \brief Constant "depth approximation" for percentage closer filtering
 *
 * When performing percentage closer filtering, we need a depth approximation
 * for the surface in order to compare the surface depth to the recorded
 * texture map depth.  This class provides the simplest possible depth
 * approximation - a constant.
 *
 * \see CqSampleQuadDepthApprox
 */
class CqConstDepthApprox
{
	private:
		TqFloat m_depth;
	public:
		CqConstDepthApprox(TqFloat depth)
			: m_depth(depth)
		{ }
		TqFloat operator()(TqFloat x, TqFloat y) const
		{
			return m_depth;
		}
};


} // namespace Aqsis
