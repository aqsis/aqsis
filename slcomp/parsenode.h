////---------------------------------------------------------------------
////    Associated header file: PARSENODE.H
////    Class definition file:  PARSENODE.CPP
////
////    Author:					Paul C. Gregory
////    Creation date:			17/07/99
////---------------------------------------------------------------------

//? Is .h included already?
#ifndef PARSENODE_H_INCLUDED
#define PARSENODE_H_INCLUDED 1

#include	<ostream>
#include	<vector>

#include	"specific.h"	// Needed for namespace macros.

#include	"list.h"
#include	"sstring.h"
#include	"vector3D.h"
#include	"color.h"
#include	"shaders.h"
#include	"vardef.h"
#include	"funcdef.h"

START_NAMESPACE(Aqsis)

enum EqMathOp
{
	Op_Nil=0,

	Op_Add,
	Op_Sub,
	Op_Mul,
	Op_Div,
	Op_Dot,
	Op_Crs,

	Op_Mod,
	Op_Lft,
	Op_Rgt,
	Op_And,
	Op_Xor,
	Op_Or,
};


enum EqRelOp
{
	Op_EQ=0,
	Op_NE,
	Op_L,
	Op_G,
	Op_GE,
	Op_LE,
};


enum EqUnaryOp
{
	Op_Plus=0,
	Op_Neg,
	Op_BitwiseComplement,
	Op_LogicalNot,
};


enum EqLogicalOp
{
	Op_LogAnd=0,
	Op_LogOr,
};

enum EqTextureType
{
	Type_Texture=0,
	Type_Environment,
	Type_Bump,
	Type_Shadow,
};

enum EqCommType
{
	CommTypeAtmosphere=0,
	CommTypeDisplacement,
	CommTypeLightsource,
	CommTypeSurface,
	CommTypeAttribute,
	CommTypeOption,
	CommTypeRendererInfo,
	CommTypeIncident,
	CommTypeOpposite,
};

#define	TAB					("\t")
#define	S_CLEAR				(Out << TAB << "S_CLEAR" << std::endl)
#define	RS_PUSH				(Out << TAB << "RS_PUSH" << std::endl)
#define	RS_POP				(Out << TAB << "RS_POP" << std::endl)
#define	RS_GET				(Out << TAB << "RS_GET" << std::endl)
#define	S_GET				(Out << TAB << "S_GET" << std::endl)
#define	RS_JZ(L)			(Out << TAB << "RS_JZ " << (L) << std::endl)
#define	RS_JNZ(L)			(Out << TAB << "RS_JNZ " << (L) << std::endl)
#define	S_JZ(L)				(Out << TAB << "S_JZ " << (L) << std::endl)
#define	S_JNZ(L)			(Out << TAB << "S_JNZ " << (L) << std::endl)
#define	RS_INVERSE			(Out << TAB << "RS_INVERSE" << std::endl)
#define	LABEL(L)			(Out << ":" << (L) << std::endl)
#define	MERGE				(Out << TAB << "merge" << std::endl)

///----------------------------------------------------------------------
/// CqParseNode
/// Abstract base class from which all parse nodes are define.

class CqParseNode : public CqListEntry<CqParseNode>
{
	public:
				CqParseNode() : m_pChild(0), m_pParent(0), m_fVarying(TqFalse), m_LineNo(-1)
								{}
	virtual		~CqParseNode()	{
									if(m_pParent && m_pParent->m_pChild==this)	
										m_pParent->m_pChild=pNext();
								}

			CqParseNode* pFirstChild() const	{return(m_pChild);}
			CqParseNode* pLastChild() const		{
													CqParseNode* pChild=m_pChild;
													while(pChild->pNext()!=0)	pChild=pChild->pNext();
													return(pChild);
												}
			CqParseNode* pParent() const		{return(m_pParent);}
			void		AddLastChild(CqParseNode* pChild)
												{
													pChild->UnLink();
													if(m_pChild==0)
														m_pChild=pChild;
													else
													{
														CqParseNode* pLastC=m_pChild;
														while(pLastC->pNext())	pLastC=pLastC->pNext();
														pChild->LinkAfter(pLastC);
													}
													m_fVarying|=pChild->m_fVarying;
													pChild->m_pParent=this;
												}
			void		AddFirstChild(CqParseNode* pChild)
												{
													pChild->UnLink();
													if(m_pChild==0)
														m_pChild=pChild;
													else
													{
														pChild->LinkBefore(m_pChild);
														m_pChild=pChild;
													}
													m_fVarying|=pChild->m_fVarying;
													pChild->m_pParent=this;
												}
			void		LinkAfter(CqParseNode* pN)
												{
													CqListEntry<CqParseNode>::LinkAfter(pN);
													m_pParent=pN->m_pParent;
												}
			void		LinkParent(CqParseNode* pN)
												{
													pN->UnLink();
													// If I have a prev sibling, link pN after it.
													if(pPrevious()!=0)
														pN->LinkAfter(pPrevious());
													else
													{
														// Else if I have a parent, link pN as its first child.
														if(m_pParent!=0)
															m_pParent->AddFirstChild(pN);
													}
													// Unlink myself from the tree, and then relink under pN
													UnLink();
													pN->AddLastChild(this);
												}
			void		UnLink()				{
													// Relink the next node into the parent.
													if(pPrevious()==0 && m_pParent!=0)
														m_pParent->m_pChild=pNext();
													CqListEntry<CqParseNode>::UnLink();
													m_pParent=0;
												}
			void		ClearChild()			{m_pChild=0;}
			TqInt		LineNo() const			{return(m_LineNo);}
			const char*	strFileName()			{return(m_strFileName.c_str());}
			void		SetPos(TqInt LineNo, const char* strFileName)
												{
													m_LineNo=LineNo;
													m_strFileName=strFileName;
												}

