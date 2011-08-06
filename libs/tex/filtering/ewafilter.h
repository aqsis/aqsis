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
 * \brief Utilities for working with Elliptical Weighted Average filters
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef EWAFILTER_H_INCLUDED
#define EWAFILTER_H_INCLUDED

#include <aqsis/aqsis.h>

#include <vector>

#include <aqsis/math/math.h>
#include <aqsis/tex/buffers/filtersupport.h>
#include <aqsis/math/matrix2d.h>
#include <aqsis/tex/filtering/samplequad.h>

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief A filter functor for evaluating 2D gaussian EWA filter weights.
 *
 * This filter functor class is conveniently constructed by the
 * CqEwaFilterFactory class.  It's basically a 2D gaussian filter with a
 * cutoff, evaluated as described in the class documentation for
 * CqEwaFilterFactory, and repeated briefly here:
 *
 * \verbatim
 *   Q(x)  = (x-c)^T * Q * (x-c)
 *   W(x)  =  / exp[-Q(x)]    for  Q(x) < logEdgeWeight
 *            \ 0             elsewhere
 * \endverbatim
 *
 * Where c is the filter center, Q is the quadratic form matrix and x is the
 * point at which the filter is being evaluated.  The total fraction of the
 * ideal filter weight which is lost outside the support is equal to
 * exp(-logEdgeWeight).
 */
class CqEwaFilter
{
	public:
		/** Construct a 2D gaussian filter with the given coefficients.
		 *
		 * \param quadForm - quadratic form matrix
		 * \param filterCenter - the filter is centered on this point.
		 * \param logEdgeWeight - log of the filter weight at the edge cutoff.
		 */
		CqEwaFilter(SqMatrix2D quadForm, CqVector2D filterCenter,
				TqFloat logEdgeWeight);

		/// EWA filters are never pre-noramlized; return false.
		static bool isNormalized() { return false; }

		/** \brief Evaluate the filter at the given point in image space.
		 *
		 * \param x
		 * \param y - these parameters are the position in raster space which
		 *            the filter weight is to be calculated at.  (So for an
		 *            image of size WxH, x and y would normally lie somewhere
		 *            in the ranges [0,W] and [0,H] respectively, thought they
		 *            don't have to)
		 */
		TqFloat operator()(TqFloat x, TqFloat y) const;
		/// Get the extent of the filter in integer raster coordinates.
		SqFilterSupport support() const;

	private:
		/// Quadratic form matrix
		const SqMatrix2D m_quadForm;
		/// Center point of the gaussian filter function
		const CqVector2D m_filterCenter;
		/// The log of the filter weight at the filter edge cutoff.
		const TqFloat m_logEdgeWeight;
};

//------------------------------------------------------------------------------
/** \brief A class encapsulating Elliptically Weighted Average (EWA) filter
 * weight computation.
 *
 * EWA filtering is based on the convolution of several gaussian filters and
 * composition with the linear approximation to the image warp at the sampling
 * point.  The original theory was developed by Paul Heckbert et al. and is
 * well-explained in his masters thesis, "Fundamentals of Texture Mapping and
 * Image Warping", which may be found at http://www.cs.cmu.edu/~ph/.
 * "Physically Based Rendering" also has a small section mentioning EWA.
 *
 * The derivation of an EWA filter may be broken into four conceptual stages:
 *   1) Reconstruct a continuous image from the discrete samples using a
 *      gaussian *reconstruction filter*.  The reconstruction filter support
 *      should be wide enough to avoid samples "falling between" the discrete
 *      points of the source image.
 *   2) Apply a texture blur filter to this continuous image.
 *   3) Warp the 2D image into another part of the 2D plane with an
 *      arbitrary transformation.  This effect of the transformation on the
 *      filter kernel is approximated by the local linear approximation (ie,
 *      the Jacobian).
 *   4) Filter the resulting image with a gaussian "prefilter" before
 *      converting back to discrete samples.  The support of the prefilter
 *      should be wide enough to remove aliasing.
 *
 * The neat thing about EWA is that the full filter which you get after putting
 * the steps 1-4 together is just another gaussian filter acting on the
 * original image.  This means we can implement it by simple iteration over a
 * box in the source image which contains the filter support.  The result may
 * be computed deterministically and hence suffers from no sampling error.  The
 * inclusion of the linear transformation implies good anisotropic filtering
 * characteristics.
 *
 * A 2D gaussian filter may be defined by a quadratic form,
 *
 * \verbatim
 *   Q(x,y) = a*x^2 + (b+c)*x*y + d*y^2,
 * \endverbatim
 *
 * such that the filter weights are given by
 *
 * \verbatim
 *   W(x,y) = exp(-Q(x,y)).
 * \endverbatim
 *
 * This filter has infinite support; in practise we need to truncate it at some
 * point, ideally such that we leave only a fixed fraction of the filter
 * weight outside the truncated region.  To do this, we choose a cutoff
 * parameter, C for the weight function, and set all weights less than that
 * cutoff to zero.  The newly truncated filter weight, W', looks like:
 *
 * \verbatim
 *   W'(x,y) = W(x,y)     for W(x,y) >= C
 *           = 0          for W(x,y) < C
 * \endverbatim
 *
 * A curious and convenient feature of such gaussian filters in two dimensions
 * (and only in two dimensions!) is that the fraction of the *total* weight
 * which we're ignoring by doing this is just (1-C).  (To see this is true, do
 * the gaussian integrals inside and outside the cutoff. ;-) )  In practise we
 * work with the quantity logEdgeWeight = -ln(C) below.
 */
