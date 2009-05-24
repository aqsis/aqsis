// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
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

#include <aqsis/slcomp/icodegen.h>

#include	<sstream>
#include	<deque>
#include	<map>

#include	<aqsis/version.h>
#include	"vmdatagather.h"
#include	"vmoutput.h"
#include	<aqsis/slcomp/iparsenode.h>


namespace Aqsis {

void CqCodeGenVM::OutputTree( IqParseNode* pNode, std::string strOutName )
{
	CqCodeGenDataGather DG;
	CqCodeGenOutput V( &DG, strOutName );
	pNode->Accept( DG );
	pNode->Accept( V );
}


void CreateTranslationTable( IqParseNode* pParam, IqParseNode* pArg, std::vector<std::vector<SqVarRefTranslator> >& Stack )
{
	//	assert( NULL != pParam && NULL != pArg );

	std::vector<SqVarRefTranslator> aTransTable;

	while ( pParam != 0 )
	{
		if ( pArg->IsVariableRef() )
		{
			IqParseNodeVariable * pVarArg;
			pVarArg = static_cast<IqParseNodeVariable*>(pArg->GetInterface( ParseNode_Variable ));

			IqParseNodeVariable* pParamArg;
			if ( ( pParamArg = static_cast<IqParseNodeVariable*>(pParam->GetInterface( ParseNode_Variable ))) != 0 )
			{
				SqVarRefTranslator Trans;
				Trans.m_From = pParamArg->VarRef();
				Trans.m_To = pVarArg->VarRef();
				aTransTable.push_back( Trans );
			}
		}
		pParam = pParam->pNextSibling();
		pArg = pArg->pNextSibling();
	}

	Stack.push_back( aTransTable );
}


IqVarDef* pTranslatedVariable( SqVarRef& Ref, std::vector<std::vector<SqVarRefTranslator> >& Stack )
{
	SqVarRef RealRef = Ref;
	// Firstly see if the top translation table knows about this variable.

	if ( !Stack.empty() )
	{
		std::vector<std::vector<SqVarRefTranslator> >::reverse_iterator iTable = Stack.rbegin();

		int depth = 0;
		while ( iTable != Stack.rend() )
		{
			TqUint i;
			for ( i = 0; i < ( *iTable ).size(); i++ )
			{
				if ( ( *iTable ) [ i ].m_From == RealRef )
				{
					RealRef = ( *iTable ) [ i ].m_To;
					break;
				}
			}
			// Only continue looking for nested translations if it was found at the current level.
			if ( i == ( *iTable ).size() )
				break;
			iTable++;
			depth++;
		}
	}
	return ( IqVarDef::GetVariablePtr( RealRef ) );
}


void CreateTempMap( IqParseNode* pParam, IqParseNode* pArg, std::deque<std::map<std::string, std::string> >& Stack,
                    std::vector<std::vector<SqVarRefTranslator> >& Trans,
                    std::map<std::string, IqVarDef*>& TempVars )
{
	assert( NULL != pParam && NULL != pArg );

	std::map<std::string, std::string> mapTrans;
	Stack.push_back( mapTrans );

	while ( pParam != 0 )
	{
		if ( !pArg->IsVariableRef() )
		{
			IqParseNodeVariable * pLocalVar;
			pLocalVar = static_cast<IqParseNodeVariable*>(pParam->GetInterface( ParseNode_Variable ));
			std::stringstream strTempName;
			strTempName << "_" << Stack.size() << "$" << pLocalVar->strName() << std::ends;
			Stack.back() [ pLocalVar->strName() ] = std::string( strTempName.str() );

			SqVarRef temp( pLocalVar->VarRef() );
			IqVarDef* pVD = pTranslatedVariable( temp, Trans );
			TempVars[ strTempName.str() ] = pVD;
			pVD->IncUseCount();
		}
		pParam = pParam->pNextSibling();
		pArg = pArg->pNextSibling();
	}
}

std::string* FindTemporaryVariable( std::string strName, std::deque<std::map<std::string, std::string> >& Stack )
{
	std::deque<std::map<std::string, std::string> >::reverse_iterator iStackEntry;
	for ( iStackEntry = Stack.rbegin(); iStackEntry != Stack.rend(); iStackEntry++ )
		if ( ( *iStackEntry ).find( strName ) != ( *iStackEntry ).end() )
			return ( &( *iStackEntry ) [ strName ] );
	return ( NULL );
}

//-----------------------------------------------------------------------

} // namespace Aqsis