	virtual	TqBool	IsVariableRef() const	{return(TqFalse);}
	virtual	void		Output(std::ostream& Out) const;
	virtual	EqVariableType	ResType() const			{
													if(m_pChild==0)
														return(Type_Nil);
													else
														return(m_pChild->ResType());
												}
	virtual	TqBool	Optimise();
	virtual EqVariableType TypeCheck(EqVariableType* pTypes, TqInt Count=1, TqBool CheckOnly=TqFalse)
												{
													EqVariableType NewType=Type_Nil;
													CqParseNode* pChild=pFirstChild();
													while(pChild!=0)
													{
														// Get the next pointer nowm, incase the TypeCheck inserts a Cast operator.
														CqParseNode* pNext=pChild->pNext();
														NewType=pChild->TypeCheck(pTypes, Count, CheckOnly);
														pChild=pNext;
													}
													return(NewType);
												}
	virtual	void		NoDup()					{}
#ifdef	_DEBUG
	virtual	void		OutTree(std::ostream& Out) const
												{
													TqInt i;
													for(i=0; i<m_DebugOutTab; i++)	Out << "\t";
													OutTreeDetails(Out);
													m_DebugOutTab++;
													CqParseNode* pChild=pFirstChild();
													while(pChild!=0)
													{
														pChild->OutTree(Out);
														pChild=pChild->pNext();
													}
													m_DebugOutTab--;
												}
	virtual	void		OutTreeDetails(std::ostream& Out) const
												{
													Out << "Node::" << std::endl;
												}
#endif
	virtual	CqParseNode* Clone(CqParseNode* pParent=0)				
												{
													CqParseNode* pNew=new CqParseNode();
													if(m_pChild)	pNew->m_pChild=m_pChild->Clone(pNew);
													pNew->m_pParent=pParent;
													return(pNew);
												}
	virtual	TqBool	UpdateStorageStatus()	{
													m_fVarying=TqFalse;
													CqParseNode* pChild=m_pChild;
													while(pChild!=0)
													{
														if(pChild->UpdateStorageStatus())
															m_fVarying=TqTrue;
														pChild=pChild->pNext();
													}
													return(m_fVarying);
												}

	virtual	TqBool	fVarying() const		{return(m_fVarying);}
			void		SetfVarying(TqBool fVarying)
												{m_fVarying=fVarying;}
			CqParseNode* pNextProcess() const;
			
	static	char*			TypeIdentifier(int Type);
	static	EqVariableType	TypeFromIdentifier(char Id);
	static	char*			TypeName(int Type);
	static	EqVariableType	FindCast(EqVariableType CurrType, EqVariableType* pTypes, TqInt Count);
	static	EqVariableType* pAllTypes()	{return(m_aAllTypes);}
	static	TqBool		fInLoop()			{return(m_cInLoop>0);}
	static	TqBool		fInConditional()	{return(m_cInConditional>0);}
	static	void			IncrLoop()			{m_cInLoop++;}
	static	void			DecrLoop()			{m_cInLoop--;}
	static	void			IncrConditional()	{m_cInLoop++;}
	static	void			DecrConditional()	{m_cInLoop--;}

	protected:
				CqParseNode*	m_pChild;
				CqParseNode*	m_pParent;
				TqBool		m_fVarying;
				TqInt			m_LineNo;
				CqString		m_strFileName;
		static	TqInt			m_cLabels;
		static	TqInt			m_aaTypePriorities[Type_Last][Type_Last];
		static	EqVariableType	m_aAllTypes[Type_Last-1];
		static	TqInt			m_cInLoop;
		static	TqInt			m_cInConditional;
		static	TqInt			m_fInBlock;
#ifdef	_DEBUG
		static	TqInt			m_DebugOutTab;
#endif
};

std::ostream& operator<<(std::ostream& Stream, CqParseNode& Node);
std::ostream& operator<<(std::ostream& Stream, CqParseNode* pNode);

///----------------------------------------------------------------------
/// CqParseDeclaration
/// Parsenode specifying a variable declaration.

class CqParseNodeDeclaration : public CqParseNode
{
	public:
				CqParseNodeDeclaration(const CqParseNodeDeclaration& from) :
								CqParseNode(from),
								m_strName(from.m_strName),
								m_SType(from.m_SType),
								m_Type(from.m_Type)
								{}
				CqParseNodeDeclaration(const char* strName="", EqVariableType Type=Type_Nil, EqShaderType SType=Type_Surface) : 
								CqParseNode(),
								m_strName(strName),
								m_SType(SType),
								m_Type(Type)
								{
									m_fVarying=(m_Type&Type_Varying)!=0;
								}
	virtual		~CqParseNodeDeclaration()	
								{}
				const char*		strName()		{return(m_strName.c_str());}
				EqVariableType	Type() const	{return(m_Type);}
				void			SetType(EqVariableType Type)
												{m_Type=Type;}
				EqShaderType	SType() const	{return(m_SType);}
	virtual		void			Output(std::ostream& Out) const	{}
#ifdef	_DEBUG
	virtual	void		OutTreeDetails(std::ostream& Out) const
												{
													Out << "Decl " << m_strName.c_str() << " " << m_Type << std::endl;
												}
#endif
	virtual		CqParseNode*	Clone(CqParseNode* pParent=0)
												{
													CqParseNodeDeclaration* pNew=new CqParseNodeDeclaration(*this);
													if(m_pChild)	pNew->m_pChild=m_pChild->Clone(pNew);
													pNew->m_pParent=pParent;
													return(pNew);
												}
	protected:
				CqString	m_strName;
				EqVariableType	m_Type;
				EqShaderType m_SType;
};


///----------------------------------------------------------------------
/// CqParseNodeFunction
/// Parsenode specifying a function declaration.

