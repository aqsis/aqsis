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

#include	<strstream>
#include	<fstream>

#include	"version.h"
#include	"vmoutput.h"

#include	"parsenode.h"
#include	"version.h"

START_NAMESPACE(Aqsis)

void OutputLocalVariable(const IqVarDef* pVar, std::ostream& out);
void OutputFunctionCall(const IqFuncDef* pFunc, IqParseNode* pArguments, std::ostream& out);


static char* gVariableTypeNames[]=
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
static TqInt gcVariableTypeNames=sizeof(gVariableTypeNames)/sizeof(gVariableTypeNames[0]);

static TqInt gInternalFunctionUsage=0;


///---------------------------------------------------------------------
/// Global stack of translation tables

std::vector<std::vector<SqVarRefTranslator>*>	saTransTable;


void OutputTree(const IqParseNode* pNode)
{
	TqInt i=0;

	if(pNode)
		OutputTreeNode(pNode, std::cout);
}

TqInt	gcLabels=0;


const char* MathOpName(TqInt op)
{
	// Output this nodes operand name.
	switch(op)
	{
		case Op_Add:
			return("add");
			break;

		case Op_Sub:
			return("sub");
			break;

		case Op_Mul:
			return("mul");
			break;

		case Op_Div:
			return("div");
			break;

		case Op_Dot:
			return("dot");
			break;

		case Op_Crs:
			return("crs");
			break;

		case Op_Mod:
			return("mod");
			break;

		case Op_Lft:
			return("left");
			break;

		case Op_Rgt:
			return("right");
			break;

		case Op_And:
			return("and");
			break;

		case Op_Xor:
			return("xor");
			break;

		case Op_Or:
			return("or");
			break;

		case Op_L:
			return( "ls");
			break;

		case Op_G:
			return( "gt");
			break;

		case Op_GE:
			return( "ge");
			break;

		case Op_LE:
			return( "le");
			break;

		case Op_EQ:
			return( "eq");
			break;

		case Op_NE:
			return( "ne");
			break;

		case Op_Plus:
			break;

		case Op_Neg:
			return( "neg");
			break;

		case Op_BitwiseComplement:
			return( "cmpl");
			break;

		case Op_LogicalNot:
			return( "not");
			break;

		case Op_LogAnd:
			return( "land");
			break;

		case Op_LogOr:
			return( "lor");
			break;
	}
	return("error");
}


