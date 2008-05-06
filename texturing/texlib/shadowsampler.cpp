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
 * \brief Shadow texture sampler implementation
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#include "shadowsampler.h"

#include "ewafilter.h"
#include "itexinputfile.h"
#include "sampleaccum.h"
#include "filtertexture.h"
#include "texexception.h"
#include "tilearray.h"

namespace Aqsis {

//------------------------------------------------------------------------------
// CqShadowSampler implementation

CqShadowSampler::CqShadowSampler(const boost::shared_ptr<IqTiledTexInputFile>& file,
				const CqMatrix& currToWorld)
	: m_currToLight(),
	m_currToLightTexCoords(),
	m_pixelBuf(),
	m_defaultSampleOptions()
{
	if(!file)
		AQSIS_THROW(XqInternal, "Cannot construct shadow map from NULL file handle");

	const CqTexFileHeader& header = file->header();
	if(header.channelList().sharedChannelType() != Channel_Float32)
		AQSIS_THROW(XqBadTexture, "Shadow maps must hold 32-bit floating point data");

	// Get matrix which transforms the sample points to the light camera coordinates.
	const CqMatrix* worldToLight = header.findPtr<Attr::WorldToCameraMatrix>();
	if(!worldToLight)
	{
		AQSIS_THROW(XqBadTexture, "No world -> camera matrix found in file \""
				<< file->fileName() << "\"");
	}
	m_currToLight = (*worldToLight) * currToWorld;

	// Get matrix which transforms the sample points to texture coordinates.
	const CqMatrix* worldToLightScreen = header.findPtr<Attr::WorldToScreenMatrix>();
	if(!worldToLightScreen)
	{
		AQSIS_THROW(XqBadTexture, "No world -> screen matrix found in file \""
				<< file->fileName() << "\"");
	}
	m_currToLightTexCoords = (*worldToLightScreen) * currToWorld;
	// worldToLightScreen transforms world coordinates to "screen" coordinates,
	// ie, onto the 2D box [-1,1]x[-1,1].  We instead want texture coordinates,
	// which correspond to the box [0,1]x[0,1].  In addition, the direction of
	// increase of the y-axis should be swapped, since texture coordinates
	// define the origin to be in the top left of the texture rather than the
	// bottom right.
	m_currToLightTexCoords.Translate(CqVector3D(1,-1,0));
	m_currToLightTexCoords.Scale(0.5f, -0.5f, 1);

	m_defaultSampleOptions.fillFromFileHeader(header);

	// Connect pixel array to file
	m_pixelBuf.reset(new CqTileArray<TqFloat>(file, 0));
}

namespace {

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

} // unnamed namespace

void CqShadowSampler::sample(const Sq3DSampleQuad& sampleQuad,
		const CqShadowSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	// Get depths of sample positions.
	Sq3DSampleQuad quadLightCoord = sampleQuad;
	quadLightCoord.transform(m_currToLight);

	// Get texture coordinates of sample positions.
	Sq3DSampleQuad texQuad3D = sampleQuad;
	texQuad3D.transform(m_currToLightTexCoords);
	// Copy into (x,y) coordinates of texQuad and scale by the filter width.
	SqSampleQuad texQuad = texQuad3D;
	texQuad.scaleWidth(sampleOpts.sWidth(), sampleOpts.tWidth());

	// Get the EWA filter weight functor.  We use a relatively low edge cutoff
	// of 2, since we want to avoid taking samples which don't contribute much
	// to the average.  This problem would be a relative non-issue if we did
	// proper importance sampling.
	//
	/// \todo Investigate proper importance sampling to reduce the variance in
	/// shadow sampling?
	CqEwaFilterFactory ewaFactory(texQuad, m_pixelBuf->width(),
			m_pixelBuf->height(), sampleOpts.sBlur(), sampleOpts.tBlur(), 2);
	CqEwaFilter ewaWeights = ewaFactory.createFilter();

	/** \todo Optimization: Cull the query if it's outside the [min,max] depth
	 * range of the support.  Being able to determine the range from the tiles
	 * covered by the filter support will be a big advantage.
	 */

	SqFilterSupport support = ewaWeights.support();
	if(support.intersectsRange(0, m_pixelBuf->width(), 0, m_pixelBuf->height()))
	{
		// Get a functor which approximates the surface depth across the filter
		// support.  This deduced depth will be compared with the depths from
		// the stored texture buffer.
		quadLightCoord.copy2DCoords(texQuad);
		CqSampleQuadDepthApprox depthFunc(quadLightCoord, m_pixelBuf->width(),
				m_pixelBuf->height());

		// a PCF accumulator for the samples.
		CqPcfAccum<CqEwaFilter, CqSampleQuadDepthApprox> accumulator(
				ewaWeights, depthFunc, sampleOpts.startChannel(),
				sampleOpts.biasLow(), sampleOpts.biasHigh(), outSamps);
		// Finally, perform percentage closer filtering over the texture buffer.
		if(support.area() <= sampleOpts.numSamples() || sampleOpts.numSamples() < 0)
		{
			// If the filter support is small enough compared to the requested
			// number of samples, iterate over the whole support.  This results
			// in a completely noise-free result.
			//
			// A negative number of samples is also used as a flag to trigger
			// the deterministic integrator.
			filterTextureNowrap(accumulator, *m_pixelBuf, support);
		}
		else
		{
			// Otherwise use stochastic filtering (choose points randomly in
			// the filter support).  This is absolutely necessary when the
			// filter support is very large, as can occur with large blur
			// factors.
//			// \todo FIXME - add stochastic sampling back in.
			filterTextureNowrap(accumulator, *m_pixelBuf, support);
//			filterTextureNowrapStochastic(accumulator, *m_pixelBuf, support,
//					sampleOpts.numSamples());
		}
	}
	else
	{
		// If the filter support lies wholly outside the texture, return
		// fully visible == 0.
		*outSamps = 0;
	}
}

const CqShadowSampleOptions& CqShadowSampler::defaultSampleOptions() const
{
	return m_defaultSampleOptions;
}

} // namespace Aqsis
