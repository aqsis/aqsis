////---------------------------------------------------------------------
////    Class definition file:  TYPECHECK.CPP
////    Associated header file: PARSENODE.H
////
////    Author:					Paul C. Gregory
////    Creation date:			25/11/99
////---------------------------------------------------------------------

#include	"aqsis.h"
#include	"parsenode.h"

START_NAMESPACE( Aqsis )

///---------------------------------------------------------------------
/// CqParseNodeFunctionCall::TypeCheck
/// Do a type check based on suitable types requested, and add a cast if required.
/// Uses all possible functions listed to try to fund a suitable candidate.

TqInt	CqParseNodeFunctionCall::TypeCheck( TqInt* pTypes, TqInt Count, TqBool CheckOnly )
{
    TqInt NewType = Type_Nil;
    TqInt cSpecifiedArgs = 0;

    TqInt* aArgTypes = new TqInt[ m_aFuncRef.size() ];
    std::vector<CqFuncDef*> aFuncs;

    CqString strMyName = strName();

    // Prepare the error string.
    CqString strErr( strFileName() );
    strErr += " : ";
    strErr += CqString( LineNo() );
    strErr += " : ";
    strErr += strMyName;
    strErr += " : ";
    CqString strErrDesc = "Valid function declaration not found : ";

    // First discard any function possibilities which don't have the correct number of arguments.
    // Find out how many arguments have been specified.
    CqParseNode* pArg = m_pChild;
    while ( pArg != 0 )
    {
        cSpecifiedArgs++;
        pArg = pArg->pNext();
    }

    TqUint i;
    for ( i = 0; i < m_aFuncRef.size(); i++ )
    {
        aFuncs.push_back( CqFuncDef::GetFunctionPtr( m_aFuncRef[ i ] ) );
        if ( aFuncs[ i ] != 0 )
        {
            TqInt cArgs = aFuncs[ i ] ->cTypeSpecLength();
            if ( ( cArgs > 0 ) &&
                    ( ( !aFuncs[ i ] ->fVarying() && cArgs != cSpecifiedArgs ) ||
                      ( aFuncs[ i ] ->fVarying() && cSpecifiedArgs < cArgs ) ) )
            {
                m_aFuncRef.erase( m_aFuncRef.begin() + i );
                aFuncs.erase( aFuncs.begin() + i );
                i--;
            }
        }
    }

    // If we have eliminated all possibilities, then we have an error.
    if ( m_aFuncRef.size() == 0 )
    {
        strErr += "Arguments to function not valid : ";
        throw( strErr );
        return ( Type_Nil );
    }

    // Then typecheck the arguments.
    pArg = m_pChild;
    TqUint iArg = 0;
    TqBool fVarLen = TqFalse;
    while ( pArg != 0 )
    {
        CqParseNode * pNext = pArg->pNext();
        // Build an array of possible types.
        TqInt ctypes = 0;
        for ( i = 0; i < m_aFuncRef.size(); i++ )
        {
            if ( aFuncs[ i ] != 0 )
            {
                if ( aFuncs[ i ] ->cTypeSpecLength() <= iArg || ( aFuncs[ i ] ->fVarying() && aFuncs[ i ] ->cTypeSpecLength() == iArg ) )
                {
                    if ( aFuncs[ i ] ->cTypeSpecLength() >= iArg && aFuncs[ i ] ->fVarying() )
                        fVarLen = TqTrue;
                    continue;
                }
                else
                {
                    aArgTypes[ i ] = aFuncs[ i ] ->aTypeSpec() [ iArg ];
                    ctypes++;
                }
            }
        }

        // If we have no valid types to check, it must be a variable length function.
        if ( ctypes == 0 )
        {
            pArg->TypeCheck( pAllTypes(), Type_Last - 1, TqTrue );
            break;
        }
        // Now type check the argument
        TqInt ArgType = pArg->TypeCheck( aArgTypes, ctypes, TqTrue );
        // If no valid function exists for the argument, then this is an error.
        if ( ArgType == Type_Nil && !fVarLen )
        {
            strErr += strErrDesc;
            throw( strErr );
            return ( Type_Nil );
        }
        else if ( ArgType != Type_Nil )
            pArg->TypeCheck( &ArgType, 1 );	// Now do a real typecast

        // Check the list of possibles, and remove any that don't take the returned argument at the current point in the
        // argument list.
        for ( i = 0; i < m_aFuncRef.size(); i++ )
        {
            if ( strlen( aFuncs[ i ] ->strParams() ) <= iArg || aFuncs[ i ] ->strParams() [ iArg ] == '*' )
                continue;

            TqInt type = aFuncs[ i ] ->aTypeSpec() [ iArg ];
            if ( ( type & Type_Mask ) != ( ArgType & Type_Mask ) )
            {
                m_aFuncRef.erase( m_aFuncRef.begin() + i );
                aFuncs.erase( aFuncs.begin() + i );
                i--;
            }
            else
            {
                // If we have an upper case type in the function declaration, then we need an actual variable.
                if ( type & Type_Variable && !pArg->IsVariableRef() )
                {
                    strErrDesc = "Argument ";
                    strErrDesc += static_cast<TqInt>( iArg );
                    strErrDesc += " must be a variable";
                    m_aFuncRef.erase( m_aFuncRef.begin() + i );
                    aFuncs.erase( aFuncs.begin() + i );
                    i--;
                }
            }
        }
        pArg = pNext;
        iArg++;
    }


    // Check if any of the remaining functions return the correct type.
    for ( i = 0; i < m_aFuncRef.size(); i++ )
    {
        if ( aFuncs[ i ] != 0 )
        {
            TqInt j;
            for ( j = 0; j < Count; j++ )
            {
                if ( aFuncs[ i ] ->Type() == pTypes[ j ] )
                {
                    // Set the selected function declaration as top.
                    m_aFuncRef[ 0 ] = m_aFuncRef[ i ];
                    return ( pTypes[ j ] );
                }
            }
        }
    }

    // If we got here, we must cast the return value to a suitable type.
    for ( i = 0; i < m_aFuncRef.size(); i++ )
    {
        if ( aFuncs[ i ] != 0 )
        {
            TqInt j;
            for ( j = 0; j < Count; j++ )
            {
                if ( ( NewType = FindCast( aFuncs[ i ] ->Type(), pTypes, Count ) ) != Type_Nil )
                {
                    // Set the selected function declaration as top.
                    m_aFuncRef[ 0 ] = m_aFuncRef[ i ];
                    // Add a cast to the new type.
                    CqParseNode* pCast = new CqParseNodeCast( NewType );
                    LinkParent( pCast );
                    return ( NewType );
                }
            }
        }
    }

    strErr += strErrDesc;
    throw( strErr );
    return ( Type_Nil );
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
        if ( pFuncDef == 0 ) continue;

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
        TqBool	fValid = TqTrue;
        TqUint j;
        for ( j = 0; j < aTypes.size(); j++ )
            if ( FindCast( aArgTypes[ j ], &aTypes[ j ], 1 ) == Type_Nil ) fValid = TqFalse;

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
    if ( pFuncDef == 0 ) return ;

    // Build a list of variable types from its parameter type string.
    std::vector<TqInt>& aTypes = pFuncDef->aTypeSpec();

    // Now convert the arguments to the correct types
    CqParseNode* pArg = m_pChild;
    TqUint j = 0;
    while ( pArg != 0 && j < aTypes.size() )
    {
        // Must get the next here incase the check inserts a cast.
        CqParseNode * pNext = pArg->pNext();
        pArg->TypeCheck( &aTypes[ j++ ] );
        pArg = pNext;
    }
}


