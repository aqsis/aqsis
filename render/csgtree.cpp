// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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
		\brief Implements the CGS tree node classes.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"aqsis.h"

#include	"csgtree.h"
#include	"imagebuffer.h"

START_NAMESPACE( Aqsis )

/** Static empty children list for use by primitive node type.
 */
CqList<CqCSGTreeNode>	CqCSGNodePrimitive::m_lDefPrimChildren;

TqBool CqCSGTreeNode::m_bCSGRequired = TqFalse;

//------------------------------------------------------------------------------
/**
 *	Destructor.
 *	Takes care of unreferencing any children.
 *
 */
CqCSGTreeNode::~CqCSGTreeNode()
{
	CqCSGTreeNode * pChild = m_lChildren.pFirst();
	while ( pChild )
	{
		CqCSGTreeNode * pNext = pChild->pNext();
		pChild->Release();
		pChild = pNext;
	}
}


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
CqCSGTreeNode* CqCSGTreeNode::CreateNode( CqString& type )
{
    SetRequired(TqTrue);
	if ( type == "primitive" )
		return ( new CqCSGNodePrimitive );
	else if ( type == "union" )
		return ( new CqCSGNodeUnion );
	else if ( type == "intersection" )
		return ( new CqCSGNodeIntersection );
	else if ( type == "difference" )
		return ( new CqCSGNodeDifference );
	else
		return ( NULL );
}

TqBool CqCSGTreeNode::IsRequired()
{
	return m_bCSGRequired;
}

void CqCSGTreeNode::SetRequired(TqBool value)
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
	TqInt iChild = 0;
	CqCSGTreeNode* pChild = lChildren().pFirst();
	while ( pChild )
	{
		if ( pChild == pNode ) return ( iChild );
		pChild = pChild->pNext();
		iChild++;
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
	CqCSGTreeNode* pChild = lChildren().pFirst();
	while ( pChild )
	{
		c++;
		pChild = pChild->pNext();
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
	CqCSGTreeNode * pTop = this;
	while ( NULL != pTop->pParent() )
		pTop = pTop->pParent();

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
	CqCSGTreeNode * pChild = lChildren().pFirst();
	while ( NULL != pChild )
	{
		// If the node is a primitive, no need to process it.
		// In fact as the primitive, just nulls out its owned samples
		// this would break the CSG code.
		if ( pChild->NodeType() != CSGNodeType_Primitive )
			pChild->ProcessSampleList( samples );
		pChild = pChild->pNext();
	}

	std::vector<TqBool> abChildState( cChildren() );
	std::vector<TqInt> aChildIndex( samples.size() );
	TqInt iChild;
	for ( iChild = 0; iChild < cChildren(); iChild++ ) abChildState[ iChild ] = TqFalse;

	// Find out if the camera is starting inside a solid. This is the case if you
	// see an odd number of walls for that solid when looking out.
	std::vector<SqImageSample>::iterator i;
	TqInt j = 0;
	for ( i = samples.begin(); i != samples.end(); ++i, ++j )
	{
		if ( ( aChildIndex[j] = isChild( i->m_pCSGNode ) ) >= 0 )
			abChildState[ aChildIndex[j] ] = !abChildState[ aChildIndex[j] ];
	}

	// Now get the initial state
	TqBool bCurrentI = EvaluateState( abChildState );

	// Now go through samples, clearing any where the state doesn't change, and
	// promoting any where it does to this node.
	for ( i = samples.begin(), j = 0; i != samples.end(); ++j )
	{
		// Find out if sample is in out children nodes, if so are we entering or leaving.
		if ( aChildIndex[j] >= 0 )
			abChildState[ aChildIndex[j] ] = !abChildState[ aChildIndex[j] ];
		else
		{
			i++;
			continue;
		}

		// Work out the new state
		TqBool bNewI = EvaluateState( abChildState );

		// If it hasn't changed, remove the sample.
		if ( bNewI == bCurrentI )
			i = samples.erase( i );
		else
			// Otherwise promote it to this node unless we are a the top.
		{
			bCurrentI = bNewI;
			CqCSGTreeNode* poldnode = i->m_pCSGNode;
			if ( NULL != this->pParent() )
			{
				i->m_pCSGNode = this;
				AddRef();
			}
			else
				i->m_pCSGNode = NULL;
			poldnode->Release();
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
	for ( i = samples.begin(); i != samples.end(); i++ )
	{
		if ( i->m_pCSGNode == this )
		{
			i->m_pCSGNode = NULL;
			Release();
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
TqBool CqCSGNodeUnion::EvaluateState( std::vector<TqBool>& abChildStates )
{
	// Work out the new state
	std::vector<TqBool>::iterator iChildState;
	for ( iChildState = abChildStates.begin(); iChildState != abChildStates.end(); iChildState++ )
	{
		if ( *iChildState )
			return ( TqTrue );
	}
	return ( TqFalse );
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
TqBool CqCSGNodeIntersection::EvaluateState( std::vector<TqBool>& abChildStates )
{
	// Work out the new state
	std::vector<TqBool>::iterator iChildState;
	for ( iChildState = abChildStates.begin(); iChildState != abChildStates.end(); iChildState++ )
	{
		if ( !( *iChildState ) )
			return ( TqFalse );
	}
	return ( TqTrue );
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
TqBool CqCSGNodeDifference::EvaluateState( std::vector<TqBool>& abChildStates )
{
	// Work out the new state
	if ( abChildStates[ 0 ] )
	{
		std::vector<TqBool>::iterator iChildState;
		iChildState = abChildStates.begin();
		iChildState++;
		for ( ; iChildState != abChildStates.end(); iChildState++ )
		{
			if ( *iChildState )
				return ( TqFalse );
		}
		return ( TqTrue );
	}
	return ( TqFalse );
}


//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )
