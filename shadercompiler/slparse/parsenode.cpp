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

#if defined (AQSIS_SYSTEM_WIN32)
	#define tolower(x) _tolower(x)
#endif

START_NAMESPACE( Aqsis )

static char* gVariableTypeNames[] =
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
static TqInt gcVariableTypeNames = sizeof( gVariableTypeNames ) / sizeof( gVariableTypeNames[ 0 ] );

char* gShaderTypeNames[] =
    {
        "surface",
        "lightsource",
        "volume",
        "displacement",
        "transformation",
        "imager",
    };
TqInt gcShaderTypeNames = sizeof( gShaderTypeNames ) / sizeof( gShaderTypeNames[ 0 ] );


//---------------------------------------------------------------------
// Static data on CqParseNode

TqInt	CqParseNode::m_cLabels = 0;
TqInt	CqParseNode::m_aaTypePriorities[ Type_Last ][ Type_Last ] =
    {
        //				   @  f	 i	p  s  c	 t	h  n  v	 x	m
        /*Type_Nil*/	 { 99,   00,   00,   00,   00,   00,   00,   00,   00,   00,   00,   00,   00 },
        /*Type_Float*/	 { 00,   99,   98,   02,   00,   01,   02,   02,   02,   02,   00,   01,   00 },
        /*Type_Integer*/ { 00,   98,   99,   00,   00,   00,   00,   00,   00,   00,   00,   00,   00 },
        /*Type_Point*/	 { 00,   00,   00,   99,   00,   96,   98,   98,   97,   97,   00,   00,   00 },
        /*Type_String*/	 { 00,   00,   00,   00,   99,   00,   00,   00,   00,   00,   00,   00,   00 },
        /*Type_Color*/	 { 00,   00,   00,   01,   00,   99,   02,   01,   01,   01,   00,   00,   00 },
        /*Type_Triple*/	 { 00,   00,   00,   98,   00,   98,   99,   97,   98,   98,   00,   00,   00 },
        /*Type_hPoint*/	 { 00,   00,   00,   97,   00,   96,   98,   99,   98,   98,   00,   00,   00 },
        /*Type_Normal*/	 { 00,   00,   00,   98,   00,   96,   98,   97,   99,   98,   00,   00,   00 },
        /*Type_Vector*/	 { 00,   00,   00,   98,   00,   96,   98,   97,   98,   99,   00,   00,   00 },
        /*Type_Void*/	 { 00,   00,   00,   00,   00,   00,   00,   00,   00,   00,   99,   00,   00 },
        /*Type_Matrix*/	 { 00,   00,   00,   00,   00,   00,   00,   00,   00,   00,   00,   99,   00 },
        /*Type_HexTuple*/{ 00,   00,   00,   00,   00,   00,   00,   00,   00,   00,   00,   99,   98 },
    };
TqInt	CqParseNode::m_aAllTypes[ Type_Last - 1 ] =
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


const EqParseNodeType IqParseNode::m_ID = ParseNode_Base;
const EqParseNodeType IqParseNodeShader::m_ID = ParseNode_Shader;
const EqParseNodeType IqParseNodeFunctionCall::m_ID = ParseNode_FunctionCall;
const EqParseNodeType IqParseNodeUnresolvedCall::m_ID = ParseNode_UnresolvedCall;
const EqParseNodeType IqParseNodeVariable::m_ID = ParseNode_Variable;
const EqParseNodeType IqParseNodeArrayVariable::m_ID = ParseNode_ArrayVariable;
const EqParseNodeType IqParseNodeVariableAssign::m_ID = ParseNode_VariableAssign;
const EqParseNodeType IqParseNodeArrayVariableAssign::m_ID = ParseNode_ArrayVariableAssign;
const EqParseNodeType IqParseNodeOperator::m_ID = ParseNode_Operator;
const EqParseNodeType IqParseNodeMathOp::m_ID = ParseNode_MathOp;
const EqParseNodeType IqParseNodeRelationalOp::m_ID = ParseNode_RelationalOp;
const EqParseNodeType IqParseNodeUnaryOp::m_ID = ParseNode_UnaryOp;
const EqParseNodeType IqParseNodeLogicalOp::m_ID = ParseNode_LogicalOp;
const EqParseNodeType IqParseNodeDiscardResult::m_ID = ParseNode_DiscardResult;
const EqParseNodeType IqParseNodeConstantFloat::m_ID = ParseNode_ConstantFloat;
const EqParseNodeType IqParseNodeConstantString::m_ID = ParseNode_ConstantString;
const EqParseNodeType IqParseNodeWhileConstruct::m_ID = ParseNode_WhileConstruct;
const EqParseNodeType IqParseNodeIlluminateConstruct::m_ID = ParseNode_IlluminateConstruct;
const EqParseNodeType IqParseNodeIlluminanceConstruct::m_ID = ParseNode_IlluminanceConstruct;
const EqParseNodeType IqParseNodeSolarConstruct::m_ID = ParseNode_SolarConstruct;
const EqParseNodeType IqParseNodeConditional::m_ID = ParseNode_Conditional;
const EqParseNodeType IqParseNodeConditionalExpression::m_ID = ParseNode_ConditionalExpression;
const EqParseNodeType IqParseNodeTypeCast::m_ID = ParseNode_TypeCast;
const EqParseNodeType IqParseNodeTriple::m_ID = ParseNode_Triple;
const EqParseNodeType IqParseNodeSixteenTuple::m_ID = ParseNode_SixteenTuple;
const EqParseNodeType IqParseNodeMessagePassingFunction::m_ID = ParseNode_MessagePassingFunction;