class CqEwaFilterFactory
{
	public:
		/// \deprecated
		CqEwaFilterFactory(const SqSampleQuad& sQuad, 
				TqFloat baseResS, TqFloat baseResT,
				TqFloat sBlur = 0, TqFloat tBlur = 0,
				TqFloat logEdgeWeight = 4, 
				TqFloat maxAspectRatio = 20);

		/** \brief Perform EWA filter weight setup
		 *
		 * This initializes the filter to be used over a texture of resolution
		 * baseResS x baseResT.  This is assumed to be the maximum resolution
		 * texture over which we will want to use the filter. The filter can be
		 * adjusted for other lower resolutions using the function
		 * adjustTextureScale().
		 *
		 * \param samplePllgram - sample parallelogram representing an
		 *            approximate preimage of an output pixel box under the
		 *            image warp.  The sides of the parallelogram give the
		 *            linear approximation to the image warp at the centre
		 *            (that is, they represent the Jacobian of the mapping).
		 * \param baseResS - width of the base texture (used to determine a
		 *            minimum reconstruction filter variance)
		 * \param baseResT - height of the base texture (used to determine a
		 *            minimum reconstruction filter variance)
		 * \param blurVariance - Variance matrix for additional filter blur
		 *            (see ewaBlurMatrix() )
		 * \param tBlur - Additional filter blur in the t-direction
		 * \param logEdgeWeight - Related to the total fraction of the ideal
		 *            filter weight, which is equal to exp(-logEdgeWeight).
		 * \param maxAspectRatio - maximum anisotropy at which the filter will
		 *            be clamped.
		 */
		CqEwaFilterFactory(const SqSamplePllgram& samplePllgram,
				TqFloat baseResS, TqFloat baseResT,
				const SqMatrix2D& blurVariance,
				TqFloat logEdgeWeight = 4, 
				TqFloat maxAspectRatio = 20);

		/** \brief Create an EWA filter functor.
		 *
		 * Create an EWA filter functor using a transformation relative to the
		 * base texture coordinates for which the factory coefficients are
		 * calculated.  This is useful for mipmapping where you'd like to
		 * create the filter for the base texture, but adjust it for use with
		 * the raster coordinate system of a higher mipmap level where
		 * necessary.
		 *
		 * The new raster coordinate system relates to the old one via a scale
		 * factor and offset.  For example, the new raster x-coordinates are
		 * given by:
		 *
		 *   xNew = xScale*(xOld + xOff).
		 *
		 * \param xScale - scale factor for old -> new coords
		 * \param xOff - offset for old -> new coords
		 * \param yScale - see xScale
		 * \param yOff - see xOff
		 */
		CqEwaFilter createFilter(TqFloat xScale = 1, TqFloat xOff = 0,
				TqFloat yScale = 1, TqFloat yOff = 0) const;

