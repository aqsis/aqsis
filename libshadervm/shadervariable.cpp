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
		\brief Implements the classes for supporting shader variables on micropolygrids.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"aqsis.h"

#include	<iostream>

#include	"shadervariable.h"
#include	"shadervm.h"
#include	"irenderer.h"
//#include	"renderer.h"

START_NAMESPACE( Aqsis )


/*
 * FIXME: These are commented out as they introduce a dependency on libaqsis
 * via the extern pCurrRenderer in renderer.h
 */
CqShaderVariable::CqShaderVariable() : m_fParameter( TqFalse )
{
    

    
}

CqShaderVariable::CqShaderVariable( const char* strName, TqBool fParameter ) : m_strName( strName ), m_fParameter( fParameter )
{
    

    
}

CqShaderVariable::~CqShaderVariable()
{
}


//----------------------------------------------------------------------
/** Outputs a variable type to an output stream.
 */

std::ostream &operator<<( std::ostream &Stream, EqVariableType t )
{
    Stream << gVariableTypeNames[ t ];
    return ( Stream );
}

//----------------------------------------------------------------------
/** Outputs a variable class to an output stream.
 */

std::ostream &operator<<( std::ostream &Stream, EqVariableClass t )
{
    Stream << gVariableClassNames[ t ];
    return ( Stream );
}


END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------