///---------------------------------------------------------------------
/// CqParseNodeUnresolvedCall::TypeCheck
/// This is rather awkward, we will convert to whichever type we are asked
/// to be, then later when the shader vm parses the .slx we can try and build
/// a cast then, the same goes for the arguments.

TqInt	CqParseNodeUnresolvedCall::TypeCheck( TqInt* pTypes, TqInt Count, TqBool CheckOnly )
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
        pArg->TypeCheck( pAllTypes(), Type_Last - 1 );
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

TqInt	CqParseNodeVariable::TypeCheck( TqInt* pTypes, TqInt Count, TqBool CheckOnly )
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
    TqInt NewType = FindCast( MyType, pTypes, Count );
    CqParseNodeCast* pCast = new CqParseNodeCast( NewType );
    LinkParent( pCast );

    if ( NewType == Type_Nil && !CheckOnly )
    {
        CqString strErr( strFileName() );
        strErr += " : ";
        strErr += LineNo();
        strErr += " : ";
        strErr += "Cannot convert from type ";
        strErr += CqParseNode::TypeName( MyType );
        strErr += " to any of the required types";
        throw( strErr );
        return ( Type_Nil );
    }
    return ( NewType );
}


///---------------------------------------------------------------------
/// CqParseNodeVariableArray::TypeCheck
/// Do a type check based on suitable types requested, and add a cast in required.

