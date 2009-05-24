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
 * \brief Implementation of a randomized quasi-random-number table
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#include "randomtable.h"

namespace Aqsis {

namespace detail {

Cq2dQuasiRandomTable g_randTab;

}

//------------------------------------------------------------------------------
// Cq2dQuasiRandomTable implementation

Cq2dQuasiRandomTable::Cq2dQuasiRandomTable()
	: m_rand()
{
	CqLowDiscrepancy rand(2);
	for(TqUint i = 0; i < m_tableSize; ++i)
	{
		m_x[i] = rand.Generate(0, i+1);
		m_y[i] = rand.Generate(1, i+1);
	}
}

} // namespace Aqsis
