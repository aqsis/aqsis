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
		\brief Compiler backend to output VM code.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef VMOUTPUT_H_INCLUDED
#define VMOUTPUT_H_INCLUDED 1

#include	<vector>

#include	"aqsis.h"

#include	"iparsenode.h"
#include	"ivardef.h"
#include	"ifuncdef.h"

START_NAMESPACE(Aqsis)


#define	VM_SHADER_EXTENSION	".slx"

///----------------------------------------------------------------------
/// SqVarRefTranslator
/// Structure storing a variable reference translation.

struct SqVarRefTranslator
{
	SqVarRef	m_From;
	SqVarRef	m_To;
};

void OutputTree(const IqParseNode* pNode);
void OutputTreeNode(const IqParseNode* pNode, std::ostream& out);
IqVarDef* pTranslatedVariable(SqVarRef& Ref);
void PushTransTable(std::vector<SqVarRefTranslator>* paTransTable);
std::vector<SqVarRefTranslator>*	PopTransTable();

//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif	// !VMOUTPUT_H_INCLUDED
