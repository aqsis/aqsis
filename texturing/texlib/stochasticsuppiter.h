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
 * \brief Randomized (stochastic) iteration over a filter support
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef STOCHASTICSUPPITER_H_INCLUDED
#define STOCHASTICSUPPITER_H_INCLUDED

#include "aqsis.h"

#include "aqsismath.h"
#include "filtersupport.h"
#include "lowdiscrep.h"
#include "random.h"

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief An implementation of 2D randomized-QMC points.
 *
 * There is one potentially severe problem with using a quasi monte carlo (QMC)
 * method to do integrations: The sequence of low discrepency points used is
 * the *same* each time.  If such integrations are used to generate pixels in
 * an image, artefacts will be present due to the correlation between
 * integration points used in neighbouring pixels.
 *
 * Randomized quasi-monte-carlo gets around this problem by somehow
 * "randomizing" the fixed low-discrepency sequence.  One way to do this is to
 * add an offset in the interval [0,1), and map the result back onto the
 * interval [0,1) modulo 1.  Relevant offsets can be obtained by simply using a
 * normal psuedo random number generator.
 */
class AQSISTEX_SHARE Cq2dQuasiRandomTable
{
	public:
		/// Initialize the table with quasi random numbers.
		Cq2dQuasiRandomTable();

		/// "randomize" the low-discrepency sequence by resetting the offset.
		void randomize();

		/// Get the x sample point at the given index.
		TqFloat x(TqUint index) const;
		/// Get the y sample point at the given index.
		TqFloat y(TqUint index) const;
	private:
		static const TqUint m_tableSize = (1 << 10);
		TqFloat m_x[m_tableSize];
		TqFloat m_y[m_tableSize];
		CqRandom m_rand;
		TqFloat m_offsetX;
		TqFloat m_offsetY;
};


//------------------------------------------------------------------------------
/** \brief Stochastic support iterator using randomized QMC
 *
 * Implements SupportIteratorConcept.
 *
 * When a filter support is very large, filtering over every point is very
 * computationally costly.  This support iterator chooses points inside the
 * support according to a randomized low-discrepency sequence (ie, performs the
 * filtering integral by RQMC).
 *
 * The result is a well-distributed set of points inside the support, which
 * nevertheless are different each time the support iterator is reset().
 */
class AQSISTEX_SHARE CqStochasticSuppIter
{
	public:
		/** \brief Construct a support iterator pointing to the beginning of the support.
		 *
		 * \param support - region to iterate over.
		 */
		CqStochasticSuppIter(TqInt numSamples = 200);

		/// Reset the iterator to the beginning of the supplied support.
		void reset(const SqFilterSupport& support);

		/// Move to the next support position
		CqStochasticSuppIter& operator++();

		/// Determine whether the current point is still in the support.
		bool inSupport() const;

		/// Return the current x-position.
		TqInt x() const;
		/// Return the current y-position.
		TqInt y() const;
	private:
		/// \todo Multithreading - This cannot be static.  It would be possible
		// to make the table itself a const static member, and have another
		// object holding the randomized QMC offsets.
		static Cq2dQuasiRandomTable m_randTab;
		/// Filter support region
		SqFilterSupport m_support;
		/// Current x-position
		TqInt m_x;
		/// Current y-position
		TqInt m_y;
		/// Total number of samples to take.
		const TqInt m_numSamples;
		/// Current sample number
		TqInt m_sampleNum;
};


//==============================================================================
// Implementation details
//==============================================================================

//------------------------------------------------------------------------------
// Cq2dQuasiRandomTable

inline void Cq2dQuasiRandomTable::randomize()
{
	m_offsetX = m_rand.RandomFloat();
	m_offsetY = m_rand.RandomFloat();
}

inline TqFloat Cq2dQuasiRandomTable::x(TqUint index) const
{
	TqFloat res = m_x[index & (m_tableSize-1)] + m_offsetX;
	return res - (res >= 1);
}

inline TqFloat Cq2dQuasiRandomTable::y(TqUint index) const
{
	TqFloat res = m_y[index & (m_tableSize-1)] + m_offsetY;
	return res - (res >= 1);
}

//------------------------------------------------------------------------------
// CqStochasticSuppIter
inline CqStochasticSuppIter::CqStochasticSuppIter(TqInt numSamples)
	: m_support(),
	m_x(0),
	m_y(0),
	m_numSamples(numSamples),
	m_sampleNum(-1)
{ }

inline void CqStochasticSuppIter::reset(const SqFilterSupport& support)
{
	m_support = support;
	m_sampleNum = -1;
	// Reset the (x,y) offsets so that consecutive points are random.
	m_randTab.randomize();
	// Call operator++ to generate valid initial sample positions.
	++(*this);
}

inline CqStochasticSuppIter& CqStochasticSuppIter::operator++()
{
	m_sampleNum++;
	m_x = m_support.sx.start + lfloor(m_support.sx.range()*m_randTab.x(m_sampleNum));
	m_y = m_support.sy.start + lfloor(m_support.sy.range()*m_randTab.y(m_sampleNum));
	return *this;
}

inline bool CqStochasticSuppIter::inSupport() const
{
	return m_sampleNum < m_numSamples;
}

inline TqInt CqStochasticSuppIter::x() const
{
	return m_x;
}

inline TqInt CqStochasticSuppIter::y() const
{
	return m_y;
}

} // namespace Aqsis

#endif
