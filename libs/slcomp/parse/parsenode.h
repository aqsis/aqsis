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

#include	<vector>
#include	<list>

#include	<aqsis/aqsis.h>

#include	<aqsis/util/sstring.h>
#include	"vardef.h"
#include	<aqsis/slcomp/iparsenode.h>
#include	"funcdef.h"
#include	<aqsis/util/list.h>

namespace Aqsis {

extern const char* gShaderTypeNames[];
extern TqInt gcShaderTypeNames;

///----------------------------------------------------------------------
/** \class CqParseNode
 * Abstract base class from which all parse nodes are define.
 */

class CqParseNodeShader;
class CqParseNode : public CqListEntry<CqParseNode>, public IqParseNode
{
	public:
		struct Pos
		{
			TqInt m_LineNo;
			const char* m_strFileName;
		};


		CqParseNode() : m_pChild( 0 ), m_pParent( 0 ), m_fVarying( false ), m_LineNo( -1 )
		{}
		virtual	~CqParseNode()
		{
			if ( m_pParent && m_pParent->m_pChild == this )
				m_pParent->m_pChild = pNext();
		}


		// Overridden from IqParseNode
		virtual	IqParseNode* pChild() const
		{
			return ( m_pChild );
		}
		virtual	IqParseNode* pParent() const
		{
			return ( m_pParent );
		}
		virtual	IqParseNode* pNextSibling() const
		{
			return ( pNext() );
		}
		virtual	IqParseNode* pPrevSibling() const
		{
			return ( pPrevious() );
		}
		virtual	TqInt	LineNo() const
		{
			return ( m_LineNo );
		}
		virtual	const char*	strFileName() const
		{
			return ( m_strFileName.c_str() );
		}
		virtual	bool	IsVariableRef() const
		{
			return ( false );
		}
		virtual	TqInt	ResType() const
		{
			if ( m_pChild == 0 )
				return ( Type_Nil );
			else
				return ( m_pChild->ResType() );
		}
		virtual	bool	fVarying() const
		{
			return ( m_fVarying );
		}
		virtual	void*	GetInterface( EqParseNodeType type) const
		{
			void* pNode;
			if ( ( pNode = ( void* ) QueryNodeType<IqParseNode>( this, type ) ) != 0 )
				return ( pNode );
			return ( 0 );
		}
		virtual	TqInt	NodeType() const
		{
			return ( IqParseNode::m_ID );
		}
		virtual	void	Accept( IqParseNodeVisitor &V)
		{
			V.Visit(*this);
		}



		CqParseNode* pFirstChild() const
		{
			return ( m_pChild );
		}
		CqParseNode* pLastChild() const
		{
			CqParseNode * pChild = m_pChild;
			while ( pChild->pNext() != 0 )
				pChild = pChild->pNext();
			return ( pChild );
		}
		void	AddLastChild( CqParseNode* pChild )
		{
			pChild->UnLink();
			if ( m_pChild == 0 )
				m_pChild = pChild;
			else
			{
				CqParseNode* pLastC = m_pChild;
				while ( pLastC->pNext() )
					pLastC = pLastC->pNext();
				pChild->LinkAfter( pLastC );
			}
			m_fVarying |= pChild->m_fVarying;
			pChild->m_pParent = this;
		}
		void	AddFirstChild( CqParseNode* pChild )
		{
			pChild->UnLink();
			if ( m_pChild == 0 )
				m_pChild = pChild;
			else
			{
				pChild->LinkBefore( m_pChild );
				m_pChild = pChild;
			}
			m_fVarying |= pChild->m_fVarying;
			pChild->m_pParent = this;
		}
		void	LinkAfter( CqParseNode* pN )
		{
			CqListEntry<CqParseNode>::LinkAfter( pN );
			m_pParent = pN->m_pParent;
		}
		void	LinkParent( CqParseNode* pN )
		{
			pN->UnLink();
			// If I have a prev sibling, link pN after it.
			if ( pPrevious() != 0 )
				pN->LinkAfter( pPrevious() );
			else
			{
				// Else if I have a parent, link pN as its first child.
				if ( m_pParent != 0 )
					m_pParent->AddFirstChild( pN );
			}
			// Unlink myself from the tree, and then relink under pN
			UnLink();
			pN->AddLastChild( this );
		}
		void	UnLink()
		{
			// Relink the next node into the parent.
			if ( pPrevious() == 0 && m_pParent != 0 )
				m_pParent->m_pChild = pNext();
			CqListEntry<CqParseNode>::UnLink();
			m_pParent = 0;
		}
		void	ClearChild()
		{
			m_pChild = 0;
		}
		void	SetPos( TqInt LineNo, const char* strFileName )
		{
			m_LineNo = LineNo;
			m_strFileName = strFileName;
		}
		void	SetPos( const Pos& pos )
		{
			m_LineNo = pos.m_LineNo;
			m_strFileName = pos.m_strFileName;
		}

		virtual	bool	Optimise();
		virtual TqInt	TypeCheck( TqInt* pTypes, TqInt Count, bool& needsCast, bool CheckOnly )
		{
			TqInt NewType = Type_Nil;
			CqParseNode* pChild = pFirstChild();
			while ( pChild != 0 )
			{
				// Get the next pointer nowm, incase the TypeCheck inserts a Cast operator.
				CqParseNode * pNext = pChild->pNext();
				NewType = pChild->TypeCheck( pTypes, Count, needsCast, CheckOnly );
				pChild = pNext;
			}
			return ( NewType );
		}
		virtual void	validTypes( std::list<std::pair<TqInt,TqInt> >& types );
		virtual	void	NoDup()
		{}
		virtual	CqParseNode* Clone( CqParseNode* pParent = 0 )
		{
			CqParseNode * pNew = new CqParseNode();
			if ( m_pChild )
				pNew->m_pChild = m_pChild->Clone( pNew );
			pNew->m_pParent = pParent;
			return ( pNew );
		}
		virtual	bool	UpdateStorageStatus()
		{
			m_fVarying = false;
			CqParseNode* pChild = m_pChild;
			while ( pChild != 0 )
			{
				if ( pChild->UpdateStorageStatus() )
					m_fVarying = true;
				pChild = pChild->pNext();
			}
			return ( m_fVarying );
		}

		CqParseNodeShader* pShaderNode();

		static	const char*	TypeIdentifier( int Type );
		static	TqInt	TypeFromIdentifier( char Id );
		static	const char*	TypeName( int Type );
		static	TqInt	FindCast( TqInt CurrType, TqInt* pTypes, TqInt Count, TqInt& index );
		static	TqInt*	pAllTypes()
		{
			return ( m_aAllTypes );
		}

	protected:
		CqParseNode*	m_pChild;
		CqParseNode*	m_pParent;
		bool	m_fVarying;
		TqInt	m_LineNo;
		CqString	m_strFileName;

		static	TqInt	m_cLabels;
		static	TqInt	m_aaTypePriorities[ Type_Last ][ Type_Last ];
		static	TqInt	m_aAllTypes[ Type_Last - 1 ];
};


///----------------------------------------------------------------------
/// CqParseShader
/// Parsenode specifying a shader definition.

class CqParseNodeShader : public CqParseNode, public IqParseNodeShader
{
	public:
		CqParseNodeShader( const CqParseNodeShader& from ) :
				CqParseNode( from ),
				m_strName( from.m_strName ),
				m_ShaderType( from.m_ShaderType )
		{}
		CqParseNodeShader( const char* strName = "", const EqShaderType Type = Type_Surface ) :
				CqParseNode(),
				m_strName( strName ),
				m_ShaderType( Type )
		{}
		virtual	~CqParseNodeShader()
		{}

		// Overridden from IqParseNodeShader



