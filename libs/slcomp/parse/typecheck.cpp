///---------------------------------------------------------------------
////    Class definition file:  TYPECHECK.CPP
////    Associated header file: PARSENODE.H
////
////    Author:					Paul C. Gregory
////    Creation date:			25/11/99
////---------------------------------------------------------------------

#include	<aqsis/aqsis.h>
#include	"parsenode.h"
#include	<aqsis/util/logging.h>
#include	<aqsis/util/exception.h>

#include	<queue>

namespace Aqsis {

///---------------------------------------------------------------------
/// CqParseNodeFunctionCall::TypeCheck
/// Do a type check based on suitable types requested, and add a cast if required.
/// Uses all possible functions listed to try to fund a suitable candidate.

class CqFunctionSignature
{
	public:
		CqFunctionSignature(SqFuncRef ref, CqFuncDef* func, TqInt weight, bool mustCast) : m_ref(ref), m_func(func), m_weight(weight), m_mustCast(mustCast)
		{}
		~CqFunctionSignature()
		{}

		const SqFuncRef& getRef() const
		{
			return(m_ref);
		}

		SqFuncRef& getRef()
		{
			return(m_ref);
		}

		CqFuncDef* getFunc() const
		{
			return(m_func);
		}

		TqInt getWeight() const
		{
			return(m_weight);
		}

		bool needsCast() const
		{
			return(m_mustCast);
		}

	private:
		SqFuncRef m_ref;
		CqFuncDef* m_func;
		TqInt m_weight;
		bool m_mustCast;

