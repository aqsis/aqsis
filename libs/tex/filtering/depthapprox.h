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