		virtual	const char*	strName() const
		{
			return ( m_strName.c_str() );
		}
		virtual	const char*	strShaderType() const
		{
			assert( m_ShaderType < gcShaderTypeNames );
			return ( gShaderTypeNames[ m_ShaderType ] );
		}
		virtual	const EqShaderType ShaderType() const
		{
			return( m_ShaderType );
		}

		// Overridden from IqParseNode
		virtual	void*	GetInterface( EqParseNodeType type) const
		{
			void* pNode;
			if ( ( pNode = ( void* ) QueryNodeType<IqParseNodeShader>( this, type ) ) != 0 )
				return ( pNode );
			return ( CqParseNode::GetInterface( type ) );
		}
		virtual	TqInt	NodeType() const
		{
			return ( IqParseNodeShader::m_ID );
		}
		virtual	void	Accept( IqParseNodeVisitor &V)
		{
			V.Visit(static_cast<IqParseNodeShader&>(*this));
		}


		virtual	CqParseNode*	Clone( CqParseNode* pParent = 0 )
		{
			CqParseNodeShader * pNew = new CqParseNodeShader( *this );
			if ( m_pChild )
				pNew->m_pChild = m_pChild->Clone( pNew );
			pNew->m_pParent = pParent;
			return ( pNew );
		}
	protected:
		CqString	m_strName;
		EqShaderType m_ShaderType;
};


///----------------------------------------------------------------------
/// CqParseNodeFunctionCall
/// Parsenode specifying a function call.

class CqParseNodeFunctionCall : public CqParseNode, public IqParseNodeFunctionCall
{
	public:
		CqParseNodeFunctionCall( const CqParseNodeFunctionCall& from ) :
				CqParseNode( from )
		{
			m_aFuncRef.resize( from.m_aFuncRef.size() );
			for ( TqUint i = 0; i < m_aFuncRef.size(); i++ )
				m_aFuncRef[ i ] = from.m_aFuncRef[ i ];
		}
		CqParseNodeFunctionCall( std::vector<SqFuncRef>& aFuncRef ) :
				CqParseNode()
		{
			m_aFuncRef.resize( aFuncRef.size() );
			for ( TqUint i = 0; i < m_aFuncRef.size(); i++ )
				m_aFuncRef[ i ] = aFuncRef[ i ];
		}
		virtual	~CqParseNodeFunctionCall()
	{}

		// Overridden from IqParseNodeFunctionCall



		virtual	const char*	strName() const;
		virtual	const IqFuncDef* pFuncDef() const;
		virtual	IqFuncDef* pFuncDef();

		// Overridden from IqParseNode
		virtual	void*	GetInterface( EqParseNodeType type) const
		{
			void* pNode;
			if ( ( pNode = ( void* ) QueryNodeType<IqParseNodeFunctionCall>( this, type ) ) != 0 )
				return ( pNode );
			return ( CqParseNode::GetInterface( type ) );
		}
		virtual	TqInt	NodeType() const
		{
			return ( IqParseNodeFunctionCall::m_ID );
		}
		virtual	void	Accept( IqParseNodeVisitor &V)
		{
			V.Visit(static_cast<IqParseNodeFunctionCall&>(*this));
		}


		virtual	TqInt	TypeCheck( TqInt* pTypes, TqInt Count, bool& needsCast, bool CheckOnly );
		virtual	bool	Optimise();
		virtual	TqInt	ResType() const;
		virtual void	validTypes( std::list<std::pair<TqInt,TqInt> >& types );
		virtual	CqParseNode*	Clone( CqParseNode* pParent = 0 )
		{
			CqParseNodeFunctionCall * pNew = new CqParseNodeFunctionCall( *this );
			if ( m_pChild )
				pNew->m_pChild = m_pChild->Clone( pNew );
			pNew->m_pParent = pParent;
			return ( pNew );
		}
		std::vector<SqFuncRef>&	aFuncRef()
		{
			return ( m_aFuncRef );
		}
		void	CheckArgCast( std::vector<TqInt>& aRes );
		void	ArgCast( TqInt iIndex );

	protected:
		std::vector<SqFuncRef>	m_aFuncRef;
};


///----------------------------------------------------------------------
/// CqParseNodeUnresolvedCall
/// Parsenode specifying an unresolved call., to be handled by a DSO.

class CqParseNodeUnresolvedCall : public CqParseNode, public IqParseNodeUnresolvedCall
{
	public:
		CqParseNodeUnresolvedCall( CqFuncDef& aFuncDef) :
				CqParseNode()
		{
			m_aFuncDef = aFuncDef ;
		}

		virtual	~CqParseNodeUnresolvedCall()
		{}

		// Overridden from IqParseNodeUnresolvedCall

		virtual	const char*	strName() const;
		virtual	const IqFuncDef* pFuncDef() const;
		virtual	IqFuncDef* pFuncDef();

		// Overridden from IqParseNode
		virtual	void*	GetInterface( EqParseNodeType type) const
		{
			void* pNode;
			if ( ( pNode = ( void* ) QueryNodeType<IqParseNodeUnresolvedCall>( this, type ) ) != 0 )
				return ( pNode );
			return ( CqParseNode::GetInterface( type ) );
		}
		virtual	TqInt	NodeType() const
		{
			return ( IqParseNodeUnresolvedCall::m_ID );
		}
		virtual	void	Accept( IqParseNodeVisitor &V)
		{
			V.Visit(static_cast<IqParseNodeUnresolvedCall&>(*this));
		}


		virtual	TqInt	TypeCheck( TqInt* pTypes, TqInt Count, bool& needsCast, bool CheckOnly );
		virtual	bool	Optimise();
		virtual	TqInt	ResType() const;
		virtual	CqParseNode*	Clone( CqParseNode* pParent = 0 )
		{
			CqParseNodeUnresolvedCall * pNew = new CqParseNodeUnresolvedCall( *this );
			if ( m_pChild )
				pNew->m_pChild = m_pChild->Clone( pNew );
			pNew->m_pParent = pParent;
			return ( pNew );
		}
		CqFuncDef&	pFuncDefInt()
		{
			return ( m_aFuncDef );
		}
		//void	CheckArgCast( std::vector<TqInt>& aRes );
		//void	ArgCast( TqInt iIndex );

	protected:
		CqFuncDef	m_aFuncDef;
};

///----------------------------------------------------------------------
/// CqParseNodeVariable
/// Parsenode specifying a variable access.

class CqParseNodeVariable : public CqParseNode, public IqParseNodeVariable
{
	public:
		CqParseNodeVariable( const CqParseNodeVariable& from ) :
				CqParseNode( from ),
				m_VarRef( from.m_VarRef ),
				m_Extra( from.m_Extra )
		{}
		CqParseNodeVariable( SqVarRef VarRef );
		CqParseNodeVariable( CqParseNodeVariable* pVar );
		virtual	~CqParseNodeVariable()
		{}

		// Overridden from IqParseNodeVariable



		virtual	const char*	strName() const;
		virtual	SqVarRef	VarRef() const
		{
			return ( m_VarRef );
		}
		virtual CqString Extra() const
		{
			return ( m_Extra );
		}
		virtual	bool	IsLocal() const
		{
			return ( m_VarRef.m_Type == VarTypeLocal );
		}

		// Overridden from IqParseNode
		virtual	bool	IsVariableRef() const
		{
			return ( true );
		}
		virtual	void*	GetInterface( EqParseNodeType type) const
		{
			void* pNode;
			if ( ( pNode = ( void* ) QueryNodeType<IqParseNodeVariable>( this, type ) ) != 0 )
				return ( pNode );
			return ( CqParseNode::GetInterface( type ) );
		}
		virtual	TqInt	NodeType() const
		{
			return ( IqParseNodeVariable::m_ID );
		}
		virtual	void	Accept( IqParseNodeVisitor &V)
		{
			V.Visit(static_cast<IqParseNodeVariable&>(*this));
		}



