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
		\brief Implements a generic templatised K-DTree handling class.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/


#ifndef	KDTREE_H_LOADED
#define	KDTREE_H_LOADED

#include	"aqsis.h"
#include	"refcount.h"
#include	<vector>

#include	<boost/shared_ptr.hpp>

START_NAMESPACE( Aqsis )


// Simple empty extra data structure for the default implementation of the extra data template argument
// any decent compiler should optimise this away to nothing.
struct SqEmptyExtraData	{};

//------------------------------------------------------------------------------
/**
 *	Interface to the data handled by the K-DTree, implementation provided.
 */

template<class T, class D>
class CqKDTreeNode;


template<class T, class D>
struct IqKDTreeData
{
    /** Function to sort the elements in the given array, in ascending order based on the specified dimension index.
     */
    virtual void SortElements(std::vector<T>& aLeaves, TqInt dimension) = 0;
    /** Return the number of dimensions in the tree data.
     */
    virtual TqInt Dimensions() const = 0;
	/** Function called during node subdivision
	 */
	virtual void Subdivided(CqKDTreeNode<T, D>& original, 
							CqKDTreeNode<T, D>& leftResult, 
							CqKDTreeNode<T, D>& rightResult, 
							TqInt dimension, TqInt median) = 0;
	/** Function to call to initialise the data on the node.
	 */
	virtual void Initialise(CqKDTreeNode<T,D>& treenode) = 0;

	/** Function to call to propagate changes up the tree.
	 */
	virtual TqBool PropagateChangesFromChild(CqKDTreeNode<T,D>& treenode, CqKDTreeNode<T,D>& child) = 0;

#ifdef _DEBUG
    CqString className() const { return CqString("IqKDTreeData"); }
#endif
};


template<class T, class D>
class CqKDTreeNode
{
public:
    CqKDTreeNode(boost::shared_ptr<IqKDTreeData<T,D> > pDataInterface) : m_pDataInterface( pDataInterface ), m_Dim( 0 )
    {}
    CqKDTreeNode() : m_Dim( 0 )
	{}
    virtual	~CqKDTreeNode()		{}

    TqInt	Subdivide( CqKDTreeNode<T,D>& side1, CqKDTreeNode<T,D>& side2 )
    {
        m_pDataInterface->SortElements( m_aLeaves, m_Dim );

        TqInt median = static_cast<TqInt>( aLeaves().size() / 2.0f );

        side1.aLeaves().assign( aLeaves().begin(), aLeaves().begin() + median );
        side2.aLeaves().assign( aLeaves().begin() + median, aLeaves().end() );

        side1.m_Dim = ( m_Dim + 1 ) % m_pDataInterface->Dimensions();
        side2.m_Dim = ( m_Dim + 1 ) % m_pDataInterface->Dimensions();

		return(median);
    }

	void SetData(boost::shared_ptr<IqKDTreeData<T,D> > pDataInterface)
	{
		m_pDataInterface = pDataInterface;
	}

	void Initialise()
	{
		assert(m_pDataInterface);
		m_pDataInterface->Initialise(*this);
	}


    /// Accessor for leaves array
    std::vector<T>&	aLeaves()	{return(m_aLeaves);}
    const std::vector<T>&	aLeaves() const	{return(m_aLeaves);}

	/// Accessor for the extra data
	const D&	ExtraData() const	{return(m_ExtraData);}
	D&			ExtraData()			{return(m_ExtraData);}

protected:
    std::vector<T>		m_aLeaves;
    boost::shared_ptr<IqKDTreeData<T,D> >	m_pDataInterface;
    TqInt				m_Dim;
    D					m_ExtraData;
};

template<class T, class D>
class CqKDTree : public CqKDTreeNode<T,D>
{
public:
    CqKDTree( boost::shared_ptr<IqKDTreeData<T,D> > pDataInterface) : CqKDTreeNode<T,D>( pDataInterface ), m_Left(0), m_Right(0), m_Parent(0)
    {}
    CqKDTree() : CqKDTreeNode<T,D>()
	{}
    virtual	~CqKDTree()	
	{
		delete(m_Left);
		delete(m_Right);
	}

    TqInt	Subdivide( )
    {
		if(m_Left == 0)
		{
			m_Left = new CqKDTree<T,D>(this->m_pDataInterface);
			m_Left->m_Parent = this;
		}
		if(m_Right == 0)
		{
			m_Right = new CqKDTree<T,D>(this->m_pDataInterface);
			m_Right->m_Parent = this;
		}
	
		TqInt median = CqKDTreeNode<T,D>::Subdivide(*m_Left, *m_Right);

		return(median);
	}

    void	Subdivide( TqInt maxLeaves )
	{
		TqInt median = Subdivide();
		// Call the subdivided callback on the data interface.
		this->m_pDataInterface->Subdivided(*this, *m_Left, *m_Right, this->m_Dim, median); 
		if( m_Left->aLeaves().size() > maxLeaves)
			m_Left->Subdivide(maxLeaves);
		if( m_Right->aLeaves().size() > maxLeaves)
			m_Right->Subdivide(maxLeaves);
	}

	void Initialise(TqBool recursive)
	{
		assert(m_pDataInterface);
		// Initialise the children first, in case we need access to the processed child data.
		if( recursive )
		{
			if( m_Left )
				m_Left->Initialise(recursive);
			if( m_Right )
				m_Right->Initialise(recursive);
		}
		// Now do this one.
		CqKDTreeNode<T,D>::Initialise();
	}

	/// Propagate any changes from the child nodes up the tree.
	void PropagateChanges()
	{
		// Call the data handler for each child.
		TqBool prop = this->m_pDataInterface->PropagateChangesFromChild(*this, *m_Left);
		prop = prop | this->m_pDataInterface->PropagateChangesFromChild(*this, *m_Right);

		// If the child changes caused a change in this node, propagate it up, otherwise, no need.
		if(prop && m_Parent)
			m_Parent->PropagateChanges();
	}

	CqKDTree<T,D>* Left()
	{
		return(m_Left);
	}

	CqKDTree<T,D>* Right()
	{
		return(m_Right);
	}

	
private:
	CqKDTree<T,D>* m_Left;	
	CqKDTree<T,D>* m_Right;	
	CqKDTree<T,D>* m_Parent;
};


END_NAMESPACE( Aqsis )

#endif	//	KDTREE_H_LOADED