class CqFuncDef;
class CqParseNodeFunction : public CqParseNode
{
	public:
				CqParseNodeFunction(const CqParseNodeFunction& from) :
								CqParseNode(from)
								{
									m_aFuncRef.resize(from.m_aFuncRef.size());
									TqInt i;
									for(i=0; i<m_aFuncRef.size(); i++)
										m_aFuncRef[i]=from.m_aFuncRef[i];
								}
				CqParseNodeFunction(std::vector<SqFuncRef>& aFuncRef) : 
								CqParseNode()
								{
									m_aFuncRef.resize(aFuncRef.size());
									TqInt i;
									for(i=0; i<m_aFuncRef.size(); i++)
										m_aFuncRef[i]=aFuncRef[i];
								}
	virtual		~CqParseNodeFunction()	
								{}
	virtual		void			Output(std::ostream& Out) const;
#ifdef	_DEBUG
	virtual	void		OutTreeDetails(std::ostream& Out) const
												{
													CqFuncDef* pFD=CqFuncDef::GetFunctionPtr(m_aFuncRef[0]);
													if(pFD)
													{
														Out << pFD->strName() << std::endl;
														if(pFD->pDef())
														{
															m_DebugOutTab++;
															pFD->pDef()->OutTree(Out);
															m_DebugOutTab--;
														}
													}
												}
#endif
	virtual		EqVariableType	TypeCheck(EqVariableType* pTypes, TqInt Count=1, TqBool CheckOnly=TqFalse);
	virtual		TqBool		Optimise();
	virtual		EqVariableType	ResType() const;
	virtual		CqParseNode*	Clone(CqParseNode* pParent=0)
												{
													CqParseNodeFunction* pNew=new CqParseNodeFunction(*this);
													if(m_pChild)	pNew->m_pChild=m_pChild->Clone(pNew);
													pNew->m_pParent=pParent;
													return(pNew);
												}
				std::vector<SqFuncRef>&	aFuncRef()		{return(m_aFuncRef);}
				const char*		strName() const	{
													CqFuncDef* pFuncDef=CqFuncDef::GetFunctionPtr(m_aFuncRef[0]);
													if(pFuncDef!=0)
														return(pFuncDef->strName());
													else
														return("");
												}
				void			CheckArgCast(std::vector<TqInt>& aRes);
				void			ArgCast(TqInt iIndex);

	protected:
				std::vector<SqFuncRef>	m_aFuncRef;
};


///----------------------------------------------------------------------
/// CqParseNodeVariable
/// Parsenode specifying a variable access.

class CqParseNodeVariable : public CqParseNode
{
	public:
				CqParseNodeVariable(const CqParseNodeVariable& from) :
								CqParseNode(from),
								m_VarRef(from.m_VarRef)
								{}
				CqParseNodeVariable(SqVarRef VarRef);
				CqParseNodeVariable(CqParseNodeVariable* pVar);
	virtual		~CqParseNodeVariable()	
								{}
	
		const	char*			strName() const;

	virtual		TqBool		IsVariableRef() const	{return(TqTrue);}
	virtual		void			Output(std::ostream& Out) const;
#ifdef	_DEBUG
	virtual	void		OutTreeDetails(std::ostream& Out) const
												{
													CqVarDef* pVD=CqVarDef::GetVariablePtr(m_VarRef);
													if(pVD)
														Out << pVD->strName() << std::endl;
												}
#endif
	virtual		TqBool		Optimise();
	virtual		EqVariableType	TypeCheck(EqVariableType* pTypes, TqInt Count=1, TqBool CheckOnly=TqFalse);
	virtual		EqVariableType	ResType() const;
	virtual		CqParseNode*	Clone(CqParseNode* pParent=0)
												{
													CqParseNodeVariable* pNew=new CqParseNodeVariable(*this);
													if(m_pChild)	pNew->m_pChild=m_pChild->Clone(pNew);
													pNew->m_pParent=pParent;
													return(pNew);
												}
				SqVarRef&		VarRef()		{return(m_VarRef);}
				void			SetParam()		{
													CqVarDef* pVarDef=CqVarDef::GetVariablePtr(m_VarRef);
													if(pVarDef!=0)	pVarDef->SetParam();
												}
				void			SetDefaultStorage(EqVariableType Storage)		
												{
													// If a storage method has not been specified, default to the specified type.
													CqVarDef* pVarDef=CqVarDef::GetVariablePtr(m_VarRef);
													if(pVarDef!=0)	pVarDef->SetDefaultStorage(Storage);
												}

	protected:
				SqVarRef		m_VarRef;	
};


///----------------------------------------------------------------------
/// CqParseNodeVariableArray
/// Parsenode specifying an array variable access.

class CqParseNodeVariableArray : public CqParseNodeVariable
{
	public:
				CqParseNodeVariableArray(const CqParseNodeVariableArray& from) :
								CqParseNodeVariable(from)
								{}
				CqParseNodeVariableArray(SqVarRef VarRef);
				CqParseNodeVariableArray(CqParseNodeVariableArray* pVar);
	virtual		~CqParseNodeVariableArray()	
								{}
	
	virtual		void			Output(std::ostream& Out) const;
#ifdef	_DEBUG
	virtual	void		OutTreeDetails(std::ostream& Out) const
												{
													CqVarDef* pVD=CqVarDef::GetVariablePtr(m_VarRef);
													if(pVD)
													{
														Out << pVD->strName() << "[";
														m_pChild->OutTreeDetails(Out);
														Out << "]" << std::endl;
													}
												}
#endif
	virtual		EqVariableType	TypeCheck(EqVariableType* pTypes, TqInt Count=1, TqBool CheckOnly=TqFalse);
	virtual		CqParseNode*	Clone(CqParseNode* pParent=0)
												{
													CqParseNodeVariableArray* pNew=new CqParseNodeVariableArray(*this);
													if(m_pChild)	pNew->m_pChild=m_pChild->Clone(pNew);
													pNew->m_pParent=pParent;
													return(pNew);
												}
	protected:
};

///----------------------------------------------------------------------
/// CqParseNodePop
/// Parsenode for use in popping a variable from the stack. Used ONLY in local function parameter
/// initialisation, CqParseNodeAssign is used normally.

class CqParseNodePop : public CqParseNodeVariable
{
	public:
				CqParseNodePop(CqParseNodeVariable* pVar) : 
								CqParseNodeVariable(pVar)
								{}
	virtual		~CqParseNodePop()	
								{}
	virtual		void			Output(std::ostream& Out) const;
	private:
};

