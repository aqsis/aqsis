////---------------------------------------------------------------------
////    Class definition file:  PARSENODE.CPP
////    Associated header file: PARSENODE.H
////
////    Author:					Paul C. Gregory
////    Creation date:			18/07/99
////---------------------------------------------------------------------

#include	"aqsis.h"
#include	"parsenode.h"
#include	"funcdef.h"
#include	"vardef.h"

START_NAMESPACE(Aqsis)

///---------------------------------------------------------------------
/// Static data on CqParseNode

TqInt	CqParseNode::m_cLabels=0;
TqInt	CqParseNode::m_aaTypePriorities[Type_Last][Type_Last]=
{
	//				   @  f	 i	p  s  c	 t	h  n  v	 x	m
	/*Type_Nil*/	 { 99,00,00,00,00,00,00,00,00,00,00,00,00 },
	/*Type_Float*/	 { 00,99,98,02,00,01,02,02,02,02,00,01,00 },
	/*Type_Integer*/ { 00,98,99,00,00,00,00,00,00,00,00,00,00 },
	/*Type_Point*/	 { 00,00,00,99,00,96,98,98,97,97,00,00,00 },
	/*Type_String*/	 { 00,00,00,00,99,00,00,00,00,00,00,00,00 },
	/*Type_Color*/	 { 00,00,00,01,00,99,02,01,01,01,00,00,00 },
	/*Type_Triple*/	 { 00,00,00,98,00,98,99,97,98,98,00,00,00 },
	/*Type_hPoint*/	 { 00,00,00,97,00,96,98,99,98,98,00,00,00 },
	/*Type_Normal*/	 { 00,00,00,98,00,96,98,97,99,98,00,00,00 },
	/*Type_Vector*/	 { 00,00,00,98,00,96,98,97,98,99,00,00,00 },
	/*Type_Void*/	 { 00,00,00,00,00,00,00,00,00,00,99,00,00 },
	/*Type_Matrix*/	 { 00,00,00,00,00,00,00,00,00,00,00,99,00 },
	/*Type_HexTuple*/{ 00,00,00,00,00,00,00,00,00,00,00,99,98 },
};
EqVariableType	CqParseNode::m_aAllTypes[Type_Last-1]=
{ 
	Type_Float,
	Type_Integer,
	Type_Point,
	Type_String,
	Type_Color,
	Type_Triple,
	Type_hPoint,
	Type_Normal,
	Type_Vector,
	Type_Void,
	Type_Matrix,
	Type_HexTuple,
};
TqInt CqParseNode::m_cInLoop=0;
TqInt CqParseNode::m_cInConditional=0;
TqInt CqParseNode::m_fInBlock=0;
#ifdef	_DEBUG
TqInt	CqParseNode::m_DebugOutTab=0;
#endif

///---------------------------------------------------------------------
/// operator<<
/// Test output of a parse tree to a specified output stream.

std::ostream& operator<<(std::ostream& Stream, CqParseNode& Node)
{
	Node.Output(Stream);
	return(Stream);
}


///---------------------------------------------------------------------
/// operator<<
/// Test output of a parse tree to a specified output stream.

std::ostream& operator<<(std::ostream& Stream, CqParseNode* pNode)
{
	pNode->Output(Stream);
	return(Stream);
}


///---------------------------------------------------------------------
/// CqParseNode::TypeIdentifier
/// Return a string type identifier for the specified type.

char* CqParseNode::TypeIdentifier(int Type)
{
	return(gVariableTypeIdentifiers[Type&Type_Mask]);
}


///---------------------------------------------------------------------
/// CqParseNode::TypeFromIdentifier
/// Return a type for the specified type identifier.

EqVariableType CqParseNode::TypeFromIdentifier(char Id)
{
	TqInt i;
	for(i=0; i<Type_Last; i++)
	{
		if(gVariableTypeIdentifiers[i][0]==Id ||
		   gVariableTypeIdentifiers[i][0]==_tolower(Id))
		return((EqVariableType)i);
	}
	return(Type_Nil);
}


///---------------------------------------------------------------------
/// CqParseNode::TypeName
/// Return a string type name for the specified type.

char* CqParseNode::TypeName(int Type)
{
	return(gVariableTypeNames[Type&Type_Mask]);
}


///---------------------------------------------------------------------
/// CqParseNode::FindCast
/// Find a valid cast type from the list of available options.