TqInt	CqParseNodeVariableArray::TypeCheck( TqInt* pTypes, TqInt Count, TqBool CheckOnly )
{
    // Check if the child can be made into a float.
    TqInt aType = Type_Float;
    if ( m_pChild && m_pChild->TypeCheck( &aType, 1, CheckOnly ) == Type_Nil )
    {
        TqInt	IndexType = ( m_pChild->ResType() & Type_Mask );
        CqString strErr( strFileName() );
        strErr += " : ";
        strErr += LineNo();
        strErr += " : ";
        strErr += "Array index must be float type : ";
        strErr += CqParseNode::TypeName( IndexType );
        throw( strErr );
        return ( Type_Nil );
    }

	// Check if the variable is an array.
	CqVarDef* pVar=CqVarDef::GetVariablePtr(m_VarRef);
	// Check if the declaration marked it as an arry
	if(!(pVar->Type()&Type_Array))
	{
        TqInt	myType = ( ResType() & Type_Mask );
        CqString strErr( strFileName() );
        strErr += " : ";
        strErr += LineNo();
        strErr += " : ";
        strErr += "Attempt to access array member of non-array type : ";
        strErr += CqParseNode::TypeName( myType );
        throw( strErr );
        return ( Type_Nil );
	}

    return ( CqParseNodeVariable::TypeCheck( pTypes, Count, CheckOnly ) );
}


///---------------------------------------------------------------------
/// CqParseNodeAssign::TypeCheck
/// Do a type check based on suitable types requested, and add a cast in required.

extern EqShaderType gShaderType;

TqInt	CqParseNodeAssign::TypeCheck( TqInt* pTypes, TqInt Count, TqBool CheckOnly )
{
    // First check if the assign is to a read only variable.
    if( CqVarDef::GetVariablePtr( m_VarRef ) &&
            pShaderNode() &&
            CqVarDef::GetVariablePtr( m_VarRef )->ReadOnly( pShaderNode()->ShaderType() ) )
    {
        CqString strErr( strFileName() );
        strErr += " : ";
        strErr += LineNo();
        strErr += " : ";
        strErr += "Cannot access read only variable '";
        strErr += CqVarDef::GetVariablePtr( m_VarRef )->strName();
        strErr += "' in shader type '";
        strErr += gShaderTypeNames[ pShaderNode()->ShaderType() ];
        strErr += "'";
        throw( strErr );
        return ( Type_Nil );
    }

    TqInt	MyType = ( ResType() & Type_Mask );
    // Type check the assignment expression first.
    CqParseNode* pExpr = m_pChild;
    if ( pExpr->TypeCheck( &MyType, 1, CheckOnly ) != MyType )
        return ( Type_Nil );	// TODO: Should throw an exception here.

    // Check if the variable type matches any of the requested ones.
    TqInt i;
    for ( i = 0; i < Count; i++ )
    {
        if ( pTypes[ i ] == MyType )
            return ( MyType );
    }
    // If we got here, we are not of the correct type, so find a suitable cast.
    TqInt NewType = FindCast( MyType, pTypes, Count );
    CqParseNodeCast* pCast = new CqParseNodeCast( NewType );
    LinkParent( pCast );

    if ( NewType == Type_Nil && !CheckOnly )
    {
        CqString strErr( strFileName() );
        strErr += " : ";
        strErr += LineNo();
        strErr += " : ";
        strErr += "Cannot convert from type ";
        strErr += CqParseNode::TypeName( MyType );
        strErr += " to any of the required types";
        throw( strErr );
        return ( Type_Nil );
    }
    return ( NewType );
}


///---------------------------------------------------------------------
/// CqParseNodeAssignArray::TypeCheck
/// Do a type check based on suitable types requested, and add a cast in required.

