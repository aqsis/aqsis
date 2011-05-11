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

#include <aqsis/aqsis.h>

#ifndef AQSIS_SYSTEM_MACOSX
#include	<sstream>
#endif
#include	<fstream>
#include	<deque>
#include	<string>
#include	<map>
#include	<numeric>
#include	<cstdlib>

#include	"vmoutput.h"

#include	"parsenode.h"
#include	<aqsis/math/math.h>
#include	<aqsis/util/logging.h>

namespace Aqsis {


std::string* FindTemporaryVariable( std::string strName, std::deque<std::map<std::string, std::string> >& Stack );
IqVarDef* pTranslatedVariable( SqVarRef& Ref, std::vector<std::vector<SqVarRefTranslator> >& Stack );
void CreateTranslationTable( IqParseNode* pParam, IqParseNode* pArg, std::vector<std::vector<SqVarRefTranslator> >& Stack );
void CreateTempMap( IqParseNode* pParam, IqParseNode* pArg, std::deque<std::map<std::string, std::string> >& Stack,
                    std::vector<std::vector<SqVarRefTranslator> >& Trans, std::map<std::string, IqVarDef*>& TempVars );


void CqCodeGenOutput::Visit( IqParseNode& N )
{
	IqParseNode * pNext = N.pChild();
	while ( pNext )
	{
		pNext->Accept( *this );
		pNext = pNext->pNextSibling();
	}
}

void CqCodeGenOutput::Visit( IqParseNodeShader& S )
{
	IqParseNode * pNode;
	pNode = static_cast<IqParseNode*>(S.GetInterface( ParseNode_Base ));

	// Create a new file for this shader
	if ( strOutName().compare( "" ) == 0 )
	{
		strOutName() = S.strName();
		strOutName().append( VM_SHADER_EXTENSION );
	}

	std::map<std::string, std::string> temp;
	m_StackVarMap.push_back( temp );

	m_slxFile.open( strOutName().c_str() );
	if (m_slxFile.fail( ) )
	{
		std::cout << "Warning: Cannot open file \"" << strOutName().c_str() << "\"" << std::endl;
		exit( 1 );
	}

	// Set the precision for floats.  We need at >= 9 decimal digits to avoid
	// loss of precision for 32 bit IEEE floats.
	m_slxFile.precision(9);

	std::cout << "... " << strOutName().c_str() << std::endl;

	m_slxFile << S.strShaderType() << std::endl;

	// Output version information.
	m_slxFile << "AQSIS_V " << AQSIS_XSTR(AQSIS_SLX_VERSION) << std::endl;
	m_slxFile << std::endl << std::endl << "segment Data" << std::endl;

	// Now that we have this information, work out which standard vars are used.
	/// \todo Code Review: The use of gStandardVars here breaks the data encapsulation of the parser, and should be removed to a better location.
	TqInt Use = m_pDataGather->VariableUsage();
	TqUint i;
	for ( i = 0; i < EnvVars_Last; i++ )
	{
		if ( gStandardVars[ i ].UseCount() > 0 )
			Use |= ( 0x00000001 << i );
	}
	m_slxFile << std::endl << "USES " << Use << std::endl << std::endl;

	// Output any declared variables.
	for ( i = 0; i < gLocalVars.size(); i++ )
		OutputLocalVariable( &gLocalVars[ i ], m_slxFile, strOutName() );

	// Output temporary variables.
	std::map<std::string, IqVarDef*>::iterator iTemp;
	for ( iTemp = TempVars().begin(); iTemp != TempVars().end(); iTemp++ )
	{
		IqVarDef* pVar = ( *iTemp ).second;
		;
		m_slxFile << StorageSpec( pVar->Type() ).c_str() << " "
		<< gVariableTypeNames[ pVar->Type() & Type_Mask ] << " "
		<< ( *iTemp ).first;
		if ( pVar->Type() & Type_Array )
			m_slxFile << "[" << pVar->ArrayLength() << "]";

		m_slxFile << std::endl;
	}

	m_slxFile << std::endl << std::endl << "segment Init" << std::endl;
	for ( i = 0; i < gLocalVars.size(); i++ )
	{
		IqVarDef* pVar = &gLocalVars[ i ];
		if ( pVar->Type() & Type_Param && pVar->pInitialiser() != 0 )
			pVar->pInitialiser() ->Accept( *this );
	}

	m_slxFile << std::endl << std::endl << "segment Code" << std::endl;
	IqParseNode* pCode = pNode->pChild();
	// Output the code tree.
	if ( pCode )
		pCode->Accept( *this );
	/// \note There is another child here, it is the list of arguments, but they don't need to be
	/// output as part of the code segment.

	m_slxFile.close();
}

void CqCodeGenOutput::Visit( IqParseNodeFunctionCall& FC )
{
	// Output the function name.
	IqFuncDef * pFunc = FC.pFuncDef();
	IqParseNode* pNode;
	pNode = static_cast<IqParseNode*>(FC.GetInterface( ParseNode_Base ));
	IqParseNode* pArguments = pNode->pChild();

	if ( !pFunc->fLocal() )
	{
		// Output parameters in reverse order, so that the function can pop them as expected
		if ( pArguments != 0 )
		{
			IqParseNode * pArg = pArguments;
			while ( pArg->pNextSibling() != 0 )
				pArg = pArg->pNextSibling();
			while ( pArg != 0 )
			{
				// Push the argument...
				pArg->Accept( *this );
				pArg = pArg->pPrevSibling();
			}
		}

		// If it is a variable length parameter function, output the number of
		// additional parameters.
		TqInt iAdd = 0;
		if ( ( iAdd = pFunc->VariableLength() ) >= 0 )
		{
			const IqParseNode * pArg = pArguments;
			while ( pArg )
			{
				iAdd--;
				pArg = pArg->pNextSibling();
			}
			// Not happy about this!!
			CqParseNodeFloatConst C( static_cast<TqFloat>( abs( iAdd ) ) );
			C.Accept( *this );
		}

		m_slxFile << "\t" << pFunc->strVMName() << std::endl;
	}
	else
	{
		// Output arguments and pop the parameters off the stack.
		if ( pArguments != 0 && pFunc->pArgs() != 0 && pFunc->pDef() != 0 )
		{
			CreateTempMap( pFunc->pArgs() ->pChild(), pArguments, m_StackVarMap, m_saTransTable, TempVars() );

			IqParseNode * pParam = pFunc->pArgs() ->pChild();
			IqParseNode* pArg = pArguments;
			while ( pParam != 0 )
			{
				if ( !pArg->IsVariableRef() )
				{
					// Push the argument...
					pArg->Accept( *this );
					// ...and pop the parameter
					CqParseNodeAssign Pop( static_cast<CqParseNodeVariable*>( pParam ) );
					Pop.NoDup();
					Pop.Accept( *this );
				}
				pParam = pParam->pNextSibling();
				pArg = pArg->pNextSibling();
			}
		}
		// Output the function body.
		if( NULL != pFunc->pArgs() )
		{
			if ( NULL != pFunc->pDef() )
			{
				CreateTranslationTable( pFunc->pArgs() ->pChild(), pArguments, m_saTransTable );
				pFunc->pDef() ->Accept( *this );
				m_saTransTable.erase( m_saTransTable.end() - 1 );
			}
			m_StackVarMap.pop_back( );
		}
		else
		{
			if ( NULL != pFunc->pDef() )
			{
				CreateTranslationTable( NULL, NULL, m_saTransTable );
				pFunc->pDef() ->Accept( *this );
				m_saTransTable.erase( m_saTransTable.end() - 1 );
			}
		}
	}
}

void CqCodeGenOutput::Visit( IqParseNodeUnresolvedCall& UFC )
{
	// Output the function name.
	IqFuncDef * pFunc = UFC.pFuncDef();

	IqParseNode* pNode;
	pNode = static_cast<IqParseNode*>(UFC.GetInterface( ParseNode_Base ));
	IqParseNode* pArguments = pNode->pChild();

	// Output parameters in reverse order, so that the function can pop them as expected
	if ( pArguments != 0 )
	{
		IqParseNode * pArg = pArguments;
		while ( pArg->pNextSibling() != 0 )
			pArg = pArg->pNextSibling();
		while ( pArg != 0 )
		{
			// Push the argument...
			pArg->Accept( *this );
			pArg = pArg->pPrevSibling();
		}
	}

	// If it is a variable length parameter function, output the number of
	// additional parameters.
	TqInt iAdd = 0;
	if ( ( iAdd = pFunc->VariableLength() ) >= 0 )
	{
		const IqParseNode * pArg = pArguments;
		while ( pArg )
		{
			iAdd--;
			pArg = pArg->pNextSibling();
		}
		// Not happy about this!!
		CqParseNodeFloatConst C( static_cast<TqFloat>( abs( iAdd ) ) );
		C.Accept( *this );
	}

	//  Here I just dump out a string describing my external call requirements.
	m_slxFile << "\texternal \"" << pFunc->strName() << "\" \"" << CqParseNode::TypeIdentifier( pFunc->Type() ) << "\" \"" << pFunc->strParams() << "\"" << std::endl;
}

void CqCodeGenOutput::Visit( IqParseNodeVariable& V )
{
	IqParseNode * pNode;
	pNode = static_cast<IqParseNode*>(V.GetInterface( ParseNode_Base ));

	IqParseNodeVariable* pVN;
	pVN = static_cast<IqParseNodeVariable*>(V.GetInterface( ParseNode_Variable ));

	m_slxFile << "\tpushv ";

	SqVarRef temp( pVN->VarRef() );
	IqVarDef* pVD = pTranslatedVariable( temp, m_saTransTable );
	if ( pVD )
	{
		pVD->IncUseCount();
		std::string* strTempName;
		if ( ( strTempName = FindTemporaryVariable( pVD->strName(), m_StackVarMap ) ) != NULL )
			m_slxFile << strTempName->c_str() << std::endl;
		else
			m_slxFile << pVD->strName() << std::endl;
	}
}

void CqCodeGenOutput::Visit( IqParseNodeArrayVariable& AV )
{
	IqParseNode * pNode;
	pNode = static_cast<IqParseNode*>(AV.GetInterface( ParseNode_Base ));

	IqParseNodeVariable* pVN;
	pVN = static_cast<IqParseNodeVariable*>(AV.GetInterface( ParseNode_Variable ));

	IqParseNode * pExpr = pNode->pChild();
	if ( pExpr != 0 )
	{
		pExpr->Accept( *this );
		m_slxFile << "\tipushv ";
	}
	else
	{
		m_slxFile << "\tpushv ";
	}

	SqVarRef temp( pVN->VarRef() );
	IqVarDef* pVD = pTranslatedVariable( temp, m_saTransTable );
	if ( pVD )
	{
		pVD->IncUseCount();
		std::string* strTempName;
		if ( ( strTempName = FindTemporaryVariable( pVD->strName(), m_StackVarMap ) ) != NULL )
			m_slxFile << strTempName->c_str() << std::endl;
		else
			m_slxFile << pVD->strName() << std::endl;
	}
}

void CqCodeGenOutput::Visit( IqParseNodeVariableAssign& VA )
{
	// Output the assignment expression
	IqParseNode * pNode;
	pNode = static_cast<IqParseNode*>(VA.GetInterface( ParseNode_Base ));

	IqParseNodeVariable* pVN;
	pVN = static_cast<IqParseNodeVariable*>(VA.GetInterface( ParseNode_Variable ));

	IqParseNode * pExpr = pNode->pChild();
	if ( pExpr != 0 )
		pExpr->Accept( *this );

	// Output a dup so that the result remains on the stack.
	if ( !VA.fDiscardResult() )
		m_slxFile << "\tdup" << std::endl;
	m_slxFile << "\tpop ";

	// Output a pop for this variable.
	SqVarRef temp( pVN->VarRef() );
	IqVarDef* pVD = pTranslatedVariable( temp, m_saTransTable );
	if ( pVD )
	{
		pVD->IncUseCount();
		std::string* strTempName;
		if ( ( strTempName = FindTemporaryVariable( pVD->strName(), m_StackVarMap ) ) != NULL )
			m_slxFile << strTempName->c_str() << std::endl;
		else
			m_slxFile << pVD->strName() << std::endl;
	}
}

void CqCodeGenOutput::Visit( IqParseNodeArrayVariableAssign& AVA )
{
	// Output the assignment expression
	IqParseNode * pNode;
	pNode = static_cast<IqParseNode*>(AVA.GetInterface( ParseNode_Base ));

	IqParseNodeVariable* pVN;
	pVN = static_cast<IqParseNodeVariable*>(AVA.GetInterface( ParseNode_Variable ));

	IqParseNodeVariableAssign* pVA;
	pVA = static_cast<IqParseNodeVariableAssign*>(AVA.GetInterface( ParseNode_VariableAssign ));

	IqParseNode * pExpr = pNode->pChild();
	if ( pExpr != 0 )
		pExpr->Accept( *this );

	// Output a dup so that the result remains on the stack.
	if ( !pVA->fDiscardResult() )
		m_slxFile << "\tdup" << std::endl;

	IqParseNode * pIndex = pExpr->pNextSibling();
	pIndex->Accept( *this );
	m_slxFile << "\tipop ";

	// Output a pop for this variable.
	SqVarRef temp( pVN->VarRef() );
	IqVarDef* pVD = pTranslatedVariable( temp, m_saTransTable );
	if ( pVD )
	{
		pVD->IncUseCount();
		std::string* strTempName;
		if ( ( strTempName = FindTemporaryVariable( pVD->strName(), m_StackVarMap ) ) != NULL )
			m_slxFile << strTempName->c_str() << std::endl;
		else
			m_slxFile << pVD->strName() << std::endl;
	}
}

void CqCodeGenOutput::Visit( IqParseNodeOperator& OP )
{
	IqParseNode * pNode;
	pNode = static_cast<IqParseNode*>(OP.GetInterface( ParseNode_Base ));

	IqParseNode* pOperandA = pNode->pChild();
	IqParseNode* pOperandB = pOperandA->pNextSibling();

	const char* pstrAType = "";
	if ( pOperandA )
		pstrAType = gVariableTypeIdentifiers[ pOperandA->ResType() & Type_Mask ];
	const char* pstrBType = "";
	if ( pOperandB )
		pstrBType = gVariableTypeIdentifiers[ pOperandB->ResType() & Type_Mask ];

	if ( pOperandA )
		pOperandA->Accept( *this );
	if ( pOperandB )
		pOperandB->Accept( *this );
	m_slxFile << "\t" << MathOpName( OP.Operator() );
	if ( pNode->NodeType() != ParseNode_LogicalOp )
	{
		if ( pOperandA )
			m_slxFile << pstrAType;
		if ( pOperandB )
			m_slxFile << pstrBType;
	}
	m_slxFile << std::endl;
}

void CqCodeGenOutput::Visit( IqParseNodeMathOp& OP )
{
	IqParseNodeOperator * pOp;
	pOp = static_cast<IqParseNodeOperator*>(OP.GetInterface( ParseNode_Operator ));
	Visit( *pOp );
}

void CqCodeGenOutput::Visit( IqParseNodeRelationalOp& OP )
{
	IqParseNodeOperator * pOp;
	pOp = static_cast<IqParseNodeOperator*>(OP.GetInterface( ParseNode_Operator ));
	Visit( *pOp );
}

void CqCodeGenOutput::Visit( IqParseNodeUnaryOp& OP )
{
	IqParseNodeOperator * pOp;
	pOp = static_cast<IqParseNodeOperator*>(OP.GetInterface( ParseNode_Operator ));
	Visit( *pOp );
}

void CqCodeGenOutput::Visit( IqParseNodeLogicalOp& OP )
{
	IqParseNodeOperator * pOp;
	pOp = static_cast<IqParseNodeOperator*>(OP.GetInterface( ParseNode_Operator ));
	Visit( *pOp );
}

void CqCodeGenOutput::Visit( IqParseNodeDiscardResult& DR )
{
	IqParseNode * pNode;
	pNode = static_cast<IqParseNode*>(DR.GetInterface( ParseNode_Base ));

	IqParseNode * pNext = pNode->pChild();
	while ( pNext )
	{
		pNext->Accept( *this );
		pNext = pNext->pNextSibling();
	}
	m_slxFile << "\tdrop" << std::endl;
}

void CqCodeGenOutput::Visit( IqParseNodeConstantFloat& F )
{
	m_slxFile << "\tpushif " << F.Value() << std::endl;
}

void CqCodeGenOutput::Visit( IqParseNodeConstantString& S )
{
	m_slxFile << "\tpushis \"" << S.strValue() << "\"" << std::endl;
}

void CqCodeGenOutput::Visit( IqParseNodeWhileConstruct& WC )
{
	IqParseNode * pNode;
	pNode = static_cast<IqParseNode*>(WC.GetInterface( ParseNode_Base ));

	TqInt iLabelA = m_gcLabels++;
	TqInt iLabelB = m_gcLabels++;

	IqParseNode* pArg = pNode->pChild();
	assert( pArg != 0 );
	IqParseNode* pStmt = pArg->pNextSibling();
	assert( pStmt != 0 );
	IqParseNode* pStmtInc = pStmt->pNextSibling();

	rsPush();										// push running state
	// Mark current RS stack depth so that break/continue will know where to
	// jump to.
	m_breakDepthStack.push_back(0);
	m_slxFile << ":" << iLabelA << std::endl;		// loop back label
	m_slxFile << "\tS_CLEAR" << std::endl;			// clear current state
	pArg->Accept( *this );							// relation
	m_slxFile << "\tS_GET" << std::endl;			// Get the current state by popping the t[ value off the stack
	m_slxFile << "\tS_JZ " << iLabelB << std::endl;	// exit if false
	m_slxFile << "\tRS_GET" << std::endl;			// get current state to running state
	// The following push and associated pop are needed to implement continue statements correctly.
	rsPush();										// push running state
	pStmt->Accept( *this );							// statement
	rsPop();										// pop the running state
	if( pStmtInc )
		pStmtInc->Accept( *this );					// incrementor
	m_slxFile << "\tjmp " << iLabelA << std::endl;	// loop back jump
	m_slxFile << ":" << iLabelB << std::endl;		// completion label
	m_breakDepthStack.pop_back();
	rsPop();										// pop the running state
}

void CqCodeGenOutput::Visit( IqParseNodeLoopMod& LM )
{
	IqParseNode* pChild = static_cast<IqParseNode*>(LM.GetInterface( ParseNode_Base ))->pChild();
	TqInt breakDepth = 1;
	if(pChild)
	{
		breakDepth = lround( static_cast<IqParseNodeConstantFloat*>(
					pChild->GetInterface(ParseNode_ConstantFloat))->Value() );
	}
	if(breakDepth > static_cast<TqInt>(m_breakDepthStack.size()))
	{
		/// \todo This should have been done by the semantic analysis
		/// stage, but here is better than doing nothing (for now).
		Aqsis::log() << critical << "Break depth too deep\n";
		// kludge: break output so it can't be accidentally used...
		m_slxFile << "\tCANNOT_BREAK - COMPILE ERROR";
		return;
	}
	TqInt rsPopNum = std::accumulate(m_breakDepthStack.rbegin(),
				m_breakDepthStack.rbegin() + breakDepth, 0);
	switch(LM.modType())
	{
		case LoopMod_Break:
			// Need one extra pop for the break.
			m_slxFile << "\tRS_BREAK " << rsPopNum << "\n";
			break;
		case LoopMod_Continue:
			m_slxFile << "\tRS_BREAK " << (rsPopNum-1) << "\n";
			break;
	}
}

void CqCodeGenOutput::Visit( IqParseNodeIlluminateConstruct& IC )
{
	IqParseNode * pNode;
	pNode = static_cast<IqParseNode*>(IC.GetInterface( ParseNode_Base ));

	TqInt iLabelA = m_gcLabels++;
	TqInt iLabelB = m_gcLabels++;

	IqParseNode* pArg = pNode->pChild();
	assert( pArg != 0 );
	IqParseNode* pStmt = pArg->pNextSibling();
	assert( pStmt != 0 );
	m_slxFile << ":" << iLabelA << std::endl;		// loop back label
	m_slxFile << "\tS_CLEAR" << std::endl;			// clear current state
	pArg->Accept( *this );
	if ( IC.fHasAxisAngle() )
		m_slxFile << "\tilluminate2" << std::endl;
	else
		m_slxFile << "\tilluminate" << std::endl;
	m_slxFile << "\tS_JZ " << iLabelB << std::endl;	// exit loop if false
	rsPush();										// push running state
	m_slxFile << "\tRS_GET" << std::endl;			// Get state
	pStmt->Accept( *this );							// statement
	rsPop();										// pop the running state
	m_slxFile << "\tjmp " << iLabelA << std::endl;	// loop back jump
	m_slxFile << ":" << iLabelB << std::endl;		// completion label
}

void CqCodeGenOutput::Visit( IqParseNodeIlluminanceConstruct& IC )
{
	IqParseNode * pNode;
	pNode = static_cast<IqParseNode*>(IC.GetInterface( ParseNode_Base ));

	TqInt iLabelA = m_gcLabels++;
	TqInt iLabelB = m_gcLabels++;
	TqInt iLabelC = m_gcLabels++;

	IqParseNode* pArg = pNode->pChild();
	assert( pArg != 0 );
	IqParseNode* pStmt = pArg->pNextSibling();
	assert( pStmt != 0 );

	// The last child of the arg node is the Point to be illuminated, see Parser.y for confirmation.
	IqParseNode* pInitArg = pArg->pChild();
	while ( pInitArg->pNextSibling() != 0 )
		pInitArg = pInitArg->pNextSibling();
	pInitArg = pInitArg->pPrevSibling();
	// If it has an axisangle, then the previous one is the axis, so pass that in as the surface normal.
	if ( IC.fHasAxisAngle() )
	{
		assert( pInitArg->pPrevSibling() );
		pInitArg->pPrevSibling()->Accept( *this );
		pInitArg->Accept( *this );
		m_slxFile << "\tinit_illuminance2" << std::endl;
	}
	else
	{
		pInitArg->Accept( *this );
		m_slxFile << "\tinit_illuminance" << std::endl;
	}

	m_slxFile << "\tjz " << iLabelB << std::endl;	// Jump if no lightsources.
	m_slxFile << ":" << iLabelA << std::endl;		// loop back label
	m_slxFile << "\tS_CLEAR" << std::endl;			// clear current state
	pArg->Accept( *this );
	if ( IC.fHasAxisAngle() )
		m_slxFile << "\tilluminance2" << std::endl;
	else
		m_slxFile << "\tilluminance" << std::endl;
	m_slxFile << "\tS_JZ " << iLabelC << std::endl;	// skip processing of statement if light has no influence
	rsPush();										// push running state
	m_slxFile << "\tRS_GET" << std::endl;			// Get state
	pStmt->Accept( *this );							// statement
	rsPop();										// pop the running state
	m_slxFile << ":" << iLabelC << std::endl;		// continuation label
	m_slxFile << "\tadvance_illuminance" << std::endl;
	m_slxFile << "\tjnz " << iLabelA << std::endl; // loop back jump
	m_slxFile << ":" << iLabelB << std::endl;		// completion label
}

void CqCodeGenOutput::Visit( IqParseNodeSolarConstruct& SC )
{
	IqParseNode * pNode;
	pNode = static_cast<IqParseNode*>(SC.GetInterface( ParseNode_Base ));

	TqInt iLabelA = m_gcLabels++;
	TqInt iLabelB = m_gcLabels++;
	if ( SC.fHasAxisAngle() )
	{
		IqParseNode * pArg = pNode->pChild();
		assert( pArg != 0 );
		IqParseNode* pStmt = pArg->pNextSibling();
		//assert(pStmt!=0);

		m_slxFile << ":" << iLabelA << std::endl;		// loop back label
		m_slxFile << "\tS_CLEAR" << std::endl;			// clear current state
		pArg->Accept( *this );
		m_slxFile << "\tsolar2" << std::endl;
		m_slxFile << "\tS_JZ " << iLabelB << std::endl;	// exit loop if false
		rsPush();										// push running state
		m_slxFile << "\tRS_GET" << std::endl;			// set running state
		if ( pStmt )
			pStmt->Accept( *this );			// statement
		rsPop();										// pop the running state
		m_slxFile << "\tjmp " << iLabelA << std::endl;	// loop back jump
		m_slxFile << ":" << iLabelB << std::endl;		// completion label
	}
	else
	{
		IqParseNode* pStmt = pNode->pChild();
		m_slxFile << ":" << iLabelA << std::endl;		// loop back label
		m_slxFile << "\tS_CLEAR" << std::endl;			// clear current state
		m_slxFile << "\tsolar" << std::endl;
		m_slxFile << "\tS_JZ " << iLabelB << std::endl;	// exit loop if false
		rsPush();										// push running state
		m_slxFile << "\tRS_GET" << std::endl;			// set running state
		if ( pStmt )
			pStmt->Accept( *this );			// statement
		rsPop();										// pop the running state
		m_slxFile << "\tjmp " << iLabelA << std::endl;	// loop back jump
		m_slxFile << ":" << iLabelB << std::endl;		// completion label
	}
}

void CqCodeGenOutput::Visit( IqParseNodeGatherConstruct& IC )
{
	IqParseNode * pNode;
	pNode = static_cast<IqParseNode*>(IC.GetInterface( ParseNode_Base ));

	TqInt iLabelA = m_gcLabels++;
	TqInt iLabelB = m_gcLabels++;
	TqInt iLabelC = m_gcLabels++;
	TqInt iLabelD = m_gcLabels++;

	IqParseNode* pArgs = pNode->pChild();
	assert( pArgs != 0 );
	IqParseNode* pHitStmt = pArgs->pNextSibling();
	assert( pHitStmt != 0 );
	IqParseNode* pNoHitStmt = pHitStmt->pNextSibling();

	// samples is the 5th argument to the gather function, we need that to 
	// initialise the init_gather call.
	IqParseNode* pSamples = pArgs->pChild();
	TqInt iArg = 5;
	while(--iArg > 0)	
	{
		pSamples = pSamples->pNextSibling();
		assert(pSamples);
	}
	pSamples->Accept(*this);

	m_slxFile << "\tinit_gather" << std::endl;

	m_slxFile << ":" << iLabelA << std::endl;		// loop back label
	m_slxFile << "\tS_CLEAR" << std::endl;			// clear current state

	// Output the arguments in reverse order so that the gather() function
	// can pop them in the expected order.
	IqParseNode* pArg = pArgs->pChild(); 
	while ( pArg->pNextSibling() != 0 )
		pArg = pArg->pNextSibling();
	iArg = 0;
	while ( pArg != 0 )
	{
		// Push the argument...
		iArg++;
		pArg->Accept( *this );
		pArg = pArg->pPrevSibling();
	}
	// Now output the count of 'additional' arguments.
	iArg -= 5;
	CqParseNodeFloatConst C( static_cast<TqFloat>( abs( iArg ) ) );
	C.Accept( *this );
	m_slxFile << "\tgather" << std::endl;
	rsPush();										// push running state
	m_slxFile << "\tRS_GET" << std::endl;			// Get state
	m_slxFile << "\tRS_JZ " << iLabelD << std::endl;	// Jump to no hit statement
	pHitStmt->Accept( *this );						// ray hit statement
	m_slxFile << ":" << iLabelD << std::endl;
	if(pNoHitStmt)
	{
		m_slxFile << "\tRS_INVERSE" << std::endl;		// Inverse running state
		m_slxFile << "\tRS_JZ " << iLabelC << std::endl;	// exit if not running
		pNoHitStmt->Accept( *this );						// ray hit statement
	}
	m_slxFile << ":" << iLabelC << std::endl;		// continuation label
	rsPop();										// pop the running state
	m_slxFile << "\tadvance_gather" << std::endl;
	m_slxFile << "\tjnz " << iLabelA << std::endl; // loop back jump
	m_slxFile << ":" << iLabelB << std::endl;		// completion label
}

void CqCodeGenOutput::Visit( IqParseNodeConditional& C )
{
	IqParseNode * pNode;
	pNode = static_cast<IqParseNode*>(C.GetInterface( ParseNode_Base ));

	TqInt iLabelA = m_gcLabels++;
	TqInt iLabelB = iLabelA;

	IqParseNode* pArg = pNode->pChild();
	assert( pArg != 0 );
	IqParseNode* pTrueStmt = pArg->pNextSibling();
	assert( pTrueStmt != 0 );
	IqParseNode* pFalseStmt = pTrueStmt->pNextSibling();

	m_slxFile << "\tS_CLEAR" << std::endl;			// clear current state
	pArg->Accept( *this );							// relation
	m_slxFile << "\tS_GET" << std::endl;			// Get the current state by popping the top value off the stack
	rsPush();										// push running state
	m_slxFile << "\tRS_GET" << std::endl;			// get current state to running state
	if ( pFalseStmt )
	{
		iLabelB = m_gcLabels++;
		m_slxFile << "\tRS_JZ " << iLabelB << std::endl; // skip true statement if all false
	}
	else
		m_slxFile << "\tRS_JZ " << iLabelA << std::endl; // exit if all false
	pTrueStmt->Accept( *this );						// true statement
	if ( pFalseStmt )
	{
		m_slxFile << ":" << iLabelB << std::endl;	// false part label
		m_slxFile << "\tRS_INVERSE" << std::endl;	// Invert result
		m_slxFile << "\tRS_JZ " << iLabelA << std::endl;	// exit if not running
		pFalseStmt->Accept( *this );				// false statement
	}
	m_slxFile << ":" << iLabelA << std::endl;		// conditional exit point
	rsPop();										// pop the running state
}

void CqCodeGenOutput::Visit( IqParseNodeConditionalExpression& CE )
{
	IqParseNode * pNode;
	pNode = static_cast<IqParseNode*>(CE.GetInterface( ParseNode_Base ));

	// Extract the statement trees from the AST
	IqParseNode * pCondition = pNode->pChild();
	assert( pCondition != 0 );
	IqParseNode* pTrueStmt = pCondition->pNextSibling();
	assert( pTrueStmt != 0 );
	IqParseNode* pFalseStmt = pTrueStmt->pNextSibling();
	assert( pFalseStmt != 0 );

	// Write out VM code to evaluate each branch with the correct running state
	m_slxFile << "\tS_CLEAR\n";		// clear current tmp state
	pCondition->Accept( *this );	// evaluate conditional
	m_slxFile << "\tdup\n";			// create a copy of the conditional for the merge step
	m_slxFile << "\tS_GET\n";		// Pop main stack into current tmp state
	rsPush();						// push current running state onto state stack
	m_slxFile << "\tRS_GET\n";		// copy current tmp state to running state
	pTrueStmt->Accept( *this );		// evaluate true statement
	m_slxFile << "\tRS_INVERSE\n";	// Invert running state
	pFalseStmt->Accept( *this );	// evaluate false statement
	rsPop();						// pop the running state

	// The stack now contains:
	//  ... conditional true_stmt_result false_stmt_result
	// Merge the results together
	TqInt typeT = static_cast<TqInt>( pTrueStmt->ResType() & Type_Mask );
	const char* pstrTType = gVariableTypeIdentifiers[ typeT ];
	m_slxFile << "\tmerge" << pstrTType << std::endl;
}

void CqCodeGenOutput::Visit( IqParseNodeTypeCast& TC )
{
	IqParseNode * pNode;
	pNode = static_cast<IqParseNode*>(TC.GetInterface( ParseNode_Base ));

	IqParseNode * pOperand = pNode->pChild();
	assert( pOperand != 0 );

	TqInt typeA = pOperand->ResType() & Type_Mask;
	TqInt typeB = TC.CastTo() & Type_Mask;
	// No need to output a cast for the triple or h types.
	pOperand->Accept( *this );
	if ( !( ( typeA == Type_Point || typeA == Type_Normal || typeA == Type_Vector ) &&
	        ( typeB == Type_Point || typeB == Type_Normal || typeB == Type_Vector ) ) )
	{
		const char* pstrToType = gVariableTypeIdentifiers[ TC.CastTo() & Type_Mask ];
		const char* pstrFromType = gVariableTypeIdentifiers[ pOperand->ResType() & Type_Mask ];
		m_slxFile << "\tset" << pstrFromType << pstrToType << std::endl;
	}
}

void CqCodeGenOutput::Visit( IqParseNodeTriple& T )
{
	IqParseNode * pNode;
	pNode = static_cast<IqParseNode*>(T.GetInterface( ParseNode_Base ));

	IqParseNode * pA = pNode->pChild();
	assert( pA != 0 );
	IqParseNode* pB = pA->pNextSibling();
	assert( pB != 0 );
	IqParseNode* pC = pB->pNextSibling();
	assert( pC != 0 );

	// Output the 'push'es in reverse, so that Red/X ec is first off the stack when doing a 'sett?' instruction.
	pC->Accept( *this );
	pB->Accept( *this );
	pA->Accept( *this );
}

void CqCodeGenOutput::Visit( IqParseNodeSixteenTuple& ST )
{
	IqParseNode * pNode;
	pNode = static_cast<IqParseNode*>(ST.GetInterface( ParseNode_Base ));

	IqParseNode * p00 = pNode->pChild();
	assert( p00 != 0 );
	IqParseNode* p01 = p00->pNextSibling();
	assert( p01 != 0 );
	IqParseNode* p02 = p01->pNextSibling();
	assert( p02 != 0 );
	IqParseNode* p03 = p02->pNextSibling();
	assert( p03 != 0 );

	IqParseNode* p10 = p03->pNextSibling();
	assert( p10 != 0 );
	IqParseNode* p11 = p10->pNextSibling();
	assert( p11 != 0 );
	IqParseNode* p12 = p11->pNextSibling();
	assert( p12 != 0 );
	IqParseNode* p13 = p12->pNextSibling();
	assert( p13 != 0 );

	IqParseNode* p20 = p13->pNextSibling();
	assert( p20 != 0 );
	IqParseNode* p21 = p20->pNextSibling();
	assert( p21 != 0 );
	IqParseNode* p22 = p21->pNextSibling();
	assert( p22 != 0 );
	IqParseNode* p23 = p22->pNextSibling();
	assert( p23 != 0 );

	IqParseNode* p30 = p23->pNextSibling();
	assert( p30 != 0 );
	IqParseNode* p31 = p30->pNextSibling();
	assert( p31 != 0 );
	IqParseNode* p32 = p31->pNextSibling();
	assert( p32 != 0 );
	IqParseNode* p33 = p32->pNextSibling();
	assert( p33 != 0 );

	p00->Accept( *this );
	p01->Accept( *this );
	p02->Accept( *this );
	p03->Accept( *this );
	p10->Accept( *this );
	p11->Accept( *this );
	p12->Accept( *this );
	p13->Accept( *this );
	p20->Accept( *this );
	p21->Accept( *this );
	p22->Accept( *this );
	p23->Accept( *this );
	p30->Accept( *this );
	p31->Accept( *this );
	p32->Accept( *this );
	p33->Accept( *this );
}

void CqCodeGenOutput::Visit( IqParseNodeMessagePassingFunction& MPF )
{
	IqParseNode * pNode;
	pNode = static_cast<IqParseNode*>(MPF.GetInterface( ParseNode_Base ));

	IqParseNode * pExpr = pNode->pChild();

	if ( pExpr != 0 )
		pExpr->Accept( *this );

	CqString strCommType( "surface" );
	switch ( MPF.CommType() )
	{
			case CommTypeAtmosphere:
			strCommType = "atmosphere";
			break;

			case CommTypeDisplacement:
			strCommType = "displacement";
			break;

			case CommTypeLightsource:
			strCommType = "lightsource";
			break;

			case CommTypeAttribute:
			strCommType = "attribute";
			break;

			case CommTypeOption:
			strCommType = "option";
			break;

			case CommTypeRendererInfo:
			strCommType = "rendererinfo";
			break;

			case CommTypeIncident:
			strCommType = "incident";
			break;

			case CommTypeOpposite:
			strCommType = "opposite";
			break;

			case CommTypeTextureInfo:
			strCommType = "textureinfo";
			break;

			case CommTypeRayInfo:
			strCommType = "rayinfo";
			break;
	}
	// Output the comm function.
	SqVarRef temp( MPF.VarRef() );
	IqVarDef* pVD = pTranslatedVariable( temp, m_saTransTable );
	if ( pVD )
	{
		pVD->IncUseCount();
		if ( MPF.CommType() != CommTypeTextureInfo )
			m_slxFile << "\t" << strCommType.c_str() << " " << pVD->strName() << std::endl;
		else
		{
			IqParseNode * pFileName = pExpr->pNextSibling();
			if ( pFileName != 0 )
				pFileName->Accept( *this );
			m_slxFile << "\t" << strCommType.c_str() << " " << pVD->strName() << std::endl;
		}
	}
}

void CqCodeGenOutput::Visit( IqParseNodeTextureNameWithChannel& MPF )
{
	// Note: This node type is used internally in the parser only, it should never be output.
}

///---------------------------------------------------------------------
/// OutputLocalVariable
/// Output details of this local variable.

void CqCodeGenOutput::OutputLocalVariable( const IqVarDef* pVar, std::ostream& out, std::string strOutName )
{
	if ( pVar->UseCount() > 0 || ( pVar->Type() & Type_Param ) )
	{
		out << StorageSpec( pVar->Type() ).c_str() << " "
		<< gVariableTypeNames[ pVar->Type() & Type_Mask ] << " "
		<< pVar->strName();
		if ( pVar->Type() & Type_Array )
			out << "[" << pVar->ArrayLength() << "]";

		out << std::endl;
	}
}


CqString CqCodeGenOutput::StorageSpec( TqInt Type )
{
	CqString strSpec( "" );

	if ( Type & Type_Output )
		strSpec += "output ";
	if ( Type & Type_Param )
		strSpec += "param ";
	if ( Type & Type_Uniform )
		strSpec += "uniform ";
	if ( Type & Type_Varying )
		strSpec += "varying ";

	return ( strSpec );
}


const char* CqCodeGenOutput::MathOpName( TqInt op )
{
	// Output this nodes operand name.
	switch ( op )
	{
		case Op_Add: return "add";
		case Op_Sub: return "sub";
		case Op_Mul: return "mul";
		case Op_Div: return "div";
		case Op_Dot: return "dot";
		case Op_Crs: return "crs";
		case Op_Mod: return "mod";
		case Op_Lft: return "left";
		case Op_Rgt: return "right";
		case Op_And: return "and";
		case Op_Xor: return "xor";
		case Op_Or: return "or";
		case Op_L: return "ls";
		case Op_G: return "gt";
		case Op_GE: return "ge";
		case Op_LE: return "le";
		case Op_EQ: return "eq";
		case Op_NE: return "ne";
		case Op_Neg: return "neg";
		case Op_BitwiseComplement: return "cmpl";
		case Op_LogicalNot: return "not";
		case Op_LogAnd: return "land";
		case Op_LogOr: return "lor";
		case Op_Plus: 
		default:
			return "error";
	}
}

void CqCodeGenOutput::rsPush()
{
	// push the running state
	if(!m_breakDepthStack.empty())
		m_breakDepthStack.back()++;
	m_slxFile << "\tRS_PUSH" << std::endl;
}

void CqCodeGenOutput::rsPop()
{
	// pop the running state
	m_slxFile << "\tRS_POP" << std::endl;
	if(!m_breakDepthStack.empty())
		m_breakDepthStack.back()--;
}

//-----------------------------------------------------------------------

} // namespace Aqsis
