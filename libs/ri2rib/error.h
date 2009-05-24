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
 *  \brief Error messages storage class.
 *  \author Lionel J. Lacour (intuition01@online.fr)
 */

#ifndef RI2RIB_ERROR_H
#define RI2RIB_ERROR_H 1

#include <string>
#include <aqsis/aqsis.h>
#include <aqsis/ri/ri.h>

namespace libri2rib {

/// \todo Review: Replace this exception class with one rooted in the XqException heirarchy - it should be a subclass of XqValidationFailure.

class CqError
{
	private:
		RtInt m_Code;
		RtInt m_Severity;
		std::string m_Message1;
		std::string m_Message2;
		std::string m_Message3;
		bool m_ToRib;

	public:
		CqError( RtInt cd, RtInt sev, std::string msg, bool tr )
				: m_Code( cd ), m_Severity( sev ),
				m_Message1( msg ), m_Message2 ( "" ), m_Message3( "" ), m_ToRib( tr )
		{}

		CqError( RtInt cd, RtInt sev, std::string msg1, std::string msg2, std::string msg3, bool tr )
				: m_Code ( cd ), m_Severity( sev ),
				m_Message1( msg1 ), m_Message2( msg2 ), m_Message3( msg3 ), m_ToRib( tr )
		{}

		~CqError()
		{}

		RtVoid manage();
};

} // namespace libri2rib

#endif
