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
#include	"irenderer.h"

START_NAMESPACE(Aqsis)

char* gVariableTypeNames[]=
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
	"hextuple",
};
TqInt gcVariableTypeNames=sizeof(gVariableTypeNames)/sizeof(gVariableTypeNames[0]);

char* gVariableStorageNames[]=
{
	"invalid",
	"vertex",
	"varying",
	"uniform",
};
TqInt gcVariableStorageNames=sizeof(gVariableStorageNames)/sizeof(gVariableStorageNames[0]);

char* gVariableUsageNames[]=
{
	"function",
	"param",
	"variable",
	"invalid",
};
TqInt gcVariableUsageNames=sizeof(gVariableUsageNames)/sizeof(gVariableUsageNames[0]);

char* gVariableTypeIdentifiers[]=
{
	"@",
	"f",
	"i",
	"p",
	"s",
	"c",
	"t",
	"h",
	"n",
	"v",
	"x",
	"m",
	"w",
};
TqInt gcVariableTypeIdentifiers=sizeof(gVariableTypeIdentifiers)/sizeof(gVariableTypeIdentifiers[0]);


CqShaderVariable::CqShaderVariable() 	
{
	pCurrentRenderer()->Stats().cVariablesAllocated()++;
}

CqShaderVariable::CqShaderVariable(const char* strName) : m_strName(strName)
{
	pCurrentRenderer()->Stats().cVariablesAllocated()++;
}

CqShaderVariable::CqShaderVariable(const CqShaderVariable& From) : m_strName(From.m_strName)
{
	pCurrentRenderer()->Stats().cVariablesAllocated()++;
}

CqShaderVariable::~CqShaderVariable()
{
	pCurrentRenderer()->Stats().cVariablesDeallocated()++;
}


//----------------------------------------------------------------------
/** Outputs a variable type to an output stream.
 */

std::ostream &operator<<(std::ostream &Stream, EqVariableType t)
{
	Stream << gVariableTypeNames[t&Type_Mask];
	return(Stream);
}


//----------------------------------------------------------------------
/** Clone function for variable array.
 */

CqShaderVariable* CqShaderVariableArray::Clone() const
{
	CqShaderVariableArray* pNew=new CqShaderVariableArray(*this);
	return(pNew);
}


END_NAMESPACE(Aqsis)
//---------------------------------------------------------------------
