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

#include "csgtree.h"

START_NAMESPACE(Aqsis)

CqList<CqCSGTreeNode>	CqCSGNodePrimitive::m_lDefPrimChildren;


CqCSGTreeNode::~CqCSGTreeNode()
{
	CqCSGTreeNode* pChild = m_lChildren.pFirst();
	while(pChild)
	{
		CqCSGTreeNode* pNext = pChild->pNext();
		pChild->Release();
		pChild=pNext;
	}
}


CqCSGTreeNode* CqCSGTreeNode::CreateNode(CqString& type)
{
	if(type == "primitive")
		return(new CqCSGNodePrimitive);
	else if(type == "union")
		return(new CqCSGNodeUnion);
	else if(type == "intersection")
		return(new CqCSGNodeIntersection);
	else if(type == "difference")
		return(new CqCSGNodeDifference);
	else
		return(NULL);
}


//---------------------------------------------------------------------

END_NAMESPACE(Aqsis)
