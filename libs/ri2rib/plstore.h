// -*- C++ -*-
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

#ifndef RI2RIB_PLSTORE_H
#define RI2RIB_PLSTORE_H 1

#include <stdarg.h>
#include <vector>
#include <aqsis/ri/ri.h>
#include <aqsis/aqsis.h>

namespace libri2rib {

class CqPLStore
{
	public:
		CqPLStore ( va_list args );
		~CqPLStore ()
		{}
	private:
		std::vector<RtToken> m_Token;
		std::vector<RtPointer> m_Parameter;
	public:
		RtInt n;
		RtToken *tokens()
		{
			if(m_Token.empty())
				return(NULL);
			else
				return & m_Token[ 0 ];
		};
		RtPointer *parms()
		{
			if(m_Parameter.empty())
				return(NULL);
			else
				return & m_Parameter[ 0 ];
		};
};

} // namespace libri2rib
#endif
