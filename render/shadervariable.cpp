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

#include	<iostream>

#include	"aqsis.h"
#include	"shadervariable.h"
#include	"shadervm.h"
#include	"renderer.h"
#include	"imagebuffer.h"

START_NAMESPACE( Aqsis )

char* gVariableTypeNames[] =
    {
        "invalid",
        "float",
        "integer",
        "point",
        "string",
        "color",
        "triple",
        "hpoint",
        "normal",
        "vector",
        "void",
        "matrix",
        "sixteentuple",
    };
TqInt gcVariableTypeNames = sizeof( gVariableTypeNames ) / sizeof( gVariableTypeNames[ 0 ] );

char* gVariableStorageNames[] =
    {
        "invalid",
		"constant",
        "uniform",
        "varying",
        "vertex",
		"facevarying",
    };
TqInt gcVariableStorageNames = sizeof( gVariableStorageNames ) / sizeof( gVariableStorageNames[ 0 ] );

CqShaderVariable::CqShaderVariable()
{
	QGetRenderContext() ->Stats().IncVariablesAllocated();
}

CqShaderVariable::CqShaderVariable( const char* strName ) : m_strName( strName )
{
	QGetRenderContext() ->Stats().IncVariablesAllocated();
}

CqShaderVariable::CqShaderVariable( const CqShaderVariable& From ) : m_strName( From.m_strName )
{
	QGetRenderContext() ->Stats().IncVariablesAllocated();
}

CqShaderVariable::~CqShaderVariable()
{
	QGetRenderContext() ->Stats().IncVariablesDeallocated();
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
/** Clone function for variable array.
 */

IqShaderVariable* CqShaderVariableArray::Clone() const
{
	CqShaderVariableArray * pNew = new CqShaderVariableArray( *this );
	return ( pNew );
}


END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------
