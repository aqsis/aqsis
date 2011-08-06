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
 * \brief Occlusion texture sampler.
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#include "occlusionsampler.h"

#include <aqsis/math/math.h>
#include <aqsis/tex/filtering/filtertexture.h>
#include <aqsis/tex/filtering/sampleaccum.h>
#include <aqsis/tex/texexception.h>
#include <aqsis/tex/buffers/tilearray.h>

#include "depthapprox.h"

namespace Aqsis {

//------------------------------------------------------------------------------
// CqOcclusionSampler::CqOccMapVew implementation

namespace {

// Helper classes

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
		/** \brief Create a view from imageNum of the provided file.
		 *
		 * \param file - file from which to read the image data.
		 * \param imageNum - subimage number for this view in the input file
		 * \param currToWorld - current -> world transformation matrix.
		 */
		CqOccView(const boost::shared_ptr<IqTiledTexInputFile>& file, TqInt imageNum,
				const CqMatrix& currToWorld)
			: m_currToLight(),
			m_currToRaster(),
			m_currToRasterVec(),
			m_negViewDirec(),
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
	m_defaultSampleOptions(),
	m_random()
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
	assert(sampleOpts.numChannels() == 1);

	// Unit normal indicating the hemisphere to sample for occlusion.
	CqVector3D N = normal;
	N.Unit();

	const TqFloat sampNumMult = 4.0 * sampleOpts.numSamples() / m_maps.size();

	// Accumulate the total occlusion over all directions.  Here we use an
	// importance sampling approach: we decide how many samples each map should
	// have based on it's relative importance as measured by the map weight.
	TqFloat totOcc = 0;
	TqInt totNumSamples = 0;
	TqFloat maxWeight = 0;
	TqViewVec::const_iterator maxWeightMap = m_maps.begin();
	for(TqViewVec::const_iterator map = m_maps.begin(), end = m_maps.end();
			map != end; ++map)
	{
		TqFloat weight = (*map)->weight(N);
		if(weight > 0)
		{
			// Compute the number of samples to use.  Assuming that the shadow
			// maps are spread evenly over the sphere, we have an area of 
			//
			//    4*PI / m_maps.size()
			//
			// steradians per map.  The density of sample points per steradian
			// should be
			//
			//    sampleOpts.numSamples() * weight / PI
			//
			// Therefore the expected number of samples per map is
			TqFloat numSampFlt = sampNumMult*weight;
			// This isn't an integer though, so we take the floor,
			TqInt numSamples = lfloor(numSampFlt);
			// TODO: Investigate performance impact of using RandomFloat() here.
			if(m_random.RandomFloat() < numSampFlt - numSamples)
			{
				// And increment with a probability equal to the extra fraction
				// of samples that the current map should have.
				++numSamples;
			}
			if(numSamples > 0)
			{
				// Compute amount of occlusion from the current view.
				TqFloat occ = 0;
				(*map)->sample(samplePllgram, sampleOpts, numSamples, &occ);
				// Accumulate into total occlusion and weight.
				totOcc += occ*numSamples;
				totNumSamples += numSamples;
			}
			if(weight > maxWeight)
			{
				maxWeight = weight;
				maxWeightMap = map;
			}
		}
	}

	// The algorithm above sometimes results in no samples being computed for
	// low total sample numbers.  Here we attempt to allow very small numbers
	// of samples to be useful by sampling the most highly weighted map if no
	// samples have been taken
	if(totNumSamples == 0 && maxWeight > 0)
	{
		TqFloat occ = 0;
		(*maxWeightMap)->sample(samplePllgram, sampleOpts, 1, &occ);
		totOcc += occ;
		totNumSamples += 1;
	}

	// Normalize the sample
	*outSamps = totOcc / totNumSamples;
}

const CqShadowSampleOptions& CqOcclusionSampler::defaultSampleOptions() const
{
	return m_defaultSampleOptions;
}

} // namespace Aqsis
