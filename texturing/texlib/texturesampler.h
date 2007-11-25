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

#include <valarray>

#include "tilearray.h"
#include "aqsismath.h"
#include "samplequad.h"
#include "texturesampleoptions.h"
#include "itexturesampler.h"
#include "random.h"

namespace Aqsis {


//------------------------------------------------------------------------------
/** \brief Implementation of texture buffer samplers
 */
template<typename T>
class CqTextureSamplerImpl : public IqTextureSampler
{
	public:
		CqTextureSamplerImpl(const boost::shared_ptr<CqTileArray<T> >& texData);
		virtual void filter(const SqSampleQuad& sampleQuad,
				const SqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const;
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
				const SqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const;

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
				const SqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const;
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
				const SqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const;
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
				const SqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const;
		//@}

		/// May be better put in the CqTextureTileArray class.
		//inline bool putWithinBounds(TqInt& iStart, TqInt& iStop, TqInt& jStart, TqInt& jStop);
		TqFloat dummyGridTex(TqInt s, TqInt t) const;

	private:
		// instance data
		boost::shared_ptr<CqTileArray<T> > m_texData;
		TqFloat m_sMult;
		TqFloat m_tMult;
		/// (Analyse performance+complexity/quality tradeoff for offsets?):
		TqFloat m_sOffset;
		TqFloat m_tOffset;
};


//==============================================================================
// Implementation details
//==============================================================================

template<typename T>
inline CqTextureSamplerImpl<T>::CqTextureSamplerImpl(
		const boost::shared_ptr<CqTileArray<T> >& texData)
	: m_texData(texData),
	m_sMult(511),
	m_tMult(511),
	m_sOffset(0),
	m_tOffset(0)
{ }

template<typename T>
void CqTextureSamplerImpl<T>::filter(const SqSampleQuad& sampleQuad, const SqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	switch(sampleOpts.filterType)
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

template<typename T>
inline TqFloat CqTextureSamplerImpl<T>::wrapCoord(TqFloat pos, EqWrapMode mode) const
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

template<typename T>
inline CqVector2D CqTextureSamplerImpl<T>::texToRasterCoords(
		const CqVector2D& pos, EqWrapMode sMode, EqWrapMode tMode) const
{
	return CqVector2D(m_sOffset + m_sMult*wrapCoord(pos.x(), sMode),
			m_tOffset + m_tMult*wrapCoord(pos.y(), tMode));
}

template<typename T>
void CqTextureSamplerImpl<T>::filterSimple( const SqSampleQuad& sQuad,
		const SqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	/*
	CqVector2D st = texToRasterCoords((sQuad.v1 + sQuad.v2 + sQuad.v3 + sQuad.v4)/4,
			sampleOpts.sWrapMode, sampleOpts.tWrapMode);
	TqFloat texVal = dummyGridTex(lfloor(st.x()+0.5), lfloor(st.y()+0.5));
	for(TqInt i = 1; i > 0; --i, ++outSamps)
		*outSamps = texVal;
	*/
	sampleBilinear((sQuad.v1 + sQuad.v2 + sQuad.v3 + sQuad.v4)/4,
			sampleOpts, outSamps);
}

template<typename T>
void CqTextureSamplerImpl<T>::filterEWA( const SqSampleQuad& sQuad,
		const SqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	/// \todo implementation
}

template<typename T>
inline void CqTextureSamplerImpl<T>::sampleBilinear(const CqVector2D& st,
		const SqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	CqVector2D stRaster = texToRasterCoords(st,
			sampleOpts.sWrapMode, sampleOpts.tWrapMode);
	TqInt s = lfloor(stRaster.x());
	TqInt t = lfloor(stRaster.y());
	TqFloat sInterp = stRaster.x() - s;
	TqFloat tInterp = stRaster.y() - t;

	for(TqInt i = 0; i < sampleOpts.numChannels; ++i)
	{
		TqFloat texVal1 = dummyGridTex(s, t);
		TqFloat texVal2 = dummyGridTex(s+1, t);
		TqFloat texVal3 = dummyGridTex(s, t+1);
		TqFloat texVal4 = dummyGridTex(s+1, t+1);

		outSamps[i] = lerp(tInterp,
				lerp(sInterp, texVal1, texVal2),
				lerp(sInterp, texVal3, texVal4)
				);
	}
}

template<typename T>
void CqTextureSamplerImpl<T>::filterMC(const SqSampleQuad& sampleQuad,
		const SqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	std::vector<TqFloat> tempSamps(sampleOpts.numChannels, 0);
	// zero all samples
	for(int i = 0; i < sampleOpts.numChannels; ++i)
		outSamps[i] = 0;
	// MC integration loop
	/// \todo adjust quad for swidth, sblur etc.
	SqSampleQuad adjustedQuad(sampleQuad);
	CqRandom randGen;
	for(int i = 0; i < sampleOpts.numSamples; ++i)
	{
		TqFloat interp1 = randGen.RandomFloat();
		TqFloat interp2 = randGen.RandomFloat();
		CqVector2D samplePos = lerp(interp1,
				lerp(interp2, adjustedQuad.v1, adjustedQuad.v2),
				lerp(interp2, adjustedQuad.v3, adjustedQuad.v4)
				);
		sampleBilinear(samplePos, sampleOpts, &tempSamps[0]);
		for(int i = 0; i < sampleOpts.numChannels; ++i)
			outSamps[i] += tempSamps[i];
	}
	// normalize result by the number of samples
	TqFloat renormalizer = 1.0f/sampleOpts.numSamples;
	for(int i = 0; i < sampleOpts.numChannels; ++i)
		outSamps[i] *= renormalizer;
}

template<typename T>
TqFloat CqTextureSamplerImpl<T>::dummyGridTex(TqInt s, TqInt t) const
{
	const TqInt gridSize = 8;
	const TqInt lineWidth = 1;
	TqFloat outVal = 1;
	//if((s / gridSize) % 2 == 0 | (t / gridSize) % 2 == 0) // checkered
	if((s % gridSize) < lineWidth || (t % gridSize) < lineWidth) // grid
		outVal = 0;
	return outVal;
}

//------------------------------------------------------------------------------
/** \brief A minimal 2D matrix class for use in texture warping.
 *
 * Don't develop this further until it's found out exactly how much it's needed...
 */
#if 0
class CqMatrix2D
{
	public:
		/** Construct a 2D matrix
		 */
		CqMatrix2D(a,b,c,d);
		/** \brief Factory function to make identity matrix.
		 */
		inline static CqMatrix2D identity();
		inline TqFloat determinant();
		inline CqMatrix2D operator*(CqMatrix2D& rhs);
		inline CqMatrix2D operator+(CqMatrix2D& rhs);
		inline TqFloat operator()(TqInt i, TqInt j);
		CqMatrix2D inverse();
	private:
		TqFloat m_a;
		TqFloat m_b;
		TqFloat m_c;
		TqFloat m_d;
};
#endif

} // namespace Aqsis

#endif // TEXTURESAMPLER_H_INCLUDED