///----------------------------------------------------------------------
/// CqParseNodeAssign
/// Parsenode specifying an assignment operation.

class CqParseNodeAssign : public CqParseNode
{
	public:
				CqParseNodeAssign(const CqParseNodeAssign& from) :
								CqParseNode(from),
								m_fNoDup(from.m_fNoDup)
								{}
				CqParseNodeAssign(SqVarRef VarRef) : 
								CqParseNode(),
								m_VarRef(VarRef),
								m_fNoDup(TqFalse)
								{
									// Mark this as varying if the variable to which it assigns is varying
									CqVarDef* pVarDef=CqVarDef::GetVariablePtr(VarRef);
									if(pVarDef!=0)
										m_fVarying=(pVarDef->Type()&Type_Varying)!=0;
								}
	virtual		~CqParseNodeAssign()	
								{}
	virtual		void			Output(std::ostream& Out) const;
	virtual		TqBool		Optimise();
	virtual		EqVariableType	TypeCheck(EqVariableType* pTypes, TqInt Count=1, TqBool CheckOnly=TqFalse);
	virtual		void			NoDup()			{m_fNoDup=TqTrue;}
	virtual		EqVariableType	ResType() const;
#ifdef	_DEBUG
	virtual	void		OutTreeDetails(std::ostream& Out) const
												{
													Out << "Assign " << CqVarDef::GetVariablePtr(m_VarRef)->strName() << std::endl;
												}
#endif
	virtual		CqParseNode*	Clone(CqParseNode* pParent=0)
												{
													CqParseNodeAssign* pNew=new CqParseNodeAssign(*this);
													if(m_pChild)	pNew->m_pChild=m_pChild->Clone(pNew);
													pNew->m_pParent=pParent;
													return(pNew);
												}
	virtual	TqBool	UpdateStorageStatus()	{
													// Varying status is a combination of the varying status of all chidren
													// and the varying status of the variable to which we are assigning.
													TqBool fVarying=CqParseNode::UpdateStorageStatus();
													m_fVarying=TqFalse;
													CqVarDef* pVarDef=CqVarDef::GetVariablePtr(m_VarRef);
													if(pVarDef!=0)
														m_fVarying=(pVarDef->Type()&Type_Varying)!=0;
													
													m_fVarying=((fVarying)||(m_fVarying));

													return(m_fVarying);
												}

	protected:
				SqVarRef		m_VarRef;	
				TqBool		m_fNoDup;
};


///----------------------------------------------------------------------
/// CqParseNodeAssignArray
/// Parsenode specifying an assignment operation to an indexed array.

class CqParseNodeAssignArray : public CqParseNodeAssign
{
	public:
				CqParseNodeAssignArray(const CqParseNodeAssignArray& from) :
								CqParseNodeAssign(from)
								{}
				CqParseNodeAssignArray(SqVarRef VarRef) : 
								CqParseNodeAssign(VarRef)
								{}
	virtual		~CqParseNodeAssignArray()	
								{}
	virtual		void			Output(std::ostream& Out) const;
	virtual		EqVariableType	TypeCheck(EqVariableType* pTypes, TqInt Count=1, TqBool CheckOnly=TqFalse);
#ifdef	_DEBUG
	virtual	void		OutTreeDetails(std::ostream& Out) const
												{
													Out << "Assign Array" << CqVarDef::GetVariablePtr(m_VarRef)->strName() << std::endl;
												}
#endif
	virtual		CqParseNode*	Clone(CqParseNode* pParent=0)
												{
													CqParseNodeAssignArray* pNew=new CqParseNodeAssignArray(*this);
													if(m_pChild)	pNew->m_pChild=m_pChild->Clone(pNew);
													pNew->m_pParent=pParent;
													return(pNew);
												}
	private:
};


///----------------------------------------------------------------------
/// CqParseNodeop
/// Base parsenode specifying an operation.

class CqParseNodeOp : public CqParseNode
{
	public:
				CqParseNodeOp(const CqParseNodeOp& from) :
								CqParseNode(from)
								{}
				CqParseNodeOp() : 
								CqParseNode()
								{}
	virtual		~CqParseNodeOp()	
								{}

	virtual		EqVariableType	TypeCheck(EqVariableType* pTypes, TqInt Count=1, TqBool CheckOnly=TqFalse);
};


///----------------------------------------------------------------------
/// CqParseNodeMathOp
/// Parsenode specifying a mathematical operation.

class CqParseNodeMathOp : public CqParseNodeOp
{
	public:
				CqParseNodeMathOp(const CqParseNodeMathOp& from) :
								CqParseNodeOp(from),
								m_Operator(from.m_Operator)
								{}
				CqParseNodeMathOp() : 
								CqParseNodeOp(),
								m_Operator(Op_Nil)
								{}
				CqParseNodeMathOp(EqMathOp Operator) : 
								CqParseNodeOp(),
								m_Operator(Operator)
								{}
	virtual		~CqParseNodeMathOp()	
								{}

	virtual		void			Output(std::ostream& Out) const;
				void			OutOp(std::ostream& Out) const;
#ifdef	_DEBUG
	virtual	void		OutTreeDetails(std::ostream& Out) const
												{
													OutOp(Out);
													Out << std::endl;
												}
#endif
	virtual		EqVariableType		ResType() const;
	virtual		CqParseNode*	Clone(CqParseNode* pParent=0)
												{
													CqParseNodeMathOp* pNew=new CqParseNodeMathOp(*this);
													if(m_pChild)	pNew->m_pChild=m_pChild->Clone(pNew);
													pNew->m_pParent=pParent;
													return(pNew);
												}

	protected:
				EqMathOp		m_Operator;
};


///----------------------------------------------------------------------
/// CqParseNodeMathOpDot
/// Parsenode specifying a dot (dot product) operation.

class CqParseNodeMathOpDot : public CqParseNodeMathOp
{
	public:
				CqParseNodeMathOpDot(const CqParseNodeMathOpDot& from) :
								CqParseNodeMathOp(from)
								{}
				CqParseNodeMathOpDot() : 
								CqParseNodeMathOp(Op_Dot)
								{}
	virtual		~CqParseNodeMathOpDot()	
								{}