		friend bool operator<(const CqFunctionSignature& a, const CqFunctionSignature& b)
		{
			return(a.m_weight < b.m_weight);
		}

};


TqInt	CqParseNodeFunctionCall::TypeCheck( TqInt* pTypes, TqInt Count,  bool& needsCast, bool CheckOnly )
{
	TqInt cSpecifiedArgs = 0;

	// This is the priority queue that we will build for the available
	// signatures. The top entry will be the most suitable for use.
	std::priority_queue<CqFunctionSignature> functions;

	CqString strMyName = strName();

	// First discard any function possibilities which don't have the correct number of arguments.
	// Find out how many arguments have been specified.
	CqParseNode* pArg = m_pChild;
	while ( pArg != 0 )
	{
		cSpecifiedArgs++;
		pArg = pArg->pNext();
	}

	Aqsis::log() << info << "Typechecking function call \"" << strName() << "\", possible matches: " << m_aFuncRef.size() <<  "[Line: " << LineNo() << "]" << std::endl;

	// First build a list of valid types, ordered by preference, for each specified argument.
	pArg = m_pChild;
	TqUint iArg = 0;
	std::vector<std::list<std::pair<TqInt, TqInt> > >	argTypes;
	while(pArg != 0)
	{
		CqParseNode* pNext = pArg->pNext();
		std::list<std::pair<TqInt, TqInt> > types;
		pArg->validTypes(types);
		argTypes.push_back(types);
		++iArg;
		pArg = pNext;
	}

	// Now for each candidate, check if the valid types can match, storing a weight depending on the suitability.
	std::vector<SqFuncRef>::iterator i;
	for ( i = m_aFuncRef.begin(); i != m_aFuncRef.end();  )
	{
		CqFuncDef* pfunc = CqFuncDef::GetFunctionPtr( (*i) );
		if( pfunc != 0 )
		{
			Aqsis::log() << info << "Considering candidate (" << TypeName(pfunc->Type()) << ")" << pfunc->strName() << "[" << pfunc->strParams() << "]" << std::endl;
			TqInt weight = 0;
			bool mustCast = false;
			TqInt cArgs = pfunc->cTypeSpecLength();
			if ( ( cArgs > 0 ) &&
			        ( ( !pfunc->fVarying() && cArgs != cSpecifiedArgs ) ||
			          (  pfunc->fVarying() && cSpecifiedArgs < cArgs ) ) )
			{
				// This function isn't suitable for the arguments list, as the
				// argument counts don't match. Don't add it to the queue.
				i = m_aFuncRef.erase(i);
				continue;
			}
			else
			{
				// Check the return type against the possible return types requested,
				// a function that can return the right type without casting carries more weight.
				TqInt castType, index;
				if( (castType = FindCast(pfunc->Type(), pTypes, Count, index)) == Type_Nil )
				{
					// This signature cannot return any of the requested types,
					// Can't delete it, as it is possible that TypeCheck may be later called
					// with a different set of requested types.
					++i;
					continue;
				}
				else
				{
					// A function signature that can return one of the requested types without
					// casting has more weight.
					TqInt retType;
					mustCast = true;
					for(retType = 0; retType < Count; retType++)
					{
						if(castType == pfunc->Type())
						{
							weight++;
							mustCast = false;
							break;
						}
					}
				}
				// Check the arguments in turn to see if they can be used for this candidate.
				TqInt iArg = 0;
				bool fArgsFailed = false;
				for(std::vector<std::list<std::pair<TqInt, TqInt> > >::iterator arg = argTypes.begin(); arg != argTypes.end(); ++arg, ++iArg)
				{
					// If the candidate signature is varying, and we've hit the '*' exit at this point.
					if(pfunc->fVarying() && iArg >= cArgs)
						break;
					std::list<std::pair<TqInt, TqInt> >::const_iterator match;
					for(match = arg->begin(); match != arg->end(); ++match)
					{
						/// \todo: Need to check the 'usage' of the required argument, specifically
						/// check if Type_Variable matches.
						if((match->first & Type_Mask) == (pfunc->aTypeSpec()[iArg] & Type_Mask))
						{
							weight += match->second;
							break;
						}
					}
					if(match == arg->end())
					{
						Aqsis::log() << info << "Argument " << iArg << " doesn't match:";
						if(!arg->empty())
							Aqsis::log() << info << " got " << TypeName(arg->front().first);
						Aqsis::log() << info << " expected " << TypeName(pfunc->aTypeSpec()[iArg]) << std::endl;
						fArgsFailed = true;
					}
				}

				// If we got this far, the function signature matches, and we have a weight
				// from the various data gathered, so add it to the queue.
				if(!fArgsFailed)
				{
					functions.push(CqFunctionSignature((*i), pfunc, weight, mustCast));
					++i;
				}
				else
				{
					// If the arguments check failed, this one can actually be removed from the list on the
					// node, as the arguments aren't going to change, only the potential return type
					// for different calls to TypeCheck.
					i = m_aFuncRef.erase(i);
				}
			}
		}
		else
			++i;
	}

	// If we don't have any candidate function signatures at all, then there is an error, so report
	// it and retun nil.
	if( functions.empty() )
	{
		if(!CheckOnly)
		{
			AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
				strFileName() << " : " << LineNo() << " : Arguments to function not valid : " <<  strMyName);
		}
		return ( Type_Nil );
	}

	const CqFunctionSignature& best = functions.top();
	Aqsis::log() << info << "Selected (" << TypeName(best.getFunc()->Type()) << ")" << best.getFunc()->strName() << "[" << best.getFunc()->strParams() << "]" << std::endl;
	needsCast = best.needsCast();

	// If we are not just checking, at this point, we have the best function candidate,
	// so cast the arguments if necessary, and apply a cast to the return if necessary.
	if(!CheckOnly)
	{
		// Set the top function signature to the chosen one.
		m_aFuncRef[0] = best.getRef();

		CqFuncDef* pfunc = best.getFunc();
		// Apply casts to the arguments first.
		pArg = m_pChild;
		TqUint iArg = 0;
		while ( pArg != 0 )
		{
			CqParseNode * pNext = pArg->pNext();
			// Get the required type for the function signature.
			TqInt argType;
			if ( (TqUint) pfunc->cTypeSpecLength() <= iArg || ( pfunc->fVarying() && (TqUint) pfunc->cTypeSpecLength() == iArg ) )
				break;
			else
				argType = pfunc->aTypeSpec() [ iArg ];

			// Now type check the argument, and cast if necessary
			argType = pArg->TypeCheck( &argType, 1, needsCast, false );

			pArg = pNext;
			iArg++;
		}

		// Now check if the return value needs to be cast.
		TqInt retType, index;
		if ( ( retType = FindCast( pfunc->Type(), pTypes, Count, index ) ) != Type_Nil )
		{
			// Add a cast to the new type.
			CqParseNode* pCast = new CqParseNodeCast( retType );
			LinkParent( pCast );
		}
	}
	return(best.getFunc()->Type());
}