void OutputTreeNode(const IqParseNode* pNode, std::ostream& out)
{
	TqInt i;
	IqParseNodeShader* pS;
	IqParseNodeFunctionCall* pFC;
	IqParseNodeVariable* pV;
	IqParseNodeArrayVariable* pAV;
	IqParseNodeVariableAssign* pVA;
	IqParseNodeArrayVariableAssign* pAVA;
	IqParseNodeOperator* pO;
	IqParseNodeDiscardResult* pDR;
	IqParseNodeConstantFloat* pF;
	IqParseNodeConstantString* pStr;
	IqParseNodeWhileConstruct* pWC;
	IqParseNodeIlluminateConstruct* pIC;
	IqParseNodeIlluminanceConstruct* pIC2;
	IqParseNodeSolarConstruct* pSC;
	IqParseNodeConditional* pCond;
	IqParseNodeConditionalExpression* pCondExpr;
	IqParseNodeTypeCast* pCast;
	IqParseNodeTriple* pTriple;
	IqParseNodeSixteenTuple* pSixteenTuple;
	IqParseNodeMessagePassingFunction* pMessagePassingFunction;

	if(pNode->GetInterface(IqParseNodeShader::m_ID, (void**)&pS))
	{
		// Create a new file for this shader
		CqString slxName(pS->strName());
		slxName.append(VM_SHADER_EXTENSION);
		std::ofstream slxFile(slxName.c_str());
		std::cout << "... " << slxName.c_str() << std::endl;

		slxFile << pS->strShaderType() << std::endl;

		// Output version information.
		slxFile << "AQSIS_V " << VERSION_STR << std::endl;

		slxFile << std::endl << std::endl << "segment Data" << std::endl;

		// Do a first pass output to find out which variables are used.
		if(pNode)
		{
			gInternalFunctionUsage=0;
			std::ostrstream strNull;
			OutputTreeNode(pNode->pChild(),strNull);
		}

		// Now that we have this information, work out which standard vars are used.
		TqInt Use=gInternalFunctionUsage;
		for(i=0; i<EnvVars_Last; i++)
		{
			if(gStandardVars[i].UseCount()>0)
				Use|=(0x00000001<<i);
		}
		slxFile << std::endl << "USES " << Use << std::endl << std::endl;
		
		// Output any declared variables.
		for(i=0; i<gLocalVars.size(); i++)
			OutputLocalVariable(&gLocalVars[i],slxFile);

		slxFile << std::endl << std::endl << "segment Init" << std::endl;
		for(i=0; i<gLocalVars.size(); i++)
		{
			IqVarDef* pVar=&gLocalVars[i];
			if(pVar->Type()&Type_Param && pVar->pInitialiser()!=0)
				OutputTreeNode(pVar->pInitialiser(),slxFile);
		}

		slxFile << std::endl << std::endl << "segment Code" << std::endl;
		IqParseNode* pNext=pNode->pChild();
		while(pNext)
		{
			OutputTreeNode(pNext,slxFile);
			pNext=pNext->pNextSibling();
		}
		slxFile.close();
	}
	else if(pNode->GetInterface(ParseNode_FunctionCall,(void**)&pFC))
	{
		// Output the function name.
		const IqFuncDef* pFunc=pFC->pFuncDef();
		// TODO: Better handling of local functions.
		OutputFunctionCall(pFunc,pNode->pChild(),out);

		// If it is a builtin function, lets just check its standard variable usage.
		if(!pFunc->fLocal())
			gInternalFunctionUsage|=pFunc->InternalUsage();
	}
	else if(pNode->GetInterface(ParseNode_Variable,(void**)&pV))
	{
		if(pNode->GetInterface(ParseNode_VariableAssign,(void**)&pVA))
		{
			// Output the assignment expression
			IqParseNode* pExpr=pNode->pChild();
			if(pExpr!=0) OutputTreeNode(pExpr,out);

			// Output a dup so that the result remains on the stack.
			if(!pVA->fDiscardResult())
				out << "\tdup" << std::endl;

			if(pNode->GetInterface(ParseNode_ArrayVariableAssign,(void**)&pAVA))
			{
				IqParseNode* pIndex=pExpr->pNextSibling();
				OutputTreeNode(pIndex,out);
				out << "\tipop ";
			}
			else
				out << "\tpop ";

			// Output a pop for this variable.
			IqVarDef* pVD=pTranslatedVariable(pV->VarRef());
			if(pVD)
			{
				pVD->IncUseCount();
				out << pVD->strName() << std::endl;
			}
		}
		else
		{
			if(pNode->GetInterface(ParseNode_ArrayVariable,(void**)&pAV))
			{
				OutputTreeNode(pNode->pChild(),out);
				out << "\tipushv ";
			}
			else
				out << "\tpushv ";

			IqVarDef* pVD=pTranslatedVariable(pV->VarRef());
			if(pVD)
			{
				pVD->IncUseCount();
				out << pVD->strName() << std::endl;
			}
		}
	}
	else if(pNode->GetInterface(ParseNode_Operator,(void**)&pO))
	{
		IqParseNode* pOperandA=pNode->pChild();
		IqParseNode* pOperandB=pOperandA->pNextSibling();
		
		char* pstrAType="";
		if(pOperandA)	pstrAType=gVariableTypeIdentifiers[pOperandA->ResType()&Type_Mask];
		char* pstrBType="";
		if(pOperandB)	pstrBType=gVariableTypeIdentifiers[pOperandB->ResType()&Type_Mask];

		if(pOperandA)	OutputTreeNode(pOperandA,out);
		if(pOperandB)	OutputTreeNode(pOperandB,out);
		out << "\t" << MathOpName(pO->Operator());
		if(pNode->NodeType()!=ParseNode_LogicalOp) 
		{
			if(pOperandA)	out << pstrBType;
			if(pOperandB)	out << pstrAType;
		}
		out << std::endl;
	}
	else if(pNode->NodeType()==ParseNode_Base)
	{
		IqParseNode* pNext=pNode->pChild();
		while(pNext)
		{
			OutputTreeNode(pNext,out);
			pNext=pNext->pNextSibling();
		}
	}
	else if(pNode->GetInterface(ParseNode_DiscardResult,(void**)&pDR))
	{
		IqParseNode* pNext=pNode->pChild();
		while(pNext)
		{
			OutputTreeNode(pNext,out);
			pNext=pNext->pNextSibling();
		}
		out << "\tdrop" << std::endl;
	}
	else if(pNode->GetInterface(ParseNode_ConstantFloat,(void**)&pF))
	{
		out << "\tpushif " << pF->Value() << std::endl;
	}
	else if(pNode->GetInterface(ParseNode_ConstantString,(void**)&pStr))
	{
		out << "\tpushis \"" << pStr->strValue() << "\"" << std::endl;
	}
	else if(pNode->GetInterface(ParseNode_WhileConstruct, (void**)&pWC))
	{
		TqInt iLabelA=gcLabels++;
		TqInt iLabelB=gcLabels++;

		IqParseNode* pArg=pNode->pChild();
		assert(pArg!=0);
		IqParseNode* pStmt=pArg->pNextSibling();
		assert(pStmt!=0);
		IqParseNode* pStmtInc=pStmt->pNextSibling();
		
		out << ":" << iLabelA << std::endl;		// loop back label
		out << "\tS_CLEAR" << std::endl;			// clear current state
		OutputTreeNode(pArg,out);				// relation
		out << "\tS_GET" << std::endl;			// Get the current state by popping the t[ value off the stack
		out << "\tS_JZ " << iLabelB << std::endl;	// exit if false
		out << "\tRS_PUSH" << std::endl;			// push running state
		out << "\tRS_GET" << std::endl;			// get current state to running state
		OutputTreeNode(pStmt,out);				// statement
		if(pStmtInc)	
			OutputTreeNode(pStmtInc,out);		// incrementor
		out << "\tRS_POP" << std::endl;			// Pop the running state
		out << "\tjmp " << iLabelA << std::endl;// loop back jump
		out << ":" << iLabelB << std::endl;		// completion label
	}
	else if(pNode->GetInterface(ParseNode_IlluminateConstruct, (void**)&pIC))
	{
		TqInt iLabelA=gcLabels++;
		TqInt iLabelB=gcLabels++;

		IqParseNode* pArg=pNode->pChild();
		assert(pArg!=0);
		IqParseNode* pStmt=pArg->pNextSibling();
		assert(pStmt!=0);
		IqParseNode* pStmtInc=pStmt->pNextSibling();
		
		TqInt L1=gcLabels++;					// Get the two labels for the relation statement.
		TqInt L2=gcLabels++;

		out << ":" << iLabelA << std::endl;		// loop back label
		out << "\tS_CLEAR" << std::endl;			// clear current state
		OutputTreeNode(pArg,out);
		if(pIC->fHasAxisAngle())	out << "\tilluminate2" << std::endl;
		else						out << "\tilluminate" << std::endl;
		out << "\tS_JZ " << iLabelB << std::endl;	// exit loop if false
		out << "\tRS_PUSH" << std::endl;			// Push running state
		out << "\tRS_GET" << std::endl;			// Get state
		OutputTreeNode(pStmt,out);				// statement
		out << "\tRS_POP" << std::endl;			// Pop the running state
		out << "\tjmp " << iLabelA << std::endl;// loop back jump
		out << ":" << iLabelB << std::endl;		// completion label
	}
	else if(pNode->GetInterface(ParseNode_IlluminanceConstruct,(void**)&pIC2))
	{
		TqInt iLabelA=gcLabels++;
		TqInt iLabelB=gcLabels++;

		IqParseNode* pArg=pNode->pChild();
		assert(pArg!=0);
		IqParseNode* pStmt=pArg->pNextSibling();
		assert(pStmt!=0);
		IqParseNode* pStmtInc=pStmt->pNextSibling();
		
		// The last child of the arg node is the Point to be illuminated, see Parser.y for confirmation.
		IqParseNode* pInitArg=pArg->pChild();
		while(pInitArg->pNextSibling()!=0)	pInitArg=pInitArg->pNextSibling();
		OutputTreeNode(pInitArg,out);
		out << "\tinit_illuminance" << std::endl;
		out << "\tjz " << iLabelB << std::endl;	// Jump if no lightsources.
		out << ":" << iLabelA << std::endl;		// loop back label
		out << "\tS_CLEAR" << std::endl;			// clear current state
		OutputTreeNode(pArg,out);
		if(pIC2->fHasAxisAngle())	out << "\tilluminance2" << std::endl;
		else						out << "\tilluminance" << std::endl;
		out << "\tS_JZ " << iLabelB << std::endl;	// exit loop if false
		out << "\tRS_PUSH" << std::endl;			// Push running state
		out << "\tRS_GET" << std::endl;			// Get state
		OutputTreeNode(pStmt,out);				// statement
		out << "\tRS_POP" << std::endl;			// Pop the running state
		out << "\tadvance_illuminance" << std::endl;
		out << "\tjnz " << iLabelA << std::endl;// loop back jump
		out << ":" << iLabelB << std::endl;		// completion label
	}
	else if(pNode->GetInterface(ParseNode_SolarConstruct,(void**)&pSC))
	{
		TqInt iLabelA=gcLabels++;
		TqInt iLabelB=gcLabels++;

		IqParseNode* pArg=pNode->pChild();
		assert(pArg!=0);
		IqParseNode* pStmt=pArg->pNextSibling();
		assert(pStmt!=0);
		IqParseNode* pStmtInc=pStmt->pNextSibling();
		
		out << ":" << iLabelA << std::endl;		// loop back label
		out << "\tS_CLEAR" << std::endl;		// clear current state
		OutputTreeNode(pArg,out);
		if(pSC->fHasAxisAngle())	out << "\tsolar2" << std::endl;
		else						out << "\tsolar" << std::endl;
		out << "\tS_JZ " << iLabelB << std::endl;	// exit loop if false
		out << "\tRS_PUSH" << std::endl;		// Push running state
		out << "\tRS_GET" << std::endl;			// set running state
		OutputTreeNode(pStmt,out);				// statement
		out << "\tRS_POP" << std::endl;			// Pop the running state
		out << "\tjmp " << iLabelA << std::endl;// loop back jump
		out << ":" << iLabelB << std::endl;		// completion label
	}
	else if(pNode->GetInterface(ParseNode_Conditional,(void**)&pCond))
	{
		TqInt iLabelA=gcLabels++;
		TqInt iLabelB;

		IqParseNode* pArg=pNode->pChild();
		assert(pArg!=0);
		IqParseNode* pTrueStmt=pArg->pNextSibling();
		assert(pTrueStmt!=0);
		IqParseNode* pFalseStmt=pTrueStmt->pNextSibling();

		out << "\tS_CLEAR" << std::endl;		// clear current state
		OutputTreeNode(pArg,out);				// relation
		out << "\tS_GET" << std::endl;			// Get the current state by popping the top value off the stack
		out << "\tRS_PUSH" << std::endl;		// push the running state
		out << "\tRS_GET" << std::endl;			// get current state to running state
		if(pFalseStmt)
		{
			iLabelB=gcLabels++;
			out << "\tRS_JZ " << iLabelB << std::endl; // skip true statement if all false
		}
		else
			out << "\tRS_JZ " << iLabelA << std::endl; // exit if all false
		OutputTreeNode(pTrueStmt,out);			// true statement
		if(pFalseStmt)
		{
			out << ":" << iLabelB << std::endl;	// false part label
			out << "\tRS_JNZ " << iLabelA << std::endl;	// exit if all true
			out << "\tRS_INVERSE" << std::endl;	// Invert result
			OutputTreeNode(pFalseStmt,out);		// false statement
		}
		out << ":" << iLabelA << std::endl;		// conditional exit point
		out << "\tRS_POP" << std::endl;			// pop running state
	}
	else if(pNode->GetInterface(ParseNode_ConditionalExpression, (void**)&pCondExpr))
	{
		IqParseNode* pArg=pNode->pChild();
		assert(pArg!=0);
		IqParseNode* pTrueStmt=pArg->pNextSibling();
		assert(pTrueStmt!=0);
		IqParseNode* pFalseStmt=pTrueStmt->pNextSibling();

		TqInt typeT=static_cast<TqInt>(pTrueStmt->ResType()&Type_Mask);
		char* pstrTType=gVariableTypeIdentifiers[typeT];

		OutputTreeNode(pTrueStmt,out);			// true statement
		OutputTreeNode(pFalseStmt,out);			// false statement
		OutputTreeNode(pArg,out);				// relation
		out << "\tmerge" << pstrTType << std::endl;
	}
	else if(pNode->GetInterface(ParseNode_TypeCast,(void**)&pCast))
	{
		IqParseNode* pOperand=pNode->pChild();
		assert(pOperand!=0);
		
		TqInt typeA=pOperand->ResType()&Type_Mask;
		TqInt typeB=pCast->CastTo()&Type_Mask;
		// No need to output a cast for the triple or h types.
		OutputTreeNode(pOperand,out);
		if(!((typeA==Type_Point || typeA==Type_Normal || typeA==Type_Vector) &&
			 (typeB==Type_Point || typeB==Type_Normal || typeB==Type_Vector)))
		{	
			char* pstrToType=gVariableTypeIdentifiers[pCast->CastTo()&Type_Mask];
			char* pstrFromType=gVariableTypeIdentifiers[pOperand->ResType()&Type_Mask];
			out << "\tset" << pstrFromType << pstrToType << std::endl;
		}
	}
	else if(pNode->GetInterface(ParseNode_Triple,(void**)&pTriple))
	{
		IqParseNode* pA=pNode->pChild();
		assert(pA!=0);
		IqParseNode* pB=pA->pNextSibling();
		assert(pB!=0);
		IqParseNode* pC=pB->pNextSibling();
		assert(pC!=0);

		// Output the 'push'es in reverse, so that Red/X ec is first off the stack when doing a 'sett?' instruction.
		OutputTreeNode(pC,out);
		OutputTreeNode(pB,out);
		OutputTreeNode(pA,out);
	}
	else if(pNode->GetInterface(ParseNode_SixteenTuple,(void**)&pSixteenTuple))
	{
		IqParseNode* p00=pNode->pChild();		assert(p00!=0);
		IqParseNode* p01=p00->pNextSibling();	assert(p01!=0);
		IqParseNode* p02=p01->pNextSibling();	assert(p02!=0);
		IqParseNode* p03=p02->pNextSibling();	assert(p03!=0);

		IqParseNode* p10=p03->pNextSibling();	assert(p10!=0);
		IqParseNode* p11=p10->pNextSibling();	assert(p11!=0);
		IqParseNode* p12=p11->pNextSibling();	assert(p12!=0);
		IqParseNode* p13=p12->pNextSibling();	assert(p13!=0);

		IqParseNode* p20=p13->pNextSibling();	assert(p20!=0);
		IqParseNode* p21=p20->pNextSibling();	assert(p21!=0);
		IqParseNode* p22=p21->pNextSibling();	assert(p22!=0);
		IqParseNode* p23=p22->pNextSibling();	assert(p23!=0);

		IqParseNode* p30=p23->pNextSibling();	assert(p30!=0);
		IqParseNode* p31=p30->pNextSibling();	assert(p31!=0);
		IqParseNode* p32=p31->pNextSibling();	assert(p32!=0);
		IqParseNode* p33=p32->pNextSibling();	assert(p33!=0);

		OutputTreeNode(p00,out);
		OutputTreeNode(p01,out);
		OutputTreeNode(p02,out);
		OutputTreeNode(p03,out);
		OutputTreeNode(p10,out);
		OutputTreeNode(p11,out);
		OutputTreeNode(p12,out);
		OutputTreeNode(p13,out);
		OutputTreeNode(p20,out);
		OutputTreeNode(p21,out);
		OutputTreeNode(p22,out);
		OutputTreeNode(p23,out);
		OutputTreeNode(p30,out);
		OutputTreeNode(p31,out);
		OutputTreeNode(p32,out);
		OutputTreeNode(p33,out);
	}
	else if(pNode->GetInterface(ParseNode_MessagePassingFunction, (void**)&pMessagePassingFunction))
	{
		IqParseNode* pExpr=pNode->pChild();
		OutputTreeNode(pExpr,out);

		CqString strCommType("surface");
		switch(pMessagePassingFunction->CommType())
		{
			case CommTypeAtmosphere:
				strCommType="atmosphere";
				break;

			case CommTypeDisplacement:
				strCommType="displacement";
				break;

			case CommTypeLightsource:
				strCommType="lightsource";
				break;

			case CommTypeAttribute:
				strCommType="attribute";
				break;

			case CommTypeOption:
				strCommType="option";
				break;

			case CommTypeRendererInfo:
				strCommType="rendererinfo";
				break;

			case CommTypeIncident:
				strCommType="incident";
				break;

			case CommTypeOpposite:
				strCommType="opposite";
				break;
		}
		// Output the comm function.
		IqVarDef* pVD=pTranslatedVariable(pMessagePassingFunction->VarRef());
		if(pVD)
		{
			pVD->IncUseCount();
			out << "\t" << strCommType.c_str() << " " << pVD->strName() << std::endl;
		}
	}
}



