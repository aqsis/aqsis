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

#include <aqsis/util/logging.h>

namespace Aqsis {

namespace {

/* \brief Clamp the filter anisotropy to a maximum value.
 *
 * \param covariance - Covariance matrix for a gaussian filter
 * \param minorAxisWidth - return the width of the minor ellipse axis in here.
 * \param maxAspectRatio - maximum allowable aspect ratio for the filter.
 * \param logEdgeWeight - log of the filter weight at the filter edge.
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

} // unnamed namespace

namespace detail {
// Lookup-table for exp(-x) for filter weighting.  20 Data points is probably
// about right, though it's possible to use even less.  Using 10
CqNegExpTable negExpTable(20, 6);

} // namespace detail

void CqEwaFilterFactory::computeFilter(const SqSamplePllgram& samplePllgram,
		TqFloat baseResS, TqFloat baseResT, const SqMatrix2D& blurVariance,
		TqFloat maxAspectRatio)
{
	// Get Jacobian of the inverse texture warp, and scale it by the resolution
	// of the base texture.
	SqMatrix2D invJ = SqMatrix2D(
				samplePllgram.s1.x(), samplePllgram.s2.x(),
				samplePllgram.s1.y(), samplePllgram.s2.y()
			);

	// Reconstruction filter variance (conceptually the filter which
	// reconstructs a continuous image from the underlying discrete samples)
	//
	// A variance of 1/(2*pi) gives a filter with centeral weight 1, but in
	// practise this is slightly too small (resulting in a little bit of
	// aliasing).  Therefore it's adjusted up slightly.  In his original work,
	// Heckbert appears to recommend using 1 for these variances, but this
	// results in excess blurring.
	const TqFloat reconsVar = 1.3/(2*M_PI);
	// "Prefilter" variance (antialiasing filter which is used immediately
	// before resampling onto the discrete grid)
	const TqFloat prefilterVar = 1.3/(2*M_PI);

	// Construct the covariance matrix, coVar.
	//
	// The inverse Jacobian is combined with the blur variance matrix first.
	// The transpose is in the opposite position from Heckbert's thesis since
	// we're using a column-vector rather than row-vector convention.
	//
	// The RI spec (version 3.2, page 149) says that blur is "an additional
	// area to be added to the texture area filtered in both the s and t
	// directions, expressed in units of texture coordinates".  Though from
	// their example it's obvious that they meant length rather than area.
	//
	// The closure of gaussian filters under convolution provides a very nice
	// way of incorporating extra filter blurring in a systematic way:  We
	// imagine that any blur is an extra filter convoluted with the
	// reconstructed image.  For gaussian filters this corresponds to adding
	// covariance matrices.
	SqMatrix2D coVar = prefilterVar*invJ*invJ.transpose() + blurVariance;
	// Transform variance matrix to put it in base texture _raster_ units
	// if S = SqMatrix2D(baseResS, baseResT) then this is just
	// coVar = S*coVar*S;
	coVar.a *= baseResS*baseResS;
	coVar.b *= baseResS*baseResT;
	coVar.c *= baseResS*baseResT;
	coVar.d *= baseResT*baseResT;
	// Finally, add in the reconstruction variance.
	coVar += SqMatrix2D(reconsVar);

	// Clamp the eccentricity of the filter.
	clampEccentricity(coVar, m_minorAxisWidth, maxAspectRatio, m_logEdgeWeight);
	// Get the quadratic form
	m_quadForm = 0.5*coVar.inv();
}

} // namespace Aqsis