///---------------------------------------------------------------------
/** Find the shader node, to aid in identifying the shader type.
 */

CqParseNodeShader* CqParseNode::pShaderNode()
{
	// Search up in the hierarchy.
	CqParseNode* pShader = this;
	while( ( NULL != pShader ) && ( pShader->NodeType() != IqParseNodeShader::m_ID ) )
		pShader = pShader->m_pParent;
	return( static_cast<CqParseNodeShader*>( pShader ) );
}


///---------------------------------------------------------------------
/** Return a string type identifier for the specified type.
 */

char* CqParseNode::TypeIdentifier( int Type )
{
	return ( gVariableTypeIdentifiers[ Type & Type_Mask ] );
}


///---------------------------------------------------------------------
/** Return a type for the specified type identifier.
 */

TqInt CqParseNode::TypeFromIdentifier( char Id )
{
	TqInt i;
	for ( i = 0; i < Type_Last; i++ )
	{
		if ( gVariableTypeIdentifiers[ i ][ 0 ] == Id || gVariableTypeIdentifiers[ i ][ 0 ] == tolower( Id ) )
			return ( i );
	}
	return ( Type_Nil );
}


///---------------------------------------------------------------------
/** Return a string type name for the specified type.
 */

char* CqParseNode::TypeName( int Type )
{
	return ( gVariableTypeNames[ Type & Type_Mask ] );
}


///---------------------------------------------------------------------
/** Find a valid cast type from the list of available options.
 */

TqInt	CqParseNode::FindCast( TqInt CurrType, TqInt* pTypes, TqInt Count, TqInt& index )
{
	// Check if the current type exists in the list first.
	TqInt i;
	for ( i = 0; i < Count; i++ )
		if ( ( pTypes[ i ] & Type_Mask ) == ( CurrType & Type_Mask ) )
		{
			index = i;
			return ( static_cast<TqInt>( CurrType & Type_Mask ) );
		}

	// Else search for the best option.
	TqInt Ret = Type_Nil;
	TqInt Pri = 0;
	for ( i = 0; i < Count; i++ )
	{
		if ( m_aaTypePriorities[ CurrType & Type_Mask ][ ( pTypes[ i ] & Type_Mask ) ] > Pri )
		{
			index = i;
			Ret = pTypes[ i ];
			Pri = m_aaTypePriorities[ CurrType & Type_Mask ][ ( pTypes[ i ] & Type_Mask ) ];
		}
	}
	return ( Ret );
}


///---------------------------------------------------------------------
/// CqParseNodeFunctionCall::ResType

TqInt CqParseNodeFunctionCall::ResType() const
{
	// The return type of a function depends on its arguments.
	return ( CqFuncDef::GetFunctionPtr( m_aFuncRef[ 0 ] ) ->Type() );
}


const char*	CqParseNodeFunctionCall::strName() const
{
	CqFuncDef * pFuncDef = CqFuncDef::GetFunctionPtr( m_aFuncRef[ 0 ] );
	if ( pFuncDef != 0 )
		return ( pFuncDef->strName() );
	else
		return ( "" );
}


const IqFuncDef* CqParseNodeFunctionCall::pFuncDef() const
{
	CqFuncDef * pFuncDef = CqFuncDef::GetFunctionPtr( m_aFuncRef[ 0 ] );
	if ( pFuncDef != 0 )
		return ( pFuncDef );
	else
		return ( 0 );
}


