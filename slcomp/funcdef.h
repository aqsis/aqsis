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

#include	"specific.h"	// Needed for namespace macros.
#include	"sstring.h"
#include	"shaders.h"

START_NAMESPACE(Aqsis)

///----------------------------------------------------------------------
/// EqFuncType
/// Type of function

enum EqFuncType
{
	FuncTypeStandard=0,
	FuncTypeSpecial,
	FuncTypeLocal,
};



///----------------------------------------------------------------------
/// SqFuncRef
/// Structure storing a function reference.

struct SqFuncRef
{
	EqFuncType	m_Type;
	TqInt		m_Index;
};

///----------------------------------------------------------------------
/// CqFuncDef
/// Class storing information about functions.

class CqParseNode;
class CqFuncDef
{
	public:
					CqFuncDef() :
								m_Type(Type_Nil),
								m_strName(""),
								m_strVMName(""),
								m_strParamTypes(""),
								m_SType(Type_Surface),
								m_fLocal(TqFalse),
								m_pDef(0),
								m_fSpecial(TqFalse),
								m_InternalUses(0),
								m_fVarying(TqFalse)
									{}
					CqFuncDef(EqVariableType Type, const char* strName, 
												   const char* strVMName, 
												   const char* strParams, 
												   TqInt InternalUses=0, 
												   TqBool fSpecial=TqFalse, 
												   EqShaderType SType=Type_Surface) :
								m_Type(Type),
								m_strName(strName),
								m_strVMName(strVMName),
								m_strParamTypes(strParams),
								m_InternalUses(InternalUses),
								m_SType(SType),
								m_fLocal(TqFalse),
								m_pDef(0),
								m_fSpecial(fSpecial),
								m_fVarying(TqFalse)
									{
										// Build the type array.
										TypeArray();
									}
					CqFuncDef(EqVariableType Type, const char* strName, const char* strVMName, const char* strParams, CqParseNode* pDef, CqParseNode* pArgs);
					~CqFuncDef()	{}

		EqVariableType	Type() const	{return(m_Type);}
		EqShaderType	SType() const	{return(m_SType);}
		const char*		strSType() const{return(gShaderTypeNames[SType()]);}
		TqBool		fLocal() const	{return(m_fLocal);}
		void			SetSType(const EqShaderType SType) 
										{m_SType=SType;}
		int				TypeArray();
		TqInt			InternalUsage() const	{return(m_InternalUses);}

		void			Output(std::ostream& Out, CqParseNode* pArguments);

	const char*		strName() const	{return(m_strName.c_str());}
	const char*		strVMName() const {return(m_strVMName.c_str());}
	const char*		strParams() const {return(m_strParamTypes.c_str());}
		CqParseNode*	pArgs()		{return(m_pArgs);}
		CqParseNode*	pDef()		{return(m_pDef);}
	
	static	TqBool	FindFunction(const char* strName, std::vector<SqFuncRef>& Refs);
	static	CqFuncDef*	GetFunctionPtr(const SqFuncRef& Ref);
	static	TqInt		AddFunction(CqFuncDef& Def);

		TqBool		fVarying() const	{return(m_fVarying);}
		TqInt			VariableLength() const
									{
										if(m_fVarying)	return(m_aTypeSpec.size());
										else			return(-1);
									}
		std::vector<EqVariableType>& aTypeSpec()
									{
										return(m_aTypeSpec);
									}
		TqInt			cTypeSpecLength() const	{return(m_aTypeSpec.size());}

	private:
		EqVariableType	m_Type;
		EqShaderType	m_SType;
		CqString		m_strName;
		CqString		m_strVMName;
		CqString		m_strParamTypes;
		TqBool		m_fLocal;
		TqBool		m_fSpecial;
		CqParseNode*	m_pDef;
		CqParseNode*	m_pArgs;
		TqInt			m_InternalUses;
		std::vector<EqVariableType>	m_aTypeSpec;
		TqBool		m_fVarying;
};

extern std::vector<CqFuncDef>	gLocalFuncs;
extern TqInt					gInternalFunctionUsage;

//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif	// !FUNCDEF_H_INCLUDED