///---------------------------------------------------------------------
/// CqParseNodeFunctionCall::CheckArgCast
/// Check if it is possible to convert the arguments passed to this function
/// into valid arguments for any of its function declarations.

void CqParseNodeFunctionCall::CheckArgCast( std::vector<TqInt>& aRes )
{
	// Find out how many arguments have been specified.
	TqUint cArgs = 0;
	std::vector<TqInt> aArgTypes;
	CqParseNode* pArg = m_pChild;
	while ( pArg != 0 )
	{
		cArgs++;
		aArgTypes.push_back( pArg->ResType() );
		pArg = pArg->pNext();
	}

	// For each posible declaration...
	TqUint i;
	for ( i = 0; i < aRes.size(); i++ )
	{
		CqFuncDef* pFuncDef = CqFuncDef::GetFunctionPtr( m_aFuncRef[ aRes[ i ] ] );
		if ( pFuncDef == 0 )
			continue;

		// ...build a list of variable types from its parameter type string.
		std::vector<TqInt>& aTypes = pFuncDef->aTypeSpec();

		// Check the number of arguments first.
		if ( ( aTypes.size() != cArgs && !pFuncDef->fVarying() ) ||
		        aTypes.size() > cArgs )
		{
			aRes.erase( aRes.begin() + i );
			i--;
			continue;
		}

		// Now check through to see if the specified arguments can be converted to the required ones.
		bool	fValid = true;
		TqUint j;
		TqInt index;
		for ( j = 0; j < aTypes.size(); j++ )
			if ( FindCast( aArgTypes[ j ], &aTypes[ j ], 1, index ) == Type_Nil )
				fValid = false;

		// If we have found a match, return it.
		if ( !fValid )
		{
			aRes.erase( aRes.begin() + i );
			i--;
		}
	}
}


///---------------------------------------------------------------------
/// CqParseNodeFunctionCall::ArgCast
/// Convert the arguments passed to this function into valid arguments for the specified function declaration.

void CqParseNodeFunctionCall::ArgCast( TqInt iIndex )
{
	// NOTE: It is presumed that CheckArgCast has been called prior to this to
	// ensure that the argument list can validly be cast the the required parameters.

	CqFuncDef * pFuncDef = CqFuncDef::GetFunctionPtr( m_aFuncRef[ iIndex ] );
	if ( pFuncDef == 0 )
		return ;

	// Build a list of variable types from its parameter type string.
	std::vector<TqInt>& aTypes = pFuncDef->aTypeSpec();

	// Now convert the arguments to the correct types
	CqParseNode* pArg = m_pChild;
	TqUint j = 0;
	while ( pArg != 0 && j < aTypes.size() )
	{
		// Must get the next here incase the check inserts a cast.
		CqParseNode * pNext = pArg->pNext();
		bool needsCast;
		pArg->TypeCheck( &aTypes[ j++ ], 1, needsCast, false );
		pArg = pNext;
	}
}


///---------------------------------------------------------------------
/// CqParseNodeUnresolvedCall::TypeCheck
/// This is rather awkward, we will convert to whichever type we are asked
/// to be, then later when the shader vm parses the .slx we can try and build
/// a cast then, the same goes for the arguments.

TqInt	CqParseNodeUnresolvedCall::TypeCheck( TqInt* pTypes, TqInt Count,  bool& needsCast, bool CheckOnly )
{
	TqInt NewType = Type_Nil;

	// TypeCheck the args, we don't know what types will be needed
	// later, so there is little else we can do. While doing so, rebuild the params string for the
	// external declaration in the .slx
	CqString strArgTypes("");
	CqParseNode *pArg = m_pChild;
	while (pArg != NULL)
	{
		CqParseNode *pNext = pArg->pNext();
		pArg->TypeCheck( pAllTypes(), Type_Last - 1, needsCast, false );
		strArgTypes+=CqParseNode::TypeIdentifier(pArg->ResType());
		pArg = pNext;
	};

	// Store the newly decided parameter list string. This now reflects the actual parameter types
	// passed in.
	m_aFuncDef.SetstrParams(strArgTypes);

	// If we have no type set yet we take the first type given as an option
	if(m_aFuncDef.Type() == Type_Nil || !CheckOnly)
	{
		// Is Void acceptable, if so we prefer it.
		int i;
		for( i=0; i < Count; i++)
		{
			if( pTypes[i] == Type_Void)
				NewType = Type_Void;
		};
		// Otherwise we fall back to the first option given
		if( NewType == Type_Nil )
			NewType = pTypes[0];

		m_aFuncDef = CqFuncDef( NewType,
		                        m_aFuncDef.strName(),
		                        "unresolved",
		                        m_aFuncDef.strParams(),
		                        m_aFuncDef.pDefNode(),
		                        m_aFuncDef.pArgs());
	};

	return m_aFuncDef.Type();
}