IqFuncDef* CqParseNodeFunctionCall::pFuncDef()
{
	CqFuncDef * pFuncDef = CqFuncDef::GetFunctionPtr( m_aFuncRef[ 0 ] );
	if ( pFuncDef != 0 )
		return ( pFuncDef );
	else
		return ( 0 );
}


///---------------------------------------------------------------------
/// CqParseNodeUnresolvedCall::ResType

TqInt CqParseNodeUnresolvedCall::ResType() const
{
	// The return type of a function depends on its arguments.
	return ( m_aFuncDef.Type() );
}


const char*	CqParseNodeUnresolvedCall::strName() const
{
	return ( m_aFuncDef.strName() );
}


const IqFuncDef* CqParseNodeUnresolvedCall::pFuncDef() const
{
	return ( &m_aFuncDef );
}

IqFuncDef* CqParseNodeUnresolvedCall::pFuncDef()
{
	return ( &m_aFuncDef );
}


///---------------------------------------------------------------------
/// CqParseNodeVariable::CqParseNodeVariable

CqParseNodeVariable::CqParseNodeVariable( SqVarRef VarRef ) :
		CqParseNode(),
		m_VarRef( VarRef )
{
	m_fVarying = ( CqVarDef::GetVariablePtr( VarRef ) ->Type() & Type_Varying ) != 0;
}


///---------------------------------------------------------------------
/// CqParseNodeVariable::CqParseNodeVariable

CqParseNodeVariable::CqParseNodeVariable( CqParseNodeVariable* pVar ) :
		CqParseNode(),
		m_VarRef( pVar->m_VarRef )
{
	m_fVarying = ( CqVarDef::GetVariablePtr( m_VarRef ) ->Type() & Type_Varying ) != 0;
}


///---------------------------------------------------------------------
/// CqParseNodeVariable::strName
/// Returnt the variable name.

const char* CqParseNodeVariable::strName() const
{
	return ( CqVarDef::GetVariablePtr( m_VarRef ) ->strName() );
}


///---------------------------------------------------------------------
/// CqParseNodeVariable::ResType
/// Return a string type identifier for this variable.

TqInt CqParseNodeVariable::ResType() const
{
	IqVarDef * pVD = IqVarDef::GetVariablePtr( m_VarRef );
	if ( pVD )
		return ( pVD->Type() & Type_Mask );
	else
		return ( Type_Nil );
}


///---------------------------------------------------------------------
/// CqParseNodeVariableArray::CqParseNodeVariableArray
/// Constructor from a variable reference.

CqParseNodeVariableArray::CqParseNodeVariableArray( SqVarRef VarRef ) :
		CqParseNodeVariable( VarRef )
{}


///---------------------------------------------------------------------
/// CqParseNodeVariableArray::CqParseNodeVariableArray
/// Construct from another variable




CqParseNodeVariableArray::CqParseNodeVariableArray( CqParseNodeVariableArray* pVar ) :
		CqParseNodeVariable( pVar )
{
	m_fVarying = ( CqVarDef::GetVariablePtr( m_VarRef ) ->Type() & Type_Varying ) != 0;
	if ( pVar->m_pChild )
		m_pChild = pVar->m_pChild->Clone( this );
}


///---------------------------------------------------------------------
/// CqParseNodeAssign::ResType
/// Return a string type identifier for this variable.

TqInt CqParseNodeAssign::ResType() const
{
	return ( CqVarDef::GetVariablePtr( m_VarRef ) ->Type() );
}


///---------------------------------------------------------------------
/// CqParseNodeMathOp::ResType
/// Return a string type identifier for this variable.

TqInt CqParseNodeMathOp::ResType() const
{
	CqParseNode * pOperandA = m_pChild;
	CqParseNode* pOperandB = m_pChild->pNext();
	assert( pOperandA != 0 && pOperandB != 0 );

	TqInt ResAType = pOperandA->ResType();
	TqInt ResBType = pOperandB->ResType();

	switch ( m_Operator )
	{
			case Op_Dot:
			return ( Type_Float );
			case Op_Mul:
			case Op_Div:
			case Op_Add:
			case Op_Sub:
			default:
			// TODO: Should check here for valid types.
			if ( ( ResAType & Type_Mask ) == Type_Point ||
			        ( ResAType & Type_Mask ) == Type_Color )
				return ( ResAType );
			else
				return ( ResBType );
			break;
	}
}


END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------
