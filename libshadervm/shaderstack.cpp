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
		\brief Implements the classes and support structures for the shader VM stack.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"aqsis.h"
#include	"shaderstack.h"
#include	"ishaderdata.h"

START_NAMESPACE( Aqsis )

std::vector<CqShaderVariableUniformFloat>	CqShaderStack::m_aUFPool;
// Integer
std::vector<CqShaderVariableUniformPoint>	CqShaderStack::m_aUPPool;
std::vector<CqShaderVariableUniformString>	CqShaderStack::m_aUSPool;
std::vector<CqShaderVariableUniformColor>	CqShaderStack::m_aUCPool;
// Triple
// hPoint
std::vector<CqShaderVariableUniformNormal>	CqShaderStack::m_aUNPool;
std::vector<CqShaderVariableUniformVector>	CqShaderStack::m_aUVPool;
// Void
std::vector<CqShaderVariableUniformMatrix>	CqShaderStack::m_aUMPool;
// SixteenTuple

std::vector<CqShaderVariableVaryingFloat>	CqShaderStack::m_aVFPool;
// Integer
std::vector<CqShaderVariableVaryingPoint>	CqShaderStack::m_aVPPool;
std::vector<CqShaderVariableVaryingString>	CqShaderStack::m_aVSPool;
std::vector<CqShaderVariableVaryingColor>	CqShaderStack::m_aVCPool;
// Triple
// hPoint
std::vector<CqShaderVariableVaryingNormal>	CqShaderStack::m_aVNPool;
std::vector<CqShaderVariableVaryingVector>	CqShaderStack::m_aVVPool;
// Void
std::vector<CqShaderVariableVaryingMatrix>	CqShaderStack::m_aVMPool;

TqInt	CqShaderStack::m_iUPoolTops[ type_last ];
TqInt	CqShaderStack::m_iVPoolTops[ type_last ];

END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------
