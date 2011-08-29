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
 * \brief Parse tree visitor to create graphviz dot diagrams of the parse tree.
 * \author Chris Foster - chris42f (at) gmail (dot) com
*/

#ifndef PARSETREEVIS_H_INCLUDED
#define PARSETREEVIS_H_INCLUDED 1

#include <aqsis/aqsis.h>

#include <iosfwd>
#include <string>
#include <map>
#include <set>

#include <aqsis/slcomp/iparsenode.h>
#include <aqsis/slcomp/ifuncdef.h>

namespace Aqsis {

//-----------------------------------------------------------------------
/** \brief Compiler backend to generate AST graphs
 *
 * This compiler backend is intended for debugging use and can generate graphs
 * of the abstract syntax tree (AST) produced by the parser.  Specifically, the
 * backend produces input files for the graphviz "dot" automatic graph-drawing
 * tool.
 *
 * In the graph, elliptical shaped nodes generally deal with data, while
 * rectangular nodes indicate function calls.
 */
class CqParseTreeViz : public IqParseNodeVisitor
{
	public:
		/** \brief Construct an AST visulizer 
		 *
		 * The constructor makes sure that the relevant graphviz header
		 * structure is in place.
		 *
		 * \param outStream - stream which the graph will be inserted into.
		 */
		CqParseTreeViz(std::ostream& outStream);

		/** \brief Fininalize the graph
		 *
		 * The destructor generates any finalizing parts of the graph file
		 * structure.
		 */
		~CqParseTreeViz();

		/** \name Functions which Visit parse nodes based on their type.
		 */
		//@{
		virtual	void Visit( IqParseNode& );
		virtual	void Visit( IqParseNodeShader& );
		virtual	void Visit( IqParseNodeFunctionCall& );
		virtual	void Visit( IqParseNodeUnresolvedCall& );
		virtual	void Visit( IqParseNodeVariable& );
		virtual	void Visit( IqParseNodeArrayVariable& );
		virtual	void Visit( IqParseNodeVariableAssign& );
		virtual	void Visit( IqParseNodeArrayVariableAssign& );
		virtual	void Visit( IqParseNodeOperator& );
		virtual	void Visit( IqParseNodeMathOp& );
		virtual	void Visit( IqParseNodeRelationalOp& );
		virtual	void Visit( IqParseNodeUnaryOp& );
		virtual	void Visit( IqParseNodeLogicalOp& );
		virtual	void Visit( IqParseNodeDiscardResult& );
		virtual	void Visit( IqParseNodeConstantFloat& );
		virtual	void Visit( IqParseNodeConstantString& );
		virtual	void Visit( IqParseNodeWhileConstruct& );
		virtual	void Visit( IqParseNodeLoopMod& );
		virtual	void Visit( IqParseNodeIlluminateConstruct& );
		virtual	void Visit( IqParseNodeIlluminanceConstruct& );
		virtual	void Visit( IqParseNodeSolarConstruct& );
		virtual	void Visit( IqParseNodeGatherConstruct& );
		virtual	void Visit( IqParseNodeConditional& );
		virtual	void Visit( IqParseNodeConditionalExpression& );
		virtual	void Visit( IqParseNodeTypeCast& );
		virtual	void Visit( IqParseNodeTriple& );
		virtual	void Visit( IqParseNodeSixteenTuple& );
		virtual	void Visit( IqParseNodeMessagePassingFunction& );
		virtual	void Visit( IqParseNodeTextureNameWithChannel& );
		//@}

	private:
		typedef std::map<const void*, std::string> TqNodeNameMap;
		typedef std::set<const IqFuncDef*> TqFuncDefSet;

		/** \brief Get a unique name string for a node.
		 *
		 * \param node - a node type which can be safely cast to IqParseNode
		 * via the GetInterface() call
		 */
		template<typename T>
		const std::string& getNodeName(const T& node);
		/** \brief Get a unique graphviz node name for the given pointer
		 */
		const std::string& getNodeName(const void* ptr);

		/// \name Graph output functions.
		//@{
		/** \brief Produce a graph edge pointing from node1 to node2
		 *
		 * If fromTag is not empty, the edge is something like
		 *   node1_name:fromTag -> node2_name
		 * which makes the edge originate from a particular spatial part of
		 * node1.
		 *
		 * \param node1 - origin node
		 * \param node2 - destination node
		 * \param fromTag - origin tag name for the node
		 */
		template<typename T, typename U>
		void makeEdge(const T& node1, const U& node2, const char* fromTag = "");
		/** \brief Set a graph node property
		 *
		 * Properties relate to the appearance of the node in the output graph.
		 * some examples are the label text and fill colour.
		 *
		 * \brief node - node which will have the property set
		 * \brief key - name of the property
		 * \brief value - any value which can be turned into text with the
		 * stream insertion operator.
		 */
		template<typename T, typename U>
		void setNodeProperty(const T& node, const char* key,
				const U& value);
		//@}

		/// Visit all the children of the given node starting from the first.
		template<typename T>
		void visitChildren(T& node);

		/// Output a graph of the given function definition
		void makeFunctionGraph(const IqFuncDef& funcDef);

		/// Stream which the graph description text will be sent to.
		std::ostream& m_outStream;
		/// A mapping of node pointers to unique names.  Used to identify nodes
		/// in the output text.
		TqNodeNameMap m_nodeNames;
		/// Set of functions called by the shader which should have graphs produced.
		TqFuncDefSet m_calledFunctions;
};


//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	// !PARSETREEVIS_H_INCLUDED
