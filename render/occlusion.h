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
	TqInt		m_MinDofBoundIndex;
	TqInt		m_MaxDofBoundIndex;
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
      if (treenode.aLeaves().size() < 1) return;

      TqFloat minXVal = treenode.aLeaves()[0]->m_Position.x();
		TqFloat maxXVal = minXVal;
		TqFloat minYVal = treenode.aLeaves()[0]->m_Position.y();
		TqFloat maxYVal = minYVal;
		TqFloat minTime = treenode.aLeaves()[0]->m_Time;
		TqFloat maxTime = minTime;
		TqInt	minDofIndex = treenode.aLeaves()[0]->m_DofOffsetIndex;
		TqInt	maxDofIndex = minDofIndex;
		std::vector<TqOcclusionKDTreeData>::iterator i;
		for(i = treenode.aLeaves().begin(); i!=treenode.aLeaves().end(); ++i)
		{
			minXVal = MIN(minXVal, (*i)->m_Position.x());
			maxXVal = MAX(maxXVal, (*i)->m_Position.x());
			minYVal = MIN(minYVal, (*i)->m_Position.y());
			maxYVal = MAX(maxYVal, (*i)->m_Position.y());
			minTime = MIN(minTime, (*i)->m_Time);
			maxTime = MAX(maxTime, (*i)->m_Time);
			minDofIndex = MIN(minDofIndex, (*i)->m_DofOffsetIndex);
			maxDofIndex = MAX(maxDofIndex, (*i)->m_DofOffsetIndex);
		}
		treenode.ExtraData().m_MinSamplePoint[0] = minXVal;
		treenode.ExtraData().m_MaxSamplePoint[0] = maxXVal;
		treenode.ExtraData().m_MinSamplePoint[1] = minYVal;
		treenode.ExtraData().m_MaxSamplePoint[1] = maxYVal;
		treenode.ExtraData().m_MinTime = minTime;
		treenode.ExtraData().m_MaxTime = maxTime;
		treenode.ExtraData().m_MinDofBoundIndex = minDofIndex;
		treenode.ExtraData().m_MaxDofBoundIndex = maxDofIndex;

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

    static TqBool CanCull( CqBound* bound )
    {
        //return m_Hierarchy[ 0 ].IsCullable( bound );
		return(TqFalse);
    }
	static TqOcclusionKDTree& KDTree()
	{
		return(m_KDTree);
	}

protected:
    CqOcclusionBox();
    ~CqOcclusionBox();

    static CqBucket* m_Bucket;
    static TqOcclusionKDTree	m_KDTree;			///< KD Tree representing the samples in the bucket.
};



END_NAMESPACE( Aqsis )


#endif // OCCLUSION_H_INCLUDED