TqInt	CqParseNodeAssignArray::TypeCheck( TqInt* pTypes, TqInt Count, TqBool CheckOnly )
{
    // Check if the child can be made into a float.
    TqInt aType = Type_Float;
    if ( m_pChild->pNext() ->TypeCheck( &aType, 1, CheckOnly ) == Type_Nil )
    {
        TqInt	IndexType = ( m_pChild->pNext() ->ResType() & Type_Mask );
        CqString strErr( strFileName() );
        strErr += " : ";
        strErr += LineNo();
        strErr += " : ";
        strErr += "Array index must be float type : ";
        strErr += CqParseNode::TypeName( IndexType );
        throw( strErr );
        return ( Type_Nil );
    }

    return ( CqParseNodeAssign::TypeCheck( pTypes, Count, CheckOnly ) );
}


///---------------------------------------------------------------------
/// CqParseNodeOp::TypeCheck
/// Do a type check based on suitable types requested, and add a cast in required.

TqInt	CqParseNodeOp::TypeCheck( TqInt* pTypes, TqInt Count, TqBool CheckOnly )
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
        if ( FindCast( TypeA, &pTypes[ i ], 1 ) != Type_Nil )
        {
            if ( FindCast( TypeB, &pTypes[ i ], 1 ) != Type_Nil )
            {
                // Add any cast operators required.
                pOperandA->TypeCheck( &pTypes[ i ], 1, CheckOnly );
                pOperandB->TypeCheck( &pTypes[ i ], 1, CheckOnly );
                return ( pTypes[ i ] );
            }
        }
    }

    if ( !CheckOnly )
    {
        CqString strErr( strFileName() );
        strErr += " : ";
        strErr += LineNo();
        strErr += " : ";
        strErr += "Cannot find a suitable cast for the operands.";
        throw( strErr );
        return ( Type_Nil );
    }

    return ( Type_Nil );
}


///---------------------------------------------------------------------
/// CqParseNodeRelOp::TypeCheck
/// Do a type check based on suitable types requested, and add a cast in required.

TqInt	CqParseNodeRelOp::TypeCheck( TqInt* pTypes, TqInt Count, TqBool CheckOnly )
{
    TqInt RelType;
    RelType = CqParseNodeOp::TypeCheck( pAllTypes(), Type_Last - 1, CheckOnly );  

    if( RelType == Type_Nil)
    	return( RelType );

    // See if float is a requested type.
    TqInt NewType;
    if ( ( NewType = FindCast( Type_Float, pTypes, Count ) ) != Type_Nil )
    {
        if ( NewType == Type_Float ) return ( Type_Float );
        else
        {
            CqParseNodeCast* pCast = new CqParseNodeCast( NewType );
            LinkParent( pCast );
            return ( NewType );
        }
    }

    if ( !CheckOnly )
    {
        CqString strErr( strFileName() );
        strErr += " : ";
        strErr += LineNo();
        strErr += " : ";
        strErr += "Relational can operators only return float.";
        throw( strErr );
        return ( Type_Nil );
    }

    return ( Type_Nil );
}


///---------------------------------------------------------------------
/// CqParseNodeUnaryOp::TypeCheck
/// Do a type check based on suitable types requested, and add a cast in required.

TqInt	CqParseNodeUnaryOp::TypeCheck( TqInt* pTypes, TqInt Count, TqBool CheckOnly )
{
    // Check if the operand can be cast to a requested type.
    if ( m_pChild != 0 )
        return ( m_pChild->TypeCheck( pTypes, Count, CheckOnly ) );
    else
        return ( Type_Nil );
}


///---------------------------------------------------------------------
/// CqParseNodeOpDot::TypeCheck
/// Do a type check based on suitable types requested, and add a cast in required.