///---------------------------------------------------------------------
/// CqParseNodeVariable::TypeCheck
/// Do a type check based on suitable types requested, and add a cast in required.

TqInt	CqParseNodeVariable::TypeCheck( TqInt* pTypes, TqInt Count,  bool& needsCast, bool CheckOnly )
{
	// Check if the variable type matches any of the requested ones.
	TqInt	MyType = ( ResType() & Type_Mask );
	TqInt i;
	for ( i = 0; i < Count; i++ )
	{
		if ( pTypes[ i ] == MyType )
			return ( MyType );
	}
	// If we got here, we are not of the correct type, so find a suitable cast.
	TqInt index;
	TqInt NewType = FindCast( MyType, pTypes, Count, index );
	needsCast = true;
	if(!CheckOnly)
	{
		CqParseNodeCast* pCast = new CqParseNodeCast( NewType );
		LinkParent( pCast );
	}

	if ( NewType == Type_Nil && !CheckOnly )
	{
		AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
			strFileName() << " : " << LineNo() << " : Cannot convert from type "
			<< CqParseNode::TypeName( MyType ) << " to any of the required types");
	}
	return ( NewType );
}


///---------------------------------------------------------------------
/// CqParseNodeVariableArray::TypeCheck
/// Do a type check based on suitable types requested, and add a cast in required.

TqInt	CqParseNodeVariableArray::TypeCheck( TqInt* pTypes, TqInt Count,  bool& needsCast, bool CheckOnly )
{
	// Check if the child can be made into a float.
	TqInt aType = Type_Float;
	bool temp;
	if ( m_pChild && m_pChild->TypeCheck( &aType, 1, temp, CheckOnly ) == Type_Nil )
	{
		TqInt IndexType = ( m_pChild->ResType() & Type_Mask );
		AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
			strFileName() << " : " << LineNo() << " : Array index must be float type "
			<< ": " << CqParseNode::TypeName( IndexType ));
	}

	// Check if the variable is an array.
	CqVarDef* pVar=CqVarDef::GetVariablePtr(m_VarRef);
	// Check if the declaration marked it as an arry
	if(!(pVar->Type()&Type_Array))
	{
		TqInt myType = ( ResType() & Type_Mask );
		AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
			strFileName() << " : " <<  LineNo() << " : Attempt to access array member of "
			<< "non-array type : " << CqParseNode::TypeName( myType ));
	}

	return ( CqParseNodeVariable::TypeCheck( pTypes, Count, needsCast, CheckOnly ) );
}


///---------------------------------------------------------------------
/// CqParseNodeAssign::TypeCheck
/// Do a type check based on suitable types requested, and add a cast in required.

extern EqShaderType gShaderType;

