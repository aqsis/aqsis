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
 * \brief A randomized quasi-random-number table
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef RANDOMTABLE_H_INCLUDED
#define RANDOMTABLE_H_INCLUDED

#include <aqsis/aqsis.h>

#include <aqsis/math/lowdiscrep.h>
#include <aqsis/math/random.h>

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
class AQSIS_TEX_SHARE Cq2dQuasiRandomTable
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
		/// Note that this table size
		static const TqUint m_tableSize = (1 << 10);
		/// Table of x-positions
		TqFloat m_x[m_tableSize];
		/// Table of y-positions
		TqFloat m_y[m_tableSize];
		/// Random numbers for randomizing the offsets
		CqRandom m_rand;
		/// Psudo-random x-offset
		TqFloat m_offsetX;
		/// Psudo-random y-offset
		TqFloat m_offsetY;
};


//==============================================================================
// Implementation details
//==============================================================================
namespace detail {

/// \todo Multithreading - This cannot be a singleton.
//
// It would be possible to make the table itself a singleton, and have another
// object holding the randomized QMC offsets.
extern Cq2dQuasiRandomTable g_randTab;

}

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

} // namespace Aqsis

#endif
