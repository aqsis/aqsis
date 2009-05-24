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
		\brief Implements the CSG tree node classes.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	"csgtree.h"
#include	"imagepixel.h"

namespace Aqsis {

bool CqCSGTreeNode::m_bCSGRequired = false;

std::list<boost::weak_ptr<CqCSGTreeNode> >	CqCSGNodePrimitive::m_lDefPrimChildren;		///< Static empty child list, as primitives cannot have children nodes.

//------------------------------------------------------------------------------
/**
 *	Destructor.
 *
 */
CqCSGTreeNode::~CqCSGTreeNode()
{}


//------------------------------------------------------------------------------
/**
 *	Static function to create a CSG node from the name.
 *	Used to create a node type from just the name.
 *
 *	@param	type	String identifier of CSG node type, one of "primitive",
 *					"union", "intersection", "difference"
 *
 *	@return			Pointer to the new node.
 */
boost::shared_ptr<CqCSGTreeNode> CqCSGTreeNode::CreateNode( CqString& type )
{
	SetRequired(true);
	if ( type == "primitive" )
		return boost::shared_ptr<CqCSGTreeNode>( new CqCSGNodePrimitive );
	else if ( type == "union" )
		return boost::shared_ptr<CqCSGTreeNode>( new CqCSGNodeUnion );
	else if ( type == "intersection" )
		return boost::shared_ptr<CqCSGTreeNode>( new CqCSGNodeIntersection );
	else if ( type == "difference" )
		return boost::shared_ptr<CqCSGTreeNode>( new CqCSGNodeDifference );
	else
		return boost::shared_ptr<CqCSGTreeNode>( );
}


/**
 * Get the state of the flag indicating if CSG processing is required at all.
 */
bool CqCSGTreeNode::IsRequired()
{
	return m_bCSGRequired;
}

/**
 * Set the flag indicating CSG is now required.
 */
void CqCSGTreeNode::SetRequired(bool value)
{
	m_bCSGRequired = value;
}


//------------------------------------------------------------------------------
/**
 *	Determine if the given node is a child of this one.
 *	If the node is a child, the index is returned, else -1.
 *
 *	@param	pNode	Pointer to a CSG node to test.
 *
 *	@return			Index of the node in the children list, or -1.
 */
TqInt CqCSGTreeNode::isChild( const CqCSGTreeNode* pNode )
{
	if ( !pNode )
	{
		return ( -1 );
	}
	TqInt iChild = 0;
	std::list<boost::weak_ptr<CqCSGTreeNode> >::const_iterator
	ii = lChildren().begin(), ie = lChildren().end();
	for (; ii != ie; ++ii, ++iChild)
	{
		boost::shared_ptr<CqCSGTreeNode> pChild = ii->lock()
		        ;
		if ( pChild.get() == pNode )
			return ( iChild );
	}
	return ( -1 );
}


//------------------------------------------------------------------------------
/**
 *	Get the number of children.
 *	Count the number of children nodes this node has.
 *
 *	@return	Integer children count.
 */
TqInt CqCSGTreeNode::cChildren()
{
	TqInt c = 0;
	std::list<boost::weak_ptr<CqCSGTreeNode> >::const_iterator
	ii = lChildren().begin(), ie = lChildren().end();
	for (; ii != ie; ++ii)
	{
		++c;
	}
	return ( c );
}


//------------------------------------------------------------------------------
/**
 *	Process the CSG tree over the given sample list.
 *	First goes back up the tree to the top, then starts processing nodes from
 *	there using ProcessSampleList.
 *
 *	@param	samples	Array of samples to pass through the CSG tree.
 */
void CqCSGTreeNode::ProcessTree( std::vector<SqImageSample>& samples )
{
	// Follow the tree back up to the top, then process the list from there
	boost::shared_ptr<CqCSGTreeNode> pTop = shared_from_this();
	while ( pTop->pParent() )
	{
		pTop = pTop->pParent();
	}

	pTop->ProcessSampleList( samples );
}


//------------------------------------------------------------------------------
/**
 *	Pass the sample list through the CSG node.
 *	The sample list will contain only the relevant entry and exit points for
 *	the resulting surface for this operation, and they will be promoted to
 *	this node for further processing up the tree.
 *
 *	@param	samples	Array of samples to process.
 */
void CqCSGTreeNode::ProcessSampleList( std::vector<SqImageSample>& samples )
{
	// First process any children nodes.
	// Process all nodes depth first.
	std::list<boost::weak_ptr<CqCSGTreeNode> >::const_iterator
	ii = lChildren().begin(), ie = lChildren().end();
	for (; ii != ie; ++ii)
	{
		// If the node is a primitive, no need to process it.
		// In fact as the primitive, just nulls out its owned samples
		// this would break the CSG code.
		boost::shared_ptr<CqCSGTreeNode> pChild = ii->lock()
		        ;
		if ( pChild.get() && pChild->NodeType() != CSGNodeType_Primitive )
			pChild->ProcessSampleList( samples );
	}

	std::vector<bool> abChildState( cChildren() );
	std::vector<TqInt> aChildIndex( samples.size() );
	TqInt iChild;
	for ( iChild = 0; iChild < cChildren(); iChild++ )
		abChildState[ iChild ] = false;

	// Now get the initial state
	bool bCurrentI = false;

	// Find out if the camera is starting inside a solid. This is the case if you
	// see an odd number of walls for that solid when looking out.
	std::vector<SqImageSample>::iterator i;
	TqInt j = 0;
	for ( i = samples.begin(); i != samples.end(); ++i, ++j )
	{
		if ( ( aChildIndex[j] = isChild( i->csgNode.get() ) ) >= 0 )
		{
			if ( ((i->csgNode.get())->NodeType() == CSGNodeType_Primitive ) &&
			        ((i->csgNode.get())->NodeType() == CSGNodeType_Union ) )
			{
				abChildState[ aChildIndex[j] ] = !abChildState[ aChildIndex[j] ];
			}
		}
	}

	bCurrentI = EvaluateState( abChildState );

	// Now go through samples, clearing any where the state doesn't change, and
	// promoting any where it does to this node.
	for ( i = samples.begin(), j = 0; i != samples.end(); ++j )
	{
		// Find out if sample is in out children nodes, if so are we entering or leaving.
		if ( aChildIndex[j] >= 0 )
			abChildState[ aChildIndex[j] ] = !abChildState[ aChildIndex[j] ];
		else
		{
			++i;
			continue;
		}

		// Work out the new state
		bool bNewI = EvaluateState( abChildState );

		// If it hasn't changed, remove the sample.
		if ( bNewI == bCurrentI )
			i = samples.erase( i );
		else
			// Otherwise promote it to this node unless we are a the top.
		{
			bCurrentI = bNewI;
			if ( pParent() )
			{
				i->csgNode = shared_from_this();
			}
			else
			{
				i->csgNode = boost::shared_ptr<CqCSGTreeNode>();
			}
			i++;
		}
	}
}

//------------------------------------------------------------------------------
/**
 *	Pass the sample list through the CSG node.
 *  For a primitive, we should just nullify all csg node pointers for samples in this node.
 *	\note This should only be called if the Primitive node is the top level parent.
 *
 *	@param	samples	Array of samples to process.
 */
void CqCSGNodePrimitive::ProcessSampleList( std::vector<SqImageSample>& samples )
{
	// Now go through samples, clearing samples related to this node.
	std::vector<SqImageSample>::iterator i;
	for ( i = samples.begin(); i != samples.end(); ++i )
	{
		if ( i->csgNode.get() == this )
		{
			i->csgNode = boost::shared_ptr<CqCSGTreeNode>();
		}
	}
}


//------------------------------------------------------------------------------
/**
 *	Evaluate the in/out state of the children and determine if the result is
 *	in or out after the operatioin.
 *	Given an array of booleans representing in or out of the children of this
 *	node, apply the rules for the union operator and determine if we are
 *	inside or out the union surface.
 *
 *	@param	abChildStates	Array of booleans representing in or out for each child.
 *
 *	@return					Boolean indicating in or out for the resulting solid.
 */
bool CqCSGNodeUnion::EvaluateState( std::vector<bool>& abChildStates )
{
	// Work out the new state
	std::vector<bool>::iterator iChildState;
	for ( iChildState = abChildStates.begin(); iChildState != abChildStates.end(); iChildState++ )
	{
		if ( *iChildState )
			return ( true );
	}
	return ( false );
}


//------------------------------------------------------------------------------
/**
 *	Evaluate the in/out state of the children and determine if the result is
 *	in or out after the operatioin.
 *	Given an array of booleans representing in or out of the children of this
 *	node, apply the rules for the intersection operator and determine if we are
 *	inside or out the intersection surface.
 *
 *	@param	abChildStates	Array of booleans representing in or out for each child.
 *
 *	@return					Boolean indicating in or out for the resulting solid.
 */
bool CqCSGNodeIntersection::EvaluateState( std::vector<bool>& abChildStates )
{
	// Work out the new state
	std::vector<bool>::iterator iChildState;
	for ( iChildState = abChildStates.begin(); iChildState != abChildStates.end(); iChildState++ )
	{
		if ( !( *iChildState ) )
			return ( false );
	}
	return ( true );
}


//------------------------------------------------------------------------------
/**
 *	Evaluate the in/out state of the children and determine if the result is
 *	in or out after the operatioin.
 *	Given an array of booleans representing in or out of the children of this
 *	node, apply the rules for the difference operator and determine if we are
 *	inside or out the difference surface.
 *
 *	@param	abChildStates	Array of booleans representing in or out for each child.
 *
 *	@return					Boolean indicating in or out for the resulting solid.
 */
bool CqCSGNodeDifference::EvaluateState( std::vector<bool>& abChildStates )
{
	// Work out the new state
	if ( abChildStates[ 0 ] )
	{
		std::vector<bool>::iterator iChildState;
		iChildState = abChildStates.begin();
		iChildState++;
		for ( ; iChildState != abChildStates.end(); iChildState++ )
		{
			if ( *iChildState )
				return ( false );
		}
		return ( true );
	}
	return ( false );
}


//---------------------------------------------------------------------

} // namespace Aqsis
