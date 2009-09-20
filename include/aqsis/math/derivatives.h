// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
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
 * \brief Utilities for computing first differences on grids.
 * \author Chris Foster [ chris42f -at- gmail dot com ]
 */

#ifndef DERIVATIVES_H_INCLUDED
#define DERIVATIVES_H_INCLUDED

#include <aqsis/aqsis.h>

namespace Aqsis {

/** \brief Compute first differences on a grid.
 *
 * This class performs the discrete analogue to differentiation on a regular grid.
 * The discrete first difference on a 1D grid is conceptually just the
 * difference between consecutive grid points:
 *
 *   diff(Y, i) = Y[i+1] - Y[i];
 *
 * This difference is most naturally identified geometrically with the line
 * segment between the grid points i+1 and i, but often we want to return a
 * difference which is relevant to the point i, the centred difference:
 *
 *   diff(Y, i) = (Y[i+1] - Y[i-1])/2;
 *
 * This class provides a choice between the two, depending on the values of
 * "useCentred" in the constructor.
 *
 * For 1D and 0D grids, differences may not be defined in one or both
 * directions; the variables uDiffZero and vDiffZero exist to turn off such
 * derivatives.
 */
class CqGridDiff
{
	public:
		/// Dummy set up for a zero-sized grid (can be fixed later with reset())
		CqGridDiff();

		/// Set up the difference computation.
		CqGridDiff(TqInt uRes, TqInt vRes, bool uDiffZero, bool vDiffZero,
				   bool useCentred);

		/// Reset settings for the difference computation
		void reset(TqInt uRes, TqInt vRes, bool uDiffZero, bool vDiffZero,
				   bool useCentred);

		/** \brief Compute the first difference on the grid in the u-direction.
		 *
		 * This function acts on the grid at the given point.
		 *
		 * \param data - grid holding the data
		 * \param u - u index on the grid
		 * \param v - v index on the grid
		 */
		template<typename T>
		T diffU(const T* data, TqInt u, TqInt v) const;

		/** \brief Compute the first difference on the grid in the v-direction
		 *
		 * \see diffU
		 */
		template<typename T>
		T diffV(const T* data, TqInt u, TqInt v) const;

	private:
		template<typename T>
		static T diff(const T* data, bool useCentred, TqInt stride,
					  TqInt n, TqInt nSize);

		/// u-resolution of the grid.
		TqInt m_uRes;
		/// v-resolution of the grid.
		TqInt m_vRes;
		/// derivatives in the u-direction are assumed to be zero
		bool m_uDiffZero;
		/// derivatives in the v-direction are assumed to be zero
		bool m_vDiffZero;
		/// If true, use centred differences, if false, use one-sided differences.
		bool m_useCentred;
};


//==============================================================================
// Implementation details
//==============================================================================
inline CqGridDiff::CqGridDiff()
	: m_uRes(0),
	m_vRes(0),
	m_uDiffZero(false),
	m_vDiffZero(false),
	m_useCentred(true)
{ }

inline CqGridDiff::CqGridDiff(TqInt uRes, TqInt vRes, bool uDiffZero, bool vDiffZero,
					          bool useCentred)
	: m_uRes(uRes),
	m_vRes(vRes),
	m_uDiffZero(uDiffZero),
	m_vDiffZero(vDiffZero),
	m_useCentred(useCentred)
{ }

inline void CqGridDiff::reset(TqInt uRes, TqInt vRes, bool uDiffZero, bool vDiffZero,
			bool useCentred)
{
	m_uRes = uRes;
	m_vRes = vRes;
	m_uDiffZero = uDiffZero;
	m_vDiffZero = vDiffZero;
	m_useCentred = useCentred;
}

template<typename T>
inline T CqGridDiff::diffU(const T* data, TqInt u, TqInt v) const
{
	// Early return if we have no derivative in the u-direction.
	if(m_uDiffZero)
		return T(0.0f);
	assert(u >= 0 && u < m_uRes);
	assert(v >= 0 && v < m_vRes);
	return diff(data + v*m_uRes + u, m_useCentred,
				1, u, m_uRes);
}

template<typename T>
inline T CqGridDiff::diffV(const T* data, TqInt u, TqInt v) const
{
	// Early return if we have no derivative in the v-direction.
	if(m_vDiffZero)
		return T(0.0f);
	assert(u >= 0 && u < m_uRes);
	assert(v >= 0 && v < m_vRes);
	return diff(data + v*m_uRes + u, m_useCentred,
				m_uRes, v, m_vRes);
}

/** Compute the first difference on a grid; general strided version
 *
 * \param data - array of values from which to compute the differences
 * \param useCentred - if true, use a centred difference scheme, if false use one-sided.
 * \param stride - stride for the dimension along which we're taking the differences.
 * \param n - distance along the dimension of the current point.
 * \param nSize - number of data points in the dimension
 */
template<typename T>
inline T CqGridDiff::diff(const T* data, bool useCentred, TqInt stride,
		TqInt n, TqInt nSize)
{
	if(useCentred && nSize > 2)
	{
		// 2nd order difference scheme, appropriate for use with smooth
		// shading interpolation.  A symmetric centred difference is
		// used where possible, with second order 3-point stencils at
		// the edges of the grids.
		//
		// Using a second order scheme like this is very important to
		// avoid artifacts when neighbouring grids have u and v
		// increasing in different directions, which this is
		// unavoidable for some surface types like SDS.
		if(n == 0)
			return -1.5*data[0] + 2*data[stride] - 0.5*data[2*stride];
		else if(n == nSize-1)
			return 1.5*data[0] - 2*data[-1*stride] + 0.5*data[-2*stride];
		else
			return 0.5*(data[stride] - data[-stride]);
	}
	else
	{
		// Use 1st order one-sided difference scheme.  This is
		// appropriate for use with constant shading interpolation:
		// The one-sided difference may be thought of as a centred
		// differece *between* grid points, which corresponds to
		// micropolygons centres.
		if(n == nSize-1)
			return 0.5*(data[0] - data[-stride]);
		else
			return 0.5*(data[stride] - data[0]);
	}
}

} // namespace Aqsis

#endif // DERIVATIVES_H_INCLUDED