TqInt	CqParseNodeAssign::TypeCheck( TqInt* pTypes, TqInt Count,  bool& needsCast, bool CheckOnly )
{
	// First check if the assign is to a read only variable.
	if( CqVarDef::GetVariablePtr( m_VarRef ) &&
	        pShaderNode() &&
	        CqVarDef::GetVariablePtr( m_VarRef )->ReadOnly( pShaderNode()->ShaderType() ) )
	{
		AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
			strFileName() << " : " << LineNo() << " : Cannot access read only variable '"
			<< CqVarDef::GetVariablePtr( m_VarRef )->strName() << "' in shader type '"
			<< gShaderTypeNames[ pShaderNode()->ShaderType() ] << "'");
	}

	// First check if we are assigning a varying value to a uniform variable.
	bool assigneeIsVarying = false;
	IqVarDef* pVarDef = CqVarDef::GetVariablePtr( m_VarRef );
	if ( pVarDef != 0 )
		assigneeIsVarying = ( pVarDef->Type() & Type_Varying ) != 0;
	if(!assigneeIsVarying && m_fVarying)
	{
		AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
			strFileName() << " : " << LineNo() << " : Cannot assign a varying value to the "
			<< "uniform variable '" << CqVarDef::GetVariablePtr( m_VarRef )->strName() << "'");
	}
	
	TqInt	MyType = ( ResType() & Type_Mask );
	// Type check the assignment expression first.
	CqParseNode* pExpr = m_pChild;
	if ( pExpr->TypeCheck( &MyType, 1, needsCast, CheckOnly ) != MyType )
		return ( Type_Nil );	// TODO: Should throw an exception here.

	// Check if the variable type matches any of the requested ones.
	TqInt i;
	for ( i = 0; i < Count; i++ )
	{
		if ( pTypes[ i ] == MyType )
			return ( MyType );
	}
	// If we got here, we are not of the correct type, so find a suitable cast.
	TqInt index;
	TqInt NewType = FindCast( MyType, pTypes, Count, index );
	needsCast = true;
	if(!CheckOnly)
	{
		CqParseNodeCast* pCast = new CqParseNodeCast( NewType );
		LinkParent( pCast );
	}

	if ( NewType == Type_Nil && !CheckOnly )
	{
		AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
			strFileName() << " : " << LineNo() << " : Cannot convert from type "
			<< CqParseNode::TypeName( MyType ) << " to any of the required types");
	}
	return ( NewType );
}


///---------------------------------------------------------------------
/// CqParseNodeAssignArray::TypeCheck
/// Do a type check based on suitable types requested, and add a cast in required.

TqInt	CqParseNodeAssignArray::TypeCheck( TqInt* pTypes, TqInt Count,  bool& needsCast, bool CheckOnly )
{
	// Check if the child can be made into a float.
	TqInt aType = Type_Float;
	bool temp;
	if ( m_pChild->pNext() ->TypeCheck( &aType, 1, temp, CheckOnly ) == Type_Nil )
	{
		TqInt	IndexType = ( m_pChild->pNext() ->ResType() & Type_Mask );
		AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
			strFileName() << " : " << LineNo()<< " : Array index must be float type : "
			<< CqParseNode::TypeName( IndexType ));
	}

	return ( CqParseNodeAssign::TypeCheck( pTypes, Count, needsCast, CheckOnly ) );
}


///---------------------------------------------------------------------
/// CqParseNodeOp::TypeCheck
/// Do a type check based on suitable types requested, and add a cast in required.

TqInt	CqParseNodeOp::TypeCheck( TqInt* pTypes, TqInt Count,  bool& needsCast, bool CheckOnly )
{
	// Check if both arguments to the operator match type.
	CqParseNode * pOperandA = m_pChild;
	CqParseNode* pOperandB = m_pChild->pNext();
	assert( pOperandA != 0 && pOperandB != 0 );
	TqInt TypeA = ( pOperandA->ResType() & Type_Mask );
	TqInt TypeB = ( pOperandB->ResType() & Type_Mask );

	// See if they can both be cast to a requested type.
	TqInt i;
	for ( i = 0; i < Count; i++ )
	{
		TqInt index;
		if ( FindCast( TypeA, &pTypes[ i ], 1, index ) != Type_Nil )
		{
			if ( FindCast( TypeB, &pTypes[ i ], 1, index ) != Type_Nil )
			{
				// Add any cast operators required.
				if(!CheckOnly)
				{
					pOperandA->TypeCheck( &pTypes[ i ], 1, needsCast, CheckOnly );
					pOperandB->TypeCheck( &pTypes[ i ], 1, needsCast, CheckOnly );
				}
				return ( pTypes[ i ] );
			}
		}
	}

	if ( !CheckOnly )
	{
		AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
			strFileName() << " : " << LineNo() << " : Cannot find a suitable cast for the "
			<< "operands.");
	}

	return ( Type_Nil );
}


