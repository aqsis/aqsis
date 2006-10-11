// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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
 *  \brief Error reporting class implementation.
 *  \author Lionel J. Lacour (intuition01@online.fr)
 */
#include <iostream>
#include <stdlib.h>
#include "error.h"
#include "logging.h"


/// Storage for the last error number reported.
RtInt RiLastError;

/// Default implementation of ignore error handler.
RtVoid RiErrorIgnore( RtInt cd, RtInt sev, const char *msg )
{}
/// Default implementation of print error handler.

RtVoid RiErrorPrint( RtInt cd, RtInt sev, const char *msg )
{}
/// Default implementation of abort error handler.

RtVoid RiErrorAbort( RtInt cd, RtInt sev, const char *msg )
{}


USING_NAMESPACE( libri2rib );

RtVoid CqError::manage ()
{
	RiLastError = m_Code;
	std::cerr << m_Message1 << m_Message2 << m_Message3 << std::endl;
	if ( m_Severity == RIE_SEVERE )
		exit( EXIT_FAILURE );

	if ( m_ToRib == TqTrue )
	{
		std::string tmp;
		switch ( m_Severity )
		{
				case RIE_INFO:
				tmp = std::string( "INFO: " );
				break;
				case RIE_WARNING:
				tmp = std::string( "WARNING: " );
				break;
				case RIE_ERROR:
				tmp = std::string( "ERROR: " );
				break;
				default:
				break;
		}
		tmp += m_Message1 + m_Message2 + m_Message3;
		RiArchiveRecord( RI_COMMENT, const_cast<char *> ( tmp.c_str() ) );
	}
}


