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

START_NAMESPACE( Aqsis )

class CqBound;
class CqBucket;


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

		static bool CanCull( CqBound* bound )
		{
			return m_Hierarchy[ 0 ].IsCullable( bound );
		}
		static void MarkForUpdate( int id )
		{
			assert( id >= 0 && id < m_TotalBoxes );
			m_Hierarchy[ id ].MarkForUpdate();
		}

	protected:
		CqOcclusionBox();
		~CqOcclusionBox();

		void SetupChildren();
		static void UpdateLevel( int level );
		bool UpdateZValues(); // returns true if we changed anything
		void Clear();

		void SetBounds( TqInt x0, TqInt y0, TqInt x1, TqInt y1 );
		bool Overlaps( CqBound* bound );

		bool IsCullable( CqBound* bound );
		bool NeedsUpdating()
		{
			return m_NeedsUpdating;
		}
		void MarkForUpdate()
		{
			m_NeedsUpdating = true;
		}

		TqInt m_MinX; // pixel positions of box boundary
		TqInt m_MinY;
		TqInt m_MaxX;
		TqInt m_MaxY;

		float m_MinZ;
		float m_MaxZ;

		int m_Id;

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
		static int m_HierarchyLevels; // the depth of the tree
		static int m_TotalBoxes;
		static int* m_LevelStartId; // the id for the start of each level, ie 0,1,5,21... etc

		bool m_NeedsUpdating;
};

END_NAMESPACE( Aqsis )


#endif // OCCLUSION_H_INCLUDED

