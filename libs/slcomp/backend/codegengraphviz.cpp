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
 * \brief Code generator backend to output a graphviz dot diagram of the parse tree.
 * \author Chris Foster - chris42f (at) gmail (dot) com
*/

#include <aqsis/slcomp/icodegen.h>

#include <fstream>
#include <algorithm>

#include "parsetreeviz.h"
#include <aqsis/slcomp/iparsenode.h>

namespace Aqsis {

//-----------------------------------------------------------------------
/** \brief Visitor class to extract the shader name from the given tree.
 */
class CqShaderNameVisitor : public IqParseNodeVisitor
{
	private:
		std::string m_shaderName;
	public:
		CqShaderNameVisitor()
			: m_shaderName("unknown")
		{ }
		const std::string& shaderName()
		{
			return m_shaderName;
		}
		virtual	void Visit( IqParseNode& node)
		{
			IqParseNode* child = static_cast<IqParseNode*>(
					node.GetInterface(ParseNode_Base))->pChild();
			while(child != NULL)
			{
				child->Accept(*this);
				child = child->pNextSibling();
			}
		}
		virtual	void Visit( IqParseNodeShader& node)
		{
			m_shaderName = node.strName();
		}
		virtual	void Visit( IqParseNodeFunctionCall& ) { }
		virtual	void Visit( IqParseNodeUnresolvedCall& ) { }
		virtual	void Visit( IqParseNodeVariable& ) { }
		virtual	void Visit( IqParseNodeArrayVariable& ) { }
		virtual	void Visit( IqParseNodeVariableAssign& ) { }
		virtual	void Visit( IqParseNodeArrayVariableAssign& ) { }
		virtual	void Visit( IqParseNodeOperator& ) { }
		virtual	void Visit( IqParseNodeMathOp& ) { }
		virtual	void Visit( IqParseNodeRelationalOp& ) { }
		virtual	void Visit( IqParseNodeUnaryOp& ) { }
		virtual	void Visit( IqParseNodeLogicalOp& ) { }
		virtual	void Visit( IqParseNodeDiscardResult& ) { }
		virtual	void Visit( IqParseNodeConstantFloat& ) { }
		virtual	void Visit( IqParseNodeConstantString& ) { }
		virtual	void Visit( IqParseNodeWhileConstruct& ) { }
		virtual	void Visit( IqParseNodeLoopMod& ) { }
		virtual	void Visit( IqParseNodeIlluminateConstruct& ) { }
		virtual	void Visit( IqParseNodeIlluminanceConstruct& ) { }
		virtual	void Visit( IqParseNodeSolarConstruct& ) { }
		virtual	void Visit( IqParseNodeGatherConstruct& ) { }
		virtual	void Visit( IqParseNodeConditional& ) { }
		virtual	void Visit( IqParseNodeConditionalExpression& ) { }
		virtual	void Visit( IqParseNodeTypeCast& ) { }
		virtual	void Visit( IqParseNodeTriple& ) { }
		virtual	void Visit( IqParseNodeSixteenTuple& ) { }
		virtual	void Visit( IqParseNodeMessagePassingFunction& ) { }
		virtual	void Visit( IqParseNodeTextureNameWithChannel& ) { }
};


//-----------------------------------------------------------------------
// CqCodeGenGraphviz implementation
void CqCodeGenGraphviz::OutputTree(IqParseNode* pNode, std::string outFileName)
{
	if(outFileName == "")
	{
		CqShaderNameVisitor nameVisitor;
		pNode->Accept(nameVisitor);
		outFileName = nameVisitor.shaderName();
	}

	// append ".dot" to the name if it doesn't already exist.
	if(static_cast<TqInt>(outFileName.size()) >= 4
			&& ! std::equal(outFileName.end()-4, outFileName.end(), ".dot"))
		outFileName += ".dot";

	// Open file and produce graph using CqParseTreeViz.
	std::ofstream outFile(outFileName.c_str());
	if(outFile)
	{
		CqParseTreeViz treeViz(outFile);
		pNode->Accept(treeViz);
		std::cout << "... " << outFileName << "\n";
	}
	else
	{
		std::cerr << "Could not open output file \"" << outFileName << "\"\n";
	}
}


} // namespace Aqsis
