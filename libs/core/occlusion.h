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

#include <aqsis/aqsis.h>

#include <vector>

#include <aqsis/math/vector2d.h>

namespace Aqsis {

class CqBound;
class CqBucketProcessor;

/** \brief A tree for occlusion culling of bounded objects.
 *
 * Given the bound for an object, the task of CqOcclusionTree is to decide
 * whether the object is partially visible or completely hidden by previously
 * rendered objects.  (If the latter, the object can usually be thrown away
 * immediately.)
 *
 * The tree uses a binary space partition in alternate directions (x,y) and is
 * stored in an array for efficiency.  Each tree node stores the maximum depth
 * found in occluding samples connected to its child nodes.  This means that if
 * a surface is further away than the root node (for example) it can be safely
 * culled, and so on down the tree.
 *
 * The tree covers the samples in a bucket such that each leaf node corresponds
 * to exactly one sample position.  In general, the number of samples in a
 * bucket won't be a power of two.  In this case we allow the tree to "hang
 * off the edge" of the bucket to the bottom right.  For example, consider 3x3
 * samples in a bucket (small bucket!).  The next bigger occlusion tree that
 * can cover the samples is 4x4; the covering looks like this:
 * 
 * \verbatim
 * 
 *           bucket extent
 *           <------------>
 *         
 *          +====+====+====+----+
 *     ^   ||    |    |    ||   |
 *     |   ||    |    |    || 0 |
 *     |   |+----+----+----+----+
 *     |   ||    |    |    ||   |
 *     |   ||    |    |    || 0 |
 *     |   |+----+----+----+----+
 *     |   ||    |    |    ||   |
 *     v   ||    |    |    || 0 |
 *          +====+====+====+----+
 *          |    |    |    |    |
 *          | 0  | 0  | 0  |  0 |
 *          +----+----+----+----+
 *
 * \endverbatim
 *
 * Leaf nodes marked with a 0 are unused, with depths set to zero so they don't
 * interfere with the tree updates.
 *
 */
class CqOcclusionTree
{
	public:
		/// Construct an uninitialized tree.
		CqOcclusionTree();

		/** \brief Setup the hierarchy for one bucket and cache samples.
		 *
		 * This should be called before rendering each bucket; it sets up the
		 * tree based on the sample positions, and associates individual
		 * samples from the bucket to leaf nodes of the tree.  The association
		 * is recorded by storing the computed leaf node index into the
		 * occlusionIndex field of the sample point, packed together with the
		 * index into the vector of depths for the leaf node.
		 *
		 * \param bp - the bucket processor for the current bucket.
		 */
		void setupTree(CqBucketProcessor& bp);

		/** \brief Update the occlusion tree depth at the leaf node index.
		 *
		 * If the depth is smaller than the current depth at the given leaf
		 * node index, the depth is stored in the tree.
		 *
		 * \param depth - new depth for the leaf node
		 * \param index - index of the leaf node.
		 */
		void setSampleDepth(TqFloat depth, TqInt index);

		/** \brief Update the occlusion tree if necessary.
		 *
		 * Depths are propagated from the leaf nodes down to the the root if
		 * setSampleDepth() actually changed the tree since the last time
		 * updateTree() was called.
		 */
		void updateTree();

		/** \brief Determine whether a bounded object can be culled.
		 *
		 * \param bound - bound of the object.
		 * \return true if the object is occlueded behind previously rendered
		 *         objects and can be culled, false otherwise.
		 */
		bool canCull(const CqBound& bound) const;

	private:
		void propagateDepths();

		static TqInt treeIndexForPoint(TqInt treeDepth, bool splitXFirst,
				TqInt x, TqInt y);

		/// Number of bits in which to store the sample subindex within a leaf.
		static const TqInt m_subIndexBits = 8;

		/// min (top left) of the area straddled by the tree
		CqVector2D m_treeBoundMin;
		/// max (bottom right) of the area straddled by the tree
		CqVector2D m_treeBoundMax;
		/// Binary tree of depths stored in an array.
		std::vector<TqFloat> m_depthTree;
		/// The index in the depth tree of the first terminal node.
		TqInt m_firstLeafNode;
		/// Number of tree levels (having only the root gives m_numLevels == 1)
		TqInt m_numLevels;
		/// True if the occlusion tree is split in the x direction first.
		bool m_splitXFirst;
		/// True if the tree needs depth propagation since the last time.
		bool m_needsUpdate;
	public:
		/// Class to expose private functions for testing.
		//TODO: refactor so that we don't need this!
		struct Test;
};


} // namespace Aqsis

#endif // OCCLUSION_H_INCLUDED
