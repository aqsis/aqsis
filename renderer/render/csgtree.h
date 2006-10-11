// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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
		\brief Declares the classes for storing information about a CSG tree structure.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef CSGTREE_H_INCLUDED
#define CSGTREE_H_INCLUDED 1

#include	"aqsis.h"

#include	<vector>
#include	<list>
#include	<deque>

#include	<boost/weak_ptr.hpp>
#include	<boost/shared_ptr.hpp>
#include	<boost/enable_shared_from_this.hpp>
#include	"sstring.h"


START_NAMESPACE( Aqsis )

struct SqImageSample;


//------------------------------------------------------------------------------
/**
 *	Base CSG node class.
 *	Handles all linkage and basic processing, derived classes provide operation specific details.
 */
class CqCSGTreeNode : public boost::enable_shared_from_this<CqCSGTreeNode>
{
	public:
		/** Default constructor.
		 */
		CqCSGTreeNode()
		{}
		virtual	~CqCSGTreeNode();

#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqGSGTreeNode");
		}
#endif

		/** Enumeration of known CSG operation types.
		 */
		enum EqCSGNodeType
		{
		    CSGNodeType_Primitive,
		    CSGNodeType_Union,
		    CSGNodeType_Intersection,
		    CSGNodeType_Difference,
	};

		/** Get a reference to the list of children of this node.
		 */
		virtual std::list<boost::weak_ptr<CqCSGTreeNode> >& lChildren()
		{
			return ( m_lChildren );
		}
		/** Add a child to this node.
		 *  Takes care of unreferencing the parent if not already done.
		 */
		virtual	void	AddChild( const boost::shared_ptr<CqCSGTreeNode>& pChild )
		{
			lChildren().push_back( pChild );
			pChild->m_pParent = shared_from_this();
		}
		virtual	TqInt	isChild( const CqCSGTreeNode* pNode );
		/** Get the pointer to the parent CSG node for this node.
		 */
		virtual	boost::shared_ptr<CqCSGTreeNode>	pParent() const
		{
			return ( m_pParent );
		}
		virtual	TqInt	cChildren();

		/** Get the type identifier for this CSG node, overridded per derived node.
		 */
		virtual	EqCSGNodeType	NodeType() const = 0;
		/** Evaluate the state of the CSG operation for the given in/out states.
		 *  Given an array of in/out booleans for each of the children of this node,
		 *  evaluate the resulting in/out state after applying the operation.
		 */
		virtual	TqBool	EvaluateState( std::vector<TqBool>& abChildStates ) = 0;

		virtual	void	ProcessSampleList( std::deque<SqImageSample>& samples );

		void	ProcessTree( std::deque<SqImageSample>& samples );

		static boost::shared_ptr<CqCSGTreeNode> CreateNode( CqString& type );
		static TqBool IsRequired();
		static void SetRequired(TqBool value);


	private:
		boost::shared_ptr<CqCSGTreeNode>	m_pParent;		///< Pointer to the parent CSG node.
		std::list<boost::weak_ptr<CqCSGTreeNode> >	m_lChildren;	///< List of children nodes.
		static TqBool m_bCSGRequired;    ///< Tell imagebuffer the processing for CSG is not required
}
;



//------------------------------------------------------------------------------
/**
 *	Primitive CSG node.
 *	Does very little, primitive is basically a grouping node.
 */
class CqCSGNodePrimitive : public CqCSGTreeNode
{
	public:
		///	Default constructor.
		CqCSGNodePrimitive() : CqCSGTreeNode()
		{}
		///	Destructor.



		virtual ~CqCSGNodePrimitive()
		{}

		virtual std::list<boost::weak_ptr<CqCSGTreeNode> >& lChildren()
		{
			assert( TqFalse );
			return ( m_lDefPrimChildren );
		}
		virtual	void	AddChild( const boost::weak_ptr<CqCSGTreeNode>& pChild )
		{
			assert( TqFalse );
		}

		virtual	EqCSGNodeType	NodeType() const
		{
			return ( CSGNodeType_Primitive );
		}
		virtual	void	ProcessSampleList( std::deque<SqImageSample>& samples );
		virtual	TqBool	EvaluateState( std::vector<TqBool>& abChildStates )
		{
			return ( TqTrue );
		}

	private:
		static	std::list<boost::weak_ptr<CqCSGTreeNode> >	m_lDefPrimChildren;		///< Static empty child list, as primitives cannot have children nodes.
}
;


//------------------------------------------------------------------------------
/**
 *	Union CSG node.
 *	Creates a union of the surfaces of all its children.
 */
class CqCSGNodeUnion : public CqCSGTreeNode
{
	public:
		///	Default constructor.
		CqCSGNodeUnion() : CqCSGTreeNode()
		{}
		///	Destructor.



		virtual ~CqCSGNodeUnion()
		{}

		virtual	EqCSGNodeType	NodeType() const
		{
			return ( CSGNodeType_Union );
		}
		virtual	TqBool	EvaluateState( std::vector<TqBool>& abChildStates );

	private:
};


//------------------------------------------------------------------------------
/**
 *	Intersection CSG node.
 *	Creates an intersection of the surfaces of all its children.
 */
class CqCSGNodeIntersection : public CqCSGTreeNode
{
	public:
		///	Default constructor.
		CqCSGNodeIntersection() : CqCSGTreeNode()
		{}
		///	Destructor.



		virtual ~CqCSGNodeIntersection()
		{}

		virtual	EqCSGNodeType	NodeType() const
		{
			return ( CSGNodeType_Intersection );
		}
		virtual	TqBool	EvaluateState( std::vector<TqBool>& abChildStates );

	private:
};


//------------------------------------------------------------------------------
/**
 *	Intersection CSG node.
 *	Creates an difference between the first child surfaces and all other children.
 */
class CqCSGNodeDifference : public CqCSGTreeNode
{
	public:
		///	Default constructor.
		CqCSGNodeDifference() : CqCSGTreeNode()
		{}
		///	Destructor.



		virtual ~CqCSGNodeDifference()
		{}

		virtual	EqCSGNodeType	NodeType() const
		{
			return ( CSGNodeType_Difference );
		}
		virtual	TqBool	EvaluateState( std::vector<TqBool>& abChildStates );

	private:
};


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !CSGTREE_H_INCLUDED
