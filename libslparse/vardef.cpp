////---------------------------------------------------------------------
////    Class definition file:  VARDEF.CPP
////    Associated header file: VARDEF.H
////
////    Author:					Paul C. Gregory
////    Creation date:			22/07/99
////---------------------------------------------------------------------

#include	"aqsis.h"
#include	"parsenode.h"
#include	"vardef.h"

START_NAMESPACE( Aqsis )

///---------------------------------------------------------------------
/// Global array of standard variable definitions

CqVarDef	gStandardVars[] = {
                               CqVarDef( Type_VaryingColor, "Cs" ),
                               CqVarDef( Type_VaryingColor, "Os" ),
                               CqVarDef( Type_VaryingPoint, "Ng" ),
                               CqVarDef( Type_UniformFloat, "du" ),
                               CqVarDef( Type_UniformFloat, "dv" ),
                               CqVarDef( Type_VaryingPoint, "L" ),
                               CqVarDef( Type_VaryingColor, "Cl" ),
                               CqVarDef( Type_VaryingColor, "Ol" ),
                               CqVarDef( Type_VaryingPoint, "P" ),
                               CqVarDef( Type_VaryingPoint, "dPdu" ),
                               CqVarDef( Type_VaryingPoint, "dPdv" ),
                               CqVarDef( Type_VaryingPoint, "N" ),
                               CqVarDef( Type_VaryingFloat, "u" ),
                               CqVarDef( Type_VaryingFloat, "v" ),
                               CqVarDef( Type_VaryingFloat, "s" ),
                               CqVarDef( Type_VaryingFloat, "t" ),
                               CqVarDef( Type_VaryingPoint, "I" ),
                               CqVarDef( Type_VaryingColor, "Ci" ),
                               CqVarDef( Type_VaryingColor, "Oi" ),
                               CqVarDef( Type_VaryingPoint, "Ps" ),
                               CqVarDef( Type_UniformPoint, "E" ),
                               CqVarDef( Type_UniformFloat, "ncomps" ),
                               CqVarDef( Type_UniformFloat, "time" ),
                               CqVarDef( Type_UniformFloat, "alpha" ),
                           };
TqInt	gcStandardVars = sizeof( gStandardVars ) / sizeof( gStandardVars[ 0 ] );

///---------------------------------------------------------------------
/// Global array of standard variable definitions

std::vector<CqVarDef>	gLocalVars;

///---------------------------------------------------------------------
/// CqVarDef::CqVarDef
/// Copy constructor.

CqVarDef::CqVarDef( const CqVarDef& from )
{
	*this = from;
}


///---------------------------------------------------------------------
/// CqVarDef::~CqVarDef
/// Destructor.

CqVarDef::~CqVarDef()
{
	delete( m_pDefValue );
}

///---------------------------------------------------------------------
/// CqVarDef::operator=
/// Assignment operator.

CqVarDef& CqVarDef::operator=( const CqVarDef& from )
{
	m_Type = from.m_Type;
	m_strName = from.m_strName;
	m_UseCount = 0;
	m_fExtern = from.m_fExtern;
	m_vrExtern = from.m_vrExtern;
	m_ArrayLength = from.m_ArrayLength;
	if ( from.m_pDefValue )
		m_pDefValue = from.m_pDefValue->Clone( 0 );
	else
		m_pDefValue = 0;

	return ( *this );
}


const IqParseNode* CqVarDef::pInitialiser() const
{
	return ( m_pDefValue );
}


///---------------------------------------------------------------------
/// CqVarDef::FindFunction
/// Find a variable definition by searching the standard definitions list.

TqBool CqVarDef::FindVariable( const char* strName, SqVarRef& Ref )
{
	// Search the local definitions next.
	TqUint i;
	for ( i = 0; i < gLocalVars.size(); i++ )
	{
		if ( gLocalVars[ i ].m_strName == strName )
		{
			Ref.m_Type = VarTypeLocal;
			Ref.m_Index = i;
			return ( TqTrue );
		}
	}

	// Search the standard definitions first.
	for ( i = 0; i < gcStandardVars; i++ )
	{
		if ( gStandardVars[ i ].m_strName == strName )
		{
			Ref.m_Type = VarTypeStandard;
			Ref.m_Index = i;
			return ( TqTrue );
		}
	}

	return ( TqFalse );
}


///---------------------------------------------------------------------
/// CqVarDef::GetVariablePointer
/// Return a temporary pointer to a variable definition..

TqInt CqVarDef::AddVariable( CqVarDef& Def )
{
	gLocalVars.push_back( Def );
	return ( gLocalVars.size() - 1 );
}


///---------------------------------------------------------------------
/// CqVarDef::GetVariablePointer
/// Return a temporary pointer to a variable definition..

CqVarDef* CqVarDef::GetVariablePtr( const SqVarRef& Ref )
{
	// Look up the variable reference according to its type.
	if ( Ref.m_Type == VarTypeStandard && Ref.m_Index < gcStandardVars )
		return ( &gStandardVars[ Ref.m_Index ] );

	if ( Ref.m_Type == VarTypeLocal && Ref.m_Index < gLocalVars.size() )
	{
		if ( gLocalVars[ Ref.m_Index ].fExtern() )
			return ( GetVariablePtr( gLocalVars[ Ref.m_Index ].vrExtern() ) );
		else
			return ( &gLocalVars[ Ref.m_Index ] );
	}
	return ( 0 );
}


///---------------------------------------------------------------------
/// CqVarDef::GetVariablePointer
/// Return a temporary pointer to a variable definition..

IqVarDef* IqVarDef::GetVariablePtr( const SqVarRef& Ref )
{
	return ( CqVarDef::GetVariablePtr( Ref ) );
}


END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------
