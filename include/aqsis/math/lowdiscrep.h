// Aqsis
// Copyright (C) 1997 - 2005, Paul C. Gregory
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
        \brief Declares the CqLowDiscrepancy class responsible for
                producing low-discrepancy numbers.
        \author Andrew Bromage (ajb@spamcop.net)
*/

#ifndef LOWDISCREP_H_INCLUDED
#define LOWDISCREP_H_INCLUDED 1

#include    <aqsis/aqsis.h>
#include    <aqsis/math/random.h>
#include    <vector>


namespace Aqsis {

//----------------------------------------------------------------------
/** \class CqLowDiscrepancy
 * A quasi-random number generator class.
 */

class AQSIS_MATH_SHARE CqLowDiscrepancy
{
	public:
		/// Constructor
		/**
		    \param p_dim The number of dimensions.
		 */
		CqLowDiscrepancy(TqUint p_dim);

		/// Reset to next set of bases.
		void Reset();

		/// Generate low-discrepancy number
		/**
		    \param p_axis The axis along which to generate the number.
		    \param p_i The ordinal of this sample.
		 */
		TqFloat Generate(TqUint p_axis, TqUint p_i);

	private:
		CqRandom m_Random;
		TqUint m_NextBase;
		TqUint m_Dimensions;
		std::vector<TqUint> m_Bases;
};

//-----------------------------------------------------------------------

} // namespace Aqsis

#endif   // !LOWDISCREP_H_INCLUDED

// vim: ts=4:sts=4:expandtab
