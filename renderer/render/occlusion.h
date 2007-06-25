// Aqsis
// Copyright © 1997 - 2002, Paul C. Gregory
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
		\brief Declares the hierarchical occlusion culling class.
		\author Andy Gill (billybobjimboy@users.sf.net)
*/

//? Is .h included already?
#ifndef OCCLUSION_H_INCLUDED
#define OCCLUSION_H_INCLUDED 1

#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "aqsis.h"
#include "kdtree.h"
#include "imagepixel.h"
#include "bucket.h"

START_NAMESPACE( Aqsis )

class CqBound;
class CqBucket;

struct SqMpgSampleInfo;
struct SqGridInfo;

class CqOcclusionTree;
//typedef boost::shared_ptr<CqOcclusionTree> CqOcclusionTreePtr;
//typedef boost::weak_ptr<CqOcclusionTree> CqOcclusionTreeWeakPtr;
typedef CqOcclusionTree* CqOcclusionTreePtr;
typedef CqOcclusionTree* CqOcclusionTreeWeakPtr;


/**	\brief	The CqOcclusionKDTreeData class
	Specialisation of the KDTree data class to support generation of a KDTree
	representing the sample data of a bucket.
*/
class CqOcclusionTree// : public boost::enable_shared_from_this<CqOcclusionTree>
{
		class CqOcclusionTreeComparator
		{
			public:
				CqOcclusionTreeComparator(TqInt dimension) : m_Dim( dimension )
				{}

				bool operator()(const std::pair<TqInt, TqInt>& a, const std::pair<TqInt, TqInt>& b);

			private:
				TqInt		m_Dim;
		};

	public:
		CqOcclusionTree(TqInt dimension = 0);
		~CqOcclusionTree();

		void SortElements(TqInt dimension)
		{
			std::sort(m_SampleIndices.begin(), m_SampleIndices.end(), CqOcclusionTreeComparator(dimension) );
		}
		TqInt Dimensions() const
		{
			return(2);
		}

		SqSampleData& Sample() const;

		void AddSample(const std::pair<TqInt, TqInt>& sample)
		{
			m_SampleIndices.push_back(sample);
		}
		void ConstructTree();
		void OutputTree(const char* name);

		void PropagateChanges();

		void InitialiseBounds();
		void UpdateBounds();

		bool CanCull( CqBound* bound );
		void SampleMPG( CqMicroPolygon* pMPG, const CqBound& bound, bool usingMB, TqFloat time0, TqFloat time1, bool usingDof, TqInt dofboundindex, const SqMpgSampleInfo& MpgSampleInfo, bool usingLOD, const SqGridInfo& gridInfo);

		TqInt NumSamples() const
		{
			return(m_SampleIndices.size());
		}

		TqFloat MaxOpaqueZ() const
		{
			return(m_MaxOpaqueZ);
		}

		void SetMaxOpaqueZ(TqFloat z)
		{
			m_MaxOpaqueZ = z;
		}

		const CqVector2D& MinSamplePoint() const
		{
			return(m_MinSamplePoint);
		}

		const CqVector2D& MaxSamplePoint() const
		{
			return(m_MaxSamplePoint);
		}

	private:
		enum { s_ChildrenPerNode = 4 };
		typedef boost::array<CqOcclusionTreePtr,s_ChildrenPerNode> TqChildArray;

		typedef std::vector<std::pair<TqInt, TqInt> > TqSampleIndices;

		void SplitNode(CqOcclusionTreePtr& a, CqOcclusionTreePtr& b);

		CqOcclusionTreeWeakPtr	m_Parent;
		TqInt		m_Dimension;
		CqVector2D	m_MinSamplePoint;
		CqVector2D	m_MaxSamplePoint;
		TqFloat		m_MinTime;
		TqFloat		m_MaxTime;
		TqFloat		m_MaxOpaqueZ;
		TqInt		m_MinDofBoundIndex;
		TqInt		m_MaxDofBoundIndex;
		TqFloat		m_MinDetailLevel;
		TqFloat		m_MaxDetailLevel;
		TqChildArray	m_Children;
		TqSampleIndices	m_SampleIndices;

	public:
		static TqInt		m_Tab;
};


class CqOcclusionBox
{
	public:
		static void DeleteHierarchy();
		static void SetupHierarchy( const CqBucket* bucket );

		static bool CanCull( CqBound* bound );

		static CqOcclusionTreePtr& KDTree()
		{
			return(m_KDTree);
		}

	protected:
		CqOcclusionBox();
		~CqOcclusionBox();

		static CqOcclusionTreePtr	m_KDTree;			///< Tree representing the samples in the bucket.
};



END_NAMESPACE( Aqsis )


#endif // OCCLUSION_H_INCLUDED