EqVariableType	CqParseNode::FindCast(EqVariableType CurrType, EqVariableType* pTypes, TqInt Count)
{
	// Check if the current type exists in the list first.
	TqInt i;
	for(i=0; i<Count; i++)
		if((pTypes[i]&Type_Mask)==(CurrType&Type_Mask))	return(static_cast<EqVariableType>(CurrType&Type_Mask));

	// Else search for the best option.
	EqVariableType Ret=Type_Nil;
	TqInt Pri=0;
	for(i=0; i<Count; i++)
	{
		if(m_aaTypePriorities[CurrType&Type_Mask][(pTypes[i]&Type_Mask)]>Pri)
		{
			Ret=pTypes[i];
			Pri=m_aaTypePriorities[CurrType&Type_Mask][(pTypes[i]&Type_Mask)];
		}
	}
	return(Ret);
}


///---------------------------------------------------------------------
/// CqParseNode::Output
/// Test output of a parse tree to a specified output stream.

void CqParseNode::Output(std::ostream& Out) const
{
	// Output children nodes
	CqParseNode* pChild=m_pChild;
	while(pChild)
	{
		pChild->Output(Out);
		pChild=pChild->pNext();
	}
}


///---------------------------------------------------------------------
/// CqParseNode::pNextProcess
/// Test output of a parse tree to a specified output stream.

CqParseNode* CqParseNode::pNextProcess() const
{
	if(pNext()!=0)	return(pNext());

	CqParseNode* pN=m_pParent;
	while(pN)
	{
		if(pN->pNext())	return(pN->pNext());
		pN=pN->m_pParent;
	}
	return(0);
}


///---------------------------------------------------------------------
/// CqParseNodeFunction::Output
/// Output a function to the .slx file.
/// NOTE: It is presumed that type checking has taken place and the funcref at the top of the
/// list is the correct one to use.

void CqParseNodeFunction::Output(std::ostream& Out) const
{
	if(m_aFuncRef.size()<=0)
		return;

	if(m_aFuncRef[0].m_Type!=FuncTypeSpecial)
	{
		// Output this node.
		CqFuncDef* pFunc=CqFuncDef::GetFunctionPtr(m_aFuncRef[0]);
		pFunc->Output(Out,m_pChild);

		// If it is a builtin function, lets just check its standard variable usage.
		if(!pFunc->fLocal())
			gInternalFunctionUsage|=pFunc->InternalUsage();
	}
	else
	{
		// Output the function name.
		CqFuncDef* pFunc=CqFuncDef::GetFunctionPtr(m_aFuncRef[0]);
		pFunc->Output(Out, m_pChild);

		// If it is a builtin function, lets just check its standard variable usage.
		if(!pFunc->fLocal())
			gInternalFunctionUsage|=pFunc->InternalUsage();

		// Output the variable names.
		CqParseNode* pArgs=m_pChild;
		CqParseNodeVariable* pVar=static_cast<CqParseNodeVariable*>(pArgs);
		while(pVar!=0)
		{
			Out << " " << pVar->strName();
			pVar=static_cast<CqParseNodeVariable*>(pVar->pNext());
		}
		Out << std::endl;
	}
}


///---------------------------------------------------------------------
/// CqParseNodeFunction::ResType
/// Test output of a parse tree to a specified output stream.

EqVariableType CqParseNodeFunction::ResType() const
{
	// The return type of a function depends on its arguments.
	return(CqFuncDef::GetFunctionPtr(m_aFuncRef[0])->Type());
}


///---------------------------------------------------------------------
/// CqParseNodeVariable::CqParseNodeVariable
/// Test output of a parse tree to a specified output stream.

CqParseNodeVariable::CqParseNodeVariable(SqVarRef VarRef) : 
								CqParseNode(),
								m_VarRef(VarRef)
{
	m_fVarying=(CqVarDef::GetVariablePtr(VarRef)->Type()&Type_Varying)!=0;
}


///---------------------------------------------------------------------
/// CqParseNodeVariable::CqParseNodeVariable
/// Test output of a parse tree to a specified output stream.

CqParseNodeVariable::CqParseNodeVariable(CqParseNodeVariable* pVar) : 
								CqParseNode(),
								m_VarRef(pVar->m_VarRef)
{
	m_fVarying=(CqVarDef::GetVariablePtr(m_VarRef)->Type()&Type_Varying)!=0;
}


///---------------------------------------------------------------------
/// CqParseNodeVariable::strName
/// Returnt the variable name.

