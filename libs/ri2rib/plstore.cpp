// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
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
 *  \brief A class to temporary store RtTokens and RtPointers from parameter lists.
 *  \author Lionel J. Lacour (intuition01@online.fr)
 */
#include "plstore.h"

namespace libri2rib {

CqPLStore::CqPLStore ( va_list args )
{
	RtToken t;
	RtPointer p;
	n = 0;
	t = va_arg( args, RtToken );
	while ( t != RI_NULL )
	{
		m_Token.push_back( t );
		p = va_arg( args, RtPointer );
		m_Parameter.push_back( p );

		n++;
		t = va_arg( args, RtToken );
	}
}

} // namespace libri2rib
