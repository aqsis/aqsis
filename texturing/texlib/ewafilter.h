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
 * \brief Utilities for working with Elliptical Weighted Average filters
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef EWAFILTER_H_INCLUDED
#define EWAFILTER_H_INCLUDED

#include "aqsis.h"

#include <cmath>

#include "samplequad.h"
#include "aqsismath.h"
#include "matrix2d.h"
#include "filtersupport.h"

namespace Aqsis {

/** \brief A functor class encapsulating Elliptically Weighted Average (EWA)
 * filter weight calculation.
 *
 * EWA filtering is based on the convolution of several gaussian filters and
 * composition with the linear approximation to the image warp at the sampling
 * point.  The original theory was developed by Paul Heckbert et al. and is
 * well-explained in his masters thesis, "Fundamentals of Texture Mapping and
 * Image Warping", which may be found at http://www.cs.cmu.edu/~ph/.
 * "Physically Based Rendering" also has a small section mentioning EWA.
 *
 * The derivation of an EWA filter may be broken into three conceptual stages:
 *   1) Reconstruct a continuous image from the discrete samples using a
 *      gaussian *reconstruction filter*.  The reconstruction filter support
 *      should be wide enough to avoid samples "falling between" the discrete
 *      points of the source image.
 *   2) Warp this continuous 2D image into another part of the 2D plane with an
 *      arbitrary transformation.  This effect of the transformation on the
 *      filter kernel is approximated by the local linear approximation (ie,
 *      the Jacobian).
 *   3) Filter the resulting image with a gaussian "prefilter" before
 *      converting back to discrete samples.  The support of the prefilter
 *      should be wide enough to remove aliasing.
 *
 * The neat thing about EWA is that the full filter which you get after putting
 * the steps 1-3 together is just another gaussian filter acting on the
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
class CqEwaFilterWeights
{
	public:
		/** \brief Perform EWA filter weight setup
		 *
		 * This initializes the filter to be used over a texture of resolution
		 * baseResS x baseResT.  This is assumed to be the maximum resolution
		 * texture over which we will want to use the filter. The filter can be
		 * adjusted for other lower resolutions using the function
		 * adjustTextureScale().
		 *
		 * \param sQuad - sample quadrilateral representing the preimage of an
		 *          output pixel under the image warp.
		 * \param baseResS - width of the base texture (used to determine a
		 *          minimum reconstruction filter variance)
		 * \param baseResT - height of the base texture (used to determine a
		 *          minimum reconstruction filter variance)
		 * \param sBlur - Additional filter blur in the s-direction
		 * \param tBlur - Additional filter blur in the t-direction
		 * \param logEdgeWeight - Related to the total fraction of the ideal
		 *          filter weight, which is equal to exp(-logEdgeWeight).
		 * \param maxAspectRatio - maximum anisotropy at which the filter will
		 *          be clamped.
		 */
		inline CqEwaFilterWeights(const SqSampleQuad& sQuad, 
				TqFloat baseResS, TqFloat baseResT,
				TqFloat sBlur = 0, TqFloat tBlur = 0,
				TqFloat logEdgeWeight = 4, 
				TqFloat maxAspectRatio = 20);

		/** \brief Adjust the filter to use a texture of a different resolution
		 *
		 * This is useful for adjusting mipmapping where you'd like to 
		 */
		void adjustTextureScale(TqFloat xScale, TqFloat yScale);

		/** \brief Evaluate the filter at the given point in image space.
		 *
		 * \param x
		 * \param y - these parameters are the position in raster space which
		 *            the filter weight is to be calculated at.  (So for an
		 *            image of size WxH, x and y would normally lie somewhere
		 *            in the ranges [0,W] and [0,H] respectively, thought they
		 *            don't have to)
		 */
		inline TqFloat operator()(TqFloat x, TqFloat y) const;

		/// We can't pre-normalize EWA filters; return false.
		inline static bool isNormalized() { return false; }

		/// Get the width of the filter along the minor axis of the ellipse
		inline TqFloat minorAxisWidth();

		/// Get the extent of the filter in integer raster coordinates.
		inline SqFilterSupport support() const;
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
		 * CqEwaFilterWeights constructor.
		 *
		 */
		void computeFilter(const SqSampleQuad& sQuad, TqFloat
				baseResS, TqFloat baseResT, TqFloat sBlur, TqFloat tBlur,
				TqFloat maxAspectRatio);

		/// Quadratic form matrix
		SqMatrix2D m_quadForm;
		/// Center point of the gaussian filter function
		CqVector2D m_filterCenter;
		/// The log of the filter weight at the edge cutoff.
		TqFloat m_logEdgeWeight;
		/// Width of the semi-minor axis of the elliptical filter
		TqFloat m_minorAxisWidth;
};


//==============================================================================
// Implementation details
//==============================================================================
inline CqEwaFilterWeights::CqEwaFilterWeights(const SqSampleQuad& sQuad, 
		TqFloat baseResS, TqFloat baseResT, TqFloat sBlur, TqFloat tBlur,
		TqFloat logEdgeWeight, TqFloat maxAspectRatio)
	: m_quadForm(0),
	m_filterCenter(sQuad.center()),
	m_logEdgeWeight(logEdgeWeight),
	m_minorAxisWidth(0)
{
	// Scale the filterCenter up to the dimensions of the base texture.
	m_filterCenter.x(m_filterCenter.x()*baseResS);
	m_filterCenter.y(m_filterCenter.y()*baseResT);
	// compute and cache the filter
	computeFilter(sQuad, baseResS, baseResT, sBlur, tBlur, maxAspectRatio);
}

inline void CqEwaFilterWeights::adjustTextureScale(TqFloat xScale, TqFloat yScale)
{
	m_filterCenter.x(m_filterCenter.x()*xScale);
	m_filterCenter.y(m_filterCenter.y()*yScale);
	SqMatrix2D scaleMatrix(1/xScale, 1/yScale);
	m_quadForm = scaleMatrix*m_quadForm*scaleMatrix;
}

inline TqFloat CqEwaFilterWeights::operator()(TqFloat x, TqFloat y) const
{
	x -= m_filterCenter.x();
	y -= m_filterCenter.y();
	// evaluate quadratic form
	TqFloat q = m_quadForm.a*x*x + (m_quadForm.b+m_quadForm.c)*x*y
		+ m_quadForm.d*y*y;
	// check whether we're inside the filter cutoff.
	if(q < m_logEdgeWeight)
		return exp(-q);
	/// \todo: Possible optimization: lookup table for exp?
	return 0;
}

inline TqFloat CqEwaFilterWeights::minorAxisWidth()
{
	return m_minorAxisWidth;
}

inline SqFilterSupport CqEwaFilterWeights::support() const
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
