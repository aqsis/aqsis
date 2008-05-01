// Aqsis
// Copyright (C) 1997 - 2002, Paul C. Gregory
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

namespace Aqsis {

class CqBound;
class CqBucket;

struct SqMpgSampleInfo;
struct SqGridInfo;

class CqOcclusionBox
{
	public:
		static void SetupHierarchy( CqBucket* bucket, TqInt xMin, TqInt yMin, TqInt xMax, TqInt yMax );

		static bool CanCull( CqBound* bound );

		struct SqOcclusionNode
		{
			SqOcclusionNode(TqFloat D) : depth(D)	{}
			TqFloat depth;
			std::vector<SqSampleData*>	samples;
		};

		// New system
		static void RefreshDepthMap();
		static TqFloat propagateDepths(std::vector<CqOcclusionBox::SqOcclusionNode>::size_type index);

	protected:
		CqOcclusionBox();
		~CqOcclusionBox();

		static CqBucket* m_Bucket;
		static std::vector<SqOcclusionNode>	m_depthTree;
		static TqLong m_firstTerminalNode;				///< The index in the depth tree of the first terminal node.
};



} // namespace Aqsis


#endif // OCCLUSION_H_INCLUDED