const char* CqParseNodeVariable::strName() const
{
	return(CqVarDef::GetVariablePtr(m_VarRef)->strName());
}


///---------------------------------------------------------------------
/// CqParseNodeVariable::Output
/// Test output of a parse tree to a specified output stream.

void CqParseNodeVariable::Output(std::ostream& Out) const
{
	// Output this node.
	CqVarDef* pVD=CqVarDef::GetVariablePtr(m_VarRef);
	if(pVD)
	{
		pVD->IncUseCount();
		Out << "\tpushv " << pVD->strName() << std::endl;
	}

	#ifdef _DEBUG
		Out << std::flush;
	#endif;
}


///---------------------------------------------------------------------
/// CqParseNodeVariable::ResType
/// Return a string type identifier for this variable.

EqVariableType CqParseNodeVariable::ResType() const
{
	CqVarDef* pVD=CqVarDef::GetVariablePtr(m_VarRef);
	if(pVD)
		return(pVD->Type());
	else
		return(Type_Nil);
}


///---------------------------------------------------------------------
/// CqParseNodeVariableArray::CqParseNodeVariableArray
/// Constructor from a variable reference.

CqParseNodeVariableArray::CqParseNodeVariableArray(SqVarRef VarRef) : 
								CqParseNodeVariable(VarRef)
{
}


///---------------------------------------------------------------------
/// CqParseNodeVariableArray::CqParseNodeVariableArray
/// Construct from another variable

CqParseNodeVariableArray::CqParseNodeVariableArray(CqParseNodeVariableArray* pVar) : 
								CqParseNodeVariable(pVar)
{
	m_fVarying=(CqVarDef::GetVariablePtr(m_VarRef)->Type()&Type_Varying)!=0;
	if(pVar->m_pChild)
		m_pChild=pVar->m_pChild->Clone(this);
}


///---------------------------------------------------------------------
/// CqParseNodeVariableArray::Output
/// Output the opcodes for this array access to the specified stream.

void CqParseNodeVariableArray::Output(std::ostream& Out) const
{
	// Output this node.
	m_pChild->Output(Out);
	CqVarDef* pVD=CqVarDef::GetVariablePtr(m_VarRef);
	if(pVD)
	{
		pVD->IncUseCount();
		Out << "\tipushv " << pVD->strName() << std::endl;
	}

	#ifdef _DEBUG
		Out << std::flush;
	#endif;
}


///---------------------------------------------------------------------
/// CqParseNodePop::Output
/// Test output of a parse tree to a specified output stream.

void CqParseNodePop::Output(std::ostream& Out) const
{
	// Output this node.
	CqVarDef* pVD=CqVarDef::GetVariablePtr(m_VarRef);
	if(pVD)
	{
		pVD->IncUseCount();
		Out << "\tpop" << " " << pVD->strName() << std::endl;
	}
}


///---------------------------------------------------------------------
/// CqParseNodeAssign::Output
/// Test output of a parse tree to a specified output stream.

void CqParseNodeAssign::Output(std::ostream& Out) const
{
	// Output the assignment expression
	CqParseNode* pExpr=m_pChild;
	pExpr->Output(Out);

	// Output a dup so that the result remains on the stack.
	if(!m_fNoDup)
		Out << "\tdup" << std::endl;

	// Output a pop for this variable.
	CqVarDef* pVD=CqVarDef::GetVariablePtr(m_VarRef);
	if(pVD)
	{
		pVD->IncUseCount();
		Out << "\tpop" << " " << pVD->strName() << std::endl;
	}
}


///---------------------------------------------------------------------
/// CqParseNodeAssign::ResType
/// Return a string type identifier for this variable.

EqVariableType CqParseNodeAssign::ResType() const
{
	return(CqVarDef::GetVariablePtr(m_VarRef)->Type());
}


///---------------------------------------------------------------------
/// CqParseNodeAssignArray::Output
/// Output the opcodes for an array assignment to the specified stream.

void CqParseNodeAssignArray::Output(std::ostream& Out) const
{
	// Output the assignment expression
	CqParseNode* pExpr=m_pChild;
	pExpr->Output(Out);

	CqParseNode* pIndex=m_pChild->pNext();
	pIndex->Output(Out);

	// Output a dup so that the result remains on the stack.
	if(!m_fNoDup)
		Out << "\tdup" << std::endl;

	// Output a pop for this variable.
	CqVarDef* pVD=CqVarDef::GetVariablePtr(m_VarRef);
	if(pVD)
	{
		pVD->IncUseCount();
		Out << "\tipop" << " " << pVD->strName() << std::endl;
	}
}


