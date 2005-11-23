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

#include "aqsis.h"

#ifndef AQSIS_SYSTEM_MACOSX
#include	<sstream>
#endif
#include	<fstream>
#include	<deque>
#include	<string>
#include	<map>


#include	"version.h"
#include	"vmdatagather.h"

#include	"parsenode.h"

START_NAMESPACE( Aqsis )

std::string* FindTemporaryVariable( std::string strName, std::deque<std::map<std::string, std::string> >& Stack );
IqVarDef* pTranslatedVariable( SqVarRef& Ref, std::vector<std::vector<SqVarRefTranslator> >& Stack );
void CreateTranslationTable( IqParseNode* pParam, IqParseNode* pArg, std::vector<std::vector<SqVarRefTranslator> >& Stack );
void CreateTempMap( IqParseNode* pParam, IqParseNode* pArg, std::deque<std::map<std::string, std::string> >& Stack,
                    std::vector<std::vector<SqVarRefTranslator> >& Trans, std::map<std::string, IqVarDef*>& TempVars );

void CqCodeGenDataGather::Visit( IqParseNode& N )
{
    IqParseNode * pNext = N.pChild();
    while ( pNext )
    {
        pNext->Accept( *this );
        pNext = pNext->pNextSibling();
    }
}

void CqCodeGenDataGather::Visit( IqParseNodeShader& S )
{
    IqParseNode * pNode;
    S.GetInterface( ParseNode_Base, ( void** ) & pNode );

    // Clear the temp variables array.
    TempVars().clear();

    std::map<std::string, std::string> temp;
    m_StackVarMap.push_back( temp );

    // Do a first pass output to find out which variables are used.
    if ( pNode )
    {
        m_VariableUsage = 0;
        pNode->pChild() ->Accept( *this );
    }
}

void CqCodeGenDataGather::Visit( IqParseNodeFunctionCall& FC )
{
    // Output the function name.
    IqFuncDef * pFunc = FC.pFuncDef();
    IqParseNode* pNode;
    FC.GetInterface( ParseNode_Base, ( void** ) & pNode );
    IqParseNode* pArguments = pNode->pChild();

    if ( !pFunc->fLocal() )
    {
        IqParseNode * pArg = pArguments;
        while ( NULL != pArg )
        {
            pArg->Accept( *this );
            pArg = pArg->pNextSibling();
        }
        // If it is a builtin function, lets just check its standard variable usage.
        m_VariableUsage |= pFunc->InternalUsage();
    }
    else
    {
        if( NULL != pFunc->pArgs() )
        {
            IqParseNode * pParam = pFunc->pArgs() ->pChild();
            IqParseNode* pArg = pArguments;

            CreateTempMap( pParam, pArg, m_StackVarMap, m_saTransTable, TempVars() );

            while ( pParam != 0 )
            {
                if ( !pArg->IsVariableRef() )
                    pArg->Accept( *this );

                pParam = pParam->pNextSibling();
                pArg = pArg->pNextSibling();
            }

            IqParseNode* pDef = pFunc->pDef();
            if ( NULL != pDef )
            {
                CreateTranslationTable( pFunc->pArgs() ->pChild(), pArguments, m_saTransTable );
                pDef->Accept( *this );
                m_saTransTable.erase( m_saTransTable.end() - 1 );
            }
        }
        else
        {
            IqParseNode* pDef = pFunc->pDef();
            if ( NULL != pDef )
            {
                CreateTranslationTable( NULL, NULL, m_saTransTable );
                pDef->Accept( *this );
                m_saTransTable.erase( m_saTransTable.end() - 1 );
            }
        }
        m_StackVarMap.pop_back( );
    }
}

void CqCodeGenDataGather::Visit( IqParseNodeUnresolvedCall& UFC )
{
    IqParseNode * pNode;
    UFC.GetInterface( ParseNode_Base, ( void** ) & pNode );
    IqParseNode* pArguments = pNode->pChild();

    IqParseNode * pArg = pArguments;
    while ( pArg != 0 )
    {
        pArg->Accept( *this );
        pArg = pArg->pNextSibling();
    }
}

void CqCodeGenDataGather::Visit( IqParseNodeVariable& V )
{
    IqParseNodeVariable * pVN;
    V.GetInterface( ParseNode_Variable, ( void** ) & pVN );

    SqVarRef temp( pVN->VarRef() );
    IqVarDef* pVD = pTranslatedVariable( temp, m_saTransTable );
    if ( pVD )
        pVD->IncUseCount();
}

void CqCodeGenDataGather::Visit( IqParseNodeArrayVariable& AV )
{
    IqParseNode * pNode;
    AV.GetInterface( ParseNode_Base, ( void** ) & pNode );

    IqParseNodeVariable* pVN;
    AV.GetInterface( ParseNode_Variable, ( void** ) & pVN );

    pNode->pChild() ->Accept( *this );

    SqVarRef temp( pVN->VarRef() );
    IqVarDef* pVD = pTranslatedVariable( temp, m_saTransTable );
    if ( pVD )
        pVD->IncUseCount();
}

