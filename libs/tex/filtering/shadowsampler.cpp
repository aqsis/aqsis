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

/** \brief Class representing a single view out of the shadow map.
 *
 * Each view encapsulates transformation matrices from the "current" coordinate
 * system to the view coordinates, along with a depth map as rendered from that
 * direction, and light position information pre-transformed to "current" space
 * for determining the best shadow map to use when multiple are supplied for point
 * shadowing.
 */
class CqShadowSampler::CqShadowView
{
	private:
		/// transformation: current -> light coordinates
		CqMatrix m_currToLight;
		/// transformation: current -> raster coordinates ( [0,width]x[0,height] )
		CqMatrix m_currToRaster;
		/** transformation: current -> raster coordinates.  This is the vector
		 * transformation associated with the point transformation
		 * m_currToRaster.
		 */
		CqMatrix m_currToRasterVec;
		/** \brief direction from which the depth map was created.
		 *
		 * This is used to determine how visible the point being shadowed is
		 * in relation to this view.
		 */
		CqVector3D m_viewDirec;
		/** \brief point of origin of the lightsource in "current" space.
		 *
		 * This is used to transform the point being shaded to calculate
		 * how visible it is to this light view.
		 */
		CqVector3D m_lightPos;
		/// Pixel data for shadow map.
		CqTileArray<TqFloat> m_pixels;

	public:
		/** \brief Create a view from imageNum of the provided file.
		 *
		 * \param file - file from which to read the image data.
		 * \param imageNum - subimage number for this view in the input file
		 * \param currToWorld - current -> world transformation matrix.
		 */
		CqShadowView(const boost::shared_ptr<IqTiledTexInputFile>& file, TqInt imageNum,
				const CqMatrix& currToWorld)
			: m_currToLight(),
			m_currToRaster(),
			m_currToRasterVec(),
			m_viewDirec(),
			m_pixels(file, imageNum)
		{
			// TODO refactor with CqShadowSampler, also refactor this function,
			// since it's a bit unweildly...
			if(!file)
				AQSIS_THROW_XQERROR(XqInternal, EqE_NoFile,
						"Cannot construct shadow map from NULL file handle");

			const CqTexFileHeader& header = file->header(imageNum);
			if(header.channelList().sharedChannelType() != Channel_Float32)
				AQSIS_THROW_XQERROR(XqBadTexture, EqE_BadFile,
						"Shadow maps must hold 32-bit floating point data");

			// Get matrix which transforms the sample points to the light
			// camera coordinates.
			const CqMatrix* worldToLight
				= header.findPtr<Attr::WorldToCameraMatrix>();
			if(!worldToLight)
			{
				AQSIS_THROW_XQERROR(XqBadTexture, EqE_BadFile,
						"No world -> camera matrix found in file \""
						<< file->fileName() << "\"");
			}
			m_currToLight = (*worldToLight) * currToWorld;

			// Get matrix which transforms the sample points to texture coordinates.
			const CqMatrix* worldToLightScreen
				= header.findPtr<Attr::WorldToScreenMatrix>();
			if(!worldToLightScreen)
			{
				AQSIS_THROW_XQERROR(XqBadTexture, EqE_BadFile,
						"No world -> screen matrix found in file \""
						<< file->fileName() << "\"");
			}
			m_currToRaster = (*worldToLightScreen) * currToWorld;
			// worldToLightScreen transforms world coordinates to "screen" coordinates,
			// ie, onto the 2D box [-1,1]x[-1,1].  We instead want texture coordinates,
			// which correspond to the box [0,width]x[0,height].  In
			// addition, the direction of increase of the y-axis should be
			// swapped, since texture coordinates define the origin to be in
			// the top left of the texture rather than the bottom right.
			m_currToRaster.Translate(CqVector3D(1,-1,0));
			m_currToRaster.Scale(0.5f, -0.5f, 1);

			// Transform the light origin to "current" space to use
			// when checking the visibility of a point.
			m_lightPos = m_currToLight.Inverse()*CqVector3D(0,0,0);
			// Transform the normal (0,0,1) in light space into a normal in
			// "current" space.  The appropriate matrix is the inverse of the
			// cam -> light normal transformation, which itself is the inverse
			// transpose of currToLightVec.
			CqMatrix currToLightVec = m_currToLight;
			currToLightVec[3][0] = 0;
			currToLightVec[3][1] = 0;
			currToLightVec[3][2] = 0;
			m_viewDirec = currToLightVec.Transpose()*CqVector3D(0,0,1);
			m_viewDirec.Unit();
		}