///---------------------------------------------------------------------
/// CqParseNodeRelOp::TypeCheck
/// Do a type check based on suitable types requested, and add a cast in required.

TqInt	CqParseNodeRelOp::TypeCheck( TqInt* pTypes, TqInt Count,  bool& needsCast, bool CheckOnly )
{
	TqInt RelType;
	RelType = CqParseNodeOp::TypeCheck( pAllTypes(), Type_Last - 1, needsCast, CheckOnly );

	if( RelType == Type_Nil)
		return( RelType );

	// See if float is a requested type.
	TqInt NewType, index;
	if ( ( NewType = FindCast( Type_Float, pTypes, Count, index ) ) != Type_Nil )
	{
		if ( NewType == Type_Float )
			return ( Type_Float );
		else
		{
			needsCast = true;
			if(!CheckOnly)
			{
				CqParseNodeCast* pCast = new CqParseNodeCast( NewType );
				LinkParent( pCast );
			}
			return ( NewType );
		}
	}

	if ( !CheckOnly )
	{
		AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
			strFileName() << " : " << LineNo() << " : Relational can operators only return "
			<< "float.");
	}

	return ( Type_Nil );
}


///---------------------------------------------------------------------
/// CqParseNodeUnaryOp::TypeCheck
/// Do a type check based on suitable types requested, and add a cast in required.

TqInt	CqParseNodeUnaryOp::TypeCheck( TqInt* pTypes, TqInt Count,  bool& needsCast, bool CheckOnly )
{
	// Check if the operand can be cast to a requested type.
	if ( m_pChild != 0 )
		return ( m_pChild->TypeCheck( pTypes, Count, needsCast, CheckOnly ) );
	else
		return ( Type_Nil );
}


///---------------------------------------------------------------------
/// CqParseNodeOpDot::TypeCheck
/// Do a type check based on suitable types requested, and add a cast in required.

TqInt	CqParseNodeMathOpDot::TypeCheck( TqInt* pTypes, TqInt Count,  bool& needsCast, bool CheckOnly )
{
	static TqInt aArgTypes[ 3 ] = {Type_Point, Type_Vector, Type_Normal};

	// Get the argument types.
	CqParseNode* pOperandA = m_pChild;
	CqParseNode* pOperandB = m_pChild->pNext();
	assert( pOperandA != 0 && pOperandB != 0 );
	TqInt TypeA = ( pOperandA->ResType() & Type_Mask );
	TqInt TypeB = ( pOperandB->ResType() & Type_Mask );

	// Dot operator can only take normal/vector/point types, check the
	// arguments can be made to match this, and always returns a float,
	// check this is valid.

	TqInt RetType, index;
	if ( ( RetType = FindCast( Type_Float, pTypes, Count, index ) ) != Type_Nil )
	{
		bool fValid = false;
		TqUint i;
		for ( i = 0; i < sizeof( aArgTypes ) / sizeof( aArgTypes[ 0 ] ); i++ )
		{
			if ( FindCast( TypeA, &aArgTypes[ i ], 1, index ) != Type_Nil )
			{
				if ( FindCast( TypeB, &aArgTypes[ i ], 1, index ) != Type_Nil )
				{
					// Add any cast operators required.
					if(!CheckOnly)
					{
						pOperandA->TypeCheck( &aArgTypes[ i ], 1, needsCast, CheckOnly );
						pOperandB->TypeCheck( &aArgTypes[ i ], 1, needsCast, CheckOnly );
					}
					fValid = true;
					break;
				}
			}
		}
		if ( fValid )
		{
			if ( RetType != Type_Float)
			{
				needsCast = true;
				if(!CheckOnly)
				{
					CqParseNodeCast * pCast = new CqParseNodeCast( RetType );
					LinkParent( pCast );
				}
			}
			return ( RetType );
		}
	}

	if ( !CheckOnly )
	{
		AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
			strFileName() << " : " << LineNo() << " : Cannot find a suitable cast for the "
			<< "operands.");
	}

	return ( Type_Nil );
}


///---------------------------------------------------------------------
/// CqParseNodeConst::TypeCheck
/// Perform a typecheck, and add cast if required.