IqVarDef* pTranslatedVariable(SqVarRef& Ref)
{
	SqVarRef RealRef=Ref;
	// Firstly see if the top translation table knows about this variable.
	
	if(!saTransTable.empty())
	{
		std::vector<SqVarRefTranslator>* pTransTable=0;
		std::vector<std::vector<SqVarRefTranslator>*>::reverse_iterator iTable=saTransTable.rbegin();
		
		int i=0;
		while(iTable!=saTransTable.rend())
		{
			pTransTable=*iTable;
			if(pTransTable!=0)
			{
				TqInt i;
				for(i=0; i<pTransTable->size(); i++)
				{
					if((*pTransTable)[i].m_From==RealRef)
					{
						RealRef=(*pTransTable)[i].m_To;
						break;
					}
				}
				// Only continue looking for nested translations if it was found at the current level.
				if(i==pTransTable->size())	break;
			}
			iTable++;
			i++;
		}
	}
	return(CqVarDef::GetVariablePtr(RealRef));
}


///---------------------------------------------------------------------
/// PushTransTable
/// Push a new variable reference translation table onto the stack.

void PushTransTable(std::vector<SqVarRefTranslator>* pTransTable)
{
	saTransTable.push_back(pTransTable);
}


///---------------------------------------------------------------------
/// PopTransTable
/// Pop the previous variable reference translation table from the stack.

