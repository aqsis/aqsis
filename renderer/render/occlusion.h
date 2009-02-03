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
 * \brief Hierarchical occlusion culling tree
 * \author Paul Gregory
 * \author Chris Foster
 */

#ifndef OCCLUSION_H_INCLUDED
#define OCCLUSION_H_INCLUDED

#include "aqsis.h"

#include <vector>

#include "vector2d.h"
#include "imagepixel.h"

namespace Aqsis {

class CqBound;
class CqBucketProcessor;

/** \brief An tree for occlusion culling of bounded objects.
 *
 * Given the bound for an object, the task of CqOcclusionTree is to decide
 * whether the object is partially visible or completely hidden by previously
 * rendered objects.  (If the latter, the object can usually be thrown away
 * immediately.)
 *
 * The tree uses a binary space partition in alternate directions (x,y) and is
 * stored in an array for efficiency.  Each tree node stores the maximum depth
 * found in opaque samples connected to its child nodes.  This means that if a
 * surface is further away than the root node (for example) it can be safely
 * culled, and so on down the tree.
 */
class CqOcclusionTree
{
	public:
		/// Construct an uninitialized tree.
		CqOcclusionTree();

		/** \brief Setup the hierarchy for one bucket and cache samples.
		 *
		 * This should be called before rendering each bucket, it sets up the
		 * tree based on the sample positions, and associates individual
		 * samples from the bucket to leaf nodes of the tree.
		 *
		 * \param bp - the bucket processor for the current bucket.
		 * \param xMin - left edge of culling region.
		 * \param yMin - top edge of culling region.
		 * \param xMax - right edge of culling region.
		 * \param yMax - bottom edge of culling region.
		 *
		 * Note that the culling region may be smaller than the bucket in
		 * general (in particular for buckets which fall partially off the edge
		 * of the image).
		 */
		void setupTree(const CqBucketProcessor* bp, TqInt xMin, TqInt yMin,
				TqInt xMax, TqInt yMax);

		/** \brief Rebuild the occlusion tree.
		 *
		 * The tree is rebuilt based on the current opaque sample depths from
		 * the bucket provided to SetupHierarchy()
		 */
		void updateDepths();

		/** \brief Determine whether a bounded object can be culled.
		 *
		 * \param bound - bound of the object.
		 * \return true if the object is occlueded behind previously rendered
		 *         objects and can be culled, false otherwise.
		 */
		bool canCull(const CqBound& bound) const;

	private:
		static TqInt treeIndexForPoint(TqInt treeDepth, const CqVector2D& p);
		void propagateDepths();

		/// min (top left) of the area straddled by the tree
		CqVector2D m_treeBoundMin;
		/// max (bottom right) of the area straddled by the tree
		CqVector2D m_treeBoundMax;
		/// min (top left) of the culling area
		CqVector2D m_cullBoundMin;
		/// max (bottom right) of the culling area
		CqVector2D m_cullBoundMax;
		/// Vector holding samples points associated with the leaf nodes.
		std::vector<std::vector<SqSampleDataPtr> > m_leafSamples;
		/// Binary tree of depths stored in an array.
		std::vector<TqFloat> m_depthTree;
		/// The index in the depth tree of the first terminal node.
		TqInt m_firstLeafNode;
		/// Number of tree levels (having only the root gives m_numLevels == 1)
		TqInt m_numLevels;
};


extern CqOcclusionTree g_occlusionTree;

} // namespace Aqsis

#endif // OCCLUSION_H_INCLUDED