TqInt CqParseNodeConst::TypeCheck( TqInt* pTypes, TqInt Count,  bool& needsCast, bool CheckOnly )
{
	// Check if the variable type matches any of the requested ones.
	TqInt	MyType = ResType();
	TqInt i;
	for ( i = 0; i < Count; i++ )
	{
		if ( pTypes[ i ] == MyType )
			return ( MyType );
	}

	// If we got here, we are not of the correct type, so find a suitable cast.
	TqInt index;
	TqInt NewType = FindCast( MyType, pTypes, Count, index );
	needsCast = true;
	if ( !CheckOnly )
	{
		CqParseNodeCast * pCast = new CqParseNodeCast( NewType );
		LinkParent( pCast );
	}

	if ( NewType == Type_Nil && !CheckOnly )
	{
		AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
			strFileName() << " : " << LineNo() << " : Cannot convert from type "
			<< CqParseNode::TypeName( MyType ) << " to any of the required types");
	}
	return ( NewType );
}


///---------------------------------------------------------------------
/// CqParseNodeCast::TypeCheck
/// Do a type check based on suitable types requested, and add a cast in required.

TqInt	CqParseNodeCast::TypeCheck( TqInt* pTypes, TqInt Count,  bool& needsCast, bool CheckOnly )
{
	// Perform a typecheck on the cast expression
	CqParseNode * pExpr = m_pChild;
	pExpr->TypeCheck( &m_tTo, 1, needsCast, CheckOnly );

	// Check the return type, and add another cast if necessary.
	// Note: We add another cast to allow for forced casts of function return types,
	// i.e. we want the sl code "Ci=float noise(P)"
	// to call float noise THEN cast to a color, not call color noise.
	TqInt i;
	for ( i = 0; i < Count; i++ )
		if ( pTypes[ i ] == m_tTo )
			return ( m_tTo );

	TqInt index;
	TqInt NewType = FindCast( m_tTo, pTypes, Count, index );
	if ( NewType == Type_Nil && !CheckOnly )
	{
		AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
			strFileName() << " : " << LineNo() << " : Cannot convert from type "
			<< CqParseNode::TypeName( NewType ) << " to any of the required types");
	}
	else
	{
		needsCast = true;
		if ( !CheckOnly )
		{
			CqParseNodeCast * pCast = new CqParseNodeCast( NewType );
			LinkParent( pCast );
		}
	}

	return ( NewType );
}


///---------------------------------------------------------------------
/// CqParseNodeTriple::TypeCheck
/// Do a type check based on suitable types requested, and add a cast in required.

TqInt	CqParseNodeTriple::TypeCheck( TqInt* pTypes, TqInt Count,  bool& needsCast, bool CheckOnly )
{
	static TqInt ExprType = Type_Float;
	// Perform a typecheck on the three float expressions
	CqParseNode* pExpr = m_pChild;
	while ( pExpr != 0 )
	{
		CqParseNode* pNextExpr = pExpr->pNext();
		pExpr->TypeCheck( &ExprType, 1, needsCast, CheckOnly );
		pExpr = pNextExpr;
	}
	// Check if expecting a triple, if not add a cast
	TqInt i;
	for ( i = 0; i < Count; i++ )
		if ( pTypes[ i ] == Type_Triple )
			return ( Type_Triple );

	// If we got here, we are not of the correct type, so find a suitable cast.
	needsCast = true;
	TqInt index;
	TqInt NewType = FindCast( Type_Triple, pTypes, Count, index );
	if(!CheckOnly)
	{
		CqParseNodeCast* pCast = new CqParseNodeCast( NewType );
		LinkParent( pCast );
	}

	if ( NewType == Type_Nil && !CheckOnly )
	{
		AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
			strFileName() << " : " << LineNo() << " : Cannot convert from type "
			<< CqParseNode::TypeName( NewType ) << " to any of the required types");
	}
	return ( NewType );
}


///---------------------------------------------------------------------
/// CqParseNodeHexTuple::TypeCheck
/// Do a type check based on suitable types requested, and add a cast in required.