		/// Get the width of the filter along the minor axis of the ellipse
		TqFloat minorAxisWidth() const;
	private:
		/** \brief Compute and cache EWA filter coefficients
		 *
		 * This function initializes the appropriate matrix for the quadratic
		 * form,
		 *   Q = [a b]
		 *       [c d]
		 * which represents the EWA filter over the quadrilateral given by
		 * sampleQuad.  Q is cached in m_quadForm.  The minimum width along
		 * the minor axis of the filter is cached in m_minorAxisWidth.
		 *
		 * For parameters, see the corresponding ones in the
		 * CqEwaFilterFactory constructor.
		 *
		 */
		void computeFilter(const SqSamplePllgram& samplePllgram, TqFloat baseResS,
				TqFloat baseResT, const SqMatrix2D& blurVariance,
				TqFloat maxAspectRatio);

		/// Quadratic form matrix
		SqMatrix2D m_quadForm;
		/// Center point of the gaussian filter function
		CqVector2D m_filterCenter;
		/// The log of the filter weight at the filter edge cutoff.
		TqFloat m_logEdgeWeight;
		/// Width of the semi-minor axis of the elliptical filter
		TqFloat m_minorAxisWidth;
};


/** \brief Compute the blur variance matrix for axis-aligned blur.
 *
 * The returned matrix gives an appropriate blur ellipse aligned with
 * the x and y axes.
 *
 * \param sBlur - blur in x-direction
 * \param tBlur - blur in y-direction
 */
SqMatrix2D ewaBlurMatrix(TqFloat sBlur, TqFloat tBlur);


//==============================================================================
// Implementation details
//==============================================================================
// CqEwaFilterFactory implementation
/// TODO: - Remove
inline CqEwaFilterFactory::CqEwaFilterFactory(const SqSampleQuad& sQuad, 
		TqFloat baseResS, TqFloat baseResT, TqFloat sBlur, TqFloat tBlur,
		TqFloat logEdgeWeight, TqFloat maxAspectRatio)
	: m_quadForm(0),
	m_filterCenter(sQuad.center()),
	m_logEdgeWeight(logEdgeWeight),
	m_minorAxisWidth(0)
{
	// Scale the filterCenter up to the dimensions of the base texture, and
	// adjust by -0.5 in both directions such that the base texture is
	// *centered* on the unit square.
	m_filterCenter.x(m_filterCenter.x()*baseResS - 0.5);
	m_filterCenter.y(m_filterCenter.y()*baseResT - 0.5);
	// compute and cache the filter
	computeFilter(SqSamplePllgram(sQuad), baseResS, baseResT, ewaBlurMatrix(sBlur, tBlur),
			maxAspectRatio);
}
inline CqEwaFilterFactory::CqEwaFilterFactory(
		const SqSamplePllgram& samplePllgram,
		TqFloat baseResS, TqFloat baseResT,
		const SqMatrix2D& blurVariance,
		TqFloat logEdgeWeight, 
		TqFloat maxAspectRatio)
	: m_quadForm(0),
	m_filterCenter(samplePllgram.c),
	m_logEdgeWeight(logEdgeWeight),
	m_minorAxisWidth(0)
{
	// Scale the filterCenter up to the dimensions of the base texture, and
	// adjust by -0.5 in both directions such that the base texture is
	// *centered* on the unit square.
	m_filterCenter.x(m_filterCenter.x()*baseResS - 0.5);
	m_filterCenter.y(m_filterCenter.y()*baseResT - 0.5);
	// compute and cache the filter
	computeFilter(samplePllgram, baseResS, baseResT, blurVariance, maxAspectRatio);
}

inline SqMatrix2D ewaBlurMatrix(TqFloat sBlur, TqFloat tBlur)
{
	if(sBlur > 0 || tBlur > 0)
	{
		// The factor blurScale gives an amount of blur which is roughly
		// consistent with that used by PRMan (and 3delight) for the same
		// scenes.
		const TqFloat blurScale = 0.5f;
		TqFloat sStdDev = sBlur*blurScale;
		TqFloat tStdDev = tBlur*blurScale;
		return SqMatrix2D(sStdDev*sStdDev, tStdDev*tStdDev);
	}
	else
		return SqMatrix2D(0);
}