	virtual		EqVariableType	TypeCheck(EqVariableType* pTypes, TqInt Count=1, TqBool CheckOnly=TqFalse);
};


///----------------------------------------------------------------------
/// CqParseNodeRelop
/// Parsenode specifying a relational operation.

class CqParseNodeRelOp : public CqParseNodeOp
{
	public:
				CqParseNodeRelOp(const CqParseNodeRelOp& from) :
								CqParseNodeOp(from),
								m_Operator(from.m_Operator)
								{}
				CqParseNodeRelOp(EqRelOp Operator) : 
								CqParseNodeOp(),
								m_Operator(Operator)
								{}
	virtual		~CqParseNodeRelOp()	
								{}

	virtual		void			Output(std::ostream& Out) const;
				void			OutOp(std::ostream& Out) const;
	virtual		EqVariableType		ResType() const	{return(Type_Float);}
#ifdef	_DEBUG
	virtual	void		OutTreeDetails(std::ostream& Out) const
												{
													OutOp(Out);
													Out << std::endl;
												}
#endif
	virtual		CqParseNode*	Clone(CqParseNode* pParent=0)
												{
													CqParseNodeRelOp* pNew=new CqParseNodeRelOp(*this);
													if(m_pChild)	pNew->m_pChild=m_pChild->Clone(pNew);
													pNew->m_pParent=pParent;
													return(pNew);
												}
	virtual		EqVariableType	TypeCheck(EqVariableType* pTypes, TqInt Count=1, TqBool CheckOnly=TqFalse);

	protected:
				EqRelOp			m_Operator;
};


///----------------------------------------------------------------------
/// CqParseNodeUnaryop
/// Parsenode specifying a unary operation.

class CqParseNodeUnaryOp : public CqParseNode
{
	public:
				CqParseNodeUnaryOp(const CqParseNodeUnaryOp& from) :
								CqParseNode(from),
								m_Operator(from.m_Operator)
								{}
				CqParseNodeUnaryOp(EqUnaryOp Operator) : 
								CqParseNode(),
								m_Operator(Operator)
								{}
	virtual		~CqParseNodeUnaryOp()	
								{}

	virtual		void			Output(std::ostream& Out) const;
				void			OutOp(std::ostream& Out) const;
#ifdef	_DEBUG
	virtual	void		OutTreeDetails(std::ostream& Out) const
												{
													OutOp(Out);
													Out << std::endl;
												}
#endif
	virtual		EqVariableType	TypeCheck(EqVariableType* pTypes, TqInt Count=1, TqBool CheckOnly=TqFalse);
	virtual		CqParseNode*	Clone(CqParseNode* pParent=0)
												{
													CqParseNodeUnaryOp* pNew=new CqParseNodeUnaryOp(*this);
													if(m_pChild)	pNew->m_pChild=m_pChild->Clone(pNew);
													pNew->m_pParent=pParent;
													return(pNew);
												}

	protected:
				EqUnaryOp		m_Operator;
};


///----------------------------------------------------------------------
/// CqParseNodeLogicalop
/// Parsenode specifying a logical operation.

class CqParseNodeLogicalOp : public CqParseNodeOp
{
	public:
				CqParseNodeLogicalOp(const CqParseNodeLogicalOp& from) :
								CqParseNodeOp(from),
								m_Operator(from.m_Operator)
								{}
				CqParseNodeLogicalOp(EqLogicalOp Operator) : 
								CqParseNodeOp(),
								m_Operator(Operator)
								{}
	virtual		~CqParseNodeLogicalOp()	
								{}

	virtual		void			Output(std::ostream& Out) const;
				void			OutOp(std::ostream& Out) const;
#ifdef	_DEBUG
	virtual	void		OutTreeDetails(std::ostream& Out) const
												{
													OutOp(Out);
													Out << std::endl;
												}
#endif
	virtual		EqVariableType	ResType() const	{return(Type_Float);}
	virtual		CqParseNode*	Clone(CqParseNode* pParent=0)
												{
													CqParseNodeLogicalOp* pNew=new CqParseNodeLogicalOp(*this);
													if(m_pChild)	pNew->m_pChild=m_pChild->Clone(pNew);
													pNew->m_pParent=pParent;
													return(pNew);
												}

	protected:
				EqLogicalOp		m_Operator;
};


///----------------------------------------------------------------------
/// CqParseNodeDrop
/// Parsenode specifying an assignment operation.

class CqParseNodeDrop : public CqParseNode
{
	public:
				CqParseNodeDrop(const CqParseNodeDrop& from) :
								CqParseNode(from)
								{}
				CqParseNodeDrop() : 
								CqParseNode()
								{}
	virtual		~CqParseNodeDrop()	
								{}

	virtual		void			Output(std::ostream& Out) const
								{
									CqParseNode::Output(Out);
									Out << "\tdrop" << std::endl;
								}
#ifdef	_DEBUG
	virtual	void		OutTreeDetails(std::ostream& Out) const
												{
													Out << "Drop " << std::endl;
												}
#endif
	virtual		CqParseNode*	Clone(CqParseNode* pParent=0)
												{
													CqParseNodeDrop* pNew=new CqParseNodeDrop(*this);
													if(m_pChild)	pNew->m_pChild=m_pChild->Clone(pNew);
													pNew->m_pParent=pParent;
													return(pNew);
												}

	private:
};


///----------------------------------------------------------------------
/// CqParseNodeConst
/// Base class for all const types.

class CqParseNodeConst : public CqParseNode
{
	public:
				CqParseNodeConst() :
								CqParseNode()
								{}
				CqParseNodeConst(const CqParseNodeConst& from) :
								CqParseNode(from)
								{}
	virtual		~CqParseNodeConst()	
								{}

	virtual		EqVariableType	TypeCheck(EqVariableType* pTypes, TqInt Count=1, TqBool CheckOnly=TqFalse);
	private:
};


