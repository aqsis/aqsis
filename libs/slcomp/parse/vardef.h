////---------------------------------------------------------------------
////    Associated header file: VARDEF.H
////    Class definition file:  VARDEF.CPP
////
////    Author:					Paul C. Gregory
////    Creation date:			22/07/99
////---------------------------------------------------------------------

//? Is .h included already?
#ifndef VARDEF_H_INCLUDED
#define VARDEF_H_INCLUDED 1

#include	<vector>

#include	<aqsis/aqsis.h>

#include	<aqsis/util/sstring.h>
#include	<aqsis/slcomp/ivardef.h>

namespace Aqsis {


struct IqParseNode;


///----------------------------------------------------------------------
/// CqVarDef
/// Class storing information about variables.

class CqParseNode;
class CqVarDef : public IqVarDef
{
	public:
		CqVarDef() :
				m_Type( Type_Nil ),
				m_fExtern( false ),
				m_strName( "" ),
				m_pDefValue( 0 ),
				m_UseCount( 0 ),
				m_ArrayLength( 0 ),
				m_ReadOnly(0)
		{}
		CqVarDef( const CqVarDef& from );
		CqVarDef( TqInt Type, const char* strName, TqInt Length = 0, TqInt ReadOnly = 0 ) :
				m_Type( Type ),
				m_fExtern( false ),
				m_strName( strName ),
				m_pDefValue( 0 ),
				m_UseCount( 0 ),
				m_ArrayLength( Length ),
				m_ReadOnly(ReadOnly)
		{}
		virtual ~CqVarDef();

		// Overridden from IqVarDef
		virtual const IqParseNode*	pInitialiser() const;
		virtual IqParseNode*	pInitialiser();
		virtual	TqInt	Type() const
		{
			return ( m_Type );
		}
		virtual	const char*	strName() const
		{
			return ( m_strName.c_str() );
		}
		virtual	void	IncUseCount()
		{
			m_UseCount++;
		}
		void ResetUseCount()
		{
			m_UseCount = 0;
		}
		virtual	TqInt	UseCount() const
		{
			return ( m_UseCount );
		}
		virtual	TqInt	ArrayLength() const
		{
			return ( m_ArrayLength );
		}
		virtual	bool	fExtern() const
		{
			return ( m_fExtern );
		}
		virtual	SqVarRef	vrExtern() const
		{
			return ( m_vrExtern );
		}
		virtual	void	SetParam( bool fParam = true )
		{
			m_Type = ( m_Type & ~Type_Param ) | ( fParam ? Type_Param : 0 );
		}
		virtual	void	SetOutput( bool fOutput = true )
		{
			m_Type = ( m_Type & ~Type_Output ) | ( fOutput ? Type_Output : 0 );
		}
		virtual	void	SetDefaultStorage( TqInt Storage )
		{
			// If no storage has been explicitly specified, default to the
			// passed value.
			if ( ( m_Type & Storage_Mask ) == 0 )
				m_Type = ( m_Type | ( Storage & Storage_Mask ) );
		}


		CqVarDef&	operator=( const CqVarDef& from );

		void	SetType( const TqInt Type )
		{
			m_Type = Type;
		}
		CqParseNode*	pDefValue()
		{
			return ( m_pDefValue );
		}
		void	SetpDefValue( CqParseNode* pDefValue )
		{
			m_pDefValue = pDefValue;
		}
		void	SetExtern( bool f, SqVarRef vrExtern )
		{
			m_fExtern = f;
			m_vrExtern = vrExtern;
		}
		bool ReadOnly( EqShaderType type )
		{
			return( ( m_ReadOnly & (1<<type) ) != 0 );
		}

		static	bool	FindVariable( const char* strName, SqVarRef& Ref );
		static	bool	FindStandardVariable( const char* strName, SqVarRef& Ref );
		static	CqVarDef*	GetVariablePtr( const SqVarRef& Ref );
		static	TqInt	AddVariable( CqVarDef& Def );

	protected:
		TqInt	m_Type;
		bool	m_fExtern;
		SqVarRef	m_vrExtern;
		CqString	m_strName;
		CqParseNode*	m_pDefValue;
		TqInt	m_UseCount;
		TqInt	m_ArrayLength;
		TqInt	m_ReadOnly;
};

extern std::vector<CqVarDef>	gLocalVars;
extern CqVarDef	gStandardVars[];

//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	// !VARDEF_H_INCLUDED
