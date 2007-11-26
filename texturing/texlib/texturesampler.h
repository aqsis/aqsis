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

#include "tilearray.h"
#include "aqsismath.h"
#include "samplequad.h"
#include "texturesampleoptions.h"
#include "itexturesampler.h"
#include "random.h"
#include "lowdiscrep.h"

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
	sampleBilinear((sQuad.v1 + sQuad.v2 + sQuad.v3 + sQuad.v4)/4,
			sampleOpts, outSamps);
}

/// \todo Factor this out into aqsistypes
struct SqMatrix2D
{
	// matrix components.  This class considers vectors to be column vectors,
	// and to multipy matrices on the RHS.  This is the usual form used in
	// linear algebra courses.
	//
	// The matrix is
	//
	// [a b]
	// [c d]
	//
	/// \todo Consider whether the components should be public or not.
	TqFloat a;
	TqFloat b;
	TqFloat c;
	TqFloat d;
	// Construct a multiple of the identity.
	inline SqMatrix2D(TqFloat diag)
		: a(diag), b(0), c(0), d(diag)
	{ }
	inline SqMatrix2D(TqFloat a, TqFloat b, TqFloat c, TqFloat d)
		: a(a), b(b), c(c), d(d)
	{ }
	/// Matrix addition
	inline SqMatrix2D operator+(const SqMatrix2D& rhs) const
	{
		return SqMatrix2D(a+rhs.a, b+rhs.b, c+rhs.c, d+rhs.d);
	}
	/// Matrix multiplication
	//@{
	inline SqMatrix2D operator*(const SqMatrix2D& rhs) const
	{
		return SqMatrix2D(
				a*rhs.a + b*rhs.c, a*rhs.b + b*rhs.d,
				c*rhs.a + d*rhs.c, c*rhs.b + d*rhs.d
				);
	}
	inline SqMatrix2D operator*(TqFloat mult) const
	{
		return SqMatrix2D(a*mult, b*mult, c*mult, d*mult);
	}
	friend inline SqMatrix2D operator*(TqFloat mult, const SqMatrix2D& mat);
	//@}

	/// Return the inverse
	inline SqMatrix2D inv() const
	{
		// There's a simple formula for the inverse of a 2D matrix.
		TqFloat D = det();
		return SqMatrix2D(d/D, -b/D, -c/D, a/D);
	}
	/// Return the determinant.
	inline TqFloat det() const
	{
		return a*d - b*c;
	}
	/// Return the matrix transpose
	inline SqMatrix2D transpose() const
	{
		return SqMatrix2D(a, c, b, d);
	}
};

inline SqMatrix2D operator*(TqFloat mult, const SqMatrix2D& mat)
{
	return mat*mult;
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
	// computes what is essentially a scaled version of
	//
	// [ds/du ds/dv]
	// [dt/du dt/dv]
	//
	// by averaging (hence the factor of 0.5)
	return SqMatrix2D(
			0.5*(sQuad.v2.x() - sQuad.v1.x() + sQuad.v4.x() - sQuad.v3.x()),
			0.5*(sQuad.v3.x() - sQuad.v1.x() + sQuad.v4.x() - sQuad.v2.x()),
			0.5*(sQuad.v2.y() - sQuad.v1.y() + sQuad.v4.y() - sQuad.v3.y()),
			0.5*(sQuad.v3.y() - sQuad.v1.y() + sQuad.v4.y() - sQuad.v2.y())
			);
}

/** Get the quadratic form matrix for an EWA filter.
 */
inline SqMatrix2D getEWAQuadForm(const SqSampleQuad& sQuad)
{
	// Get Jacobian
	SqMatrix2D invJ = estimateJacobianInverse(sQuad);
	// Variances for the reconstruction & prefilters.  A variance of 1/(2*pi)
	// gives a filter with centeral weight 1, but in practise this is slightly
	// too small (resulting in a little bit of aliasing).  Therefore it's
	// adjusted up slightly.
	const TqFloat reconsVar = 1.3/(2*M_PI);
	const TqFloat prefilterVar = 1.3/(2*M_PI);
	// Get covariance matrix
	SqMatrix2D coVar = reconsVar*(invJ*invJ.transpose()) + prefilterVar*SqMatrix2D(1);
	// Get the quadratic form
	return 0.5*coVar.inv();
}

template<typename T>
void CqTextureSamplerImpl<T>::filterEWA( const SqSampleQuad& sQuad,
		const SqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	/// \todo Write a function for this.
	// Zero the samples
	for(TqInt i = 0; i < sampleOpts.numChannels; ++i)
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
	SqMatrix2D Q = getEWAQuadForm(rasterSampleQuad);
	TqFloat detQ = Q.det();
	// Radius of filter.
	TqFloat sRad = std::sqrt(Q.d*logEdgeWeight/detQ);
	TqFloat tRad = std::sqrt(Q.a*logEdgeWeight/detQ);
	// Center point of filter
	CqVector2D center = 0.25*(rasterSampleQuad.v1 + rasterSampleQuad.v2
			+ rasterSampleQuad.v3 + rasterSampleQuad.v4);
	// Starting texture coordinates for the averaging
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
				TqFloat weight = exp(-q);
				for(TqInt i = 0; i < sampleOpts.numChannels; ++i)
					outSamps[i] += weight*dummyGridTex(s,t);
				totWeight += weight;
			}
		}
	}
	// Renormalize the samples
	TqFloat renormalizer = 1/totWeight;
	for(TqInt i = 0; i < sampleOpts.numChannels; ++i)
		outSamps[i] *= renormalizer;
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
	/// \todo Decide whether to use MC or QMC samples here.
	/// \todo Possible optimizaton: tabulate the random numbers?
//	CqRandom randGen;
	CqLowDiscrepancy randGen(2);
	for(int i = 0; i < sampleOpts.numSamples; ++i)
	{
//		TqFloat interp1 = randGen.RandomFloat();
//		TqFloat interp2 = randGen.RandomFloat();
		TqFloat interp1 = randGen.Generate(0, i);
		TqFloat interp2 = randGen.Generate(1, i);
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