///----------------------------------------------------------------------
/// CqParseNodeFloatConst
/// Parsenode specifying a float constant value.

class CqParseNodeFloatConst : public CqParseNodeConst
{
	public:
				CqParseNodeFloatConst(const CqParseNodeFloatConst& from) :
								CqParseNodeConst(from),
								m_Value(from.m_Value)
								{}
				CqParseNodeFloatConst(TqFloat Val) : 
								CqParseNodeConst(),
								m_Value(Val)
								{}
	virtual		~CqParseNodeFloatConst()	
								{}

	virtual		void			Output(std::ostream& Out) const;
#ifdef	_DEBUG
	virtual	void		OutTreeDetails(std::ostream& Out) const
												{
													Out << "FloatConst " << m_Value << std::endl;
												}
#endif
	virtual		EqVariableType	ResType() const	{return(Type_Float);}
	virtual		CqParseNode*	Clone(CqParseNode* pParent=0)
												{
													CqParseNodeFloatConst* pNew=new CqParseNodeFloatConst(*this);
													if(m_pChild)	pNew->m_pChild=m_pChild->Clone(pNew);
													pNew->m_pParent=pParent;
													return(pNew);
												}

	private:
				TqFloat			m_Value;
};


///----------------------------------------------------------------------
/// CqParseNodeStringConst
/// Parsenode specifying a string constant value.

class CqParseNodeStringConst : public CqParseNodeConst
{
	public:
				CqParseNodeStringConst(const CqParseNodeStringConst& from) :
								CqParseNodeConst(from),
								m_Value(from.m_Value)
								{}
				CqParseNodeStringConst(const char* Val) : 
								CqParseNodeConst(),
								m_Value(Val)
								{}
	virtual		~CqParseNodeStringConst()	
								{}

	virtual		void			Output(std::ostream& Out) const;
#ifdef	_DEBUG
	virtual	void		OutTreeDetails(std::ostream& Out) const
												{
													if(m_Value.empty())
														Out << "StringConst \"\"" << std::endl;
													else
														Out << "StringConst \"" << m_Value.c_str() << "\"" << std::endl;
												}
#endif
	virtual		EqVariableType	ResType() const	{return(Type_String);}
	virtual		CqParseNode*	Clone(CqParseNode* pParent=0)
												{
													CqParseNodeStringConst* pNew=new CqParseNodeStringConst(*this);
													if(m_pChild)	pNew->m_pChild=m_pChild->Clone(pNew);
													pNew->m_pParent=pParent;
													return(pNew);
												}

	private:
				CqString		m_Value;
};


///----------------------------------------------------------------------
/// CqParseNodeWhileConstruct
/// Parsenode specifying an assignment operation.

class CqParseNodeWhileConstruct : public CqParseNode
{
	public:
				CqParseNodeWhileConstruct(const CqParseNodeWhileConstruct& from) :
								CqParseNode(from)
								{}
				CqParseNodeWhileConstruct() : 
								CqParseNode()
								{}
	virtual		~CqParseNodeWhileConstruct()	
								{}

	virtual		void			Output(std::ostream& Out) const;
#ifdef	_DEBUG
	virtual	void		OutTreeDetails(std::ostream& Out) const
												{
													Out << "While " << std::endl;
												}
#endif
	virtual		CqParseNode*	Clone(CqParseNode* pParent=0)
												{
													CqParseNodeWhileConstruct* pNew=new CqParseNodeWhileConstruct(*this);
													if(m_pChild)	pNew->m_pChild=m_pChild->Clone(pNew);
													pNew->m_pParent=pParent;
													return(pNew);
												}

	private:
};


///----------------------------------------------------------------------
/// CqParseNodeIlluminateConstruct
/// Parsenode an illuminate construct.

class CqParseNodeIlluminateConstruct : public CqParseNode
{
	public:
				CqParseNodeIlluminateConstruct(const CqParseNodeIlluminateConstruct& from) :
								CqParseNode(from),
								m_fAxisAngle(from.m_fAxisAngle)
								{}
				CqParseNodeIlluminateConstruct(TqBool fAxisAngle=TqFalse) : 
								CqParseNode(),
								m_fAxisAngle(fAxisAngle)
								{}
	virtual		~CqParseNodeIlluminateConstruct()	
								{}

	virtual		void			Output(std::ostream& Out) const;
#ifdef	_DEBUG
	virtual	void		OutTreeDetails(std::ostream& Out) const
												{
													Out << "Illuminate " << std::endl;
												}
#endif
	virtual		CqParseNode*	Clone(CqParseNode* pParent=0)
												{
													CqParseNodeIlluminateConstruct* pNew=new CqParseNodeIlluminateConstruct(*this);
													if(m_pChild)	pNew->m_pChild=m_pChild->Clone(pNew);
													pNew->m_pParent=pParent;
													return(pNew);
												}

	private:
				TqBool	m_fAxisAngle;
};


///----------------------------------------------------------------------
/// CqParseNodeIlluminanceConstruct
/// Parsenode an illuminance construct.

class CqParseNodeIlluminanceConstruct : public CqParseNode
{
	public:
				CqParseNodeIlluminanceConstruct(const CqParseNodeIlluminanceConstruct& from) :
								CqParseNode(from),
								m_fAxisAngle(from.m_fAxisAngle)
								{}
				CqParseNodeIlluminanceConstruct(TqBool fAxisAngle=TqFalse) : 
								CqParseNode(),
								m_fAxisAngle(fAxisAngle)
								{}
	virtual		~CqParseNodeIlluminanceConstruct()	
								{}

	virtual		void			Output(std::ostream& Out) const;
#ifdef	_DEBUG
	virtual	void		OutTreeDetails(std::ostream& Out) const
												{
													Out << "Illuminance " << std::endl;
												}
#endif
	virtual		CqParseNode*	Clone(CqParseNode* pParent=0)
												{
													CqParseNodeIlluminanceConstruct* pNew=new CqParseNodeIlluminanceConstruct(*this);
													if(m_pChild)	pNew->m_pChild=m_pChild->Clone(pNew);
													pNew->m_pParent=pParent;
													return(pNew);
												}

