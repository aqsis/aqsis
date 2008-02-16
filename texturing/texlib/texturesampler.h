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
 * \brief Texture buffer sampling machinery - implementation
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef TEXTURESAMPLER_H_INCLUDED
#define TEXTURESAMPLER_H_INCLUDED

#include "aqsis.h"

#include "samplequad.h"
#include "texturesampleoptions.h"
#include "itexturesampler.h"
#include "lowdiscrep.h"
#include "matrix2d.h"
#include "logging.h"
#include "ewafilter.h"
#include "sampleaccum.h"
#include "texbufsampler.h"

namespace Aqsis {


//------------------------------------------------------------------------------
/** \brief Implementation of texture buffer samplers
 */
template<typename ArrayT>
class AQSISTEX_SHARE CqTextureSampler : public IqTextureSampler
{
	public:
		CqTextureSampler(const boost::shared_ptr<ArrayT>& texData);
		// from IqTextureSampler
		virtual void sample(const SqSampleQuad& sampleQuad,
				const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const;
	private:
		inline TqFloat wrapCoord(TqFloat pos, EqWrapMode mode) const;
		inline CqVector2D texToRasterCoords(const CqVector2D& pos,
				EqWrapMode sMode, EqWrapMode tMode) const;
		/** \brief An extra simple, extra dumb sampler with no filtering.
		 *
		 * Useful for debugging only.
		 *
		 * \param sampleQuad - quadrilateral region to sample over
		 * \param sampleOpts - options to the sampler, including filter widths etc.
		 * \param outSamps - the outSamps samples will be placed here.  
		 */
		void filterSimple(const SqSampleQuad& sampleQuad,
				const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const;

		//--------------------------------------------------
		/** \brief Filter using an Elliptical Weighted Average
		 *
		 * \param sampleQuad - quadrilateral region to sample over
		 * \param sampleOpts - options to the sampler, including filter widths etc.
		 * \param outSamps - the outSamps samples will be placed here.  
		 */
		void filterEWA(const SqSampleQuad& sampleQuad,
				const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const;

		//--------------------------------------------------
		/// Methods for Monte Carlo filtering
		//@{
		/** \brief Perform Bilinear sampling on the underlying image data.
		 *
		 * \param st - texture coordinates
		 * \param sampleOpts - options to the sampler, including filter widths etc.
		 * \param samples - outSamps array for the samples.
		 */
		inline void sampleBilinear(const CqVector2D& st,
				const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const;
		/** \brief Filter using Monte Carlo integration with a box filter.
		 *
		 * Given an arbitrary filter weighting function, determining the
		 * necessary filter coefficients for points in the original image is a
		 * difficult task.  Therefore, when faced with this task, we resort to
		 * Monte Carlo integration over the domain given by the sampling
		 * quadrilateral.  The image values at the Monte Carlo sampling points
		 * are reconstructed via bilinear filtering of the underlying discrete
		 * data.
		 *
		 * \param sampleQuad - quadrilateral region to sample over
		 * \param sampleOpts - options to the sampler, including filter widths etc.
		 * \param outSamps - the outSamps samples will be placed here.  
		 */
		void filterMC(const SqSampleQuad& sampleQuad,
				const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const;
		//@}

		/// May be better put in the CqTextureTileArray class.
		TqFloat dummyGridTex(TqInt s, TqInt t, TqInt sampIdx) const;

	private:
		// instance data
		boost::shared_ptr<ArrayT> m_texData;
		TqFloat m_sMult;
		TqFloat m_tMult;
		// (Analyse performance+complexity/quality tradeoff for offsets?):
		TqFloat m_sOffset;
		TqFloat m_tOffset;
};


//==============================================================================
// Implementation details
//==============================================================================

template<typename ArrayT>
inline CqTextureSampler<ArrayT>::CqTextureSampler(
		const boost::shared_ptr<ArrayT>& texData)
	: m_texData(texData),
	m_sMult(texData->width()),
	m_tMult(texData->height()),
	m_sOffset(0),
	m_tOffset(0)
{ }

template<typename ArrayT>
void CqTextureSampler<ArrayT>::sample(const SqSampleQuad& sampleQuad, const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	SqSampleQuad sampleQuadRemap(sampleQuad);
	sampleQuadRemap.remapPeriodic(sampleOpts.sWrapMode() == WrapMode_Periodic,
			sampleOpts.tWrapMode() == WrapMode_Periodic);
	switch(sampleOpts.filterType())
	{
		case TextureFilter_Box:
			filterMC(sampleQuadRemap, sampleOpts, outSamps);
			break;
		case TextureFilter_None:
			filterSimple(sampleQuadRemap, sampleOpts, outSamps);
			break;
		case TextureFilter_Gaussian:
		default:
			filterEWA(sampleQuadRemap, sampleOpts, outSamps);
			break;
	}
}

template<typename ArrayT>
inline TqFloat CqTextureSampler<ArrayT>::wrapCoord(TqFloat pos, EqWrapMode mode) const
{
	switch(mode)
	{
		case WrapMode_Periodic:
			return pos - lfloor(pos);
			break;
		case WrapMode_Clamp:
			return clamp(pos, 0.0f, 1.0f);
		case WrapMode_Black:
			// Can't handle this case here!
			return pos;
		default:
			// Keep the compiler happy with a default.
			assert(0);
			return 0;
	}
}

template<typename ArrayT>
inline CqVector2D CqTextureSampler<ArrayT>::texToRasterCoords(
		const CqVector2D& pos, EqWrapMode sMode, EqWrapMode tMode) const
{
	return CqVector2D(m_sOffset + m_sMult*wrapCoord(pos.x(), sMode),
			m_tOffset + m_tMult*wrapCoord(pos.y(), tMode));
}

template<typename ArrayT>
void CqTextureSampler<ArrayT>::filterSimple( const SqSampleQuad& sQuad,
		const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	assert(0);
	sampleBilinear((sQuad.v1 + sQuad.v2 + sQuad.v3 + sQuad.v4)/4,
			sampleOpts, outSamps);
}

template<typename ArrayT>
void CqTextureSampler<ArrayT>::filterEWA( const SqSampleQuad& sQuad,
		const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	CqEwaFilterWeights weights(sQuad, m_sMult, m_tMult,
			sampleOpts.sBlur(), sampleOpts.tBlur());
	CqSampleAccum<CqEwaFilterWeights> accumulator(weights,
			sampleOpts.startChannel(), sampleOpts.numChannels(), outSamps);
	CqTexBufSampler<ArrayT>(*m_texData).applyFilter(accumulator, weights.support(),
			sampleOpts.sWrapMode(), sampleOpts.tWrapMode());
}

template<typename ArrayT>
inline void CqTextureSampler<ArrayT>::sampleBilinear(const CqVector2D& st,
		const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	CqVector2D stRaster = texToRasterCoords(st,
			sampleOpts.sWrapMode(), sampleOpts.tWrapMode());
	TqInt s = lfloor(stRaster.x());
	TqInt t = lfloor(stRaster.y());
	TqFloat sInterp = stRaster.x() - s;
	TqFloat tInterp = stRaster.y() - t;

	for(TqInt i = 0; i < sampleOpts.numChannels(); ++i)
	{
		TqFloat texVal1 = dummyGridTex(s, t,i);
		TqFloat texVal2 = dummyGridTex(s+1, t,i);
		TqFloat texVal3 = dummyGridTex(s, t+1,i);
		TqFloat texVal4 = dummyGridTex(s+1, t+1,i);

		outSamps[i] = lerp(tInterp,
				lerp(sInterp, texVal1, texVal2),
				lerp(sInterp, texVal3, texVal4)
				);
	}
}

template<typename ArrayT>
void CqTextureSampler<ArrayT>::filterMC(const SqSampleQuad& sampleQuad,
		const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	assert(0);
	std::vector<TqFloat> tempSamps(sampleOpts.numChannels(), 0);
	// zero all samples
	for(int i = 0; i < sampleOpts.numChannels(); ++i)
		outSamps[i] = 0;
	// Monte Carlo integration loop
	//
	// We use low discrepency random numbers here, since they produce much
	// lower noise sampling than uniform uncorrelated samples as used in
	// traditional Monte Carlo integration.
	//
	// Note that currently this filter method doesn't support blurring, as
	// reflected by the checks in CqTextureSampleOptions.
	//
	/// \todo Possible optimizaton: tabulate the random numbers?
	CqLowDiscrepancy randGen(2);
	for(int i = 0; i < sampleOpts.numSamples(); ++i)
	{
		TqFloat interp1 = randGen.Generate(0, i);
		TqFloat interp2 = randGen.Generate(1, i);
		CqVector2D samplePos = lerp(interp1,
				lerp(interp2, sampleQuad.v1, sampleQuad.v2),
				lerp(interp2, sampleQuad.v3, sampleQuad.v4)
				);
		sampleBilinear(samplePos, sampleOpts, &tempSamps[0]);
		for(int i = 0; i < sampleOpts.numChannels(); ++i)
			outSamps[i] += tempSamps[i];
	}
	// normalize result by the number of samples
	TqFloat renormalizer = 1.0f/sampleOpts.numSamples();
	for(int i = 0; i < sampleOpts.numChannels(); ++i)
		outSamps[i] *= renormalizer;
}

template<typename ArrayT>
TqFloat CqTextureSampler<ArrayT>::dummyGridTex(TqInt s, TqInt t, TqInt sampIdx) const
{
	//assert(s >= 0);
	//assert(t >= 0);
	/*
	const TqInt gridSize = 8;
	const TqInt lineWidth = 1;
	TqFloat outVal = 1;
	if(s < 0)
		s += gridSize*(1-s/gridSize);
	if(t < 0)
		t += gridSize*(1-t/gridSize);
	// Hardcoded grid pattern
	if((s % gridSize) < lineWidth || (t % gridSize) < lineWidth)
		outVal = 0;
	return outVal;
	*/
	if(s >= 0 && t >= 0 && s < m_texData->width() && t < m_texData->height())
	{
		return (*m_texData)(s,t)[sampIdx];
	}
	if(sampIdx == 1)
		return 1;
	return 0;
}

} // namespace Aqsis

#endif // TEXTURESAMPLER_H_INCLUDED
