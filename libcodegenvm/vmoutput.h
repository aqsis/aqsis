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
#include	"icodegen.h"

START_NAMESPACE( Aqsis )


#define	VM_SHADER_EXTENSION	".slx"

class CqCodeGenVM : private IqCodeGen 
{
	protected:
	void OutputLocalVariable( const IqVarDef*, std::ostream&, std::string );
	void OutputFunctionCall( const IqFuncDef*, IqParseNode*, std::ostream&, std::string );
	void OutputUnresolvedCall( const IqFuncDef*, IqParseNode*, std::ostream&, std::string );
	const char* MathOpName( TqInt );
	CqString StorageSpec( TqInt );
	virtual std::vector<SqVarRefTranslator>* PopTransTable();
	virtual void PushTransTable( std::vector<SqVarRefTranslator>* );
	virtual IqVarDef* pTranslatedVariable( SqVarRef& );

	std::vector<std::vector<SqVarRefTranslator>*> m_saTransTable;
	TqInt m_gcLabels;
	TqUint m_gInternalFunctionUsage;

	public:
	CqCodeGenVM() : m_gcLabels(0), m_gInternalFunctionUsage(0) {};
	virtual void OutputTree( const IqParseNode*, std::string);
	virtual void OutputTreeNode( const IqParseNode*, std::ostream&, std::string);

};

//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !VMOUTPUT_H_INCLUDED