///---------------------------------------------------------------------
/// CqParseNodeMathOp::Output
/// Test output of a parse tree to a specified output stream.

void CqParseNodeMathOp::Output(std::ostream& Out) const
{
	// Output this node.
	CqParseNode* pOperandA=m_pChild;
	CqParseNode* pOperandB=m_pChild->pNext();
	assert(pOperandA!=0 && pOperandB!=0);
	
	char* pstrAType=TypeIdentifier(pOperandA->ResType());
	char* pstrBType=TypeIdentifier(pOperandB->ResType());

	Out << pOperandA << pOperandB;
	Out << "\t"; OutOp(Out);
	Out << pstrBType << pstrAType << std::endl;

	#ifdef _DEBUG
		Out << std::flush;
	#endif;
}

///---------------------------------------------------------------------
/// CqParseNodeMathOp::Output
/// Test output of a parse tree to a specified output stream.

void CqParseNodeMathOp::OutOp(std::ostream& Out) const
{
	// Output this nodes operand name.
	switch(m_Operator)
	{
		case Op_Add:
			Out << "add";
			break;

		case Op_Sub:
			Out << "sub";
			break;

		case Op_Mul:
			Out << "mul";
			break;

		case Op_Div:
			Out << "div";
			break;

		case Op_Dot:
			Out << "dot";
			break;

		case Op_Crs:
			Out << "crs";
			break;

		case Op_Mod:
			Out << "mod";
			break;

		case Op_Lft:
			Out << "left";
			break;

		case Op_Rgt:
			Out << "right";
			break;

		case Op_And:
			Out << "and";
			break;

		case Op_Xor:
			Out << "xor";
			break;

		case Op_Or:
			Out << "or";
			break;
	}
}


///---------------------------------------------------------------------
/// CqParseNodeMathOp::ResType
/// Return a string type identifier for this variable.

EqVariableType CqParseNodeMathOp::ResType() const
{
	CqParseNode* pOperandA=m_pChild;
	CqParseNode* pOperandB=m_pChild->pNext();
	assert(pOperandA!=0 && pOperandB!=0);

	EqVariableType ResAType=pOperandA->ResType();
	EqVariableType ResBType=pOperandB->ResType();

	switch(m_Operator)
	{
		case Op_Dot:
			return(Type_Float);
		case Op_Mul:
		case Op_Div:
		case Op_Add:
		case Op_Sub:
		default:
			// TODO: Should check here for valid types.
			if((ResAType&Type_Mask)==Type_Point ||
			   (ResAType&Type_Mask)==Type_Color)
				return(ResAType);
			else
				return(ResBType);
			break;
	}
}


///---------------------------------------------------------------------
/// CqParseNodeRelOp::Output
/// Test output of a parse tree to a specified output stream.

void CqParseNodeRelOp::Output(std::ostream& Out) const
{
	// Output this node.
	CqParseNode* pOperandA=m_pChild;
	CqParseNode* pOperandB=m_pChild->pNext();
	assert(pOperandA!=0 && pOperandB!=0);
	
	char* pstrAType=TypeIdentifier(pOperandA->ResType());
	char* pstrBType=TypeIdentifier(pOperandB->ResType());

	Out << pOperandA << pOperandB;
	Out << "\t"; OutOp(Out);
	Out << pstrBType << pstrAType << std::endl;

	#ifdef _DEBUG
		Out << std::flush;
	#endif;
}

///---------------------------------------------------------------------
/// CqParseNodeRelOp::OutOp
/// Test output of a parse tree to a specified output stream.

void CqParseNodeRelOp::OutOp(std::ostream& Out) const
{
	// Output this nodes operand name.
	switch(m_Operator)
	{
		case Op_L:
			Out << "ls";
			break;

		case Op_G:
			Out << "gt";
			break;

		case Op_GE:
			Out << "ge";
			break;

		case Op_LE:
			Out << "le";
			break;

		case Op_EQ:
			Out << "eq";
			break;

		case Op_NE:
			Out << "ne";
			break;
	}
}


///---------------------------------------------------------------------
/// CqParseNodeUnaryOp::Output
/// Test output of a parse tree to a specified output stream.