std::vector<SqVarRefTranslator>* PopTransTable()
{
	std::vector<SqVarRefTranslator>* pRes=saTransTable.back();
	saTransTable.erase(saTransTable.end()-1);
	return(pRes);
}



CqString StorageSpec(TqInt Type)
{
	CqString strSpec("");
	
	if(Type&Type_Param)		strSpec+="param ";
	if(Type&Type_Uniform)	strSpec+="uniform ";
	if(Type&Type_Varying)	strSpec+="varying ";

	return(strSpec);
}

///---------------------------------------------------------------------
/// OutputLocalVariable
/// Output details of this local variable.

void OutputLocalVariable(const IqVarDef* pVar, std::ostream& out)
{
	if(pVar->UseCount()>0 || (pVar->Type()&Type_Param))
	{
		out << StorageSpec(pVar->Type()).c_str() << " " 
		    << gVariableTypeNames[pVar->Type()&Type_Mask] << " " 
		    << pVar->strName();
		if(pVar->Type()&Type_Array)		   
			out << "[" << pVar->ArrayLength() << "]";
		
		out << std::endl;
	}
}


void OutputFunctionCall(const IqFuncDef* pFunc, IqParseNode* pArguments, std::ostream& out)
{
	if(!pFunc->fLocal())
	{
		// Output parameters in reverse order, so that the function can pop them as expected
		if(pArguments!=0)
		{
			const IqParseNode* pArg=pArguments;
			while(pArg->pNextSibling()!=0)	pArg=pArg->pNextSibling();
			while(pArg!=0)
			{
				// Push the argument...
				OutputTreeNode(pArg,out);
				pArg=pArg->pPrevSibling();
			}
		}

		// If it is a variable length parameter function, output the number of
		// additional parameters.
		TqInt iAdd=0;
		if((iAdd=pFunc->VariableLength())>=0)
		{
			const IqParseNode* pArg=pArguments;
			while(pArg)
			{
				iAdd--;
				pArg=pArg->pNextSibling();
			}
			// Not happy about this!!
			CqParseNodeFloatConst C(static_cast<TqFloat>(abs(iAdd)));
			OutputTreeNode(&C,out);
		}

		out << "\t" << pFunc->strVMName() << std::endl;
	}
	else
	{
		// Build a list of variable reference translators as we go.
		std::vector<SqVarRefTranslator> aTransTable;
		// Output arguments and pop the parameters off the stack.
		if(pArguments!=0 && pFunc->pArgs()!=0 && pFunc->pDef()!=0)
		{
			IqParseNode* pParam=pFunc->pArgs()->pChild();
			const IqParseNode* pArg=pArguments;
			while(pParam!=0)
			{
				if(pArg->IsVariableRef())
				{
					IqParseNodeVariable* pVarArg;
					pArg->GetInterface(ParseNode_Variable,(void**)&pVarArg);
					IqParseNodeVariable* pParamArg;
					if(pParam->GetInterface(ParseNode_Variable,(void**)&pParamArg))
					{
						SqVarRefTranslator Trans;
						Trans.m_From=pParamArg->VarRef();
						Trans.m_To=pVarArg->VarRef();
						aTransTable.push_back(Trans);
					}
					// TODO: Print warning, invalid argument.
				}
				else
				{
					// Push the argument...
					OutputTreeNode(pArg,out);
					// ...and pop the parameter
					CqParseNodeAssign Pop(static_cast<CqParseNodeVariable*>(pParam));
					Pop.NoDup();
					OutputTreeNode(&Pop,out);
				}					
				pParam=pParam->pNextSibling();
				pArg=pArg->pNextSibling();
			}
		}
		// Output the function body.
		PushTransTable(&aTransTable);
		OutputTreeNode(pFunc->pDef(),out);
		PopTransTable();
	}
}


//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)