TqInt	CqParseNodeHexTuple::TypeCheck( TqInt* pTypes, TqInt Count,  bool& needsCast, bool CheckOnly )
{
	static TqInt ExprType = Type_Float;
	// Perform a typecheck on the three float expressions
	CqParseNode* pExpr = m_pChild;
	while ( pExpr != 0 )
	{
		// Have to get this here, in case the type check inserts a cast above the current expression.
		CqParseNode* pnextChild = pExpr->pNext();
		pExpr->TypeCheck( &ExprType, 1, needsCast, CheckOnly );
		pExpr = pnextChild;
	}
	// Check if expecting a sixteentuple, if not add a cast
	TqInt i;
	for ( i = 0; i < Count; i++ )
		if ( pTypes[ i ] == Type_HexTuple )
			return ( Type_HexTuple );

	// If we got here, we are not of the correct type, so find a suitable cast.
	needsCast = true;
	TqInt index;
	TqInt NewType = FindCast( Type_Matrix, pTypes, Count, index );
	if(!CheckOnly)
	{
		CqParseNodeCast* pCast = new CqParseNodeCast( NewType );
		LinkParent( pCast );
	}

	if ( NewType == Type_Nil && !CheckOnly )
	{
		AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
			strFileName() << " : " << LineNo() << " : Cannot convert from type "
			<< CqParseNode::TypeName( NewType ) << " to any of the required types");
	}
	return ( NewType );
}


///---------------------------------------------------------------------
/// CqParseNodeCommFunction::TypeCheck

TqInt CqParseNodeCommFunction::TypeCheck( TqInt* pTypes, TqInt Count,  bool& needsCast, bool CheckOnly )
{
	// Check if the variable type matches any of the requested ones.
	TqInt	MyType = ResType();
	TqInt i;
	for ( i = 0; i < Count; i++ )
	{
		if ( pTypes[ i ] == MyType )
			return ( MyType );
	}
	// If we got here, we are not of the correct type, so find a suitable cast.
	needsCast = true;
	TqInt index;
	TqInt NewType = FindCast( MyType, pTypes, Count, index );
	if(!CheckOnly)
	{
		CqParseNodeCast* pCast = new CqParseNodeCast( NewType );
		LinkParent( pCast );
	}

	if ( NewType == Type_Nil && !CheckOnly )
	{
		AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
			strFileName() << " : " << LineNo() << " : Cannot convert from type "
			<< CqParseNode::TypeName( MyType ) << " to any of the required types");
	}
	return ( NewType );
}


///---------------------------------------------------------------------
/// CqParseNodeQCond::TypeCheck
/// Type check the return from a conditional statement and cast the true/false expressions if necessary.

TqInt	CqParseNodeQCond::TypeCheck( TqInt* pTypes, TqInt Count,  bool& needsCast, bool CheckOnly )
{
	// Ensure that the conditional expression is type checked.
	CqParseNode * pCondExp = m_pChild;
	assert( pCondExp != 0 );
	pCondExp->TypeCheck( pAllTypes(), Type_Last - 1, needsCast, CheckOnly );

	// Check if both expressions match type.
	CqParseNode * pTrue = m_pChild->pNext();
	assert( pTrue != 0 );
	CqParseNode* pFalse = pTrue->pNext();
	assert( pFalse != 0 );

	TqInt TypeT = ( pTrue->ResType() & Type_Mask );
	TqInt TypeF = ( pFalse->ResType() & Type_Mask );

	// See if they can both be cast to a requested type.
	TqInt i;
	for ( i = 0; i < Count; i++ )
	{
		TqInt index;
		if ( FindCast( TypeT, &pTypes[ i ], 1, index ) != Type_Nil )
		{
			if ( FindCast( TypeF, &pTypes[ i ], 1, index ) != Type_Nil )
			{
				// Add any cast operators required.
				if(!CheckOnly)
				{
					pTrue->TypeCheck( &pTypes[ i ], 1, needsCast, CheckOnly );
					pFalse->TypeCheck( &pTypes[ i ], 1, needsCast, CheckOnly );
				}
				return ( pTypes[ i ] );
			}
		}
	}

	if ( !CheckOnly )
	{
		AQSIS_THROW_XQERROR(XqParseError, EqE_Syntax,
			strFileName() << " : " << LineNo() << " : Cannot find a suitable cast for the "
			<< "expressions.");
	}

	return ( Type_Nil );
}


} // namespace Aqsis
//---------------------------------------------------------------------
