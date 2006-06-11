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
		\brief Declares the storage structures for renderman symbols.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef SYMBOLS_H_INCLUDED
#define SYMBOLS_H_INCLUDED 1

#include	<vector>

#include	"aqsis.h"

#include	"ishaderdata.h"
#include	"parameters.h"

START_NAMESPACE( Aqsis )

//----------------------------------------------------------------------
/** \struct SqParameterDeclaration
 * A structure storing information about Renderman parameters.
 */

struct SqParameterDeclaration
{
	SqParameterDeclaration() :
			m_strName( "" ),
			m_Type( type_invalid ),
			m_Class( class_invalid ),
			m_Count( 0 ),
			m_pCreate( 0 ),
			m_hash(0),
			m_strSpace( "" )
	{}
	SqParameterDeclaration( const char* strName, EqVariableType Type, EqVariableClass Class, TqInt Count,
	                        CqParameter* ( *pCreate ) ( const char* strName, TqInt Count ), const char* strSpace ) :
			m_strName( strName ),
			m_Type( Type ),
			m_Class( Class ),
			m_Count( Count ),
			m_pCreate( pCreate ),
			m_strSpace( strSpace )
	{
		m_hash = CqString::hash(strName);
	}


	CqString	m_strName;										///< Name of the parameter.

	TqUlong         m_hash; ///< Hash key for m_strName;

	EqVariableType	m_Type;										///< Type.
	EqVariableClass	m_Class;									///< Class.
	TqInt	m_Count;										///< Array length if an array.
	CqParameter* ( *m_pCreate ) ( const char* strName, TqInt Count );		///< Constructor function.
	CqString	m_strSpace;										///< Specification coordinate system name.
}
;


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !SYMBOLS_H_INCLUDED