		virtual	bool	Optimise();
		virtual	TqInt	TypeCheck( TqInt* pTypes, TqInt Count, bool& needsCast, bool CheckOnly );
		virtual	TqInt	ResType() const;
		virtual	CqParseNode*	Clone( CqParseNode* pParent = 0 )
		{
			CqParseNodeVariable * pNew = new CqParseNodeVariable( *this );
			if ( m_pChild )
				pNew->m_pChild = m_pChild->Clone( pNew );
			pNew->m_pParent = pParent;
			return ( pNew );
		}
		void	SetParam()
		{
			IqVarDef * pVarDef = CqVarDef::GetVariablePtr( m_VarRef );
			if ( pVarDef != 0 )
				pVarDef->SetParam();
		}
		void	SetOutput()
		{
			IqVarDef * pVarDef = CqVarDef::GetVariablePtr( m_VarRef );
			if ( pVarDef != 0 )
				pVarDef->SetOutput();
		}
		void	SetDefaultStorage( TqInt Storage )
		{
			// If a storage method has not been specified, default to the specified type.
			IqVarDef * pVarDef = CqVarDef::GetVariablePtr( m_VarRef );
			if ( pVarDef != 0 )
				pVarDef->SetDefaultStorage( Storage );
		}
		virtual	bool	UpdateStorageStatus()
		{
			return fVarying();
		}

	protected:
		SqVarRef	m_VarRef;
		CqString m_Extra;
};


///----------------------------------------------------------------------
/// CqParseNodeVariableArray
/// Parsenode specifying an array variable access.

class CqParseNodeVariableArray : public CqParseNodeVariable, public IqParseNodeArrayVariable
{
	public:
		CqParseNodeVariableArray( const CqParseNodeVariableArray& from ) :
				CqParseNodeVariable( from )
		{}
		CqParseNodeVariableArray( SqVarRef VarRef );
		CqParseNodeVariableArray( CqParseNodeVariableArray* pVar );
		virtual	~CqParseNodeVariableArray()
		{}

		// Overridden from IqParseNode



		virtual	void*	GetInterface( EqParseNodeType type) const
		{
			void* pNode;
			if ( ( pNode = ( void* ) QueryNodeType<IqParseNodeArrayVariable>( this, type ) ) != 0 )
				return ( pNode );
			return ( CqParseNodeVariable::GetInterface( type ) );
		}
		virtual	TqInt	NodeType() const
		{
			return ( IqParseNodeArrayVariable::m_ID );
		}
		virtual	void	Accept( IqParseNodeVisitor &V)
		{
			V.Visit(static_cast<IqParseNodeArrayVariable&>(*this));
		}


		virtual	TqInt	TypeCheck( TqInt* pTypes, TqInt Count, bool& needsCast, bool CheckOnly );
		virtual	CqParseNode*	Clone( CqParseNode* pParent = 0 )
		{
			CqParseNodeVariableArray * pNew = new CqParseNodeVariableArray( *this );
			if ( m_pChild )
				pNew->m_pChild = m_pChild->Clone( pNew );
			pNew->m_pParent = pParent;
			return ( pNew );
		}
	protected:
};


///----------------------------------------------------------------------
/// CqParseNodeAssign
/// Parsenode specifying an assignment operation.

class CqParseNodeAssign : public CqParseNodeVariable, public IqParseNodeVariableAssign
{
	public:
		CqParseNodeAssign( const CqParseNodeAssign& from ) :
				CqParseNodeVariable( from ),
				m_fNoDup( from.m_fNoDup )
		{}
		CqParseNodeAssign( SqVarRef VarRef ) :
				CqParseNodeVariable( VarRef ),
				m_fNoDup( false )
		{}
		CqParseNodeAssign( CqParseNodeVariable* pVar ) :
				CqParseNodeVariable( pVar )
		{}
		virtual	~CqParseNodeAssign()
		{}

		// Overridden from IqParseNodeVariableAssign



		virtual	bool	fDiscardResult() const
		{
			return ( m_fNoDup );
		}
		// Overridden from IqParseNode
		virtual	void*	GetInterface( EqParseNodeType type) const
		{
			void* pNode;
			if ( ( pNode = ( void* ) QueryNodeType<IqParseNodeVariableAssign>( this, type ) ) != 0 )
				return ( pNode );
			return ( CqParseNodeVariable::GetInterface( type ) );
		}
		virtual	TqInt	NodeType() const
		{
			return ( IqParseNodeVariableAssign::m_ID );
		}
		virtual	void	Accept( IqParseNodeVisitor &V)
		{
			V.Visit(static_cast<IqParseNodeVariableAssign&>(*this));
		}



		virtual	bool	Optimise();
		virtual	TqInt	TypeCheck( TqInt* pTypes, TqInt Count, bool& needsCast, bool CheckOnly );
		virtual	void	NoDup()
		{
			m_fNoDup = true;
		}
		virtual	TqInt	ResType() const;
		virtual	CqParseNode*	Clone( CqParseNode* pParent = 0 )
		{
			CqParseNodeAssign * pNew = new CqParseNodeAssign( *this );
			if ( m_pChild )
				pNew->m_pChild = m_pChild->Clone( pNew );
			pNew->m_pParent = pParent;
			return ( pNew );
		}
		virtual	bool	UpdateStorageStatus()
		{
			// Varying status is a combination of the varying status of all chidren
			// and the varying status of the variable to which we are assigning.
			bool fVarying = CqParseNode::UpdateStorageStatus();
			m_fVarying = false;
			IqVarDef* pVarDef = CqVarDef::GetVariablePtr( m_VarRef );
			if ( pVarDef != 0 )
				m_fVarying = ( pVarDef->Type() & Type_Varying ) != 0;

			m_fVarying = ( ( fVarying ) || ( m_fVarying ) );

			return ( m_fVarying );
		}

	protected:
		bool	m_fNoDup;
};


///----------------------------------------------------------------------
/// CqParseNodeAssignArray
/// Parsenode specifying an assignment operation to an indexed array.

class CqParseNodeAssignArray : public CqParseNodeAssign, public IqParseNodeArrayVariableAssign
{
	public:
		CqParseNodeAssignArray( const CqParseNodeAssignArray& from ) :
				CqParseNodeAssign( from )
		{}
		CqParseNodeAssignArray( SqVarRef VarRef ) :
				CqParseNodeAssign( VarRef )
		{}
		CqParseNodeAssignArray( CqParseNodeVariable* pVar ) :
				CqParseNodeAssign( pVar )
		{}
		virtual	~CqParseNodeAssignArray()
		{}

		// Overridden from IqParseNode



		virtual	void*	GetInterface( EqParseNodeType type) const
		{
			void* pNode;
			if ( ( pNode = ( void* ) QueryNodeType<IqParseNodeArrayVariableAssign>( this, type ) ) != 0 )
				return ( pNode );
			return ( CqParseNodeAssign::GetInterface( type ) );
		}
		virtual	TqInt	NodeType() const
		{
			return ( IqParseNodeArrayVariableAssign::m_ID );
		}
		virtual	void	Accept( IqParseNodeVisitor &V)
		{
			V.Visit(static_cast<IqParseNodeArrayVariableAssign&>(*this));
		}



		virtual	TqInt	TypeCheck( TqInt* pTypes, TqInt Count, bool& needsCast, bool CheckOnly );
		virtual	CqParseNode*	Clone( CqParseNode* pParent = 0 )
		{
			CqParseNodeAssignArray * pNew = new CqParseNodeAssignArray( *this );
			if ( m_pChild )
				pNew->m_pChild = m_pChild->Clone( pNew );
			pNew->m_pParent = pParent;
			return ( pNew );
		}
	private:
};


///----------------------------------------------------------------------
/// CqParseNodeop
/// Base parsenode specifying an operation.

class CqParseNodeOp : public CqParseNode, public IqParseNodeOperator
{
	public:
		CqParseNodeOp( const CqParseNodeOp& from ) :
				CqParseNode( from )
		{}
		CqParseNodeOp() :
				CqParseNode()
		{}
		virtual	~CqParseNodeOp()
		{}