void CqParseNodeUnaryOp::Output(std::ostream& Out) const
{
	// Output this node.
	CqParseNode* pOperand=m_pChild;
	assert(pOperand!=0);
	
	char* pstrType=TypeIdentifier(pOperand->ResType());

	Out << pOperand;
	Out << "\t"; OutOp(Out);
	Out << pstrType << std::endl;

	#ifdef _DEBUG
		Out << std::flush;
	#endif;
}

///---------------------------------------------------------------------
/// CqParseNodeUnaryOp::OutOp
/// Test output of a parse tree to a specified output stream.

void CqParseNodeUnaryOp::OutOp(std::ostream& Out) const
{
	// Output this nodes operand name.
	switch(m_Operator)
	{
		case Op_Plus:
			break;

		case Op_Neg:
			Out << "neg";
			break;

		case Op_BitwiseComplement:
			Out << "cmpl";
			break;

		case Op_LogicalNot:
			Out << "not";
			break;
	}
}


///---------------------------------------------------------------------
/// CqParseNodeLogicalOp::Output
/// Test output of a parse tree to a specified output stream.

void CqParseNodeLogicalOp::Output(std::ostream& Out) const
{
	// Output this node.
	CqParseNode* pOperandA=m_pChild;
	CqParseNode* pOperandB=m_pChild->pNext();
	assert(pOperandA!=0 && pOperandB!=0);
	
	char* pstrAType=TypeIdentifier(pOperandA->ResType());
	char* pstrBType=TypeIdentifier(pOperandB->ResType());

	// Logical operations are only valid on float operands
	if((pOperandA->ResType()&Type_Float)==0 ||
	   (pOperandA->ResType()&Type_Float)==0)
	{
		// TODO: Report error.
		return;
	}	

	Out << pOperandA << pOperandB;
	Out << "\t"; OutOp(Out);
	Out << std::endl;

	#ifdef _DEBUG
		Out << std::flush;
	#endif;
}

///---------------------------------------------------------------------
/// CqParseNodeLogicalOp::OutOp
/// Test output of a parse tree to a specified output stream.

void CqParseNodeLogicalOp::OutOp(std::ostream& Out) const
{
	// Output this nodes operand name.
	switch(m_Operator)
	{
		case Op_LogAnd:
			Out << "land";
			break;

		case Op_LogOr:
			Out << "lor";
			break;
	}
}


///---------------------------------------------------------------------
/// CqParseNodeFloatConst::Output
/// Test output of a parse tree to a specified output stream.

void CqParseNodeFloatConst::Output(std::ostream& Out) const
{
	// Output this node.
	Out << "\tpushif " << m_Value << std::endl;

	#ifdef _DEBUG
		Out << std::flush;
	#endif;
}


///---------------------------------------------------------------------
/// CqParseNodeStringConst::Output
/// Test output of a parse tree to a specified output stream.

void CqParseNodeStringConst::Output(std::ostream& Out) const
{
	// Output this node.
	Out << "\tpushis " << "\"" << m_Value.c_str() << "\"" << std::endl;

	#ifdef _DEBUG
		Out << std::flush;
	#endif;
}


///---------------------------------------------------------------------
/// CqParseNodeWhileConstruct::Output
/// Test output of a parse tree to a specified output stream.

void CqParseNodeWhileConstruct::Output(std::ostream& Out) const
{
	TqInt iLabelA=m_cLabels++;
	TqInt iLabelB=m_cLabels++;

	CqParseNode* pArg=m_pChild;
	assert(pArg!=0);
	CqParseNode* pStmt=pArg->pNext();
	assert(pStmt!=0);
	CqParseNode* pStmtInc=pStmt->pNext();
	
	LABEL(iLabelA);							// loop back label
	S_CLEAR;								// clear current state
	Out << pArg;							// relation
	S_GET;									// Get the current state by popping the t[ value off the stack
	S_JZ(iLabelB);							// exit if false
	RS_PUSH;								// push running state
	RS_GET;									// get current state to running state
	Out << pStmt;							// statement
	if(pStmtInc)	Out << pStmtInc;		// incrementor
	RS_POP;									// Pop the running state
	Out << "\tjmp " << iLabelA << std::endl;		// loop back jump
	LABEL(iLabelB);							// completion label
	
	#ifdef _DEBUG
		Out << std::flush;
	#endif;
}


///---------------------------------------------------------------------
/// CqParseNodeIlluminateConstruct::Output
/// Test output of a parse tree to a specified output stream.