void CqCodeGenDataGather::Visit( IqParseNodeVariableAssign& VA )
{
    IqParseNode * pNode;
    VA.GetInterface( ParseNode_Base, ( void** ) & pNode );

    IqParseNodeVariable* pVN;
    VA.GetInterface( ParseNode_Variable, ( void** ) & pVN );

    IqParseNode * pExpr = pNode->pChild();
    if ( pExpr != 0 )
        pExpr->Accept( *this );

    SqVarRef temp( pVN->VarRef() );
    IqVarDef* pVD = pTranslatedVariable( temp, m_saTransTable );
    if ( pVD )
        pVD->IncUseCount();
}

void CqCodeGenDataGather::Visit( IqParseNodeArrayVariableAssign& AVA )
{
    // Output the assignment expression
    IqParseNode * pNode;
    AVA.GetInterface( ParseNode_Base, ( void** ) & pNode );

    IqParseNodeVariable* pVN;
    AVA.GetInterface( ParseNode_Variable, ( void** ) & pVN );

    IqParseNodeVariableAssign* pVA;
    AVA.GetInterface( ParseNode_VariableAssign, ( void** ) & pVA );

    IqParseNode * pExpr = pNode->pChild();
    if ( pExpr != 0 )
        pExpr->Accept( *this );

    IqParseNode * pIndex = pExpr->pNextSibling();
    pIndex->Accept( *this );

    SqVarRef temp( pVN->VarRef() );
    IqVarDef* pVD = pTranslatedVariable( temp, m_saTransTable );
    if ( pVD )
        pVD->IncUseCount();
}

void CqCodeGenDataGather::Visit( IqParseNodeOperator& OP )
{
    IqParseNode * pNode;
    OP.GetInterface( ParseNode_Base, ( void** ) & pNode );

    IqParseNode* pOperandA = pNode->pChild();
    IqParseNode* pOperandB = pOperandA->pNextSibling();

    if ( pOperandA ) pOperandA->Accept( *this );
    if ( pOperandB ) pOperandB->Accept( *this );
}

void CqCodeGenDataGather::Visit( IqParseNodeMathOp& OP )
{
    IqParseNodeOperator * pOp;
    OP.GetInterface( ParseNode_Operator, ( void** ) & pOp );
    Visit( *pOp );
}

void CqCodeGenDataGather::Visit( IqParseNodeRelationalOp& OP )
{
    IqParseNodeOperator * pOp;
    OP.GetInterface( ParseNode_Operator, ( void** ) & pOp );
    Visit( *pOp );
}

void CqCodeGenDataGather::Visit( IqParseNodeUnaryOp& OP )
{
    IqParseNodeOperator * pOp;
    OP.GetInterface( ParseNode_Operator, ( void** ) & pOp );
    Visit( *pOp );
}

void CqCodeGenDataGather::Visit( IqParseNodeLogicalOp& OP )
{
    IqParseNodeOperator * pOp;
    OP.GetInterface( ParseNode_Operator, ( void** ) & pOp );
    Visit( *pOp );
}

void CqCodeGenDataGather::Visit( IqParseNodeDiscardResult& DR )
{
    IqParseNode * pNode;
    DR.GetInterface( ParseNode_Base, ( void** ) & pNode );

    IqParseNode * pNext = pNode->pChild();
    while ( pNext )
    {
        pNext->Accept( *this );
        pNext = pNext->pNextSibling();
    }
}

void CqCodeGenDataGather::Visit( IqParseNodeConstantFloat& F )
{}

void CqCodeGenDataGather::Visit( IqParseNodeConstantString& S )
{}

void CqCodeGenDataGather::Visit( IqParseNodeWhileConstruct& WC )
{
    IqParseNode * pNode;
    WC.GetInterface( ParseNode_Base, ( void** ) & pNode );

    IqParseNode* pArg = pNode->pChild();
    assert( pArg != 0 );
    IqParseNode* pStmt = pArg->pNextSibling();
    assert( pStmt != 0 );
    IqParseNode* pStmtInc = pStmt->pNextSibling();

    pArg->Accept( *this );							// relation
    pStmt->Accept( *this );							// statement
    if ( pStmtInc )
        pStmtInc->Accept( *this );					// incrementor
}

void CqCodeGenDataGather::Visit( IqParseNodeIlluminateConstruct& IC )
{
    IqParseNode * pNode;
    IC.GetInterface( ParseNode_Base, ( void** ) & pNode );

    IqParseNode* pArg = pNode->pChild();
    assert( pArg != 0 );
    IqParseNode* pStmt = pArg->pNextSibling();
    assert( pStmt != 0 );

    pArg->Accept( *this );
    pStmt->Accept( *this );							// statement
}

void CqCodeGenDataGather::Visit( IqParseNodeIlluminanceConstruct& IC )
{
    IqParseNode * pNode;
    IC.GetInterface( ParseNode_Base, ( void** ) & pNode );

    IqParseNode* pArg = pNode->pChild();
    assert( pArg != 0 );
    IqParseNode* pStmt = pArg->pNextSibling();
    assert( pStmt != 0 );

    // The second to last child of the arg node is the Point to be illuminated, see Parser.y for confirmation.
    IqParseNode* pInitArg = pArg->pChild();
    while ( pInitArg->pNextSibling() != 0 )
        pInitArg = pInitArg->pNextSibling();
    pInitArg = pInitArg->pPrevSibling();
    pInitArg->Accept( *this );
    pArg->Accept( *this );
    pStmt->Accept( *this );							// statement
}