TqInt	CqParseNodeMathOpDot::TypeCheck( TqInt* pTypes, TqInt Count, TqBool CheckOnly )
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

    TqInt RetType;
    if ( ( RetType = FindCast( Type_Float, pTypes, Count ) ) != Type_Nil )
    {
        TqBool fValid = TqFalse;
        TqUint i;
        for ( i = 0; i < sizeof( aArgTypes ) / sizeof( aArgTypes[ 0 ] ); i++ )
        {
            if ( FindCast( TypeA, &aArgTypes[ i ], 1 ) != Type_Nil )
            {
                if ( FindCast( TypeB, &aArgTypes[ i ], 1 ) != Type_Nil )
                {
                    // Add any cast operators required.
                    pOperandA->TypeCheck( &aArgTypes[ i ], 1, CheckOnly );
                    pOperandB->TypeCheck( &aArgTypes[ i ], 1, CheckOnly );
                    fValid = TqTrue;
                    break;
                }
            }
        }
        if ( fValid )
        {
            if ( RetType != Type_Float )
            {
                CqParseNodeCast * pCast = new CqParseNodeCast( RetType );
                LinkParent( pCast );
            }
            return ( RetType );
        }
    }

    if ( !CheckOnly )
    {
        CqString strErr( strFileName() );
        strErr += " : ";
        strErr += LineNo();
        strErr += " : ";
        strErr += "Cannot find a suitable cast for the operands.";
        throw( strErr );
        return ( Type_Nil );
    }

    return ( Type_Nil );
}


///---------------------------------------------------------------------
/// CqParseNodeConst::TypeCheck
/// Perform a typecheck, and add cast if required.

TqInt CqParseNodeConst::TypeCheck( TqInt* pTypes, TqInt Count, TqBool CheckOnly )
{
    // Check if the variable type matches any of the requested ones.
    TqInt	MyType = ResType();
    TqInt i;
    for ( i = 0; i < Count; i++ )
    {
        if ( pTypes[ i ] == MyType )
            return ( MyType );
    }

    TqInt NewType = FindCast( MyType, pTypes, Count );
    // If we got here, we are not of the correct type, so find a suitable cast.
    if ( !CheckOnly )
    {
        CqParseNodeCast * pCast = new CqParseNodeCast( NewType );
        LinkParent( pCast );
    }

    if ( NewType == Type_Nil && !CheckOnly )
    {
        CqString strErr( strFileName() );
        strErr += " : ";
        strErr += LineNo();
        strErr += " : ";
        strErr += "Cannot convert from type ";
        strErr += CqParseNode::TypeName( MyType );
        strErr += " to any of the required types";
        throw( strErr );
        return ( Type_Nil );
    }
    return ( NewType );
}


///---------------------------------------------------------------------
/// CqParseNodeCast::TypeCheck
/// Do a type check based on suitable types requested, and add a cast in required.

