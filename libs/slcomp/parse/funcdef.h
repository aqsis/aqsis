////---------------------------------------------------------------------
////    Associated header file: FUNCDEF.H
////    Class definition file:  FUNCDEF.CPP
////
////    Author:					Paul C. Gregory
////    Creation date:			22/07/99
////---------------------------------------------------------------------

//? Is .h included already?
#ifndef FUNCDEF_H_INCLUDED
#define FUNCDEF_H_INCLUDED 1

#include	<vector>

#include	<aqsis/aqsis.h>

#include	<aqsis/util/sstring.h>
#include	<aqsis/slcomp/ifuncdef.h>
#include	"parsenode.h"

namespace Aqsis {


///----------------------------------------------------------------------
/** \class CqFuncDef
 * Class storing information about functions.
 */

class CqFuncDef : public IqFuncDef
{
	public:
		CqFuncDef() :
				m_Type( Type_Nil ),
				m_strName( "" ),
				m_strVMName( "" ),
				m_strParamTypes( "" ),
				m_fLocal( false ),
				m_pDef( 0 ),
				m_InternalUses( 0 ),
				m_fVarying( false )
		{}
		CqFuncDef( TqInt Type, const char* strName,
		           const char* strVMName,
		           const char* strParams,
		           TqInt InternalUses = 0,
		           bool fSpecial = false,
		           EqShaderType SType = Type_Surface ) :
				m_Type( Type ),
				m_strName( strName ),
				m_strVMName( strVMName ),
				m_strParamTypes( strParams ),
				m_fLocal( false ),
				m_pDef( 0 ),
				m_InternalUses( InternalUses ),
				m_fVarying( false )
		{
			// Build the type array.
			TypeArray();
		}
		CqFuncDef( TqInt Type, const char* strName, const char* strVMName, const char* strParams, CqParseNode* pDef, CqParseNode* pArgs );
		virtual ~CqFuncDef()
		{}

		// Overridden from IqFuncDef



		virtual	TqInt	Type() const
		{
			return ( m_Type );
		}
		virtual	bool	fLocal() const
		{
			return ( m_fLocal );
		}
		virtual	const char*	strName() const
		{
			return ( m_strName.c_str() );
		}
		virtual	const char*	strVMName() const
		{
			return ( m_strVMName.c_str() );
		}
		virtual	const char*	strParams() const
		{
			return ( m_strParamTypes.c_str() );
		}
		virtual	const IqParseNode* pArgs() const;
		virtual	const IqParseNode* pDef() const;
		virtual	IqParseNode* pDef();
		virtual	bool	fVarying() const
		{
			return ( m_fVarying );
		}
		virtual	TqInt	VariableLength() const
		{
			if ( m_fVarying )
				return ( m_aTypeSpec.size() );
			else
				return ( -1 );
		}
		virtual	TqInt	InternalUsage() const
		{
			return ( m_InternalUses );
		}


		CqParseNode*	pArgs()
		{
			return ( m_pArgs );
		}
		CqParseNode*	pDefNode()
		{
			return ( m_pDef );
		}
		int	TypeArray();

		std::vector<TqInt>& aTypeSpec()
		{
			return ( m_aTypeSpec );
		}
		TqInt	cTypeSpecLength() const
		{
			return ( m_aTypeSpec.size() );
		}
		void SetstrParams(const CqString& strParams)
		{
			m_strParamTypes = strParams;
		}

		static	bool	FindFunction( const char* strName, std::vector<SqFuncRef>& Refs );
		static	CqFuncDef*	GetFunctionPtr( const SqFuncRef& Ref );
		static	TqInt	AddFunction( CqFuncDef& Def );

	private:
		TqInt	m_Type;
		CqString	m_strName;
		CqString	m_strVMName;
		CqString	m_strParamTypes;
		bool	m_fLocal;
		CqParseNode*	m_pDef;
		CqParseNode*	m_pArgs;
		TqInt	m_InternalUses;
		std::vector<TqInt>	m_aTypeSpec;
		bool	m_fVarying;
};

extern std::vector<CqFuncDef>	gLocalFuncs;
extern const char*	gVariableTypeIdentifiers[];
extern TqInt	gcVariableTypeIdentifiers;

//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	// !FUNCDEF_H_INCLUDED