		// Overridden from IqParseNode



		virtual	void*	GetInterface( EqParseNodeType type) const
		{
			void* pNode;
			if ( ( pNode = ( void* ) QueryNodeType<IqParseNodeOperator>( this, type ) ) != 0 )
				return ( pNode );
			return ( CqParseNode::GetInterface( type ) );
		}
		virtual	void	Accept( IqParseNodeVisitor &V)
		{
			V.Visit(static_cast<IqParseNodeOperator&>(*this));
		}

		virtual	TqInt	TypeCheck( TqInt* pTypes, TqInt Count, bool& needsCast, bool CheckOnly );
};


///----------------------------------------------------------------------
/// CqParseNodeMathOp
/// Parsenode specifying a mathematical operation.

class CqParseNodeMathOp : public CqParseNodeOp, public IqParseNodeMathOp
{
	public:
		CqParseNodeMathOp( const CqParseNodeMathOp& from ) :
				CqParseNodeOp( from ),
				m_Operator( from.m_Operator )
		{}
		CqParseNodeMathOp() :
				CqParseNodeOp(),
				m_Operator( Op_Nil )
		{}
		CqParseNodeMathOp( EqMathOp Operator ) :
				CqParseNodeOp(),
				m_Operator( Operator )
		{}
		virtual	~CqParseNodeMathOp()
		{}

		// Overridden from IqParseNodeOperator



		virtual	TqInt	Operator() const
		{
			return ( m_Operator );
		}
		// Overridden from IqParseNode
		virtual	void*	GetInterface( EqParseNodeType type) const
		{
			void* pNode;
			if ( ( pNode = ( void* ) QueryNodeType<IqParseNodeMathOp>( this, type ) ) != 0 )
				return ( pNode );
			return ( CqParseNodeOp::GetInterface( type ) );
		}
		virtual	TqInt	NodeType() const
		{
			return ( IqParseNodeMathOp::m_ID );
		}
		virtual	void	Accept( IqParseNodeVisitor &V)
		{
			V.Visit(static_cast<IqParseNodeMathOp&>(*this));
		}



		virtual	TqInt	ResType() const;
		virtual	CqParseNode*	Clone( CqParseNode* pParent = 0 )
		{
			CqParseNodeMathOp * pNew = new CqParseNodeMathOp( *this );
			if ( m_pChild )
				pNew->m_pChild = m_pChild->Clone( pNew );
			pNew->m_pParent = pParent;
			return ( pNew );
		}

	protected:
		EqMathOp	m_Operator;
};


///----------------------------------------------------------------------
/// CqParseNodeMathOpDot
/// Parsenode specifying a dot (dot product) operation.

class CqParseNodeMathOpDot : public CqParseNodeMathOp
{
	public:
		CqParseNodeMathOpDot( const CqParseNodeMathOpDot& from ) :
				CqParseNodeMathOp( from )
		{}
		CqParseNodeMathOpDot() :
				CqParseNodeMathOp( Op_Dot )
		{}
		virtual	~CqParseNodeMathOpDot()
		{}

		virtual	TqInt	TypeCheck( TqInt* pTypes, TqInt Count, bool& needsCast, bool CheckOnly );
};


///----------------------------------------------------------------------
/// CqParseNodeRelop
/// Parsenode specifying a relational operation.

class CqParseNodeRelOp : public CqParseNodeOp, public IqParseNodeRelationalOp
{
	public:
		CqParseNodeRelOp( const CqParseNodeRelOp& from ) :
				CqParseNodeOp( from ),
				m_Operator( from.m_Operator )
		{}
		CqParseNodeRelOp( EqRelOp Operator ) :
				CqParseNodeOp(),
				m_Operator( Operator )
		{}
		virtual	~CqParseNodeRelOp()
		{}

		// Overridden from IqParseNodeOperator



		virtual	TqInt	Operator() const
		{
			return ( m_Operator );
		}
		// Overridden from IqParseNode
		virtual	void*	GetInterface( EqParseNodeType type) const
		{
			void* pNode;
			if ( ( pNode = ( void* ) QueryNodeType<IqParseNodeRelationalOp>( this, type ) ) != 0 )
				return ( pNode );
			return ( CqParseNodeOp::GetInterface( type ) );
		}
		virtual	TqInt	NodeType() const
		{
			return ( IqParseNodeRelationalOp::m_ID );
		}
		virtual	TqInt	ResType() const
		{
			return ( Type_Float );
		}
		virtual	void	Accept( IqParseNodeVisitor &V)
		{
			V.Visit(static_cast<IqParseNodeRelationalOp&>(*this));
		}



		virtual	CqParseNode*	Clone( CqParseNode* pParent = 0 )
		{
			CqParseNodeRelOp * pNew = new CqParseNodeRelOp( *this );
			if ( m_pChild )
				pNew->m_pChild = m_pChild->Clone( pNew );
			pNew->m_pParent = pParent;
			return ( pNew );
		}
		virtual	TqInt	TypeCheck( TqInt* pTypes, TqInt Count, bool& needsCast, bool CheckOnly );

	protected:
		EqRelOp	m_Operator;
};


///----------------------------------------------------------------------
/// CqParseNodeUnaryop
/// Parsenode specifying a unary operation.

class CqParseNodeUnaryOp : public CqParseNodeOp, public IqParseNodeUnaryOp
{
	public:
		CqParseNodeUnaryOp( const CqParseNodeUnaryOp& from ) :
				CqParseNodeOp( from ),
				m_Operator( from.m_Operator )
		{}
		CqParseNodeUnaryOp( EqUnaryOp Operator ) :
				CqParseNodeOp(),
				m_Operator( Operator )
		{}
		virtual	~CqParseNodeUnaryOp()
		{}

		// Overridden from IqParseNodeOperator



		virtual	TqInt	Operator() const
		{
			return ( m_Operator );
		}
		// Overridden from IqParseNode
		virtual	void*	GetInterface( EqParseNodeType type) const
		{
			void* pNode;
			if ( ( pNode = ( void* ) QueryNodeType<IqParseNodeUnaryOp>( this, type ) ) != 0 )
				return ( pNode );
			return ( CqParseNodeOp::GetInterface( type ) );
		}
		virtual	TqInt	NodeType() const
		{
			return ( IqParseNodeUnaryOp::m_ID );
		}
		virtual	void	Accept( IqParseNodeVisitor &V)
		{
			V.Visit(static_cast<IqParseNodeUnaryOp&>(*this));
		}


		virtual	TqInt	TypeCheck( TqInt* pTypes, TqInt Count, bool& needsCast, bool CheckOnly );
		virtual	CqParseNode*	Clone( CqParseNode* pParent = 0 )
		{
			CqParseNodeUnaryOp * pNew = new CqParseNodeUnaryOp( *this );
			if ( m_pChild )
				pNew->m_pChild = m_pChild->Clone( pNew );
			pNew->m_pParent = pParent;
			return ( pNew );
		}

	protected:
		EqUnaryOp	m_Operator;
};


///----------------------------------------------------------------------
/// CqParseNodeLogicalop
/// Parsenode specifying a logical operation.

class CqParseNodeLogicalOp : public CqParseNodeOp, public IqParseNodeLogicalOp
{
	public:
		CqParseNodeLogicalOp( const CqParseNodeLogicalOp& from ) :
				CqParseNodeOp( from ),
				m_Operator( from.m_Operator )
		{}
		CqParseNodeLogicalOp( EqLogicalOp Operator ) :
				CqParseNodeOp(),
				m_Operator( Operator )
		{}
		virtual	~CqParseNodeLogicalOp()
		{}

