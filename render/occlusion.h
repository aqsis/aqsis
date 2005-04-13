// Aqsis
// Copyright © 1997 - 2002, Paul C. Gregory
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
		\brief Declares the hierarchical occlusion culling class.
		\author Andy Gill (billybobjimboy@users.sf.net)
*/

//? Is .h included already?
#ifndef OCCLUSION_H_INCLUDED
#define OCCLUSION_H_INCLUDED 1

#include "aqsis.h"
#include "kdtree.h"
#include "imagepixel.h"

START_NAMESPACE( Aqsis )

class CqBound;
class CqBucket;


struct	SqOcclusionKDTreeExtraData
{
	CqVector2D	m_MinSamplePoint;
	CqVector2D	m_MaxSamplePoint;
	TqFloat		m_MinTime;
	TqFloat		m_MaxTime;
	TqFloat		m_MinOpaqueZ;
	TqFloat		m_MaxOpaqueZ;
};

typedef	SqSampleData*	TqOcclusionKDTreeData;
typedef CqKDTree<TqOcclusionKDTreeData, SqOcclusionKDTreeExtraData> TqOcclusionKDTree;

/**	\brief	The CqOcclusionKDTreeData class
	Specialisation of the KDTree data class to support generation of a KDTree
	representing the sample data of a bucket.
*/
class CqOcclusionKDTreeData : public IqKDTreeData<TqOcclusionKDTreeData, SqOcclusionKDTreeExtraData>
{
    class CqOcclusionKDTreeDataComparator
    {
    public:
        CqOcclusionKDTreeDataComparator(TqInt dimension) : m_Dim( dimension )
        {}

        bool operator()(TqOcclusionKDTreeData a, TqOcclusionKDTreeData b);

    private:
        TqInt		m_Dim;
    };

public:
    CqOcclusionKDTreeData()
    {}

    virtual void SortElements(std::vector<TqOcclusionKDTreeData>& aLeaves, TqInt dimension)
    {
        std::sort(aLeaves.begin(), aLeaves.end(), CqOcclusionKDTreeDataComparator(dimension) );
    }
    virtual TqInt Dimensions() const	{return(2);}
	virtual void Subdivided(CqKDTreeNode<TqOcclusionKDTreeData, SqOcclusionKDTreeExtraData>& original, 
							CqKDTreeNode<TqOcclusionKDTreeData, SqOcclusionKDTreeExtraData>& leftResult, 
							CqKDTreeNode<TqOcclusionKDTreeData, SqOcclusionKDTreeExtraData>& rightResult, 
							TqInt dimension, TqInt median)
	{
		Initialise(leftResult);
		Initialise(rightResult);
	}

	virtual TqBool PropagateChangesFromChild(CqKDTreeNode<TqOcclusionKDTreeData, SqOcclusionKDTreeExtraData>& treenode, CqKDTreeNode<TqOcclusionKDTreeData, SqOcclusionKDTreeExtraData>& child)
	{
		TqBool madeChange = TqFalse;
		// Propagate any changes to the opaque Z data in the samples up to the parent.
		if(child.ExtraData().m_MaxOpaqueZ > treenode.ExtraData().m_MaxOpaqueZ )
		{
			treenode.ExtraData().m_MaxOpaqueZ = child.ExtraData().m_MaxOpaqueZ;
			madeChange = TqTrue;
		}

		if(child.ExtraData().m_MinOpaqueZ < treenode.ExtraData().m_MinOpaqueZ )
		{
			treenode.ExtraData().m_MinOpaqueZ = child.ExtraData().m_MinOpaqueZ;
			madeChange = TqTrue;
		}
		return( madeChange );
	}

