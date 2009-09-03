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

#include <aqsis/tex/io/itexinputfile.h>
#include <aqsis/tex/filtering/sampleaccum.h>
#include <aqsis/tex/filtering/filtertexture.h>
#include <aqsis/tex/texexception.h>
#include <aqsis/tex/buffers/tilearray.h>

#include "depthapprox.h"
#include "ewafilter.h"

namespace Aqsis {

//------------------------------------------------------------------------------
// CqShadowSampler implementation

CqShadowSampler::CqShadowSampler(const boost::shared_ptr<IqTiledTexInputFile>& file,
				const CqMatrix& currToWorld)
	: m_currToLight(),
	m_currToTexture(),
	m_pixelBuf(),
	m_defaultSampleOptions()
{
	if(!file)
		AQSIS_THROW_XQERROR(XqInternal, EqE_NoFile,
						"Cannot construct shadow map from NULL file handle");

	const CqTexFileHeader& header = file->header();
	if(header.channelList().sharedChannelType() != Channel_Float32)
		AQSIS_THROW_XQERROR(XqBadTexture, EqE_BadFile,
						"Shadow maps must hold 32-bit floating point data");

	// Get matrix which transforms the sample points to the light camera coordinates.
	const CqMatrix* worldToLight = header.findPtr<Attr::WorldToCameraMatrix>();
	if(!worldToLight)
	{
		AQSIS_THROW_XQERROR(XqBadTexture, EqE_BadFile,
						"No world -> camera matrix found in file \""
						<< file->fileName() << "\"");
	}
	m_currToLight = (*worldToLight) * currToWorld;

	// Get matrix which transforms the sample points to raster texture coordinates.
	const CqMatrix* worldToLightScreen = header.findPtr<Attr::WorldToScreenMatrix>();
	if(!worldToLightScreen)
	{
		AQSIS_THROW_XQERROR(XqBadTexture, EqE_BadFile,
						"No world -> screen matrix found in file \""
						<< file->fileName() << "\"");
	}
	m_currToTexture = (*worldToLightScreen) * currToWorld;
	// worldToLightScreen transforms world coordinates to "screen" coordinates,
	// ie, onto the 2D box [-1,1]x[-1,1].  We instead want texture coordinates,
	// which correspond to the box [0,1]x[0,1].  In addition, the direction of
	// increase of the y-axis should be swapped, since texture coordinates
	// define the origin to be in the top left of the texture rather than the
	// bottom right.
	m_currToTexture.Translate(CqVector3D(1,-1,0));
	m_currToTexture.Scale(0.5f, -0.5f, 1);

	m_defaultSampleOptions.fillFromFileHeader(header);

	// Connect pixel array to file
	m_pixelBuf.reset(new CqTileArray<TqFloat>(file, 0));
}

namespace {

// Apply percentage closer filtering to the given buffer
template<typename DApprox>
inline void applyPCF(const CqTileArray<TqFloat>& pixelBuf,
		const CqShadowSampleOptions& sampleOpts, const SqFilterSupport& support,
		const CqEwaFilter& ewaWeights, const DApprox& depthFunc, TqFloat* outSamps)
{
	// a PCF accumulator for the samples.
	CqPcfAccum<CqEwaFilter, DApprox> accumulator(
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
		filterTextureNowrap(accumulator, pixelBuf, support);
	}
	else
	{
		// Otherwise use stochastic filtering (choose points randomly in
		// the filter support).  This is absolutely necessary when the
		// filter support is very large, as can occur with large blur
		// factors.
		filterTextureNowrapStochastic(accumulator, pixelBuf, support,
				sampleOpts.numSamples());
	}
}

} // anon namespace

void CqShadowSampler::sample(const Sq3DSampleQuad& sampleQuad,
		const CqShadowSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	// Get depths of sample positions.
	Sq3DSampleQuad quadLightCoord = sampleQuad;
	quadLightCoord.transform(m_currToLight);

	// Get texture coordinates of sample positions.
	Sq3DSampleQuad texQuad3D = sampleQuad;
	texQuad3D.transform(m_currToTexture);
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
		if(sampleOpts.depthApprox() == DApprox_Constant)
		{
			// Functor which approximates the surface depth using a constant.
			CqConstDepthApprox depthFunc(quadLightCoord.center().z());
			applyPCF(*m_pixelBuf, sampleOpts, support, ewaWeights, depthFunc, outSamps);
		}
		else
		{
			// Get a functor which approximates the surface depth across the filter
			// support with a linear approximation.  This deduced depth will be
			// compared with the depths from the stored texture buffer.
			quadLightCoord.copy2DCoords(texQuad);
			CqSampleQuadDepthApprox depthFunc(quadLightCoord, m_pixelBuf->width(),
					m_pixelBuf->height());
			applyPCF(*m_pixelBuf, sampleOpts, support, ewaWeights, depthFunc, outSamps);
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