		/** \brief Visibility of the specified point to the lightsource.
		 *
		 * If multiple maps are specified, this is used as a factor to 
		 * determine which shadow view to use for shadowing. The more
		 * a point is in the periphery, the less likely it is to be
		 * chosen.
		 *
		 * \param P - point being shaded in "current" coordinates.
		 */
		TqFloat weight(const CqVector3D& P)
		{
			return m_viewDirec*(P-m_lightPos);
		}

		/** \brief Compute occlusion from the current view direction to the
		 * given sample region.
		 *
		 * \param sampleRegion - parallelogram region over which to sample the map
		 * \param sampleOpts - set of sampling options 
		 * \param numSamples - number of samples to take for the region.
		 * \param outSamps[0] - Return parameter; amount of occlusion over the
		 *                      sample region in the viewing direction for this map.
		 */
		void sample(const Sq3DSampleQuad& sampleQuad,
				const CqShadowSampleOptions& sampleOpts,
				TqFloat* outSamps) const
		{
			// Get depths of sample positions.
			Sq3DSampleQuad quadLightCoord = sampleQuad;
			quadLightCoord.transform(m_currToLight);

			// Get texture coordinates of sample positions.
			Sq3DSampleQuad texQuad3D = sampleQuad;
			texQuad3D.transform(m_currToRaster);
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
			CqEwaFilterFactory ewaFactory(texQuad, m_pixels.width(),
					m_pixels.height(), sampleOpts.sBlur(), sampleOpts.tBlur(), 2);
			CqEwaFilter ewaWeights = ewaFactory.createFilter();

			/** \todo Optimization: Cull the query if it's outside the [min,max] depth
			 * range of the support.  Being able to determine the range from the tiles
			 * covered by the filter support will be a big advantage.
			 */

			SqFilterSupport support = ewaWeights.support();
			if(support.intersectsRange(0, m_pixels.width(), 0, m_pixels.height()))
			{
				if(sampleOpts.depthApprox() == DApprox_Constant)
				{
					// Functor which approximates the surface depth using a constant.
					CqConstDepthApprox depthFunc(quadLightCoord.center().z());
					applyPCF(m_pixels, sampleOpts, support, ewaWeights, depthFunc, outSamps);
				}
				else
				{
					// Get a functor which approximates the surface depth across the filter
					// support with a linear approximation.  This deduced depth will be
					// compared with the depths from the stored texture buffer.
					quadLightCoord.copy2DCoords(texQuad);
					CqSampleQuadDepthApprox depthFunc(quadLightCoord, m_pixels.width(),
							m_pixels.height());
					applyPCF(m_pixels, sampleOpts, support, ewaWeights, depthFunc, outSamps);
				}
			}
			else
			{
				// If the filter support lies wholly outside the texture, return
				// fully visible == 0.
				*outSamps = 0;
			}
		}
};


//------------------------------------------------------------------------------
// CqShadowSampler implementation

CqShadowSampler::CqShadowSampler(const boost::shared_ptr<IqTiledTexInputFile>& file,
				const CqMatrix& currToWorld)
	: m_maps(),
	m_defaultSampleOptions()
{
	// Connect the multiple shadow maps to the input file.
	TqInt numMaps = file->numSubImages();
	m_maps.reserve(numMaps);
	for(TqInt i = 0; i < numMaps; ++i)
	{
		m_maps.push_back(
				boost::shared_ptr<CqShadowView>(new CqShadowView(file, i, currToWorld)) );
	}

	m_defaultSampleOptions.fillFromFileHeader(file->header());
}

void CqShadowSampler::sample(const Sq3DSampleQuad& sampleQuad,
		const CqShadowSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	// Get a suitable shadow map from the multi-map.
	const CqShadowView* view = m_maps[0].get();
	if(m_maps.size() > 1)
	{
		// Get the center of the sample region, we'll use
		// this to determine which is the best view to use.
		CqVector3D PC = sampleQuad.center();

		// Choose the shadow view that sees the point most clearly,
		// the more the point is in the periphery of a view, the
		// less likely it is to be chosen.
		TqFloat maxWeight = 0.0f;
		
		for(TqViewVec::const_iterator i = m_maps.begin(), end = m_maps.end(); i != end; ++i)
		{
			TqFloat weight = (*i)->weight(PC);
			if(weight > maxWeight)
			{
				maxWeight = weight;
				view = (*i).get();
			}
		}
	}
	// Sample the shadow map for the chosen view.
	view->sample(sampleQuad, sampleOpts, outSamps);
}

const CqShadowSampleOptions& CqShadowSampler::defaultSampleOptions() const
{
	return m_defaultSampleOptions;
}

} // namespace Aqsis