	void Initialise(CqKDTreeNode<TqOcclusionKDTreeData, SqOcclusionKDTreeExtraData>& treenode)
	{
		TqFloat minXVal = treenode.aLeaves()[0]->m_Position.x();
		TqFloat maxXVal = minXVal;
		TqFloat minYVal = treenode.aLeaves()[0]->m_Position.y();
		TqFloat maxYVal = minYVal;
		TqFloat minTime = treenode.aLeaves()[0]->m_Time;
		TqFloat maxTime = minTime;
		std::vector<TqOcclusionKDTreeData>::iterator i;
		for(i = treenode.aLeaves().begin(); i!=treenode.aLeaves().end(); ++i)
		{
			minXVal = MIN(minXVal, (*i)->m_Position.x());
			maxXVal = MAX(maxXVal, (*i)->m_Position.x());
			minYVal = MIN(minYVal, (*i)->m_Position.y());
			maxYVal = MAX(maxYVal, (*i)->m_Position.y());
			minTime = MIN(minTime, (*i)->m_Time);
			maxTime = MAX(maxTime, (*i)->m_Time);
		}
		treenode.ExtraData().m_MinSamplePoint[0] = minXVal;
		treenode.ExtraData().m_MaxSamplePoint[0] = maxXVal;
		treenode.ExtraData().m_MinSamplePoint[1] = minYVal;
		treenode.ExtraData().m_MaxSamplePoint[1] = maxYVal;
		treenode.ExtraData().m_MinTime = minTime;
		treenode.ExtraData().m_MaxTime = maxTime;

		// Set the opaque depths to the limits to begin with.
		treenode.ExtraData().m_MinOpaqueZ = -FLT_MAX;
		treenode.ExtraData().m_MaxOpaqueZ = FLT_MAX;
	}

private:
};


class CqOcclusionBox
{
public:
    static void CreateHierarchy( TqInt bucketXSize, TqInt bucketYSize, TqInt XFWidth, TqInt YFWidth );
    static void DeleteHierarchy();
    static void SetupHierarchy( CqBucket* bucket, TqInt xMin, TqInt yMin, TqInt xMax, TqInt yMax );
    static void Update()
    {
        UpdateLevel( m_HierarchyLevels - 1 );
    }

    static TqBool CanCull( CqBound* bound )
    {
        return m_Hierarchy[ 0 ].IsCullable( bound );
    }
    static void MarkForUpdate( TqInt id )
    {
        assert( id >= 0 && id < m_TotalBoxes );
        m_Hierarchy[ id ].MarkForUpdate();
    }
	static TqOcclusionKDTree& KDTree()
	{
		return(m_KDTree);
	}

protected:
    CqOcclusionBox();
    ~CqOcclusionBox();

    void SetupChildren();
    static void UpdateLevel( TqInt level );
    TqBool UpdateZValues(); // returns true if we changed anything
    void Clear();

    void SetBounds( TqInt x0, TqInt y0, TqInt x1, TqInt y1 );
    bool Overlaps( CqBound* bound );

    TqBool IsCullable( CqBound* bound );
    TqBool NeedsUpdating()
    {
        return m_NeedsUpdating;
    }
    void MarkForUpdate()
    {
        m_NeedsUpdating = TqTrue;
    }

    TqInt m_MinX; // pixel positions of box boundary
    TqInt m_MinY;
    TqInt m_MaxX;
    TqInt m_MaxY;

    TqFloat m_MinZ;
    TqFloat m_MaxZ;

    TqInt m_Id;

    /*
    	m_Hierarchy is a tree but implemented as an array for speed.
    	Each box has exactly 4 children apart from the leaves (obviously).
    	For reference:
    	this = m_Hierarchy[m_Id];
    	parent = m_Hierarchy[m_Id/4]; (integer divide, rounds down)
    	first child = m_Hierarchy[m_Id*4 + 1]; (if we are a leaf this will be >= m_TotalBoxes)
    	next sibling = m_Hierarchy[m_Id + 1];
    */

    static CqBucket* m_Bucket;
    static CqOcclusionBox* m_Hierarchy; // tree of OcclusionBoxes
    static TqInt m_HierarchyLevels; // the depth of the tree
    static TqInt m_TotalBoxes;
    static TqInt* m_LevelStartId; // the id for the start of each level, ie 0,1,5,21... etc

    TqBool m_NeedsUpdating;

    static TqOcclusionKDTree	m_KDTree;			///< KD Tree representing the samples in the bucket.
};



END_NAMESPACE( Aqsis )


#endif // OCCLUSION_H_INCLUDED