void CqParseNodeIlluminateConstruct::Output(std::ostream& Out) const
{
	TqInt iLabelA=m_cLabels++;
	TqInt iLabelB=m_cLabels++;

	CqParseNode* pArg=m_pChild;
	assert(pArg!=0);
	CqParseNode* pStmt=pArg->pNext();
	assert(pStmt!=0);
	CqParseNode* pStmtInc=pStmt->pNext();
	
	TqInt L1=m_cLabels++;					// Get the two labels for the relation statement.
	TqInt L2=m_cLabels++;

	LABEL(iLabelA);							// loop back label
	S_CLEAR;								// clear current state
	Out << pArg;
	if(m_fAxisAngle)	Out << "\tilluminate2" << std::endl;
	else				Out << "\tilluminate" << std::endl;
	S_JZ(iLabelB);							// exit loop if false
	RS_PUSH;								// Push running state
	RS_GET;									// Get state
	Out << pStmt;							// statement
	RS_POP;									// Pop the running state
	Out << TAB << "jmp " << iLabelA << std::endl;		// loop back jump
	LABEL(iLabelB);							// completion label
	
	#ifdef _DEBUG
		Out << std::flush;
	#endif;
}


///---------------------------------------------------------------------
/// CqParseNodeIlluminanceConstruct::Output
/// Test output of a parse tree to a specified output stream.

void CqParseNodeIlluminanceConstruct::Output(std::ostream& Out) const
{
	TqInt iLabelA=m_cLabels++;
	TqInt iLabelB=m_cLabels++;

	CqParseNode* pArg=m_pChild;
	assert(pArg!=0);
	CqParseNode* pStmt=pArg->pNext();
	assert(pStmt!=0);
	CqParseNode* pStmtInc=pStmt->pNext();
	
	// The last child of the arg node is the Point to be illuminated, see Parser.y for confirmation.
	CqParseNode* pInitArg=pArg->pLastChild();
	Out << pInitArg;
	Out << TAB << "init_illuminance" << std::endl;
	Out << TAB << "jz " << iLabelB << std::endl;	// Jump if no lightsources.
	LABEL(iLabelA);							// loop back label
	S_CLEAR;								// clear current state
	Out << pArg;
	if(m_fAxisAngle)	Out << "\tilluminance2" << std::endl;
	else				Out << "\tilluminance" << std::endl;
	S_JZ(iLabelB);							// exit loop if false
	RS_PUSH;								// Push running state
	RS_GET;									// Get state
	Out << pStmt;							// statement
	RS_POP;									// Pop the running state
	Out << TAB << "advance_illuminance" << std::endl;
	Out << TAB << "jnz " << iLabelA << std::endl;// loop back jump
	LABEL(iLabelB);							// completion label
	
	#ifdef _DEBUG
		Out << std::flush;
	#endif;
}


///---------------------------------------------------------------------
/// CqParseNodeSolarConstruct::Output
/// Test output of a parse tree to a specified output stream.

void CqParseNodeSolarConstruct::Output(std::ostream& Out) const
{
	TqInt iLabelA=m_cLabels++;
	TqInt iLabelB=m_cLabels++;

	CqParseNode* pArg=m_pChild;
	assert(pArg!=0);
	CqParseNode* pStmt=pArg->pNext();
	assert(pStmt!=0);
	CqParseNode* pStmtInc=pStmt->pNext();
	
	LABEL(iLabelA);							// loop back label
	S_CLEAR;								// clear current state
	Out << pArg;
	if(m_fAxisAngle)	Out << "\tsolar2" << std::endl;
	else				Out << "\tsolar" << std::endl;
	S_JZ(iLabelB);							// exit loop if false
	RS_PUSH;								// Push running state
	RS_GET;									// set running state
	Out << pStmt;							// statement
	RS_POP;									// Pop the running state
	Out << "\tjmp " << iLabelA << std::endl;		// loop back jump
	LABEL(iLabelB);							// completion label

	#ifdef _DEBUG
		Out << std::flush;
	#endif;
}


///---------------------------------------------------------------------
/// CqParseNodeConditional::Output
/// Test output of a parse tree to a specified output stream.

