// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


/** \file
		\brief Interface to a parse tree, used by external backends to output.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef IPARSENODE_H_INCLUDED
#define IPARSENODE_H_INCLUDED 1

#include	<iostream>

#include	<aqsis/aqsis.h>
#include	<aqsis/slcomp/ifuncdef.h>
#include	<aqsis/slcomp/ivardef.h>
#include	<aqsis/util/sstring.h>

namespace Aqsis {

enum EqMathOp
{
    Op_Nil = 0,

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
    Op_EQ = 100,
    Op_NE,
    Op_L,
    Op_G,
    Op_GE,
    Op_LE,
};


enum EqUnaryOp
{
    Op_Plus = 200,
    Op_Neg,
    Op_BitwiseComplement,
    Op_LogicalNot,
};


enum EqLogicalOp
{
    Op_LogAnd = 300,
    Op_LogOr,
};

enum EqTextureType
{
    Type_Texture = 0,
    Type_Environment,
    Type_Bump,
    Type_Shadow,
};

enum EqCommType
{
    CommTypeAtmosphere = 0,
    CommTypeDisplacement,
    CommTypeLightsource,
    CommTypeSurface,
    CommTypeAttribute,
    CommTypeOption,
    CommTypeRendererInfo,
    CommTypeIncident,
    CommTypeOpposite,
    CommTypeTextureInfo,
    CommTypeRayInfo,
};

enum EqLoopModType
{
	LoopMod_Break,
	LoopMod_Continue,
};

enum EqParseNodeType
{
    ParseNode_Base = 0,
    ParseNode_Shader,
    ParseNode_FunctionCall,
    ParseNode_UnresolvedCall,
    ParseNode_Variable,
    ParseNode_ArrayVariable,
    ParseNode_VariableAssign,
    ParseNode_ArrayVariableAssign,
    ParseNode_Operator,
    ParseNode_MathOp,
    ParseNode_RelationalOp,
    ParseNode_UnaryOp,
    ParseNode_LogicalOp,
    ParseNode_DiscardResult,
    ParseNode_ConstantFloat,
    ParseNode_ConstantString,
    ParseNode_WhileConstruct,
    ParseNode_LoopMod,
    ParseNode_IlluminateConstruct,
    ParseNode_IlluminanceConstruct,
    ParseNode_SolarConstruct,
    ParseNode_GatherConstruct,
    ParseNode_Conditional,
    ParseNode_ConditionalExpression,
    ParseNode_TypeCast,
    ParseNode_Triple,
    ParseNode_SixteenTuple,
    ParseNode_MessagePassingFunction,
    ParseNode_TextureNameWithChannel,
};


struct IqParseNode;
struct IqParseNodeShader;
struct IqParseNodeFunctionCall;
struct IqParseNodeUnresolvedCall;
struct IqParseNodeVariable;
struct IqParseNodeArrayVariable;
struct IqParseNodeVariableAssign;
struct IqParseNodeArrayVariableAssign;
struct IqParseNodeOperator;
struct IqParseNodeMathOp;
struct IqParseNodeRelationalOp;
struct IqParseNodeUnaryOp;
struct IqParseNodeLogicalOp;
struct IqParseNodeDiscardResult;
struct IqParseNodeConstantFloat;
struct IqParseNodeConstantString;
struct IqParseNodeWhileConstruct;
struct IqParseNodeLoopMod;
struct IqParseNodeIlluminateConstruct;
struct IqParseNodeIlluminanceConstruct;
struct IqParseNodeSolarConstruct;
struct IqParseNodeGatherConstruct;
struct IqParseNodeConditional;
struct IqParseNodeConditionalExpression;
struct IqParseNodeTypeCast;
struct IqParseNodeTriple;
struct IqParseNodeSixteenTuple;
struct IqParseNodeMessagePassingFunction;
struct IqParseNodeTextureNameWithChannel;


struct IqParseNodeVisitor
{
	virtual	void Visit( IqParseNode& ) = 0;
	virtual	void Visit( IqParseNodeShader& ) = 0;
	virtual	void Visit( IqParseNodeFunctionCall& ) = 0;
	virtual	void Visit( IqParseNodeUnresolvedCall& ) = 0;
	virtual	void Visit( IqParseNodeVariable& ) = 0;
	virtual	void Visit( IqParseNodeArrayVariable& ) = 0;
	virtual	void Visit( IqParseNodeVariableAssign& ) = 0;
	virtual	void Visit( IqParseNodeArrayVariableAssign& ) = 0;
	virtual	void Visit( IqParseNodeOperator& ) = 0;
	virtual	void Visit( IqParseNodeMathOp& ) = 0;
	virtual	void Visit( IqParseNodeRelationalOp& ) = 0;
	virtual	void Visit( IqParseNodeUnaryOp& ) = 0;
	virtual	void Visit( IqParseNodeLogicalOp& ) = 0;
	virtual	void Visit( IqParseNodeDiscardResult& ) = 0;
	virtual	void Visit( IqParseNodeConstantFloat& ) = 0;
	virtual	void Visit( IqParseNodeConstantString& ) = 0;
	virtual	void Visit( IqParseNodeWhileConstruct& ) = 0;
	virtual	void Visit( IqParseNodeLoopMod& ) = 0;
	virtual	void Visit( IqParseNodeIlluminateConstruct& ) = 0;
	virtual	void Visit( IqParseNodeIlluminanceConstruct& ) = 0;
	virtual	void Visit( IqParseNodeSolarConstruct& ) = 0;
	virtual	void Visit( IqParseNodeGatherConstruct& ) = 0;
	virtual	void Visit( IqParseNodeConditional& ) = 0;
	virtual	void Visit( IqParseNodeConditionalExpression& ) = 0;
	virtual	void Visit( IqParseNodeTypeCast& ) = 0;
	virtual	void Visit( IqParseNodeTriple& ) = 0;
	virtual	void Visit( IqParseNodeSixteenTuple& ) = 0;
	virtual	void Visit( IqParseNodeMessagePassingFunction& ) = 0;
	virtual	void Visit( IqParseNodeTextureNameWithChannel& ) = 0;
	virtual ~IqParseNodeVisitor()
	{
	};
};

template <class T>
const T* const	QueryNodeType( const T* const pNode, EqParseNodeType type )
{
	if ( T::m_ID == type )
		return ( pNode );
	else
		return ( 0 );
}


struct IqParseNode
{
	virtual	IqParseNode* pChild() const = 0;
	virtual	IqParseNode* pParent() const = 0;
	virtual	IqParseNode* pNextSibling() const = 0;
	virtual	IqParseNode* pPrevSibling() const = 0;
	virtual	TqInt	LineNo() const = 0;
	virtual	const char*	strFileName() const = 0;
	virtual	bool	IsVariableRef() const = 0;
	virtual	TqInt	ResType() const = 0;
	virtual	bool	fVarying() const = 0;

	virtual	void*	GetInterface( EqParseNodeType type) const = 0;
	virtual	TqInt	NodeType() const = 0;

	virtual	void Accept( IqParseNodeVisitor& V ) = 0;

	const static EqParseNodeType m_ID;

	virtual ~IqParseNode()
	{
	};
};



struct IqParseNodeShader
{
	virtual	const char*	strName() const = 0;
	virtual	const char*	strShaderType() const = 0;
	virtual	const EqShaderType ShaderType() const = 0;
	virtual	void*	GetInterface( EqParseNodeType type) const = 0;
	virtual	TqInt	NodeType() const = 0;

	virtual	void Accept( IqParseNodeVisitor& V ) = 0;

	const static EqParseNodeType m_ID;

	virtual ~IqParseNodeShader()
	{
	};
};


struct IqParseNodeFunctionCall
{
	virtual	const char*	strName() const = 0;
	virtual	const IqFuncDef* pFuncDef() const = 0;
	virtual	IqFuncDef* pFuncDef() = 0;
	virtual	void*	GetInterface( EqParseNodeType type) const = 0;
	virtual	TqInt	NodeType() const = 0;

	virtual	void Accept( IqParseNodeVisitor& V ) = 0;

	const static EqParseNodeType m_ID;

	virtual ~IqParseNodeFunctionCall()
	{
	};
};


struct IqParseNodeUnresolvedCall
{
	virtual	const char*	strName() const = 0;
	virtual	const IqFuncDef* pFuncDef() const = 0;
	virtual	IqFuncDef* pFuncDef() = 0;
	virtual	void*	GetInterface( EqParseNodeType type) const = 0;
	virtual	TqInt	NodeType() const = 0;

	virtual	void Accept( IqParseNodeVisitor& V ) = 0;

	const static EqParseNodeType m_ID;

	virtual ~IqParseNodeUnresolvedCall()
	{
	};
};


struct IqParseNodeVariable
{
	virtual	const char*	strName() const = 0;
	virtual	SqVarRef	VarRef() const = 0;
	virtual	bool	IsLocal() const = 0;
	virtual	void*	GetInterface( EqParseNodeType type) const = 0;
	virtual	TqInt	NodeType() const = 0;

	virtual	void Accept( IqParseNodeVisitor& V ) = 0;

	const static EqParseNodeType m_ID;

	virtual ~IqParseNodeVariable()
	{
	};
};


struct IqParseNodeArrayVariable
{
	virtual	void Accept( IqParseNodeVisitor& V ) = 0;
	virtual	void*	GetInterface( EqParseNodeType type) const = 0;
	virtual	TqInt	NodeType() const = 0;

	const static EqParseNodeType m_ID;

	virtual ~IqParseNodeArrayVariable()
	{
	};
};


struct IqParseNodeVariableAssign
{
	virtual	bool fDiscardResult() const = 0;
	virtual	void*	GetInterface( EqParseNodeType type) const = 0;
	virtual	TqInt	NodeType() const = 0;

	virtual	void Accept( IqParseNodeVisitor& V ) = 0;

	const static EqParseNodeType m_ID;

	virtual ~IqParseNodeVariableAssign()
	{
	};
};


struct IqParseNodeArrayVariableAssign
{
	virtual	void*	GetInterface( EqParseNodeType type) const = 0;
	virtual	TqInt	NodeType() const = 0;

	virtual	void Accept( IqParseNodeVisitor& V ) = 0;

	const static EqParseNodeType m_ID;

	virtual ~IqParseNodeArrayVariableAssign()
	{
	};
};


struct IqParseNodeOperator
{
	virtual	TqInt	Operator() const = 0;
	virtual	void*	GetInterface( EqParseNodeType type) const = 0;
	virtual	TqInt	NodeType() const = 0;

	virtual	void Accept( IqParseNodeVisitor& V ) = 0;

	const static EqParseNodeType m_ID;

	virtual ~IqParseNodeOperator()
	{
	};
};


struct IqParseNodeMathOp
{
	virtual	void*	GetInterface( EqParseNodeType type) const = 0;
	virtual	TqInt	NodeType() const = 0;

	virtual	void Accept( IqParseNodeVisitor& V ) = 0;

	const static EqParseNodeType m_ID;

	virtual ~IqParseNodeMathOp()
	{
	};
};


struct IqParseNodeRelationalOp
{
	virtual	void*	GetInterface( EqParseNodeType type) const = 0;
	virtual	TqInt	NodeType() const = 0;

	virtual	void Accept( IqParseNodeVisitor& V ) = 0;

	const static EqParseNodeType m_ID;

	virtual ~IqParseNodeRelationalOp()
	{
	};
};


struct IqParseNodeUnaryOp
{
	virtual	void*	GetInterface( EqParseNodeType type) const = 0;
	virtual	TqInt	NodeType() const = 0;

	virtual	void Accept( IqParseNodeVisitor& V ) = 0;

	const static EqParseNodeType m_ID;

	virtual ~IqParseNodeUnaryOp()
	{
	};
};


struct IqParseNodeLogicalOp
{
	virtual	void*	GetInterface( EqParseNodeType type) const = 0;
	virtual	TqInt	NodeType() const = 0;

	virtual	void Accept( IqParseNodeVisitor& V ) = 0;

	const static EqParseNodeType m_ID;

	virtual ~IqParseNodeLogicalOp()
	{
	};
};

struct IqParseNodeDiscardResult
{
	virtual	void*	GetInterface( EqParseNodeType type) const = 0;
	virtual	TqInt	NodeType() const = 0;

	virtual	void Accept( IqParseNodeVisitor& V ) = 0;

	const static EqParseNodeType m_ID;

	virtual ~IqParseNodeDiscardResult()
	{
	};
};

struct IqParseNodeConstantFloat
{
	virtual TqFloat Value() const = 0;
	virtual	void*	GetInterface( EqParseNodeType type) const = 0;
	virtual	TqInt	NodeType() const = 0;

	virtual	void Accept( IqParseNodeVisitor& V ) = 0;

	const static EqParseNodeType m_ID;

	virtual ~IqParseNodeConstantFloat()
	{
	};
};

struct IqParseNodeConstantString
{
	virtual	const char* strValue() const = 0;
	virtual	void*	GetInterface( EqParseNodeType type) const = 0;
	virtual	TqInt	NodeType() const = 0;

	virtual	void Accept( IqParseNodeVisitor& V ) = 0;

	const static EqParseNodeType m_ID;

	virtual ~IqParseNodeConstantString()
	{
	};
};


struct IqParseNodeWhileConstruct
{
	virtual	void*	GetInterface( EqParseNodeType type) const = 0;
	virtual	TqInt	NodeType() const = 0;

	virtual	void Accept( IqParseNodeVisitor& V ) = 0;

	const static EqParseNodeType m_ID;

	virtual ~IqParseNodeWhileConstruct()
	{
	};
};


struct IqParseNodeLoopMod
{
	virtual EqLoopModType modType() = 0;
	virtual	void*	GetInterface( EqParseNodeType type) const = 0;
	virtual	TqInt	NodeType() const = 0;

	virtual	void Accept( IqParseNodeVisitor& V ) = 0;

	const static EqParseNodeType m_ID;

	virtual ~IqParseNodeLoopMod()
	{
	};
};


struct IqParseNodeIlluminateConstruct
{
	virtual	bool	fHasAxisAngle() const = 0;
	virtual	void*	GetInterface( EqParseNodeType type) const = 0;
	virtual	TqInt	NodeType() const = 0;

	virtual	void Accept( IqParseNodeVisitor& V ) = 0;

	const static EqParseNodeType m_ID;

	virtual ~IqParseNodeIlluminateConstruct()
	{
	};
};


struct IqParseNodeIlluminanceConstruct
{
	virtual	bool	fHasAxisAngle() const = 0;
	virtual	void*	GetInterface( EqParseNodeType type) const = 0;
	virtual	TqInt	NodeType() const = 0;

	virtual	void Accept( IqParseNodeVisitor& V ) = 0;

	const static EqParseNodeType m_ID;

	virtual ~IqParseNodeIlluminanceConstruct()
	{
	};
};


struct IqParseNodeSolarConstruct
{
	virtual	bool	fHasAxisAngle() const = 0;
	virtual	void*	GetInterface( EqParseNodeType type) const = 0;
	virtual	TqInt	NodeType() const = 0;

	virtual	void Accept( IqParseNodeVisitor& V ) = 0;

	const static EqParseNodeType m_ID;

	virtual ~IqParseNodeSolarConstruct()
	{
	};
};


struct IqParseNodeGatherConstruct
{
	virtual	void*	GetInterface( EqParseNodeType type) const = 0;
	virtual	TqInt	NodeType() const = 0;

	virtual	void Accept( IqParseNodeVisitor& V ) = 0;

	const static EqParseNodeType m_ID;

	virtual ~IqParseNodeGatherConstruct()
	{
	};
};


struct IqParseNodeConditional
{
	virtual	void*	GetInterface( EqParseNodeType type) const = 0;
	virtual	TqInt	NodeType() const = 0;

	virtual	void Accept( IqParseNodeVisitor& V ) = 0;

	const static EqParseNodeType m_ID;

	virtual ~IqParseNodeConditional()
	{
	};
};


struct IqParseNodeConditionalExpression
{
	virtual	void*	GetInterface( EqParseNodeType type) const = 0;
	virtual	TqInt	NodeType() const = 0;

	virtual	void Accept( IqParseNodeVisitor& V ) = 0;

	const static EqParseNodeType m_ID;

	virtual ~IqParseNodeConditionalExpression()
	{
	};
};


struct IqParseNodeTypeCast
{
	virtual	TqInt CastTo() const = 0;
	virtual	void*	GetInterface( EqParseNodeType type) const = 0;
	virtual	TqInt	NodeType() const = 0;

	virtual	void Accept( IqParseNodeVisitor& V ) = 0;

	const static EqParseNodeType m_ID;

	virtual ~IqParseNodeTypeCast()
	{
	};
};


struct IqParseNodeTriple
{
	virtual	void*	GetInterface( EqParseNodeType type) const = 0;
	virtual	TqInt	NodeType() const = 0;

	virtual	void Accept( IqParseNodeVisitor& V ) = 0;

	const static EqParseNodeType m_ID;
	
	virtual ~IqParseNodeTriple()
	{
	};
};


struct IqParseNodeSixteenTuple
{
	virtual	void*	GetInterface( EqParseNodeType type) const = 0;
	virtual	TqInt	NodeType() const = 0;

	virtual	void Accept( IqParseNodeVisitor& V ) = 0;

	const static EqParseNodeType m_ID;

	virtual ~IqParseNodeSixteenTuple()
	{
	};
};

struct IqParseNodeMessagePassingFunction
{
	virtual	SqVarRef	VarRef() const = 0;
	virtual	TqInt	CommType() const = 0;
	virtual CqString Extra() const = 0;
	virtual	void*	GetInterface( EqParseNodeType type) const = 0;
	virtual	TqInt	NodeType() const = 0;

	virtual	void Accept( IqParseNodeVisitor& V ) = 0;

	const static EqParseNodeType m_ID;

	virtual ~IqParseNodeMessagePassingFunction()
	{
	};
};

struct IqParseNodeTextureNameWithChannel
{
	virtual	void*	GetInterface( EqParseNodeType type) const = 0;
	virtual	TqInt	NodeType() const = 0;

	virtual	void Accept( IqParseNodeVisitor& V ) = 0;

	const static EqParseNodeType m_ID;

	virtual ~IqParseNodeTextureNameWithChannel()
	{
	};
};


//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	// !IPARSENODE_H_INCLUDED