		// Overridden from IqParseNodeOperator



		virtual	TqInt	Operator() const
		{
			return ( m_Operator );
		}
		// Overridden from IqParseNode
		virtual	void*	GetInterface( EqParseNodeType type) const
		{
			void* pNode;
			if ( ( pNode = ( void* ) QueryNodeType<IqParseNodeLogicalOp>( this, type ) ) != 0 )
				return ( pNode );
			return ( CqParseNodeOp::GetInterface( type ) );
		}
		virtual	TqInt	NodeType() const
		{
			return ( IqParseNodeLogicalOp::m_ID );
		}
		virtual	TqInt	ResType() const
		{
			return ( Type_Float );
		}
		virtual	void	Accept( IqParseNodeVisitor &V)
		{
			V.Visit(static_cast<IqParseNodeLogicalOp&>(*this));
		}



		virtual	CqParseNode*	Clone( CqParseNode* pParent = 0 )
		{
			CqParseNodeLogicalOp * pNew = new CqParseNodeLogicalOp( *this );
			if ( m_pChild )
				pNew->m_pChild = m_pChild->Clone( pNew );
			pNew->m_pParent = pParent;
			return ( pNew );
		}

	protected:
		EqLogicalOp	m_Operator;
};


///----------------------------------------------------------------------
/// CqParseNodeDrop
/// Parsenode specifying an assignment operation.

class CqParseNodeDrop : public CqParseNode, public IqParseNodeDiscardResult
{
	public:
		CqParseNodeDrop( const CqParseNodeDrop& from ) :
				CqParseNode( from )
		{}
		CqParseNodeDrop() :
				CqParseNode()
		{}
		virtual	~CqParseNodeDrop()
		{}

		// Overridden from IqParseNode



		virtual	void*	GetInterface( EqParseNodeType type) const
		{
			void* pNode;
			if ( ( pNode = ( void* ) QueryNodeType<IqParseNodeDiscardResult>( this, type ) ) != 0 )
				return ( pNode );
			return ( CqParseNode::GetInterface( type ) );
		}
		virtual	TqInt	NodeType() const
		{
			return ( IqParseNodeDiscardResult::m_ID );
		}
		virtual	void	Accept( IqParseNodeVisitor &V)
		{
			V.Visit(static_cast<IqParseNodeDiscardResult&>(*this));
		}


		virtual	CqParseNode*	Clone( CqParseNode* pParent = 0 )
		{
			CqParseNodeDrop * pNew = new CqParseNodeDrop( *this );
			if ( m_pChild )
				pNew->m_pChild = m_pChild->Clone( pNew );
			pNew->m_pParent = pParent;
			return ( pNew );
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
		CqParseNodeConst( const CqParseNodeConst& from ) :
				CqParseNode( from )
		{}
		virtual	~CqParseNodeConst()
		{}

		virtual	TqInt	TypeCheck( TqInt* pTypes, TqInt Count, bool& needsCast, bool CheckOnly );
	private:
};


///----------------------------------------------------------------------
/// CqParseNodeFloatConst
/// Parsenode specifying a float constant value.

class CqParseNodeFloatConst : public CqParseNodeConst, public IqParseNodeConstantFloat
{
	public:
		CqParseNodeFloatConst( const CqParseNodeFloatConst& from ) :
				CqParseNodeConst( from ),
				m_Value( from.m_Value )
		{}
		CqParseNodeFloatConst( TqFloat Val ) :
				CqParseNodeConst(),
				m_Value( Val )
		{}
		virtual	~CqParseNodeFloatConst()
		{}

		// Overridden from IqParseNodeConstantFloat



		virtual	TqFloat	Value() const
		{
			return ( m_Value );
		}
		// Overridden from IqParseNode
		virtual	void*	GetInterface( EqParseNodeType type) const
		{
			void* pNode;
			if ( ( pNode = ( void* ) QueryNodeType<IqParseNodeConstantFloat>( this, type ) ) != 0 )
				return ( pNode );
			return ( CqParseNode::GetInterface( type ) );
		}
		virtual	TqInt	NodeType() const
		{
			return ( IqParseNodeConstantFloat::m_ID );
		}
		virtual	TqInt	ResType() const
		{
			return ( Type_Float );
		}
		virtual	void	Accept( IqParseNodeVisitor &V)
		{
			V.Visit(static_cast<IqParseNodeConstantFloat&>(*this));
		}


		virtual	CqParseNode*	Clone( CqParseNode* pParent = 0 )
		{
			CqParseNodeFloatConst * pNew = new CqParseNodeFloatConst( *this );
			if ( m_pChild )
				pNew->m_pChild = m_pChild->Clone( pNew );
			pNew->m_pParent = pParent;
			return ( pNew );
		}

	private:
		TqFloat	m_Value;
};


///----------------------------------------------------------------------
/// CqParseNodeStringConst
/// Parsenode specifying a string constant value.

class CqParseNodeStringConst : public CqParseNodeConst, public IqParseNodeConstantString
{
	public:
		CqParseNodeStringConst( const CqParseNodeStringConst& from ) :
				CqParseNodeConst( from ),
				m_Value( from.m_Value )
		{}
		CqParseNodeStringConst( const char* Val ) :
				CqParseNodeConst(),
				m_Value( Val )
		{}
		virtual	~CqParseNodeStringConst()
		{}

		// Overridden from IqParseNodeConstantString



		virtual	const char*	strValue() const
		{
			return ( m_Value.c_str() );
		}
		// Overridden from IqParseNode
		virtual	void*	GetInterface( EqParseNodeType type) const
		{
			void* pNode;
			if ( ( pNode = ( void* ) QueryNodeType<IqParseNodeConstantString>( this, type ) ) != 0 )
				return ( pNode );
			return ( CqParseNode::GetInterface( type ) );
		}
		virtual	TqInt	NodeType() const
		{
			return ( IqParseNodeConstantString::m_ID );
		}
		virtual	TqInt	ResType() const
		{
			return ( Type_String );
		}
		virtual	void	Accept( IqParseNodeVisitor &V)
		{
			V.Visit(static_cast<IqParseNodeConstantString&>(*this));
		}



		virtual	CqParseNode*	Clone( CqParseNode* pParent = 0 )
		{
			CqParseNodeStringConst * pNew = new CqParseNodeStringConst( *this );
			if ( m_pChild )
				pNew->m_pChild = m_pChild->Clone( pNew );
			pNew->m_pParent = pParent;
			return ( pNew );
		}

	private:
		CqString	m_Value;
};


///----------------------------------------------------------------------
/// CqParseNodeWhileConstruct
/// Parsenode specifying an assignment operation.

class CqParseNodeWhileConstruct : public CqParseNode, public IqParseNodeWhileConstruct
{
	public:
		CqParseNodeWhileConstruct( const CqParseNodeWhileConstruct& from ) :
				CqParseNode( from )
		{}
		CqParseNodeWhileConstruct() :
				CqParseNode()
		{}
		virtual	~CqParseNodeWhileConstruct()
		{}

		// Overridden fromIqParseNode



		virtual	void*	GetInterface( EqParseNodeType type) const
		{
			void* pNode;
			if ( ( pNode = ( void* ) QueryNodeType<IqParseNodeWhileConstruct>( this, type ) ) != 0 )
				return ( pNode );
			return ( CqParseNode::GetInterface( type ) );
		}
		virtual	TqInt	NodeType() const
		{
			return ( IqParseNodeWhileConstruct::m_ID );
		}
		virtual	void	Accept( IqParseNodeVisitor &V)
		{
			V.Visit(static_cast<IqParseNodeWhileConstruct&>(*this));
		}


		virtual	CqParseNode*	Clone( CqParseNode* pParent = 0 )
		{
			CqParseNodeWhileConstruct * pNew = new CqParseNodeWhileConstruct( *this );
			if ( m_pChild )
				pNew->m_pChild = m_pChild->Clone( pNew );
			pNew->m_pParent = pParent;
			return ( pNew );
		}