void CqCodeGenDataGather::Visit( IqParseNodeSolarConstruct& SC )
{
    IqParseNode * pNode;
    SC.GetInterface( ParseNode_Base, ( void** ) & pNode );

    IqParseNode* pArg = pNode->pChild();
    if ( NULL != pArg )
    {
        pArg->Accept( *this );
        IqParseNode* pStmt = pArg->pNextSibling();
        if ( pStmt ) pStmt->Accept( *this );			// statement
    }
}

void CqCodeGenDataGather::Visit( IqParseNodeConditional& C )
{
    IqParseNode * pNode;
    C.GetInterface( ParseNode_Base, ( void** ) & pNode );

    IqParseNode* pArg = pNode->pChild();
    assert( pArg != 0 );
    IqParseNode* pTrueStmt = pArg->pNextSibling();
    assert( pTrueStmt != 0 );
    IqParseNode* pFalseStmt = pTrueStmt->pNextSibling();

    pArg->Accept( *this );							// relation
    pTrueStmt->Accept( *this );						// true statement
    if ( pFalseStmt )
        pFalseStmt->Accept( *this );				// false statement
}

void CqCodeGenDataGather::Visit( IqParseNodeConditionalExpression& CE )
{
    IqParseNode * pNode;
    CE.GetInterface( ParseNode_Base, ( void** ) & pNode );

    IqParseNode * pArg = pNode->pChild();
    assert( pArg != 0 );
    IqParseNode* pTrueStmt = pArg->pNextSibling();
    assert( pTrueStmt != 0 );
    IqParseNode* pFalseStmt = pTrueStmt->pNextSibling();

    pTrueStmt->Accept( *this );					// true statement
    pFalseStmt->Accept( *this );				// false statement
    pArg->Accept( *this );						// relation
}

void CqCodeGenDataGather::Visit( IqParseNodeTypeCast& TC )
{
    IqParseNode * pNode;
    TC.GetInterface( ParseNode_Base, ( void** ) & pNode );

    IqParseNode * pOperand = pNode->pChild();
    assert( pOperand != 0 );

    pOperand->Accept( *this );
}

void CqCodeGenDataGather::Visit( IqParseNodeTriple& T )
{
    IqParseNode * pNode;
    T.GetInterface( ParseNode_Base, ( void** ) & pNode );

    IqParseNode * pA = pNode->pChild();
    assert( pA != 0 );
    IqParseNode* pB = pA->pNextSibling();
    assert( pB != 0 );
    IqParseNode* pC = pB->pNextSibling();
    assert( pC != 0 );

    pC->Accept( *this );
    pB->Accept( *this );
    pA->Accept( *this );
}

void CqCodeGenDataGather::Visit( IqParseNodeSixteenTuple& ST )
{
    IqParseNode * pNode;
    ST.GetInterface( ParseNode_Base, ( void** ) & pNode );

    IqParseNode * p00 = pNode->pChild();	assert( p00 != 0 );
    IqParseNode* p01 = p00->pNextSibling();	assert( p01 != 0 );
    IqParseNode* p02 = p01->pNextSibling();	assert( p02 != 0 );
    IqParseNode* p03 = p02->pNextSibling();	assert( p03 != 0 );

    IqParseNode* p10 = p03->pNextSibling();	assert( p10 != 0 );
    IqParseNode* p11 = p10->pNextSibling();	assert( p11 != 0 );
    IqParseNode* p12 = p11->pNextSibling();	assert( p12 != 0 );
    IqParseNode* p13 = p12->pNextSibling();	assert( p13 != 0 );

    IqParseNode* p20 = p13->pNextSibling();	assert( p20 != 0 );
    IqParseNode* p21 = p20->pNextSibling();	assert( p21 != 0 );
    IqParseNode* p22 = p21->pNextSibling();	assert( p22 != 0 );
    IqParseNode* p23 = p22->pNextSibling();	assert( p23 != 0 );

    IqParseNode* p30 = p23->pNextSibling();	assert( p30 != 0 );
    IqParseNode* p31 = p30->pNextSibling();	assert( p31 != 0 );
    IqParseNode* p32 = p31->pNextSibling();	assert( p32 != 0 );
    IqParseNode* p33 = p32->pNextSibling();	assert( p33 != 0 );

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

void CqCodeGenDataGather::Visit( IqParseNodeMessagePassingFunction& MPF )
{
    IqParseNode * pNode;
    MPF.GetInterface( ParseNode_Base, ( void** ) & pNode );

    IqParseNode * pExpr = pNode->pChild();

    pExpr->Accept( *this );
    SqVarRef temp( MPF.VarRef() );
    IqVarDef* pVD = CqVarDef::GetVariablePtr( temp );
    if ( pVD )
        pVD->IncUseCount();
}

//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )
