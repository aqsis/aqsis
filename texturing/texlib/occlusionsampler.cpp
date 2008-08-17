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
 * \brief Occlusion texture sampler.
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#include "occlusionsampler.h"

#include "aqsismath.h"
#include "filtertexture.h"
#include "sampleaccum.h"
#include "texexception.h"
#include "tilearray.h"

namespace Aqsis {

//------------------------------------------------------------------------------
// CqOcclusionSampler::CqOccMapVew implementation

namespace {

// Helper classes

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

/// Constant filter weights for PCF on occlusion maps
class CqConstFilter
{
	public:
		TqFloat operator()(TqFloat x, TqFloat y) const
		{
			return 1;
		}
		bool isNormalized() const
		{
			return false;
		}
};

} // unnamed namespace


/** \brief Class representing a single view out of the occlusion map.
 *
 * Each view encapsulates transformation matrices from the "current" coordinate
 * system to the view coordinates, along with a depth map as rendered from that
 * direction.  The view can evaluate how much a given sample region is occluded
 * in the direction opposite to the direction that the view was rendered from.
 */
class CqOcclusionSampler::CqOccView
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
		/** \brief direction from which the depth map was created, inverted.
		 *
		 * This is the direction along which the view can evaluate a percentage
		 * occlusion for a point.
		 */
		CqVector3D m_negViewDirec;
		/// Pixel data for shadow map.
		CqTileArray<TqFloat> m_pixels;

