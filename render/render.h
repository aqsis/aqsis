// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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
		\brief Declares the structures and functions for system specific renderer initialisation.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is render.h included already?
#ifndef RENDER_H_INCLUDED
#define RENDER_H_INCLUDED 1

#include	<vector>

#include	"aqsis.h"

START_NAMESPACE( Aqsis )

//----------------------------------------------------------------------
/** \struct SqDisplayMapEntry
 * Structure to store information about display drivers, read from the .ini file.
 */

struct SqDisplayMapEntry
{
	SqDisplayMapEntry()
	{}
	SqDisplayMapEntry( const char* strName, const char* strLocation ) :
			m_strName( strName ),
			m_strLocation( strLocation )
	{}

	CqString	m_strName;			///< The name of the display.
	CqString	m_strLocation;		///< The location of the driver.
}
;


extern std::vector<SqDisplayMapEntry>	gaDisplayMap;

//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !_H_INCLUDED
