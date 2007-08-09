// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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
		\brief Compiler backend to output VM code.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#ifndef ICODEGEN_H
#define ICODEGEN_H

#include	<vector>

#include	"aqsis.h"

#include	"iparsenode.h"
#include	"ivardef.h"
#include	"ifuncdef.h"

START_NAMESPACE( Aqsis )


#define	VM_SHADER_EXTENSION	".slx"

///----------------------------------------------------------------------
/// SqVarRefTranslator
/// Structure storing a variable reference translation.

struct SqVarRefTranslator
{
	SqVarRef	m_From;
	SqVarRef	m_To;
};


static const char* const gVariableTypeNames[] =
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
        "hextuple"
    } ;
//static const TqUint gcVariableTypeNames = sizeof( gVariableTypeNames ) / sizeof( gVariableTypeNames[ 0 ] );

class IqCodeGen
{
	public:
		virtual void OutputTree( IqParseNode* pNode, std::string strOutName = "" ) = 0;

		virtual ~IqCodeGen()
		{
		};
};

//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// ICODEGEN_H
