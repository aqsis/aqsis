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