	private:
};


///----------------------------------------------------------------------
/// CqParseNodeLoopMod
/// Parsenode specifying a loop modification (break or continue)

class CqParseNodeLoopMod : public CqParseNode, public IqParseNodeLoopMod
{
	public:
		CqParseNodeLoopMod( const CqParseNodeLoopMod& from )
			: CqParseNode( from ),
			m_modType(from.m_modType)
		{}
		CqParseNodeLoopMod(EqLoopModType modType)
			: CqParseNode(),
			m_modType(modType)
		{}
		virtual	~CqParseNodeLoopMod() {}

		virtual EqLoopModType modType()
		{
			return m_modType;
		}

		// Overridden fromIqParseNode

		virtual	void*	GetInterface( EqParseNodeType type) const
		{
			void* pNode;
			if ( ( pNode = ( void* ) QueryNodeType<IqParseNodeLoopMod>( this, type ) ) != 0 )
				return ( pNode );
			return ( CqParseNode::GetInterface( type ) );
		}
		virtual	TqInt	NodeType() const
		{
			return ( IqParseNodeLoopMod::m_ID );
		}
		virtual	void	Accept( IqParseNodeVisitor &V)
		{
			V.Visit(static_cast<IqParseNodeLoopMod&>(*this));
		}

		virtual	CqParseNode*	Clone( CqParseNode* pParent = 0 )
		{
			CqParseNodeLoopMod * pNew = new CqParseNodeLoopMod( *this );
			if ( m_pChild )
				pNew->m_pChild = m_pChild->Clone( pNew );
			pNew->m_pParent = pParent;
			return ( pNew );
		}
	private:
		EqLoopModType m_modType;
};


///----------------------------------------------------------------------
/// CqParseNodeIlluminateConstruct
/// Parsenode an illuminate construct.

class CqParseNodeIlluminateConstruct : public CqParseNode, public IqParseNodeIlluminateConstruct
{
	public:
		CqParseNodeIlluminateConstruct( const CqParseNodeIlluminateConstruct& from ) :
				CqParseNode( from ),
				m_fAxisAngle( from.m_fAxisAngle )
		{}
		CqParseNodeIlluminateConstruct( bool fAxisAngle = false ) :
				CqParseNode(),
				m_fAxisAngle( fAxisAngle )
		{}
		virtual	~CqParseNodeIlluminateConstruct()
		{}

		// Overridden from IqParseNode



		virtual	bool	fHasAxisAngle() const
		{
			return ( m_fAxisAngle );
		}
		// Overridden from IqParseNode
		virtual	void*	GetInterface( EqParseNodeType type) const
		{
			void* pNode;
			if ( ( pNode = ( void* ) QueryNodeType<IqParseNodeIlluminateConstruct>( this, type ) ) != 0 )
				return ( pNode );
			return ( CqParseNode::GetInterface( type ) );
		}
		virtual	TqInt	NodeType() const
		{
			return ( IqParseNodeIlluminateConstruct::m_ID );
		}
		virtual	void	Accept( IqParseNodeVisitor &V)
		{
			V.Visit(static_cast<IqParseNodeIlluminateConstruct&>(*this));
		}


		virtual	CqParseNode*	Clone( CqParseNode* pParent = 0 )
		{
			CqParseNodeIlluminateConstruct * pNew = new CqParseNodeIlluminateConstruct( *this );
			if ( m_pChild )
				pNew->m_pChild = m_pChild->Clone( pNew );
			pNew->m_pParent = pParent;
			return ( pNew );
		}

	private:
		bool	m_fAxisAngle;
};


///----------------------------------------------------------------------
/// CqParseNodeIlluminanceConstruct
/// Parsenode an illuminance construct.

class CqParseNodeIlluminanceConstruct : public CqParseNode, public IqParseNodeIlluminanceConstruct
{
	public:
		CqParseNodeIlluminanceConstruct( const CqParseNodeIlluminanceConstruct& from ) :
				CqParseNode( from ),
				m_fAxisAngle( from.m_fAxisAngle )
		{}
		CqParseNodeIlluminanceConstruct( bool fAxisAngle = false ) :
				CqParseNode(),
				m_fAxisAngle( fAxisAngle )
		{}
		virtual	~CqParseNodeIlluminanceConstruct()
		{}

		// Overridden from IqParseNode



		virtual	bool	fHasAxisAngle() const
		{
			return ( m_fAxisAngle );
		}
		// Overridden from IqParseNode
		virtual	void*	GetInterface( EqParseNodeType type) const
		{
			void* pNode;
			if ( ( pNode = ( void* ) QueryNodeType<IqParseNodeIlluminanceConstruct>( this, type ) ) != 0 )
				return ( pNode );
			return ( CqParseNode::GetInterface( type ) );
		}
		virtual	TqInt	NodeType() const
		{
			return ( IqParseNodeIlluminanceConstruct::m_ID );
		}
		virtual	void	Accept( IqParseNodeVisitor &V)
		{
			V.Visit(static_cast<IqParseNodeIlluminanceConstruct&>(*this));
		}


		virtual	CqParseNode*	Clone( CqParseNode* pParent = 0 )
		{
			CqParseNodeIlluminanceConstruct * pNew = new CqParseNodeIlluminanceConstruct( *this );
			if ( m_pChild )
				pNew->m_pChild = m_pChild->Clone( pNew );
			pNew->m_pParent = pParent;
			return ( pNew );
		}

	private:
		bool	m_fAxisAngle;
};


///----------------------------------------------------------------------
/// CqParseNodeSolarConstruct
/// Parsenode an solar construct.

class CqParseNodeSolarConstruct : public CqParseNode, public IqParseNodeSolarConstruct
{
	public:
		CqParseNodeSolarConstruct( const CqParseNodeSolarConstruct& from ) :
				CqParseNode( from ),
				m_fAxisAngle( from.m_fAxisAngle )
		{}
		CqParseNodeSolarConstruct( bool fAxisAngle = false ) :
				CqParseNode(),
				m_fAxisAngle( fAxisAngle )
		{}
		virtual	~CqParseNodeSolarConstruct()
		{}

		// Overridden from IqParseNode



		virtual	bool	fHasAxisAngle() const
		{
			return ( m_fAxisAngle );
		}
		// Overridden from IqParseNode
		virtual	void*	GetInterface( EqParseNodeType type) const
		{
			void* pNode;
			if ( ( pNode = ( void* ) QueryNodeType<IqParseNodeSolarConstruct>( this, type ) ) != 0 )
				return ( pNode );
			return ( CqParseNode::GetInterface( type ) );
		}
		virtual	TqInt	NodeType() const
		{
			return ( IqParseNodeSolarConstruct::m_ID );
		}
		virtual	void	Accept( IqParseNodeVisitor &V)
		{
			V.Visit(static_cast<IqParseNodeSolarConstruct&>(*this));
		}


		virtual	CqParseNode*	Clone( CqParseNode* pParent = 0 )
		{
			CqParseNodeSolarConstruct * pNew = new CqParseNodeSolarConstruct( *this );
			if ( m_pChild )
				pNew->m_pChild = m_pChild->Clone( pNew );
			pNew->m_pParent = pParent;
			return ( pNew );
		}

	private:
		bool	m_fAxisAngle;
};


///----------------------------------------------------------------------
/// CqParseNodeGatherConstruct
/// Parsenode a gather construct.

class CqParseNodeGatherConstruct : public CqParseNode, public IqParseNodeGatherConstruct
{
	public:
		CqParseNodeGatherConstruct( const CqParseNodeGatherConstruct& from ) :
				CqParseNode( from )
		{}
		CqParseNodeGatherConstruct( bool fAxisAngle = false ) :
				CqParseNode()
		{}
		virtual	~CqParseNodeGatherConstruct()
		{}