inline CqEwaFilter CqEwaFilterFactory::createFilter(TqFloat xScale, TqFloat xOff,
		TqFloat yScale, TqFloat yOff) const
{
	// Special case for the first mipmap level.
	if(xScale == 1 && yScale == 1 && xOff == 0 && yOff == 0)
		return CqEwaFilter(m_quadForm, m_filterCenter, m_logEdgeWeight);
	// Generic case - need to do some scaling etc.
	TqFloat invXs = 1/xScale;
	TqFloat invYs = 1/yScale;
	// The strange-looking matrix which is passed into the CqEwaFilter
	// constructor below is simply the hand-written version of the following M:
	//
	// SqMatrix2D scaleMatrix(1/xScale, 1/yScale);
	// M = scaleMatrix*m_quadForm*scaleMatrix;
	return CqEwaFilter(
			SqMatrix2D(invXs*invXs*m_quadForm.a, invXs*invYs*m_quadForm.b,
				invXs*invYs*m_quadForm.c, invYs*invYs*m_quadForm.d),
			CqVector2D(xScale*(m_filterCenter.x() + xOff),
				yScale*(m_filterCenter.y() + yOff)),
			m_logEdgeWeight);
}

inline TqFloat CqEwaFilterFactory::minorAxisWidth() const
{
	return m_minorAxisWidth;
}


//------------------------------------------------------------------------------
namespace detail {

/** A lookup table for std::exp(-x).
 *
 * The lookup is via operator() which does linear interpolation between
 * tabulated values.
 */
class CqNegExpTable
{
	private:
		std::vector<TqFloat> m_values;
		TqFloat m_invRes;
		TqFloat m_rangeMax;
	public:
		/** \brief Construct the lookup table
		 * \param numPoints - number of points in the table.
		 * \param rangeMax - maximum value of the input variable that the table
		 *                   should be computed for.  Inputs larger than or
		 *                   equal to this will return 0.
		 */
		CqNegExpTable(TqInt numPoints, TqFloat rangeMax)
			: m_values(),
			m_invRes((numPoints-1)/rangeMax),
			m_rangeMax(rangeMax)
		{
			TqFloat res = 1/m_invRes;
			m_values.resize(numPoints);
			for(int i = 0; i < numPoints; ++i)
			{
				m_values[i] = exp(-i*res);
			}
		}

		/** \brief Look up an approximate exp(-x) for x > 0
		 *
		 * This does linear interpolation between x values.
		 * \param x - 
		 */
		TqFloat operator()(TqFloat x) const
		{
			/// \todo Optimization: Possibly may remove some of the checks here.
			if(x >= m_rangeMax)
				return 0;
			TqFloat xRescaled = x*m_invRes;
			TqInt index = lfloor(xRescaled);
			assert(index >= 0);
			TqFloat interp = xRescaled - index;
			return (1-interp)*m_values[index] + interp*m_values[index+1];
		}
};
extern CqNegExpTable negExpTable;

} // namespace detail


//------------------------------------------------------------------------------
// CqEwaFilter implementation
inline CqEwaFilter::CqEwaFilter(SqMatrix2D quadForm, CqVector2D filterCenter,
		TqFloat logEdgeWeight)
	: m_quadForm(quadForm),
	m_filterCenter(filterCenter),
	m_logEdgeWeight(logEdgeWeight)
{ }

inline TqFloat CqEwaFilter::operator()(TqFloat x, TqFloat y) const
{
	x -= m_filterCenter.x();
	y -= m_filterCenter.y();
	// evaluate quadratic form
	TqFloat q = m_quadForm.a*x*x + (m_quadForm.b+m_quadForm.c)*x*y
		+ m_quadForm.d*y*y;
	// Check whether we're inside the filter cutoff; if so use a lookup table
	// to get the filter weight.  Using a lookup table rather than directly
	// using std::exp() results in very large speedups, since the filter
	// weights are needed inside the inner loop
	if(q < m_logEdgeWeight)
		return detail::negExpTable(q);
	return 0;
}

inline SqFilterSupport CqEwaFilter::support() const
{
	TqFloat detQ = m_quadForm.det();
	// Compute filter radii
	TqFloat sRad = std::sqrt(m_quadForm.d*m_logEdgeWeight/detQ);
	TqFloat tRad = std::sqrt(m_quadForm.a*m_logEdgeWeight/detQ);
	return SqFilterSupport(
			lceil(m_filterCenter.x()-sRad),     // startX
			lfloor(m_filterCenter.x()+sRad)+1,  // endX
			lceil(m_filterCenter.y()-tRad),     // startY
			lfloor(m_filterCenter.y()+tRad)+1   // endY
		);
}

} // namespace Aqsis

#endif // EWAFILTER_H_INCLUDED
