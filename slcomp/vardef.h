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

#include	"specific.h"	// Needed for namespace macros.
#include	"sstring.h"

START_NAMESPACE(Aqsis)

///----------------------------------------------------------------------
/// EqVarType
/// Type of variable

enum EqVarType
{
	VarTypeStandard=0,
	VarTypeLocal,
};



///----------------------------------------------------------------------
/// SqVarRef
/// Structure storing a variable reference.

struct SqVarRef
{
	EqVarType	m_Type;
	TqInt		m_Index;

	TqBool	operator==(const SqVarRef& From) const	{return(From.m_Type==m_Type && From.m_Index==m_Index);}
};


///----------------------------------------------------------------------
/// SqVarRefTranslator
/// Structure storing a variable reference translation.

struct SqVarRefTranslator
{
	SqVarRef	m_From;
	SqVarRef	m_To;
};


///----------------------------------------------------------------------
/// CqVarDef
/// Class storing information about variables.

class CqParseNode;
class CqVarDef
{
	public:
					CqVarDef() :
								m_Type(Type_Nil),
								m_strName(""),
								m_pDefValue(0),
								m_UseCount(0),
								m_fExtern(TqFalse),
								m_ArrayLength(0)
									{}
					CqVarDef(const CqVarDef& from);
					CqVarDef(EqVariableType Type, const char* strName, TqInt Length=0) :
								m_Type(Type),
								m_strName(strName),
								m_pDefValue(0),
								m_UseCount(0),
								m_fExtern(TqFalse),
								m_ArrayLength(Length)
									{}
					~CqVarDef();

		CqVarDef&	operator=(const CqVarDef& from);

		EqVariableType	Type() const {return(m_Type);}
		void		SetType(const EqVariableType Type)		{m_Type=Type;}
		void		SetParam()		{m_Type=(EqVariableType)(m_Type|Type_Param);}
		void		SetDefaultStorage(EqVariableType Storage)		
									{
										// If no storage has been explicitly specified, default to the
										// passed value.
										if((m_Type&Storage_Mask)==0)
											m_Type=(EqVariableType)(m_Type|(Storage&Storage_Mask));
									}
	const char*		strName() const	{return(m_strName.c_str());}
		CqParseNode* pDefValue()	{return(m_pDefValue);}
		void		SetpDefValue(CqParseNode* pDefValue)
									{m_pDefValue=pDefValue;}
		void		IncUseCount()	{m_UseCount++;}
		TqInt		UseCount() const	{return(m_UseCount);}
		void		SetExtern(TqBool f, SqVarRef vrExtern)
									{
										m_fExtern=f;
										m_vrExtern=vrExtern;
									}
		void		OutputInit(std::ostream& Out);
	
	static	TqBool	FindVariable(const char* strName, SqVarRef& Ref);
	static	CqVarDef*	GetVariablePtr(const SqVarRef& Ref);
	static	TqInt		AddVariable(CqVarDef& Def);
	static	CqString	StorageSpec(EqVariableType Type);

	static	void		PushTransTable(std::vector<SqVarRefTranslator>* paTransTable);
	static	std::vector<SqVarRefTranslator>*	PopTransTable();

	private:
		EqVariableType	m_Type;
		TqBool		m_fExtern;
		SqVarRef		m_vrExtern;
		CqString		m_strName;
		CqParseNode*	m_pDefValue;
		TqInt			m_UseCount;
		TqInt			m_ArrayLength;

	static	std::vector<std::vector<SqVarRefTranslator>* >	m_saTransTable;

friend std::ostream& operator<<(std::ostream& Stream, CqVarDef& Var);
};

std::ostream& operator<<(std::ostream& Stream, CqVarDef& Var);

extern std::vector<CqVarDef>	gLocalVars;
extern CqVarDef	gStandardVars[];

//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif	// !VARDEF_H_INCLUDED