		// Overridden from IqParseNode

		virtual	void*	GetInterface( EqParseNodeType type) const
		{
			void* pNode;
			if ( ( pNode = ( void* ) QueryNodeType<IqParseNodeGatherConstruct>( this, type ) ) != 0 )
				return ( pNode );
			return ( CqParseNode::GetInterface( type ) );
		}
		virtual	TqInt	NodeType() const
		{
			return ( IqParseNodeGatherConstruct::m_ID );
		}
		virtual	void	Accept( IqParseNodeVisitor &V)
		{
			V.Visit(static_cast<IqParseNodeGatherConstruct&>(*this));
		}


		virtual	CqParseNode*	Clone( CqParseNode* pParent = 0 )
		{
			CqParseNodeGatherConstruct * pNew = new CqParseNodeGatherConstruct( *this );
			if ( m_pChild )
				pNew->m_pChild = m_pChild->Clone( pNew );
			pNew->m_pParent = pParent;
			return ( pNew );
		}

	private:
};


///----------------------------------------------------------------------
/// CqParseNodeConditional
/// Parsenode specifying an assignment operation.

class CqParseNodeConditional : public CqParseNode, public IqParseNodeConditional
{
	public:
		CqParseNodeConditional( const CqParseNodeConditional& from ) :
				CqParseNode( from )
		{}
		CqParseNodeConditional() :
				CqParseNode()
		{}
		virtual	~CqParseNodeConditional()
		{}


		// Overridden from IqParseNode



		virtual	void*	GetInterface( EqParseNodeType type) const
		{
			void* pNode;
			if ( ( pNode = ( void* ) QueryNodeType<IqParseNodeConditional>( this, type ) ) != 0 )
				return ( pNode );
			return ( CqParseNode::GetInterface( type ) );
		}
		virtual	TqInt	NodeType() const
		{
			return ( IqParseNodeConditional::m_ID );
		}
		virtual	void	Accept( IqParseNodeVisitor &V)
		{
			V.Visit(static_cast<IqParseNodeConditional&>(*this));
		}


		virtual	CqParseNode*	Clone( CqParseNode* pParent = 0 )
		{
			CqParseNodeConditional * pNew = new CqParseNodeConditional( *this );
			if ( m_pChild )
				pNew->m_pChild = m_pChild->Clone( pNew );
			pNew->m_pParent = pParent;
			return ( pNew );
		}
		virtual	TqInt	TypeCheck( TqInt* pTypes, TqInt Count, bool& needsCast, bool CheckOnly )
		{
			TqInt Types = Type_Float;
			TqInt NewType = Type_Nil;
			CqParseNode* pChild = pFirstChild();

			// Typecheck the conditional
			pChild->TypeCheck( &Types, 1, needsCast, CheckOnly );

			// Now typecheck the conditional statements
			pChild = pChild->pNext();
			while ( pChild != 0 )
			{
				// Get the next pointer nowm, incase the TypeCheck inserts a Cast operator.
				CqParseNode * pNext = pChild->pNext();
				pChild->TypeCheck( pAllTypes(), Type_Last - 1, needsCast, CheckOnly );
				pChild = pNext;
			}
			return ( NewType );
		}

	private:
};

///----------------------------------------------------------------------
/// CqParseNodeQCond
/// Parsenode specifying ? conditional.

class CqParseNodeQCond : public CqParseNode, public IqParseNodeConditionalExpression
{
	public:
		CqParseNodeQCond( const CqParseNodeQCond& from ) :
				CqParseNode( from )
		{}
		CqParseNodeQCond() :
				CqParseNode()
		{}
		virtual	~CqParseNodeQCond()
		{}

		// Overridden from IqParseNode



		virtual	void*	GetInterface( EqParseNodeType type) const
		{
			void* pNode;
			if ( ( pNode = ( void* ) QueryNodeType<IqParseNodeConditionalExpression>( this, type ) ) != 0 )
				return ( pNode );
			return ( CqParseNode::GetInterface( type ) );
		}
		virtual	TqInt	NodeType() const
		{
			return ( IqParseNodeConditionalExpression::m_ID );
		}
		virtual	void	Accept( IqParseNodeVisitor &V)
		{
			V.Visit(static_cast<IqParseNodeConditionalExpression&>(*this));
		}


		virtual	TqInt	TypeCheck( TqInt* pTypes, TqInt Count, bool& needsCast, bool CheckOnly );
		virtual	CqParseNode*	Clone( CqParseNode* pParent = 0 )
		{
			CqParseNodeQCond * pNew = new CqParseNodeQCond( *this );
			if ( m_pChild )
				pNew->m_pChild = m_pChild->Clone( pNew );
			pNew->m_pParent = pParent;
			return ( pNew );
		}

	private:
};



///----------------------------------------------------------------------
/// CqParseNodeCast
/// Parsenode specifying type cast operation.

class CqParseNodeCast : public CqParseNode, public IqParseNodeTypeCast
{
	public:
		CqParseNodeCast( const CqParseNodeCast& from ) :
				CqParseNode( from ),
				m_tTo( from.m_tTo )
		{}
		CqParseNodeCast( TqInt tto ) :
				CqParseNode(),
				m_tTo( tto )
		{}
		virtual	~CqParseNodeCast()
		{}


		// Overridden from IqParseNodeTypeCast



		virtual	TqInt	CastTo() const
		{
			return ( m_tTo );
		}
		// Overridden from IqParseNode
		virtual	void*	GetInterface( EqParseNodeType type) const
		{
			void* pNode;
			if ( ( pNode = ( void* ) QueryNodeType<IqParseNodeTypeCast>( this, type ) ) != 0 )
				return ( pNode );
			return ( CqParseNode::GetInterface( type ) );
		}
		virtual	TqInt	NodeType() const
		{
			return ( IqParseNodeTypeCast::m_ID );
		}
		virtual	TqInt	ResType() const
		{
			return ( m_tTo );
		}
		virtual	void	Accept( IqParseNodeVisitor &V)
		{
			V.Visit(static_cast<IqParseNodeTypeCast&>(*this));
		}



		virtual	bool	Optimise();
		virtual	TqInt	TypeCheck( TqInt* pTypes, TqInt Count, bool& needsCast, bool CheckOnly );
		virtual	CqParseNode*	Clone( CqParseNode* pParent = 0 )
		{
			CqParseNodeCast * pNew = new CqParseNodeCast( *this );
			if ( m_pChild )
				pNew->m_pChild = m_pChild->Clone( pNew );
			pNew->m_pParent = pParent;
			return ( pNew );
		}
	private:
		TqInt	m_tTo;
};


///----------------------------------------------------------------------
/// CqParseNodeTriple
/// Parsenode specifying a float constant value.

class CqParseNodeTriple : public CqParseNode, public IqParseNodeTriple
{
	public:
		CqParseNodeTriple( const CqParseNodeTriple& from ) :
				CqParseNode( from )
		{}
		CqParseNodeTriple() :
				CqParseNode()
		{}
		virtual	~CqParseNodeTriple()
		{}

		// Overridden from IqParseNode



		virtual	void*	GetInterface( EqParseNodeType type) const
		{
			void* pNode;
			if ( ( pNode = ( void* ) QueryNodeType<IqParseNodeTriple>( this, type ) ) != 0 )
				return ( pNode );
			return ( CqParseNode::GetInterface( type ) );
		}
		virtual	TqInt	NodeType() const
		{
			return ( IqParseNodeTriple::m_ID );
		}
		virtual	TqInt	ResType() const
		{
			return ( Type_Triple );
		}
		virtual	void	Accept( IqParseNodeVisitor &V)
		{
			V.Visit(static_cast<IqParseNodeTriple&>(*this));
		}


