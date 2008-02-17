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

#include "itexinputfile.h"
#include "texexception.h"
#include "texbufsampler.h" // remove when using tiled textures.
#include "texturebuffer.h" // remove when using tiled textures.
#include "ewafilter.h"
#include "sampleaccum.h"

namespace Aqsis {

CqShadowSampler::CqShadowSampler(const boost::shared_ptr<IqTexInputFile>& file,
				const CqMatrix& camToWorld)
	: m_camToLight(),
	m_camToLightRaster(),
	m_pixelBuf(new CqTextureBuffer<TqFloat>()),
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
	m_camToLight = (*worldToLight) * camToWorld;

	// Read pixel data
	file->readPixels(*m_pixelBuf);

	// Get matrix which transforms the sample points to texture coordinates.
	const CqMatrix* worldToLightRaster = header.findPtr<Attr::WorldToScreenMatrix>();
	if(!worldToLightRaster)
	{
		AQSIS_THROW(XqBadTexture, "No world -> screen matrix found in file \""
				<< file->fileName() << "\"");
	}
	m_camToLightRaster = (*worldToLightRaster) * camToWorld;
	// worldToLightRaster transforms world coordinates to NDC, ie, onto the 2D
	// box [-1,1]x[-1,1].  We instead want texture coordinates, which
	// correspond to the box [0,1]x[0,1].  In addition, the direction of
	// increase of the y-axis should be swapped, since texture coordinates
	// define the origin to be in the top left of the texture rather than the
	// bottom right.
	m_camToLightRaster.Translate(CqVector3D(1,-1,0));
	m_camToLightRaster.Scale(0.5f, -0.5f, 1);

	m_defaultSampleOptions.fillFromFileHeader(header);
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
		/// Calculate and store linear approximation coefficints for the given
		/// sample quad.
		CqSampleQuadDepthApprox(const Sq3DSampleQuad& sampleQuad)
			: m_xMult(0),
			m_yMult(0),
			m_z0(0)
		{
			// Compute an approximate normal for the sample quad
			CqVector3D quadNormal = (sampleQuad.v4 - sampleQuad.v1)
									% (sampleQuad.v3 - sampleQuad.v2);
			// Center of the sample quad.
			CqVector3D quadCenter = sampleQuad.center();

			// A normal and a point define a plane; here we use this fact to
			// compute the appropriate coefficients for the linear
			// approximation to the surface depth.
			if(quadNormal.z() != 0)
			{
				m_xMult = -quadNormal.x()/quadNormal.z();
				m_yMult = -quadNormal.y()/quadNormal.z();
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
	quadLightCoord.transform(m_camToLight);

	// Get texture coordinates of sample positions.
	Sq3DSampleQuad rasterQuad = sampleQuad;
	rasterQuad.transform(m_camToLightRaster);
	// Copy into (x,y) coordinates of texQuad.
	SqSampleQuad texQuad = rasterQuad;

	// Set the raster coordinate sample quad z values to the depths in light
	// coordinates.
	rasterQuad.v1.z(quadLightCoord.v1.z());
	rasterQuad.v2.z(quadLightCoord.v2.z());
	rasterQuad.v3.z(quadLightCoord.v3.z());
	rasterQuad.v4.z(quadLightCoord.v4.z());

	// Get a functor to approximate the surface depth across the filter support.
	CqSampleQuadDepthApprox depthFunc(rasterQuad);

	// Get the EWA filter weight functor.
	CqEwaFilterWeights weights(texQuad, m_pixelBuf->width(),
			m_pixelBuf->height(), sampleOpts.sBlur(), sampleOpts.tBlur());

	// Construct an accumulator for the samples.
	CqPcfAccum<CqEwaFilterWeights, CqSampleQuadDepthApprox> accumulator(
			weights, depthFunc, sampleOpts.startChannel(), outSamps);

	/** \todo Optimization opportunity: Cull the query if it's outside the
	 * [min,max] depth range of the texture map (also would work on a per-tile
	 * basis)
	 */

	// Finally perform PCF filtering over the texture buffer.
	CqTexBufSampler<CqTextureBuffer<TqFloat> >(*m_pixelBuf).applyFilter(
			accumulator, weights.support(), WrapMode_Black, WrapMode_Black);
}

const CqShadowSampleOptions& CqShadowSampler::defaultSampleOptions() const
{
	return m_defaultSampleOptions;
}

} // namespace Aqsis