void CqParseNodeConditional::Output(std::ostream& Out) const
{
	TqInt iLabelA=m_cLabels++;
	TqInt iLabelB;

	CqParseNode* pArg=m_pChild;
	assert(pArg!=0);
	CqParseNode* pTrueStmt=pArg->pNext();
	assert(pTrueStmt!=0);
	CqParseNode* pFalseStmt=pTrueStmt->pNext();

	S_CLEAR;								// clear current state
	Out << pArg;							// relation
	S_GET;									// Get the current state by popping the top value off the stack
	RS_PUSH;								// push the running state
	RS_GET;									// get current state to running state
	if(pFalseStmt)
	{
		iLabelB=m_cLabels++;
		RS_JZ(iLabelB);						// skip true statement if all false
	}
	else
		RS_JZ(iLabelA);						// exit if all false
	Out << pTrueStmt;						// true statement
	if(pFalseStmt)
	{
		LABEL(iLabelB);						// false part label
		RS_JNZ(iLabelA);					// exit if all true
		RS_INVERSE;							// Invert result
		Out << pFalseStmt;					// false statement
	}
	LABEL(iLabelA);							// conditional exit point
	RS_POP;									// pop running state

	#ifdef _DEBUG
		Out << std::flush;
	#endif;
}


///---------------------------------------------------------------------
/// CqParseNodeQCond::Output
/// Output a Q conditional (?A:B) to the output stream.

void CqParseNodeQCond::Output(std::ostream& Out) const
{
	CqParseNode* pArg=m_pChild;
	assert(pArg!=0);
	CqParseNode* pTrueStmt=pArg->pNext();
	assert(pTrueStmt!=0);
	CqParseNode* pFalseStmt=pTrueStmt->pNext();

	EqVariableType typeT=static_cast<EqVariableType>(pTrueStmt->ResType()&Type_Mask);
	char* pstrTType=TypeIdentifier(typeT);

	Out << pTrueStmt;						// true statement
	Out << pFalseStmt;						// false statement
	Out << pArg;							// relation
	Out << "\tmerge" << pstrTType << std::endl;

	#ifdef _DEBUG
		Out << std::flush;
	#endif;
}



///---------------------------------------------------------------------
/// CqParseNodeCast::Output
/// Test output of a parse tree to a specified output stream.

void CqParseNodeCast::Output(std::ostream& Out) const
{
	// Output this node.
	CqParseNode* pOperand=m_pChild;
	assert(pOperand!=0);
	
	EqVariableType typeA=static_cast<EqVariableType>(pOperand->ResType()&Type_Mask);
	EqVariableType typeB=static_cast<EqVariableType>(m_tTo&Type_Mask);
	// No need to output a cast for the triple or h types.
	Out << pOperand;
	if(!((typeA==Type_Point || typeA==Type_Normal || typeA==Type_Vector) &&
	     (typeB==Type_Point || typeB==Type_Normal || typeB==Type_Vector)))
	{	
		char* pstrToType=TypeIdentifier(m_tTo);
		char* pstrFromType=TypeIdentifier(pOperand->ResType());
		Out << "\tset" << pstrFromType << pstrToType << std::endl;
	}

	#ifdef _DEBUG
		Out << std::flush;
	#endif;
}


///---------------------------------------------------------------------
/// CqParseNodeTriple::Output
/// Test output of a parse tree to a specified output stream.

void CqParseNodeTriple::Output(std::ostream& Out) const
{
	// Output this node.
	CqParseNode* pA=m_pChild;
	assert(pA!=0);
	CqParseNode* pB=pA->pNext();
	assert(pB!=0);
	CqParseNode* pC=pB->pNext();
	assert(pC!=0);

	// Output the 'push'es in reverse, so that Red/X ec is first off the stack when doing a 'sett?' instruction.
	Out << pC << pB << pA;

	#ifdef _DEBUG
		Out << std::flush;
	#endif;
}


///---------------------------------------------------------------------
/// CqParseNodeHexTuple::Output
/// Test output of a parse tree to a specified output stream.

