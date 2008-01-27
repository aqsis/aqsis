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

#include "ewafilter.h"
#include "logging.h"

namespace Aqsis {

/** \brief Estimate the inverse Jacobian of the mapping defined by a sampling quad
 *
 * The four corners of the sampling quad are assumed to point to four corners
 * of a pixel box in the resampled output raster.
 *
 * The averaging in computing the derivatives may be slightly unnecessary, but
 * makes sense for user-supplied 
 */
inline SqMatrix2D estimateJacobianInverse(const SqSampleQuad& sQuad)
{
	// Computes the Jacobian of the inverse texture warp - the warp which takes
	// the destination image to the source image.
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

/* \brief Clamp the filter anisotropy to a maximum value.
 *
 * \param covariance - Covariance matrix for a gaussian filter
 * \param minorAxisWidth - 
 * \param maxAspectRatio - maximum allowable aspect ratio for the filter.
 * \param logEdgeWeight - 
 */
inline void clampEccentricity(SqMatrix2D& covariance, TqFloat& minorAxisWidth,
		TqFloat maxAspectRatio, TqFloat logEdgeWeight)
{
	TqFloat eig1 = 1;
	TqFloat eig2 = 1;
	// Eigenvalues of the covariance matrix are the squares of the lengths of
	// the semi-major and semi-minor axes of the filter support ellipse.
	covariance.eigenvalues(eig1, eig2);
	// eigenvalues() guarentees that eig1 >= eig2, which means we only have to
	// check one inequality here.
	if(maxAspectRatio*maxAspectRatio*eig2 < eig1)
	{
		// Need to perform eccentricity clamping
		SqMatrix2D R = covariance.orthogDiagonalize(eig1, eig2);
		// By construction, covariance = R^T * D * R, where
		// D = SqMatrix2D(eig1, eig2)
		//
		// We modify the diagonal matrix D to replace eig2 with something
		// larger - the clamped value.
		eig2 = eig1/(maxAspectRatio*maxAspectRatio);
		covariance = R * SqMatrix2D(eig1, eig2) * R.transpose();
	}
	minorAxisWidth = std::sqrt(8*eig2*logEdgeWeight);
}

void CqEwaFilterWeights::computeFilter(const SqSampleQuad& sQuad,
		TqFloat baseResS, TqFloat baseResT, TqFloat sBlur, TqFloat tBlur,
		TqFloat maxAspectRatio)
{
	// Get Jacobian of the inverse texture warp, and scale it by the resolution
	// of the base texture.
	SqMatrix2D invJ = SqMatrix2D(baseResS, baseResT)
			* estimateJacobianInverse(sQuad);
	// Compute covariance matrix for the gaussian filter
	//
	// Variances for the reconstruction & prefilters.  A variance of 1/(2*pi)
	// gives a filter with centeral weight 1, but in practise this is slightly
	// too small (resulting in a little bit of aliasing).  Therefore it's
	// adjusted up slightly.  In his original work, Heckbert appears to
	// recommend using 1 for these variances, but this results in excess
	// blurring.
	//
	// Default standard deviation for reconstruction filter - this is the
	// stddev (=sqrt(variance)) of the filter used to reconstruct a continuous
	// image from the underlying discrete samples.
	const TqFloat reconsStdDev = 0.454864184147; // std::sqrt(1.3/(2*M_PI))
	const TqFloat reconsVar = reconsStdDev*reconsStdDev;
	// "Prefilter" variance - this is the variance of the antialiasing filter
	// which is used immediately before resampling onto the discrete grid.
	//
	/// \todo Make sure the prefilter variance is optimized for a good tradeoff between sharpness and aliasing.
	const TqFloat prefilterVar = 1.3/(2*M_PI);
	// Construct the covariance matrix, coVar.
	//
	// Note: This looks slightly different from Heckbert's thesis, since
	// We're using a column-vector convention rather than a row-vector one.
	// That is, the transpose is in the opposite position.
	SqMatrix2D coVar = prefilterVar * invJ*invJ.transpose();
	if(sBlur > 0 || tBlur > 0)
	{
		// The reconstruction variance matrix provides a very nice way of
		// incorporating extra filter blurring if desired, without distorting
		// the filter region in a weird manner.  We do this by adding the
		// standard deviations of the base reconstruction filter with a scaled
		// blur in the s and t directions.
		//
		// The RI spec (version 3.2, page 149) says that blur is "an additional
		// area to be added to the texture area filtered in both the s and t
		// directions, expressed in units of texture coordinates".  From their
		// example it seems like they meant length rather than area...
		//
		// Since we're working in raster units, we scale the blur by the base
		// texture resolution.  This gives the blur in units of number of
		// pixel widths to add to the filter in the two directions.  To turn
		// this into a standard devation for a gaussian filter, we scale by
		// 1/sqrt(12) which is the standard deviation of a box filter of width
		// 1.  (If we don't do this, the blur is much larger than desired,
		// since the support of a gaussian of standard deviation 1 occupies
		// much more than a pixel.)
		const TqFloat oneOnSqrt12 = 0.288675134595;
		TqFloat sStdDev = sBlur*baseResS*oneOnSqrt12 + reconsStdDev;
		TqFloat tStdDev = tBlur*baseResT*oneOnSqrt12 + reconsStdDev;
		coVar += SqMatrix2D(sStdDev*sStdDev, tStdDev*tStdDev);
	}
	else
	{
		coVar += reconsVar;
	}
	// Clamp the eccentricity of the filter.
	clampEccentricity(coVar, m_minorAxisWidth, maxAspectRatio, m_logEdgeWeight);
	// Get the quadratic form
	m_quadForm = 0.5*coVar.inv();
}

} // namespace Aqsis