	public:
		/** \brief Create a view from levelNum of the provided file.
		 *
		 * \param
		 * \param
		 */
		CqOccView(const boost::shared_ptr<IqTiledTexInputFile>& file, TqInt levelNum,
				const CqMatrix& currToWorld)
			: m_currToLight(),
			m_currToRaster(),
			m_currToRasterVec(),
			m_negViewDirec(),
			m_pixels(file, levelNum)
		{
			// TODO refactor with CqShadowSampler, also refactor this function,
			// since it's a bit unweildly...
			if(!file)
				AQSIS_THROW(XqInternal,
						"Cannot construct shadow map from NULL file handle");

			const CqTexFileHeader& header = file->header(levelNum);
			if(header.channelList().sharedChannelType() != Channel_Float32)
				AQSIS_THROW(XqBadTexture,
						"Shadow maps must hold 32-bit floating point data");

			// Get matrix which transforms the sample points to the light
			// camera coordinates.
			const CqMatrix* worldToLight
				= header.findPtr<Attr::WorldToCameraMatrix>();
			if(!worldToLight)
			{
				AQSIS_THROW(XqBadTexture, "No world -> camera matrix found in file \""
						<< file->fileName() << "\"");
			}
			m_currToLight = (*worldToLight) * currToWorld;

			// Get matrix which transforms the sample points to texture coordinates.
			const CqMatrix* worldToLightScreen
				= header.findPtr<Attr::WorldToScreenMatrix>();
			if(!worldToLightScreen)
			{
				AQSIS_THROW(XqBadTexture, "No world -> screen matrix found in file \""
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
			m_currToRaster.Scale(0.5f*header.width(), -0.5f*header.height(), 1);
			// This extra translation is by half a pixel width - it moves the
			// raster coordinates 
			m_currToRaster.Translate(CqVector3D(-0.5,-0.5,0));

			// Convert current -> texture transformation into a vector
			// transform rather than a point transform.
			// TODO: Put this stuff into the CqMatrix class?
			m_currToRasterVec = m_currToRaster;
			m_currToRasterVec[3][0] = 0;
			m_currToRasterVec[3][1] = 0;
			m_currToRasterVec[3][2] = 0;
			// This only really makes sense when the matrix is affine rather
			// than projective, ie, the last column is (0,0,0,h) 
			//
			// TODO: Investigate whether this is really correct.
//			assert(m_currToRasterVec[0][3] == 0);
//			assert(m_currToRasterVec[1][3] == 0);
//			assert(m_currToRasterVec[2][3] == 0);
			m_currToRasterVec[0][3] = 0;
			m_currToRasterVec[1][3] = 0;
			m_currToRasterVec[2][3] = 0;

			// Transform the normal (0,0,1) in light space into a normal in
			// "current" space.  The appropriate matrix is the inverse of the
			// cam -> light normal transformation, which itself is the inverse
			// transpose of currToLightVec.
			CqMatrix currToLightVec = m_currToLight;
			currToLightVec[3][0] = 0;
			currToLightVec[3][1] = 0;
			currToLightVec[3][2] = 0;
			m_negViewDirec = currToLightVec.Transpose()*CqVector3D(0,0,-1);
			m_negViewDirec.Unit();
		}

		/** \brief Weight for the occlusion which this surface can contribute
		 * to a surface with normal N.
		 *
		 * The amount of ambient light reaching the surface from the direction
		 * of this view depends on a simple geometric factor: the cosine of the
		 * angle between the surface normal and the view direction.
		 *
		 * \param N - surface normal.
		 */
		TqFloat weight(const CqVector3D& N)
		{
			return N*m_negViewDirec;
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
		void sample(const Sq3DSamplePllgram& sampleRegion,
				const CqShadowSampleOptions& sampleOpts,
				const TqInt numSamples, TqFloat* outSamps)
		{
			// filter weights
			CqConstFilter filterWeights;
			// Use constant depth approximation for the surface for maximum
			// sampling speed.  We use the depth from the camera to the centre
			// of the sample region.
			CqConstDepthApprox depthFunc((m_currToLight*sampleRegion.c).z());
			// Determine rough filter support.  This results in a
			// texture-aligned box, so doesn't do proper anisotropic filtering.
			// For occlusion this isn't visible anyway because of the large
			// amount of averaging.  We also want the filter setup to be as
			// fast as possible.
//			CqVector3D side1 = m_currToRasterVec*sampleRegion.s1;
//			CqVector3D side2 = m_currToRasterVec*sampleRegion.s2;
//			CqVector3D center = m_currToRaster*sampleRegion.c;
//			TqFloat sWidthOn2 = max(side1.x(), side2.x())*m_pixels.width()/2;
//			TqFloat tWidthOn2 = max(side1.y(), side2.y())*m_pixels.height()/2;

			// TODO: Fix the above calculation so that the width is actually
			// taken into account properly.
			CqVector3D center = m_currToRaster*sampleRegion.c;
			TqFloat sWidthOn2 = 0.5*(sampleOpts.sBlur()*m_pixels.width());
			TqFloat tWidthOn2 = 0.5*(sampleOpts.tBlur()*m_pixels.height());
			SqFilterSupport support(
					lround(center.x()-sWidthOn2), lround(center.x()+sWidthOn2) + 1,
					lround(center.y()-tWidthOn2), lround(center.y()+tWidthOn2) + 1);
			// percentage closer accumulator
			CqPcfAccum<CqConstFilter, CqConstDepthApprox> accumulator(
					filterWeights, depthFunc, sampleOpts.startChannel(),
					sampleOpts.biasLow(), sampleOpts.biasHigh(), outSamps);
			// accumulate occlusion over the filter support.
			filterTextureNowrapStochastic(accumulator, m_pixels, support, numSamples);
		}
};

//------------------------------------------------------------------------------
// CqOcclusionSampler implementation

CqOcclusionSampler::CqOcclusionSampler(
		const boost::shared_ptr<IqTiledTexInputFile>& file,
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
				boost::shared_ptr<CqOccView>(new CqOccView(file, i, currToWorld)) );
	}

	m_defaultSampleOptions.fillFromFileHeader(file->header());
}

void CqOcclusionSampler::sample(const Sq3DSamplePllgram& samplePllgram,
		const CqVector3D& normal, const CqShadowSampleOptions& sampleOpts,
		TqFloat* outSamps) const
{
	// Unit normal indicating the hemisphere to sample for occlusion.
	CqVector3D N = normal;
	N.Unit();

	assert(sampleOpts.numChannels() == 1);
	// Calculate the number of samples per map.  The factor of 2 assumes that
	// the input shadow maps are evenly distributed around the scene, so that
	// roughly half of them will be culled via the normal.
	// TODO: Importance sampling!
	TqInt numSamples = max<TqInt>(2*sampleOpts.numSamples()/m_maps.size(), 1);

	// Accumulate the total occlusion over all directions
	TqFloat totOcc = 0;
	TqFloat totWeight = 0;
	for(TqViewVec::const_iterator map = m_maps.begin(), end = m_maps.end();
			map != end; ++map)
	{
		TqFloat weight = (*map)->weight(N);
		if(weight > 0)
		{
			// Compute amount of occlusion from the current view.
			TqFloat occ = 0;
			(*map)->sample(samplePllgram, sampleOpts, numSamples, &occ);
			// Accumulate into total occlusion and weight.
			totOcc += weight * occ;
			totWeight += weight;
		}
	}

	// Normalize the sample
	*outSamps = totOcc / totWeight;
}

const CqShadowSampleOptions& CqOcclusionSampler::defaultSampleOptions() const
{
	return m_defaultSampleOptions;
}

} // namespace Aqsis
