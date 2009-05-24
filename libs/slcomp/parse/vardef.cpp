////---------------------------------------------------------------------
////    Class definition file:  VARDEF.CPP
////    Associated header file: VARDEF.H
////
////    Author:					Paul C. Gregory
////    Creation date:			22/07/99
////---------------------------------------------------------------------

#include	<aqsis/aqsis.h>
#include	"parsenode.h"
#include	"vardef.h"

namespace Aqsis {

///---------------------------------------------------------------------
/// Global array of standard variable definitions

#define SU	(1<<Type_Surface)
#define LS	(1<<Type_Lightsource)
#define VO	(1<<Type_Volume)
#define DI	(1<<Type_Displacement)
#define IM	(1<<Type_Imager)
#define TR	(1<<Type_Transformation)


CqVarDef	gStandardVars[] = {
                               CqVarDef( Type_VaryingColor, "Cs", 0,	   LS|VO|DI|IM ),
                               CqVarDef( Type_VaryingColor, "Os", 0,	   LS|VO|DI|IM ),
                               CqVarDef( Type_VaryingPoint, "Ng", 0,	SU|LS|VO|DI|IM ),
                               CqVarDef( Type_UniformFloat, "du", 0,	SU|LS|VO|DI|IM ),
                               CqVarDef( Type_UniformFloat, "dv", 0,	SU|LS|VO|DI|IM ),
                               CqVarDef( Type_VaryingPoint, "L", 0,		SU|   VO|DI|IM ),
                               CqVarDef( Type_VaryingColor, "Cl",0,		SU|   VO|DI|IM ),
                               CqVarDef( Type_VaryingColor, "Ol",0,		SU|   VO|DI|IM ),
                               CqVarDef( Type_VaryingPoint, "P",0,		   LS|VO|   IM ),
                               CqVarDef( Type_VaryingPoint, "dPdu",0,	SU|LS|VO|DI|IM ),
                               CqVarDef( Type_VaryingPoint, "dPdv",0,	SU|LS|VO|DI|IM ),
                               CqVarDef( Type_VaryingPoint, "N",0,		      VO|   IM ),
                               CqVarDef( Type_VaryingFloat, "u",0,		SU|LS|VO|DI|IM ),
                               CqVarDef( Type_VaryingFloat, "v",0,		SU|LS|VO|DI|IM ),
                               CqVarDef( Type_VaryingFloat, "s",0,		      VO|   IM ),
                               CqVarDef( Type_VaryingFloat, "t",0,		      VO|   IM ),
                               CqVarDef( Type_VaryingPoint, "I",0,		SU|LS|VO|DI|IM ),
                               CqVarDef( Type_VaryingColor, "Ci",0,		   LS|   DI    ),
                               CqVarDef( Type_VaryingColor, "Oi",0,		   LS|   DI    ),
                               CqVarDef( Type_VaryingPoint, "Ps",0,		SU|LS|VO|DI|IM ),
                               CqVarDef( Type_UniformPoint, "E",0,		SU|LS|VO|DI|IM ),
                               CqVarDef( Type_UniformFloat, "ncomps",0,	SU|LS|VO|DI|IM ),
                               CqVarDef( Type_UniformFloat, "time",0,	SU|LS|VO|DI|IM ),
                               CqVarDef( Type_UniformFloat, "alpha",0,	SU|LS|VO|DI    ),

                               CqVarDef( Type_VaryingPoint, "Ns",0,		SU|LS|VO|DI|IM ),
                           };
TqUint	gcStandardVars = sizeof( gStandardVars ) / sizeof( gStandardVars[ 0 ] );

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
	m_ReadOnly = from.m_ReadOnly;
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

IqParseNode* CqVarDef::pInitialiser()
{
	return ( m_pDefValue );
}


///---------------------------------------------------------------------
/// CqVarDef::FindFunction
/// Find a variable definition by searching the standard definitions list.

bool CqVarDef::FindVariable( const char* strName, SqVarRef& Ref )
{
	// Search the local definitions next.
	TqUint i;
	TqUlong hash = CqString::hash(strName);

	for ( i = 0; i < gLocalVars.size(); i++ )
	{
		if ( CqString::hash(gLocalVars[ i ].m_strName.c_str()) == hash )
		{
			Ref.m_Type = VarTypeLocal;
			Ref.m_Index = i;
			return ( true );
		}
	}

	// Search the standard definitions first.
	for ( i = 0; i < gcStandardVars; i++ )
	{
		if ( CqString::hash(gStandardVars[ i ].m_strName.c_str()) == hash )
		{
			Ref.m_Type = VarTypeStandard;
			Ref.m_Index = i;
			return ( true );
		}
	}

	return ( false );
}



///---------------------------------------------------------------------
/// CqVarDef::FindStandardVariable
/// Find a variable definition by searching the standard definitions list.

bool CqVarDef::FindStandardVariable( const char* strName, SqVarRef& Ref )
{
	// Search the standard definitions only.
	TqUint i;
	TqUlong hash = CqString::hash(strName);

	for ( i = 0; i < gcStandardVars; i++ )
	{
		if ( CqString::hash(gStandardVars[ i ].m_strName.c_str()) == hash )
		{
			Ref.m_Type = VarTypeStandard;
			Ref.m_Index = i;
			return ( true );
		}
	}

	return ( false );
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


} // namespace Aqsis
//---------------------------------------------------------------------
