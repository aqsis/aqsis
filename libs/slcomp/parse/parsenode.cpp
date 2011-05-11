////---------------------------------------------------------------------
////    Class definition file:  PARSENODE.CPP
////    Associated header file: PARSENODE.H
////
////    Author:					Paul C. Gregory
////    Creation date:			18/07/99
////---------------------------------------------------------------------

#include	<aqsis/aqsis.h>

#include	<algorithm>
#include	<map>

#include	"parsenode.h"
#include	"funcdef.h"
#include	"vardef.h"

#if defined (AQSIS_SYSTEM_WIN32)
	#define tolower(x) _tolower(x)
#endif

namespace Aqsis {

const char* const gVariableTypeNames[] =
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

const char* gShaderTypeNames[] =
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
        //				   @     f     i     p     s     c     t     h     n     v     x     m     x
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
const EqParseNodeType IqParseNodeLoopMod::m_ID = ParseNode_LoopMod;
const EqParseNodeType IqParseNodeIlluminateConstruct::m_ID = ParseNode_IlluminateConstruct;
const EqParseNodeType IqParseNodeIlluminanceConstruct::m_ID = ParseNode_IlluminanceConstruct;
const EqParseNodeType IqParseNodeSolarConstruct::m_ID = ParseNode_SolarConstruct;
const EqParseNodeType IqParseNodeGatherConstruct::m_ID = ParseNode_GatherConstruct;
const EqParseNodeType IqParseNodeConditional::m_ID = ParseNode_Conditional;
const EqParseNodeType IqParseNodeConditionalExpression::m_ID = ParseNode_ConditionalExpression;
const EqParseNodeType IqParseNodeTypeCast::m_ID = ParseNode_TypeCast;
const EqParseNodeType IqParseNodeTriple::m_ID = ParseNode_Triple;
const EqParseNodeType IqParseNodeSixteenTuple::m_ID = ParseNode_SixteenTuple;
const EqParseNodeType IqParseNodeMessagePassingFunction::m_ID = ParseNode_MessagePassingFunction;
const EqParseNodeType IqParseNodeTextureNameWithChannel::m_ID = ParseNode_TextureNameWithChannel;


///---------------------------------------------------------------------
/** Default implementation of validTypes, just gets the return type, and then
 *  fills the array with that and the possible cast types in order of preference.
 */
bool cmpCasts(const std::pair<TqInt, TqInt>& a, const std::pair<TqInt, TqInt>& b)
{
	return(a.second > b.second);
}
void CqParseNode::validTypes( std::list<std::pair<TqInt, TqInt> >& types)
{
	TqInt mainType = ResType();
	types.clear();
	types.push_front(std::pair<TqInt, TqInt>(mainType, 99));
	std::vector<std::pair<TqInt, TqInt> > casts;
	for(TqInt castType = Type_Nil; castType < Type_Last; ++castType)
		if(m_aaTypePriorities[mainType & Type_Mask][castType & Type_Mask] != 0)
			casts.push_back(std::pair<TqInt, TqInt>(castType, m_aaTypePriorities[mainType & Type_Mask][castType & Type_Mask]));
	std::sort(casts.begin(), casts.end(), cmpCasts);
	for(std::vector<std::pair<TqInt, TqInt> >::const_iterator c = casts.begin(); c != casts.end(); ++c)
		types.push_back(*c);
}


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

const char* CqParseNode::TypeIdentifier( int Type )
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

const char* CqParseNode::TypeName( int Type )
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

void CqParseNodeFunctionCall::validTypes( std::list<std::pair<TqInt, TqInt> >& types)
{
	// First do a typecheck, which will eliminate all candidates that don't 
	// match the argument list.
	bool needsCast;
	TqInt bestType = TypeCheck( pAllTypes(), Type_Last - 1, needsCast, true );
	// Create a map of types against suitability weights.
	std::map<TqInt, TqInt> suitableTypes;
	// For each candidate function, add it's real return type.
	std::vector<SqFuncRef>::iterator i;
	for ( i = m_aFuncRef.begin(); i != m_aFuncRef.end(); ++i )
	{
		TqInt mainType = bestType;
		// Set a directly available type as a high priority no matter what.
		suitableTypes[mainType] = 99;
		// Now check the possible castable types, storing only if they have a weight greater than 
		// an existing mapping.
		for(TqInt castType = Type_Nil; castType < Type_Last; ++castType)
			if(m_aaTypePriorities[mainType & Type_Mask][castType & Type_Mask] != 0 && 
					castType != mainType && 
					( suitableTypes.find(castType) == suitableTypes.end() ||  
					  suitableTypes[castType] < m_aaTypePriorities[mainType & Type_Mask][castType & Type_Mask]) )
				suitableTypes[castType] = m_aaTypePriorities[mainType & Type_Mask][castType & Type_Mask];
	}
	// Now copy the findings to the types list
	types.clear();
	std::copy(suitableTypes.begin(), suitableTypes.end(), std::back_inserter(types));
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


} // namespace Aqsis
//---------------------------------------------------------------------
