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
 * \brief Compiler backend to output a graphviz dot diagram of the parse tree.
 * \author Chris Foster - chris42f (at) gmail (dot) com
*/

#include "parsetreeviz.h"

#include <iostream>
#include <sstream>

#include <boost/format.hpp>

#include <aqsis/slcomp/icodegen.h>

namespace Aqsis {

//-----------------------------------------------------------------------
// Helper functions

/** \brief Return a string corresponding to the given operator code
 *
 * The codes in question are described at the top of iparsenode.h.
 */
const char* opToString(TqInt type)
{
	switch(type)
	{
		// Arithmetic
		case Op_Add: return "add";
		case Op_Sub: return "sub";
		case Op_Mul: return "mul";
		case Op_Div: return "div";
		case Op_Dot: return "dot_prod";
		case Op_Crs: return "cross_prod";
		case Op_Mod: return "%";
		case Op_Lft: return "<<";
		case Op_Rgt: return ">>";
		// unary arithmetic
		case Op_Plus: return "+";
		case Op_Neg: return "neg";
		// Bitwise logical
		case Op_And: return "&";
		case Op_Xor: return "xor\\n(bitwise)";
		case Op_Or: return "|";
		case Op_BitwiseComplement: return "!\\n(bitwise)";
		// comparison
		case Op_L: return "<";
		case Op_G: return ">";
		case Op_GE: return ">=";
		case Op_LE: return "<=";
		case Op_EQ: return "==";
		case Op_NE: return "!=";
		// logical
		case Op_LogicalNot: return "!\\n(logical)";
		case Op_LogAnd: return "&&";
		case Op_LogOr: return "||";
		default: return "error";
	}
}

/** Split a fully qualified variable name at the first scope-resolution operator
 *
 * This is just for neatness - variable names in the graph which look like
 *
 * some_fxn_name::variable
 *
 * then turn out as
 *
 *  some_fxn_name::
 *     variable
 *
 * instead, which helps to make the graph more compact.
 */
std::string splitVarNameToLines(const char* varName)
{
	std::string s(varName);
	std::string::size_type pos = s.find("::");
	if(pos != std::string::npos)
		s.insert(pos+2, "\\n");
	return s;
}

// Color strings for nodes.
static const char* functionColor = "#FF7070";
static const char* functionCallColor = "#F0A0A0";
static const char* unresolvedCallColor = "#F0C0C0";
static const char* variableColor = "#C0C0F0";
static const char* variableAssignColor = "#7070FF";
static const char* operatorColor = "#E0C080";
static const char* constantColor = "#CCCCCC";
static const char* blockConstructColor = "#80E080";
static const char* typeCastColor = "#E080A0";


//-----------------------------------------------------------------------
// CqParseTreeViz constructors/destructors

CqParseTreeViz::CqParseTreeViz(std::ostream& outStream)
	: m_outStream(outStream),
	m_nodeNames(),
	m_calledFunctions()
{
	m_outStream << "digraph AST_graph {\n"
		<< "node [style=filled];\n";
}

CqParseTreeViz::~CqParseTreeViz()
{
	// Produce output parse trees for each function which was called
	for(TqFuncDefSet::const_iterator i = m_calledFunctions.begin();
			i != m_calledFunctions.end(); ++i)
	{
		makeFunctionGraph(**i);
	}
	m_outStream << "};\n";
}


//-----------------------------------------------------------------------
// Node visitor functions
void CqParseTreeViz::Visit(IqParseNode& node)
{
	setNodeProperty(node, "label", "Base");
	setNodeProperty(node, "shape", "point");
	visitChildren(node);
}

void CqParseTreeViz::Visit(IqParseNodeShader& node)
{
	setNodeProperty(node, "label", boost::format(
			"{%s shader \\\"%s\\\" | {<args> args | <code> code } }")
			% node.strShaderType() % node.strName());
	setNodeProperty(node, "fillcolor", functionColor);
	setNodeProperty(node, "shape", "record");
	IqParseNode* code = static_cast<IqParseNode*>(
			node.GetInterface(ParseNode_Base))->pChild();
	if(code)
	{
		IqParseNode* args = code->pNextSibling();
		if(args)
		{
			makeEdge(node, *args, "args");
			args->Accept(*this);
		}
		makeEdge(node, *code, "code");
		code->Accept(*this);
	}
}

void CqParseTreeViz::Visit(IqParseNodeFunctionCall& node)
{
	IqFuncDef* funcDef = node.pFuncDef();
	setNodeProperty(node, "label", funcDef->strVMName());
	setNodeProperty(node, "shape", "box");
	setNodeProperty(node, "fillcolor", functionCallColor);
	m_calledFunctions.insert(funcDef);
	visitChildren(node);
}

void CqParseTreeViz::Visit(IqParseNodeUnresolvedCall& node)
{
	setNodeProperty(node, "label", std::string("UnresolvedCall\\n") + node.strName());
	setNodeProperty(node, "fillcolor", unresolvedCallColor);
	setNodeProperty(node, "shape", "box");
	visitChildren(node);
}

void CqParseTreeViz::Visit(IqParseNodeVariable& node)
{
	setNodeProperty(node, "label", splitVarNameToLines(node.strName()));
	setNodeProperty(node, "color", variableColor);
	visitChildren(node);
}

void CqParseTreeViz::Visit(IqParseNodeArrayVariable& node)
{
	const char* varName = static_cast<IqParseNodeVariable*>(
			node.GetInterface(ParseNode_Variable))->strName();
	setNodeProperty(node, "label", splitVarNameToLines(varName) + " []");
	setNodeProperty(node, "color", variableColor);
	visitChildren(node);
}

void CqParseTreeViz::Visit(IqParseNodeVariableAssign& node)
{
	IqParseNodeVariable* varNode = static_cast<IqParseNodeVariable*>(
			node.GetInterface(ParseNode_Variable));

	setNodeProperty(node, "label", boost::format("%s := ")
			% splitVarNameToLines(varNode->strName()));
	setNodeProperty(node, "fillcolor", variableAssignColor);
	visitChildren(node);
}

void CqParseTreeViz::Visit(IqParseNodeArrayVariableAssign& node)
{
	IqParseNodeVariable* varNode = static_cast<IqParseNodeVariable*>(
			node.GetInterface(ParseNode_Variable));

	setNodeProperty(node, "label", boost::format("%s [] := ")
			% splitVarNameToLines(varNode->strName()));
	setNodeProperty(node, "fillcolor", variableAssignColor);
	visitChildren(node);
}

void CqParseTreeViz::Visit(IqParseNodeOperator& node)
{
	setNodeProperty(node, "label", opToString(node.Operator()));
	setNodeProperty(node, "shape", "box");
	setNodeProperty(node, "fillcolor", operatorColor);
	visitChildren(node);
}

void CqParseTreeViz::Visit(IqParseNodeMathOp& node)
{
	Visit(*static_cast<IqParseNodeOperator*>(
			node.GetInterface(ParseNode_Operator)));
}

void CqParseTreeViz::Visit(IqParseNodeRelationalOp& node)
{
	Visit(*static_cast<IqParseNodeOperator*>(
			node.GetInterface(ParseNode_Operator)));
}

void CqParseTreeViz::Visit(IqParseNodeUnaryOp& node)
{
	Visit(*static_cast<IqParseNodeOperator*>(
			node.GetInterface(ParseNode_Operator)));
}

void CqParseTreeViz::Visit(IqParseNodeLogicalOp& node)
{
	Visit(*static_cast<IqParseNodeOperator*>(
			node.GetInterface(ParseNode_Operator)));
}

void CqParseTreeViz::Visit(IqParseNodeDiscardResult& node)
{
	setNodeProperty(node, "label", "DiscardResult");
	visitChildren(node);
}

void CqParseTreeViz::Visit(IqParseNodeConstantFloat& node)
{
	setNodeProperty(node, "label", boost::format("%0.2f") % node.Value());
	setNodeProperty(node, "color", constantColor);
	visitChildren(node);
}

void CqParseTreeViz::Visit(IqParseNodeConstantString& node)
{
	setNodeProperty(node, "label", boost::format("\\\"%s\\\"") % node.strValue());
	setNodeProperty(node, "color", constantColor);
	visitChildren(node);
}

void CqParseTreeViz::Visit(IqParseNodeWhileConstruct& node)
{
	setNodeProperty(node, "label", "WHILE");
	setNodeProperty(node, "fillcolor", blockConstructColor);
	setNodeProperty(node, "shape", "Msquare");
	visitChildren(node);
}

void CqParseTreeViz::Visit(IqParseNodeLoopMod& node)
{
	switch(node.modType())
	{
		case LoopMod_Break:
			setNodeProperty(node, "label", "break");
			break;
		case LoopMod_Continue:
			setNodeProperty(node, "label", "continue");
			break;
	}
	setNodeProperty(node, "fillcolor", blockConstructColor);
	setNodeProperty(node, "shape", "box");
	visitChildren(node);
}

void CqParseTreeViz::Visit(IqParseNodeIlluminateConstruct& node)
{
	setNodeProperty(node, "label", "ILLUMINATE");
	setNodeProperty(node, "fillcolor", blockConstructColor);
	setNodeProperty(node, "shape", "Msquare");
	visitChildren(node);
}

void CqParseTreeViz::Visit(IqParseNodeIlluminanceConstruct& node)
{
	setNodeProperty(node, "label", "ILLUMINANCE");
	setNodeProperty(node, "fillcolor", blockConstructColor);
	setNodeProperty(node, "shape", "Msquare");
	visitChildren(node);
}

void CqParseTreeViz::Visit(IqParseNodeSolarConstruct& node)
{
	setNodeProperty(node, "label", "SOLAR");
	setNodeProperty(node, "fillcolor", blockConstructColor);
	setNodeProperty(node, "shape", "Msquare");
	visitChildren(node);
}

void CqParseTreeViz::Visit(IqParseNodeGatherConstruct& node)
{
	setNodeProperty(node, "label", "GATHER");
	setNodeProperty(node, "fillcolor", blockConstructColor);
	setNodeProperty(node, "shape", "Msquare");
	visitChildren(node);
}

void CqParseTreeViz::Visit(IqParseNodeConditional& node)
{
	setNodeProperty(node, "label", "IF");
	setNodeProperty(node, "fillcolor", blockConstructColor);
	setNodeProperty(node, "shape", "Msquare");
	visitChildren(node);
}

void CqParseTreeViz::Visit(IqParseNodeConditionalExpression& node)
{
	setNodeProperty(node, "label", "t/f ? a : b");
	visitChildren(node);
}

void CqParseTreeViz::Visit(IqParseNodeTypeCast& node)
{
	setNodeProperty(node, "fillcolor", typeCastColor);
	setNodeProperty(node, "shape", "box");

	IqParseNode* operand = static_cast<IqParseNode*>(
			node.GetInterface(ParseNode_Base))->pChild();

	const char* pstrToType = gVariableTypeNames[node.CastTo() & Type_Mask];
	const char* pstrFromType = gVariableTypeNames[operand->ResType() & Type_Mask];
	setNodeProperty(node, "label", 
			boost::format("%s<-\\n<-%s") % pstrToType % pstrFromType);
	visitChildren(node);
}

void CqParseTreeViz::Visit(IqParseNodeTriple& node)
{
	setNodeProperty(node, "label", "Triple");
	visitChildren(node);
}

void CqParseTreeViz::Visit(IqParseNodeSixteenTuple& node)
{
	setNodeProperty(node, "label", "SixteenTuple");
	visitChildren(node);
}

void CqParseTreeViz::Visit(IqParseNodeMessagePassingFunction& node)
{
	setNodeProperty(node, "label", "MessagePassingFunction");
	visitChildren(node);
}

void CqParseTreeViz::Visit(IqParseNodeTextureNameWithChannel& node)
{
	setNodeProperty(node, "label", "TextureNameWithChannel");
	visitChildren(node);
}

//-----------------------------------------------------------------------
// Private functions

template<typename T>
const std::string& CqParseTreeViz::getNodeName(const T& node)
{
	const void* ptr = node.GetInterface(ParseNode_Base);
	return getNodeName(ptr);
}

// Specialization for IqFuncDef.  IqFuncDef can't be cast to IqParseNode in the
// same way, so we've got to do something special for it.
template<>
const std::string& CqParseTreeViz::getNodeName(const IqFuncDef& node)
{
	return getNodeName(reinterpret_cast<const void*>(&node));
}

const std::string& CqParseTreeViz::getNodeName(const void* ptr)
{
	TqNodeNameMap::iterator i = m_nodeNames.find(ptr);
	if(i != m_nodeNames.end())
		return i->second;
	else
	{
		std::ostringstream ostr;
		ostr << "node_" << ptr;
		return m_nodeNames[ptr] = ostr.str();
	}
}

template<typename T, typename U>
void CqParseTreeViz::makeEdge(const T& node1, const U& node2, const char* fromTag)
{
	m_outStream << getNodeName(node1);
	if(fromTag != std::string(""))
		m_outStream << ":" << fromTag;
	m_outStream << " -> " << getNodeName(node2) << ";\n";
}

template<typename T, typename U>
void CqParseTreeViz::setNodeProperty(const T& node, const char* key, const U& value)
{
	m_outStream << getNodeName(node) << " [" <<  key << "=\"" << value << "\"];\n";
}

template<typename T>
void CqParseTreeViz::visitChildren(T& node)
{
	IqParseNode* child = static_cast<IqParseNode*>(
			node.GetInterface(ParseNode_Base))->pChild();
	while(child != NULL)
	{
		makeEdge(node, *child);
		child->Accept(*this);
		child = child->pNextSibling();
	}
}

void CqParseTreeViz::makeFunctionGraph(const IqFuncDef& funcDef)
{
	// Only try to display graphs of user-defined functions.
	if(funcDef.fLocal())
	{
		setNodeProperty(funcDef, "label",
				boost::format("{%s | {<args> args|<code> code}}")
				% funcDef.strName());
		setNodeProperty(funcDef, "fillcolor", functionColor);
		setNodeProperty(funcDef, "shape", "record");
		const IqParseNode* args = funcDef.pArgs();
		if(args)
		{
			makeEdge(funcDef, *args, "args");
			const_cast<IqParseNode*>(args)->Accept(*this);
		}
		const IqParseNode* code = funcDef.pDef();
		if(code)
		{
			makeEdge(funcDef, *code, "code");
			const_cast<IqParseNode*>(code)->Accept(*this);
		}
	}
}

} // namespace Aqsis

