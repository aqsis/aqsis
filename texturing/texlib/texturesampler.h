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

#include <cmath>

#include "aqsismath.h"
#include "samplequad.h"
#include "texturesampleoptions.h"
#include "itexturesampler.h"
#include "random.h"
#include "lowdiscrep.h"
#include "matrix2d.h"
#include "logging.h"

namespace Aqsis {


//------------------------------------------------------------------------------
/** \brief Implementation of texture buffer samplers
 */
template<typename ArrayT>
class CqTextureSampler : public IqTextureSampler
{
	public:
		CqTextureSampler(const boost::shared_ptr<ArrayT>& texData);
		// from IqTextureSampler
		virtual void filter(const SqSampleQuad& sampleQuad,
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
		/// Methods for EWA filtering
		//@{
		//inline CqMatrix2D estimateJacobian(const SqSampleQuad& sampleQuad);
		/** \brief Filter using an Elliptical Weighted Average
		 *
		 * EWA filtering is based on the convolution of several gaussian
		 * filters and composition with the linear approximation to the image
		 * warp at the sampling point.  The original theory was developed in
		 * Paul Heckbert's masters thesis, "Fundamentals of Texture Mapping and
		 * Image Warping", which may be found at http://www.cs.cmu.edu/~ph/.
		 * "Physically Based Rendering" also has a small section mentioning
		 * EWA.
		 *
		 * In EWA the total anisotropic filter acting on the original image
		 * turns out to be simply another gaussian filter with a given angle
		 * and ellipticity.  This means that the result may be computed
		 * deterministically and hence suffers from no sampling error.  The
		 * inclusion of the linear transformation implies good anisotropic
		 * filtering characteristics.
		 *
		 * \param sampleQuad - quadrilateral region to sample over
		 * \param sampleOpts - options to the sampler, including filter widths etc.
		 * \param outSamps - the outSamps samples will be placed here.  
		 */
		void filterEWA(const SqSampleQuad& sampleQuad,
				const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const;
		/** Get the quadratic form defining the gaussian filter for EWA.
		 *
		 * A 2D gaussian filter may be defined by a quadratic form,
		 *   Q(x,y) = a*x^2 + b*x*y + c*y*x + d*y^2,
		 * such that the filter weights are given by
		 *   W(x,y) = exp(-Q(x,y)).
		 *
		 * This function returns the appropriate matrix for the quadratic form,
		 *   Q = [a b]
		 *       [c d]
		 * which represents the EWA filter over the quadrilateral given by sampleQuad.
		 *
		 * \param sampleQuad - quadrilateral to sample over in texture coordinates.
		 * \param sampleOpts - sample options (needed to include blur).
		 *
		 * \return quadratic form matrix defining the EWA filter.
		 */
		inline SqMatrix2D getEWAQuadForm(const SqSampleQuad& sQuad,
				const CqTextureSampleOptions& sampleOpts) const;
		//@}

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
	m_sMult(texData->width()-1),
	m_tMult(texData->height()-1),
	m_sOffset(0),
	m_tOffset(0)
{ }

template<typename ArrayT>
void CqTextureSampler<ArrayT>::filter(const SqSampleQuad& sampleQuad, const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	switch(sampleOpts.filterType())
	{
		case TextureFilter_Box:
			filterMC(sampleQuad, sampleOpts, outSamps);
			break;
		case TextureFilter_None:
			filterSimple(sampleQuad, sampleOpts, outSamps);
			break;
		case TextureFilter_Gaussian:
		default:
			filterEWA(sampleQuad, sampleOpts, outSamps);
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

/** Estimate the inverse Jacobian of the mapping defined by a sampling quad
 *
 * The four corners of the sampling quad are assumed to point to four corners
 * of a pixel box in the resampled output raster.
 *
 * The averaging in computing the derivatives may be slightly unnecessary, but
 * makes sense for user-supplied 
 */
inline SqMatrix2D estimateJacobianInverse(const SqSampleQuad& sQuad)
{
	// Computes what is essentially a scaled version of the jacobian of the
	// mapping
	//
	//
	// [ds/du ds/dv]
	// [dt/du dt/dv]
	//
	// We use some averaging for numerical stability (giving the factor of 0.5).
	return SqMatrix2D(
			0.5*(sQuad.v2.x() - sQuad.v1.x() + sQuad.v4.x() - sQuad.v3.x()),
			0.5*(sQuad.v3.x() - sQuad.v1.x() + sQuad.v4.x() - sQuad.v2.x()),
			0.5*(sQuad.v2.y() - sQuad.v1.y() + sQuad.v4.y() - sQuad.v3.y()),
			0.5*(sQuad.v3.y() - sQuad.v1.y() + sQuad.v4.y() - sQuad.v2.y())
			);
}

/** Get the quadratic form matrix for an EWA filter.
 */
template<typename ArrayT>
inline SqMatrix2D CqTextureSampler<ArrayT>::getEWAQuadForm(const SqSampleQuad& sQuad,
		const CqTextureSampleOptions& sampleOpts) const
{
	// Get Jacobian of the texture warp
	SqMatrix2D invJ = estimateJacobianInverse(sQuad);
	// Compute covariance matrix for the gaussian filter
	//
	// Variances for the reconstruction & prefilters.  A variance of 1/(2*pi)
	// gives a filter with centeral weight 1, but in practise this is slightly
	// too small (resulting in a little bit of aliasing).  Therefore it's
	// adjusted up slightly.
	//
	// Default reconstruction filter variance - this is the variance of the
	// filter used to reconstruct a continuous image from the underlying
	// discrete samples.
	const TqFloat reconsVar = 1.3/(2*M_PI);
	// "Prefilter" variance - this is the variance of the antialiasing filter
	// which is used immediately before resampling onto the discrete grid.
	const TqFloat prefilterVar = 1.3/(2*M_PI);
	// the covariance matrix
	SqMatrix2D coVar = prefilterVar * invJ*invJ.transpose();
	if(sampleOpts.sBlur() != 0 || sampleOpts.tBlur() != 0)
	{
		// The reconstruction variance matrix provides a very nice way of
		// incorporating extra filter blurring if necessary.  Here we do this
		// by adding the extra blur to the variance matrix.
		TqFloat sVariance = sampleOpts.sBlur()*m_sMult;
		sVariance = sVariance*sVariance + reconsVar;
		TqFloat tVariance = sampleOpts.tBlur()*m_tMult;
		tVariance = tVariance*tVariance + reconsVar;
		coVar += SqMatrix2D(sVariance, tVariance);
	}
	else
	{
		// Note: This looks slightly different from Heckbert's thesis, since
		// We're using a column-vector convention rather than a row-vector one.
		// That is, the transpose is in the opposite position.
		coVar += reconsVar;
	}
	// Get the quadratic form
	return 0.5*coVar.inv();
}

template<typename ArrayT>
void CqTextureSampler<ArrayT>::filterEWA( const SqSampleQuad& sQuad,
		const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	// Zero the samples
	for(TqInt i = 0; i < sampleOpts.numChannels(); ++i)
		outSamps[i] = 0;
	// Translate the sample quad to into raster coords.
	/// \todo Deal with edge effects...  This may be tricky.
	SqSampleQuad rasterSampleQuad = SqSampleQuad(
			texToRasterCoords(sQuad.v1, WrapMode_Black, WrapMode_Black),
			texToRasterCoords(sQuad.v2, WrapMode_Black, WrapMode_Black),
			texToRasterCoords(sQuad.v3, WrapMode_Black, WrapMode_Black),
			texToRasterCoords(sQuad.v4, WrapMode_Black, WrapMode_Black)
			);
	// The filter weight at the edge is exp(-logEdgeWeight).  If we choose
	// logEdgeWeight = 4, we have maxIgnoredWeight = exp(-4) ~ 0.01, which
	// should be more than good enough.
	const TqFloat logEdgeWeight = 4;
	// Get the quadratic form matrix
	SqMatrix2D Q = getEWAQuadForm(rasterSampleQuad, sampleOpts);
	TqFloat detQ = Q.det();
	// Radius of filter.
	TqFloat sRad = std::sqrt(Q.d*logEdgeWeight/detQ);
	TqFloat tRad = std::sqrt(Q.a*logEdgeWeight/detQ);
	// Center point of filter
	CqVector2D center = 0.25*(rasterSampleQuad.v1 + rasterSampleQuad.v2
			+ rasterSampleQuad.v3 + rasterSampleQuad.v4);
	// Starting and finishing texture coordinates for the filter support
	TqInt sStart = lceil(center.x()-sRad);
	TqInt tStart = lceil(center.y()-tRad);
	TqInt sEnd = lfloor(center.x()+sRad);
	TqInt tEnd = lfloor(center.y()+tRad);
	TqFloat totWeight = 0;
	for(TqInt s = sStart; s <= sEnd; ++s)
	{
		TqFloat x = s - center.x();
		for(TqInt t = tStart; t <= tEnd; ++t)
		{
			TqFloat y = t - center.y();
			// Evaluate quadratic form.
			TqFloat q = Q.a*x*x + (Q.b+Q.c)*x*y + Q.d*y*y;
			if(q < logEdgeWeight)
			{
				/// \todo: Possible optimization: lookup table for exp?
				TqFloat weight = exp(-q);
				for(TqInt i = 0; i < sampleOpts.numChannels(); ++i)
					outSamps[i] += weight*dummyGridTex(s,t,i);
				totWeight += weight;
			}
		}
	}
	// Renormalize the samples
	TqFloat renormalizer = 1/totWeight;
	for(TqInt i = 0; i < sampleOpts.numChannels(); ++i)
		outSamps[i] *= renormalizer;
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