void CqParseNodeHexTuple::Output(std::ostream& Out) const
{
	// Output this node.
	CqParseNode* p00=m_pChild;		if(p00==0)	throw(XqException("Invalid arguments to matrix initialiser"));
	CqParseNode* p01=p00->pNext();	if(p01==0)	throw(XqException("Invalid arguments to matrix initialiser"));
	CqParseNode* p02=p01->pNext();	if(p02==0)	throw(XqException("Invalid arguments to matrix initialiser"));
	CqParseNode* p03=p02->pNext();	if(p03==0)	throw(XqException("Invalid arguments to matrix initialiser"));

	CqParseNode* p10=p03->pNext();	if(p10==0)	throw(XqException("Invalid arguments to matrix initialiser"));
	CqParseNode* p11=p10->pNext();	if(p11==0)	throw(XqException("Invalid arguments to matrix initialiser"));
	CqParseNode* p12=p11->pNext();	if(p12==0)	throw(XqException("Invalid arguments to matrix initialiser"));
	CqParseNode* p13=p12->pNext();	if(p13==0)	throw(XqException("Invalid arguments to matrix initialiser"));

	CqParseNode* p20=p13->pNext();	if(p20==0)	throw(XqException("Invalid arguments to matrix initialiser"));
	CqParseNode* p21=p20->pNext();	if(p21==0)	throw(XqException("Invalid arguments to matrix initialiser"));
	CqParseNode* p22=p21->pNext();	if(p22==0)	throw(XqException("Invalid arguments to matrix initialiser"));
	CqParseNode* p23=p22->pNext();	if(p23==0)	throw(XqException("Invalid arguments to matrix initialiser"));

	CqParseNode* p30=p23->pNext();	if(p30==0)	throw(XqException("Invalid arguments to matrix initialiser"));
	CqParseNode* p31=p30->pNext();	if(p31==0)	throw(XqException("Invalid arguments to matrix initialiser"));
	CqParseNode* p32=p31->pNext();	if(p32==0)	throw(XqException("Invalid arguments to matrix initialiser"));
	CqParseNode* p33=p32->pNext();	if(p33==0)	throw(XqException("Invalid arguments to matrix initialiser"));

	// Output the 'push'es in reverse, so that Red/X ec is first off the stack when doing a 'sett?' instruction.
	Out << p00 << p01 << p02 << p03 << p10 << p11 << p12 << p13 << p20 << p21 << p22 << p23 << p30 << p31 << p32 << p33;

	#ifdef _DEBUG
		Out << std::flush;
	#endif;
}


///---------------------------------------------------------------------
/// CqParseNodeCommFunction::Output
/// Test output of a parse tree to a specified output stream.

void CqParseNodeCommFunction::Output(std::ostream& Out) const
{
	// Output this node.
	CqParseNode* pExpr=m_pChild;
	pExpr->Output(Out);

	CqString strCommType("surface");
	switch(m_commType)
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
	CqVarDef* pVD=CqVarDef::GetVariablePtr(m_vrVariable);
	if(pVD)
	{
		pVD->IncUseCount();
		Out << TAB << strCommType.c_str() << " " << pVD->strName() << std::endl;
	}

	#ifdef _DEBUG
		Out << std::flush;
	#endif;
}


///---------------------------------------------------------------------
/// CqParseNodeSetComp::Output
/// Test output of a parse tree to a specified output stream.

void CqParseNodeSetComp::Output(std::ostream& Out) const
{
	// Output this node.
	CqParseNode* pExpr=m_pChild;

	CqVarDef* pVD=CqVarDef::GetVariablePtr(m_vrVariable);
	if(pVD)
	{
		// Output the set expression
		pExpr->Output(Out);

		CqString strCommand("p");
		switch(pVD->Type()&Type_Mask)
		{
			case Type_hPoint:
				strCommand="h";
				break;

			case Type_Vector:
				strCommand="v";
				break;

			case Type_Normal:
				strCommand="n";
				break;
		}

		switch(m_Index)
		{
			case 0:
				strCommand+="setxcomp";
				break;

			case 1:
				strCommand+="setycomp";
				break;

			case 2:
				strCommand+="setzcomp";
				break;
		}

		pVD->IncUseCount();
		Out << TAB << strCommand.c_str() << " " << pVD->strName() << std::endl;
	}

	#ifdef _DEBUG
		Out << std::flush;
	#endif;
}


///---------------------------------------------------------------------
/// CqParseNodeSetColComp::Output
/// Test output of a parse tree to a specified output stream.

void CqParseNodeSetColComp::Output(std::ostream& Out) const
{
	// Output this node.
	CqParseNode* pExpr=m_pChild;

	CqVarDef* pVD=CqVarDef::GetVariablePtr(m_vrVariable);
	if(pVD)
	{
		// Output the set expression
		pExpr->Output(Out);
		pVD->IncUseCount();
		Out << TAB << "setcomp" << " " << pVD->strName() << std::endl;
	}

	#ifdef _DEBUG
		Out << std::flush;
	#endif;
}


END_NAMESPACE(Aqsis)
//---------------------------------------------------------------------