	private:
				TqBool	m_fAxisAngle;
};


///----------------------------------------------------------------------
/// CqParseNodeSolarConstruct
/// Parsenode an solar construct.

class CqParseNodeSolarConstruct : public CqParseNode
{
	public:
				CqParseNodeSolarConstruct(const CqParseNodeSolarConstruct& from) :
								CqParseNode(from),
								m_fAxisAngle(from.m_fAxisAngle)
								{}
				CqParseNodeSolarConstruct(TqBool fAxisAngle=TqFalse) : 
								CqParseNode(),
								m_fAxisAngle(fAxisAngle)
								{}
	virtual		~CqParseNodeSolarConstruct()	
								{}

	virtual		void			Output(std::ostream& Out) const;
#ifdef	_DEBUG
	virtual	void		OutTreeDetails(std::ostream& Out) const
												{
													Out << "Solar " << std::endl;
												}
#endif
	virtual		CqParseNode*	Clone(CqParseNode* pParent=0)
												{
													CqParseNodeSolarConstruct* pNew=new CqParseNodeSolarConstruct(*this);
													if(m_pChild)	pNew->m_pChild=m_pChild->Clone(pNew);
													pNew->m_pParent=pParent;
													return(pNew);
												}

	private:
				TqBool	m_fAxisAngle;
};


///----------------------------------------------------------------------
/// CqParseNodeConditional
/// Parsenode specifying an assignment operation.

class CqParseNodeConditional : public CqParseNode
{
	public:
				CqParseNodeConditional(const CqParseNodeConditional& from) :
								CqParseNode(from)
								{}
				CqParseNodeConditional() : 
								CqParseNode()
								{}
	virtual		~CqParseNodeConditional()	
								{}

	virtual		void			Output(std::ostream& Out) const;
#ifdef	_DEBUG
	virtual	void		OutTreeDetails(std::ostream& Out) const
												{
													Out << "Conditional " << std::endl;
												}
#endif
	virtual		CqParseNode*	Clone(CqParseNode* pParent=0)
												{
													CqParseNodeConditional* pNew=new CqParseNodeConditional(*this);
													if(m_pChild)	pNew->m_pChild=m_pChild->Clone(pNew);
													pNew->m_pParent=pParent;
													return(pNew);
												}
	virtual		EqVariableType	TypeCheck(EqVariableType* pTypes, TqInt Count=1, TqBool CheckOnly=TqFalse)
												{
													EqVariableType Types=Type_Float;
													EqVariableType NewType=Type_Nil;
													CqParseNode* pChild=pFirstChild();

													// Typecheck the conditional
													pChild->TypeCheck(&Types, 1, CheckOnly);
													
													// Now typecheck the conditional statements
													pChild=pChild->pNext();
													while(pChild!=0)
													{
														// Get the next pointer nowm, incase the TypeCheck inserts a Cast operator.
														CqParseNode* pNext=pChild->pNext();
														pChild->TypeCheck(pAllTypes(), Type_Last-1, CheckOnly);
														pChild=pNext;
													}
													return(NewType);
												}

	private:
};

///----------------------------------------------------------------------
/// CqParseNodeQCond
/// Parsenode specifying ? conditional.

class CqParseNodeQCond : public CqParseNode
{
	public:
				CqParseNodeQCond(const CqParseNodeQCond& from) :
								CqParseNode(from)
								{}
				CqParseNodeQCond() : 
								CqParseNode()
								{}
	virtual		~CqParseNodeQCond()	
								{}

	virtual		void			Output(std::ostream& Out) const;
#ifdef	_DEBUG
	virtual	void		OutTreeDetails(std::ostream& Out) const
												{
													Out << "QCond " << std::endl;
												}
#endif
	virtual		EqVariableType	TypeCheck(EqVariableType* pTypes, TqInt Count=1, TqBool CheckOnly=TqFalse);
	virtual		CqParseNode*	Clone(CqParseNode* pParent=0)
												{
													CqParseNodeQCond* pNew=new CqParseNodeQCond(*this);
													if(m_pChild)	pNew->m_pChild=m_pChild->Clone(pNew);
													pNew->m_pParent=pParent;
													return(pNew);
												}

	private:
};



///----------------------------------------------------------------------
/// CqParseNodeCast
/// Parsenode specifying type cast operation.

class CqParseNodeCast : public CqParseNode
{
	public:
				CqParseNodeCast(const CqParseNodeCast& from) :
								CqParseNode(from),
								m_tTo(from.m_tTo)
								{}
				CqParseNodeCast(EqVariableType tto) : 
								CqParseNode(),
								m_tTo(tto)
								{}
	virtual		~CqParseNodeCast()	
								{}

	virtual		void			Output(std::ostream& Out) const;
#ifdef	_DEBUG
	virtual	void		OutTreeDetails(std::ostream& Out) const
												{
													Out << "Cast " << m_tTo << std::endl;
												}
#endif
	virtual		EqVariableType	ResType() const			{return(m_tTo);}
	virtual		TqBool		Optimise();
	virtual		EqVariableType	TypeCheck(EqVariableType* pTypes, TqInt Count=1, TqBool CheckOnly=TqFalse);
	virtual		CqParseNode*	Clone(CqParseNode* pParent=0)
												{
													CqParseNodeCast* pNew=new CqParseNodeCast(*this);
													if(m_pChild)	pNew->m_pChild=m_pChild->Clone(pNew);
													pNew->m_pParent=pParent;
													return(pNew);
												}
	private:
				EqVariableType	m_tTo;
};


///----------------------------------------------------------------------
/// CqParseNodeTriple
/// Parsenode specifying a float constant value.

class CqParseNodeTriple : public CqParseNode
{
	public:
				CqParseNodeTriple(const CqParseNodeTriple& from) :
								CqParseNode(from)
								{}
				CqParseNodeTriple() : 
								CqParseNode()
								{}
	virtual		~CqParseNodeTriple()	
								{}

