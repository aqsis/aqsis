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
		\brief Declares the classes for storing information about a CSG tree structure.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef CSGTREE_H_INCLUDED
#define CSGTREE_H_INCLUDED 1

#include	<ostream>

#include	"aqsis.h"

#include	"refcount.h"
#include	"list.h"
#include	"sstring.h"


START_NAMESPACE(Aqsis)


class CqCSGTreeNode : public CqRefCount, public CqListEntry<CqCSGTreeNode>
{
	public:
			CqCSGTreeNode()	{}
	virtual	~CqCSGTreeNode();

			enum EqCSGNodeType
			{
				CSGNodeType_Primitive,
				CSGNodeType_Union,
				CSGNodeType_Intersection,
				CSGNodeType_Difference,
			};

	virtual CqList<CqCSGTreeNode>& lChildren()	{return(m_lChildren);}
	
	virtual	EqCSGNodeType	NodeType() const=0;
	virtual	CqString		StrNodeType() const=0;

			void			Print(TqInt tab, std::ostream& out)
							{
								for(TqInt i=0; i<tab; i++)
									out << "\t";
								out << StrNodeType().c_str() << std::endl;

								CqCSGTreeNode* pChild = m_lChildren.pFirst();
								while(pChild)
								{
									pChild->Print(tab+1, out);
									pChild=pChild->pNext();
								}
							}

	static CqCSGTreeNode* CreateNode(CqString& type);

	private:
			CqList<CqCSGTreeNode>	m_lChildren;
};



class CqCSGNodePrimitive : public CqCSGTreeNode
{
	public:
			CqCSGNodePrimitive() : CqCSGTreeNode()	{}
	virtual ~CqCSGNodePrimitive()	{}

	virtual CqList<CqCSGTreeNode>& lChildren()	{assert(TqFalse); return(m_lDefPrimChildren);}

	virtual	EqCSGNodeType	NodeType() const	{return(CSGNodeType_Primitive);}
	virtual	CqString		StrNodeType() const	{return(CqString("primitive"));}

	private:
	static	CqList<CqCSGTreeNode>	m_lDefPrimChildren;
};


class CqCSGNodeUnion : public CqCSGTreeNode
{
	public:
			CqCSGNodeUnion() : CqCSGTreeNode()	{}
	virtual ~CqCSGNodeUnion()	{}

	virtual	EqCSGNodeType	NodeType() const	{return(CSGNodeType_Union);}
	virtual	CqString		StrNodeType() const	{return(CqString("union"));}
};

class CqCSGNodeIntersection : public CqCSGTreeNode
{
	public:
			CqCSGNodeIntersection() : CqCSGTreeNode()	{}
	virtual ~CqCSGNodeIntersection()	{}

	virtual	EqCSGNodeType	NodeType() const	{return(CSGNodeType_Intersection);}
	virtual	CqString		StrNodeType() const	{return(CqString("intersection"));}
};


class CqCSGNodeDifference : public CqCSGTreeNode
{
	public:
			CqCSGNodeDifference() : CqCSGTreeNode()	{}
	virtual ~CqCSGNodeDifference()	{}

	virtual	EqCSGNodeType	NodeType() const	{return(CSGNodeType_Difference);}
	virtual	CqString		StrNodeType() const	{return(CqString("difference"));}
};


//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif	// !CSGTREE_H_INCLUDED