TqInt	CqParseNodeCast::TypeCheck( TqInt* pTypes, TqInt Count, TqBool CheckOnly )
{
    // Perform a typecheck on the cast expression
    CqParseNode * pExpr = m_pChild;
    pExpr->TypeCheck( &m_tTo, 1, CheckOnly );

    // Check the return type, and add another cast if necessary.
    // Note: We add another cast to allow for forced casts of function return types,
    // i.e. we want the sl code "Ci=float noise(P)"
    // to call float noise THEN cast to a color, not call color noise.
    TqInt i;
    for ( i = 0; i < Count; i++ )
        if ( pTypes[ i ] == m_tTo ) return ( m_tTo );

    TqInt NewType = FindCast( m_tTo, pTypes, Count );
    if ( NewType == Type_Nil && !CheckOnly )
    {
        CqString strErr( strFileName() );
        strErr += " : ";
        strErr += LineNo();
        strErr += " : ";
        strErr += "Cannot convert from type ";
        strErr += CqParseNode::TypeName( NewType );
        strErr += " to any of the required types";
        throw( strErr );
        return ( Type_Nil );
    }
    else
    {
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

TqInt	CqParseNodeTriple::TypeCheck( TqInt* pTypes, TqInt Count, TqBool CheckOnly )
{
    static TqInt ExprType = Type_Float;
    // Perform a typecheck on the three float expressions
    CqParseNode* pExpr = m_pChild;
    while ( pExpr != 0 )
    {
        pExpr->TypeCheck( &ExprType, 1, CheckOnly );
        pExpr = pExpr->pNext();
    }
    // Check if expecting a triple, if not add a cast
    TqInt i;
    for ( i = 0; i < Count; i++ )
        if ( pTypes[ i ] == Type_Triple ) return ( Type_Triple );

    // If we got here, we are not of the correct type, so find a suitable cast.
    TqInt NewType = FindCast( Type_Triple, pTypes, Count );
    CqParseNodeCast* pCast = new CqParseNodeCast( NewType );
    LinkParent( pCast );

    if ( NewType == Type_Nil && !CheckOnly )
    {
        CqString strErr( strFileName() );
        strErr += " : ";
        strErr += LineNo();
        strErr += " : ";
        strErr += "Cannot convert from type ";
        strErr += CqParseNode::TypeName( NewType );
        strErr += " to any of the required types";
        throw( strErr );
        return ( Type_Nil );
    }
    return ( NewType );
}


///---------------------------------------------------------------------
/// CqParseNodeHexTuple::TypeCheck
/// Do a type check based on suitable types requested, and add a cast in required.

TqInt	CqParseNodeHexTuple::TypeCheck( TqInt* pTypes, TqInt Count, TqBool CheckOnly )
{
    static TqInt ExprType = Type_Float;
    // Perform a typecheck on the three float expressions
    CqParseNode* pExpr = m_pChild;
    while ( pExpr != 0 )
    {
        pExpr->TypeCheck( &ExprType, 1, CheckOnly );
        pExpr = pExpr->pNext();
    }
    // Check if expecting a sixteentuple, if not add a cast
    TqInt i;
    for ( i = 0; i < Count; i++ )
        if ( pTypes[ i ] == Type_HexTuple ) return ( Type_HexTuple );

    // If we got here, we are not of the correct type, so find a suitable cast.
    TqInt NewType = FindCast( Type_Matrix, pTypes, Count );
    CqParseNodeCast* pCast = new CqParseNodeCast( NewType );
    LinkParent( pCast );

    if ( NewType == Type_Nil && !CheckOnly )
    {
        CqString strErr( strFileName() );
        strErr += " : ";
        strErr += LineNo();
        strErr += " : ";
        strErr += "Cannot convert from type ";
        strErr += CqParseNode::TypeName( NewType );
        strErr += " to any of the required types";
        throw( strErr );
        return ( Type_Nil );
    }
    return ( NewType );
}


///---------------------------------------------------------------------
/// CqParseNodeCommFunction::TypeCheck

TqInt CqParseNodeCommFunction::TypeCheck( TqInt* pTypes, TqInt Count, TqBool CheckOnly )
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
    TqInt NewType = FindCast( MyType, pTypes, Count );
    CqParseNodeCast* pCast = new CqParseNodeCast( NewType );
    LinkParent( pCast );

    if ( NewType == Type_Nil && !CheckOnly )
    {
        CqString strErr( strFileName() );
        strErr += " : ";
        strErr += LineNo();
        strErr += " : ";
        strErr += "Cannot convert from type ";
        strErr += CqParseNode::TypeName( MyType );
        strErr += " to any of the required types";
        throw( strErr );
        return ( Type_Nil );
    }
    return ( NewType );
}


///---------------------------------------------------------------------
/// CqParseNodeQCond::TypeCheck
/// Type check the return from a conditional statement and cast the true/false expressions if necessary.

TqInt	CqParseNodeQCond::TypeCheck( TqInt* pTypes, TqInt Count, TqBool CheckOnly )
{
    // Ensure that the conditional expression is type checked.
    CqParseNode * pCondExp = m_pChild;
    assert( pCondExp != 0 );
    pCondExp->TypeCheck( pAllTypes(), Type_Last - 1, CheckOnly );

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
        if ( FindCast( TypeT, &pTypes[ i ], 1 ) != Type_Nil )
        {
            if ( FindCast( TypeF, &pTypes[ i ], 1 ) != Type_Nil )
            {
                // Add any cast operators required.
                pTrue->TypeCheck( &pTypes[ i ], 1, CheckOnly );
                pFalse->TypeCheck( &pTypes[ i ], 1, CheckOnly );
                return ( pTypes[ i ] );
            }
        }
    }

    if ( !CheckOnly )
    {
        CqString strErr( strFileName() );
        strErr += " : ";
        strErr += LineNo();
        strErr += " : ";
        strErr += "Cannot find a suitable cast for the expressions.";
        throw( strErr );
        return ( Type_Nil );
    }

    return ( Type_Nil );
}


END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------