	virtual		void			Output(std::ostream& Out) const;
#ifdef	_DEBUG
	virtual	void		OutTreeDetails(std::ostream& Out) const
												{
													Out << "Triple " << std::endl;
												}
#endif
	virtual		EqVariableType	TypeCheck(EqVariableType* pTypes, TqInt Count=1, TqBool CheckOnly=TqFalse);
	virtual		EqVariableType	ResType() const			{return(Type_Triple);}
	virtual		CqParseNode*	Clone(CqParseNode* pParent=0)
												{
													CqParseNodeTriple* pNew=new CqParseNodeTriple(*this);
													if(m_pChild)	pNew->m_pChild=m_pChild->Clone(pNew);
													pNew->m_pParent=pParent;
													return(pNew);
												}

	private:
};


///----------------------------------------------------------------------
/// CqParseNodeHexTuple
/// Parsenode specifying a matrix as 16 float values.

class CqParseNodeHexTuple : public CqParseNode
{
	public:
				CqParseNodeHexTuple(const CqParseNodeHexTuple& from) :
								CqParseNode(from)
								{}
				CqParseNodeHexTuple() : 
								CqParseNode()
								{}
	virtual		~CqParseNodeHexTuple()	
								{}

	virtual		void			Output(std::ostream& Out) const;
#ifdef	_DEBUG
	virtual	void		OutTreeDetails(std::ostream& Out) const
												{
													Out << "HexTuple " << std::endl;
												}
#endif
	virtual		EqVariableType	TypeCheck(EqVariableType* pTypes, TqInt Count=1, TqBool CheckOnly=TqFalse);
	virtual		EqVariableType	ResType() const			{return(Type_HexTuple);}
	virtual		CqParseNode*	Clone(CqParseNode* pParent=0)
												{
													CqParseNodeHexTuple* pNew=new CqParseNodeHexTuple(*this);
													if(m_pChild)	pNew->m_pChild=m_pChild->Clone(pNew);
													pNew->m_pParent=pParent;
													return(pNew);
												}

	private:
};


///----------------------------------------------------------------------
/// CqParseNodeCommFunction
/// Parsenode specifying a special comm function.

class CqParseNodeCommFunction : public CqParseNode
{
	public:
				CqParseNodeCommFunction(const CqParseNodeCommFunction& from) :
								CqParseNode(from),
								m_vrVariable(from.m_vrVariable),
								m_commType(from.m_commType)
								{}
				CqParseNodeCommFunction(TqInt commType, SqVarRef vrVariable) : 
								CqParseNode(),
								m_vrVariable(vrVariable),
								m_commType(commType)
								{}
	virtual		~CqParseNodeCommFunction()	
								{}

	virtual		void			Output(std::ostream& Out) const;
	virtual		EqVariableType	ResType() const			{return(Type_Float);}
	virtual		EqVariableType	TypeCheck(EqVariableType* pTypes, TqInt Count=1, TqBool CheckOnly=TqFalse);
#ifdef	_DEBUG
	virtual	void		OutTreeDetails(std::ostream& Out) const
												{
													Out << "Comm_Function " << std::endl;
												}
#endif
	virtual		CqParseNode*	Clone(CqParseNode* pParent=0)
												{
													CqParseNodeCommFunction* pNew=new CqParseNodeCommFunction(*this);
													if(m_pChild)	pNew->m_pChild=m_pChild->Clone(pNew);
													pNew->m_pParent=pParent;
													return(pNew);
												}

	private:
				SqVarRef		m_vrVariable;
				TqInt			m_commType;
};


///----------------------------------------------------------------------
/// CqParseNodeSetComp
/// Parsenode specifying a set component function.

class CqParseNodeSetComp : public CqParseNode
{
	public:
				CqParseNodeSetComp(const CqParseNodeSetComp& from) :
								CqParseNode(from),
								m_vrVariable(from.m_vrVariable),
								m_Index(from.m_Index)
								{}
				CqParseNodeSetComp(TqInt Index, SqVarRef vrVariable) : 
								CqParseNode(),
								m_vrVariable(vrVariable),
								m_Index(Index)
								{}
	virtual		~CqParseNodeSetComp()	
								{}

	virtual		void			Output(std::ostream& Out) const;
#ifdef	_DEBUG
	virtual	void		OutTreeDetails(std::ostream& Out) const
												{
													Out << "SetComp " << m_Index << std::endl;
												}
#endif
	virtual		CqParseNode*	Clone(CqParseNode* pParent=0)
												{
													CqParseNodeSetComp* pNew=new CqParseNodeSetComp(*this);
													if(m_pChild)	pNew->m_pChild=m_pChild->Clone(pNew);
													pNew->m_pParent=pParent;
													return(pNew);
												}

	private:
				SqVarRef		m_vrVariable;
				TqInt			m_Index;
};


///----------------------------------------------------------------------
/// CqParseNodeSetColComp
/// Parsenode specifying a set color component function.

class CqParseNodeSetColComp : public CqParseNode
{
	public:
				CqParseNodeSetColComp(const CqParseNodeSetColComp& from) :
								CqParseNode(from),
								m_vrVariable(from.m_vrVariable)
								{}
				CqParseNodeSetColComp(SqVarRef vrVariable) : 
								CqParseNode(),
								m_vrVariable(vrVariable)
								{}
	virtual		~CqParseNodeSetColComp()	
								{}

	virtual		void			Output(std::ostream& Out) const;
#ifdef	_DEBUG
	virtual	void		OutTreeDetails(std::ostream& Out) const
												{
													Out << "SetColComp " << std::endl;
												}
#endif
	virtual		CqParseNode*	Clone(CqParseNode* pParent=0)
												{
													CqParseNodeSetColComp* pNew=new CqParseNodeSetColComp(*this);
													if(m_pChild)	pNew->m_pChild=m_pChild->Clone(pNew);
													pNew->m_pParent=pParent;
													return(pNew);
												}

	private:
				SqVarRef		m_vrVariable;
};


//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif	// !PARSENODE_H_INCLUDED