		virtual	TqInt	TypeCheck( TqInt* pTypes, TqInt Count, bool& needsCast, bool CheckOnly );
		virtual	CqParseNode*	Clone( CqParseNode* pParent = 0 )
		{
			CqParseNodeTriple * pNew = new CqParseNodeTriple( *this );
			if ( m_pChild )
				pNew->m_pChild = m_pChild->Clone( pNew );
			pNew->m_pParent = pParent;
			return ( pNew );
		}

	private:
};


///----------------------------------------------------------------------
/// CqParseNodeHexTuple
/// Parsenode specifying a matrix as 16 float values.

class CqParseNodeHexTuple : public CqParseNode, public IqParseNodeSixteenTuple
{
	public:
		CqParseNodeHexTuple( const CqParseNodeHexTuple& from ) :
				CqParseNode( from )
		{}
		CqParseNodeHexTuple() :
				CqParseNode()
		{}
		virtual	~CqParseNodeHexTuple()
		{}

		// Overridden from IqParseNode



		virtual	void*	GetInterface( EqParseNodeType type) const
		{
			void* pNode;
			if ( ( pNode = ( void* ) QueryNodeType<IqParseNodeSixteenTuple>( this, type ) ) != 0 )
				return ( pNode );
			return ( CqParseNode::GetInterface( type ) );
		}
		virtual	TqInt	NodeType() const
		{
			return ( IqParseNodeSixteenTuple::m_ID );
		}
		virtual	TqInt	ResType() const
		{
			return ( Type_HexTuple );
		}
		virtual	void	Accept( IqParseNodeVisitor &V)
		{
			V.Visit(static_cast<IqParseNodeSixteenTuple&>(*this));
		}


		virtual	TqInt	TypeCheck( TqInt* pTypes, TqInt Count, bool& needsCast, bool CheckOnly );
		virtual	CqParseNode*	Clone( CqParseNode* pParent = 0 )
		{
			CqParseNodeHexTuple * pNew = new CqParseNodeHexTuple( *this );
			if ( m_pChild )
				pNew->m_pChild = m_pChild->Clone( pNew );
			pNew->m_pParent = pParent;
			return ( pNew );
		}

	private:
};


///----------------------------------------------------------------------
/// CqParseNodeCommFunction
/// Parsenode specifying a special comm function.

class CqParseNodeCommFunction : public CqParseNode, public IqParseNodeMessagePassingFunction
{
	public:
		CqParseNodeCommFunction( const CqParseNodeCommFunction& from ) :
				CqParseNode( from ),
				m_vrVariable( from.m_vrVariable ),
				m_vrExtra( from.m_vrExtra ),
				m_commType( from.m_commType )
		{}
		CqParseNodeCommFunction( TqInt Type, CqString vrExtra, SqVarRef vrVariable ) :
				CqParseNode(),
				m_vrVariable( vrVariable ),
				m_vrExtra( vrExtra ),
				m_commType( Type )
		{}
		CqParseNodeCommFunction( TqInt Type, SqVarRef vrVariable ) :
				CqParseNode(),
				m_vrVariable( vrVariable ),
				m_vrExtra( "" ),
				m_commType( Type )
		{}

		virtual	~CqParseNodeCommFunction()
		{}

		// Overridden from IqParseNodeMessagePassingFunction



		virtual	SqVarRef	VarRef() const
		{
			return ( m_vrVariable );
		}
		virtual CqString Extra() const
		{
			return ( m_vrExtra );
		}
		virtual	TqInt	CommType() const
		{
			return ( m_commType );
		}
		// Overridden from IqParseNode
		virtual	void*	GetInterface( EqParseNodeType type) const
		{
			void* pNode;
			if ( ( pNode = ( void* ) QueryNodeType<IqParseNodeMessagePassingFunction>( this, type ) ) != 0 )
				return ( pNode );
			return ( CqParseNode::GetInterface( type ) );
		}
		virtual	TqInt	NodeType() const
		{
			return ( IqParseNodeMessagePassingFunction::m_ID );
		}
		virtual	TqInt	ResType() const
		{
			return ( Type_Float );
		}
		virtual	void	Accept( IqParseNodeVisitor &V)
		{
			V.Visit(static_cast<IqParseNodeMessagePassingFunction&>(*this));
		}


		virtual	TqInt	TypeCheck( TqInt* pTypes, TqInt Count, bool& needsCast, bool CheckOnly );
		virtual	CqParseNode*	Clone( CqParseNode* pParent = 0 )
		{
			CqParseNodeCommFunction * pNew = new CqParseNodeCommFunction( *this );
			if ( m_pChild )
				pNew->m_pChild = m_pChild->Clone( pNew );
			pNew->m_pParent = pParent;
			return ( pNew );
		}

	private:
		SqVarRef	m_vrVariable;
		CqString m_vrExtra;
		TqInt	m_commType;
};


///----------------------------------------------------------------------
/// CqParseDeclaration
/// Parsenode specifying a variable declaration.

class CqParseNodeDeclaration : public CqParseNode
{
	public:
		CqParseNodeDeclaration( const CqParseNodeDeclaration& from ) :
				CqParseNode( from ),
				m_strName( from.m_strName ),
				m_Type( from.m_Type ),
				m_Output( from.m_Output )
		{}
		CqParseNodeDeclaration( const char* strName = "", TqInt Type = Type_Nil ) :
				CqParseNode(),
				m_strName( strName ),
				m_Type( Type ),
				m_Output( false )
		{
			m_fVarying = ( m_Type & Type_Varying ) != 0;
		}
		virtual	~CqParseNodeDeclaration()
		{}
		const char*	strName()
		{
			return ( m_strName.c_str() );
		}
		TqInt	Type() const
		{
			return ( m_Type );
		}
		void	SetType( TqInt Type )
		{
			m_Type = Type;
		}
		bool	Output() const
		{
			return ( m_Output );
		}
		void	SetOutput( bool Output )
		{
			m_Output = Output;
		}
		virtual	void	Accept( IqParseNodeVisitor &V)
		{
			V.Visit(*this);
		}



		virtual	CqParseNode*	Clone( CqParseNode* pParent = 0 )
		{
			CqParseNodeDeclaration * pNew = new CqParseNodeDeclaration( *this );
			if ( m_pChild )
				pNew->m_pChild = m_pChild->Clone( pNew );
			pNew->m_pParent = pParent;
			return ( pNew );
		}
	protected:
		CqString	m_strName;
		TqInt	m_Type;
		bool	m_Output;
};

///----------------------------------------------------------------------
/// CqParseNodeTextureNameWithChannel
/// Parsenode specifying a texture name including channel number.

class CqParseNodeTextureNameWithChannel : public CqParseNode
{
	public:
		CqParseNodeTextureNameWithChannel( const CqParseNodeTextureNameWithChannel& from ) :
				CqParseNode( from )
		{}
		CqParseNodeTextureNameWithChannel() :
				CqParseNode()
		{ }
		virtual	~CqParseNodeTextureNameWithChannel()
		{}

		virtual	TqInt	NodeType() const
		{
			return ( IqParseNodeTextureNameWithChannel::m_ID );
		}
		virtual	TqInt	ResType() const
		{
		// For typechecking purposes, return the type of the texture name expression
			if ( m_pChild == 0 )
				return ( Type_Nil );
			else
				return ( m_pChild->ResType() );
		}
		virtual	void	Accept( IqParseNodeVisitor &V)
		{
			V.Visit(*this);
		}

		virtual	CqParseNode*	Clone( CqParseNode* pParent = 0 )
		{
			CqParseNodeTextureNameWithChannel * pNew = new CqParseNodeTextureNameWithChannel( *this );
			if ( m_pChild )
				pNew->m_pChild = m_pChild->Clone( pNew );
			pNew->m_pParent = pParent;
			return ( pNew );
		}
	protected:
};


//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	// !PARSENODE_H_INCLUDED
